/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lfabel <lfabel@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/23 10:50:21 by lfabel            #+#    #+#             */
/*   Updated: 2025/09/25 10:07:15 by lfabel           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include "Server.hpp"

/*socket oeffnen und lauschen
verbindung annehmen und lesen
http request parsen
Routing statisch oder mit cgi*/

Server *Server::instance = NULL;

void Server::sigint(int signo)
{
	(void)signo;

	for (size_t i = 0; i < instance->_fds.size(); i++)
	{
		close(instance->_fds[i].fd);
	}
	std::cout << "server closed" << std::endl;
	exit(0);
}

Server::Server()
{
	instance = this;
	_handlerCGI = CGIHandler();
}

void Server::Initialize()
{
	for (size_t i = 0; i < _configs.size(); i++)
	{
		std::cout << "Initializing server on port: " << _configs[i].GetPort() << " and IP: " << _configs[i].GetIP() << std::endl;
		setSockAddrStruct(_configs[i].GetPort(), _configs[i].GetIP());
	}
	_listenNum = _configs.size();
}

void Server::setConfigs(std::vector<ServerBlock> &configs)
{
	_configs = configs;
}

void Server::setSockAddrStruct(in_port_t port, std::string ip)
{
	struct pollfd serverPollFd;
	struct sockaddr_in serverAddr;

	serverPollFd.fd = socket(AF_INET, SOCK_STREAM, 0); // AF_INET for IPv4, SOCK_STREAM for TCP,
	if (serverPollFd.fd < 0)
	{
		perror("socket");
		exit(EXIT_FAILURE);
	}
	serverPollFd.events = POLLIN; // Listen for incoming connections
	serverPollFd.revents = 0;

	// explanation described below
	if (fcntl(serverPollFd.fd, F_SETFL, O_NONBLOCK) < 0)
	{
		perror("fcntl");
		close(serverPollFd.fd);
		exit(EXIT_FAILURE);
	}
	// int setsockopt(int socket, int level, int option_name, const void *option_value, socklen_t option_len);
	// options described below
	int optval = 1; // Enable the option to reuse the address
	if (setsockopt(serverPollFd.fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0)
	{
		perror("setsockopt");
		close(serverPollFd.fd);
		exit(EXIT_FAILURE);
	}
	// if (inet_pton(AF_INET, ip.c_str(), &_serverAddr.sin_addr) <= 0)
	// {
	// 	perror("inet_pton");
	// 	close(serverPollFd.fd);
	// 	exit(EXIT_FAILURE);
	// }
	if (inet_pton(AF_INET, ip.c_str(), &serverAddr.sin_addr) <= 0)
	{
		perror("inet_pton");
		close(serverPollFd.fd);
		exit(EXIT_FAILURE);
	}
	serverAddr.sin_family = AF_INET;
	//_serverAddr.sin_family = AF_INET;
	//_serverAddr.sin_addr.s_addr = INADDR_ANY; // This means the server will accept connections from any IP address
	//_serverAddr.sin_port = htons(port);
	serverAddr.sin_port = htons(port);
	if (bind(serverPollFd.fd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
	{
		perror("bind");
		close(serverPollFd.fd);
		exit(EXIT_FAILURE);
	}
	if (listen(serverPollFd.fd, SOMAXCONN) < 0)
	{ // SOMAXCONN is a constant that defines the maximum number of pending connections
		perror("listen");
		close(serverPollFd.fd);
		exit(EXIT_FAILURE);
	}
	_fds.push_back(serverPollFd); // Add the server socket to the poll file descriptor vector
}

void Server::checkAndConnectNewClient()
{
	struct sockaddr_in clientAddr;
	socklen_t clientLen = sizeof(clientAddr);
	struct pollfd newClientPollFd;

	if (_fds.size() >= MAX_CLIENTS)
	{
		// Reject pending connections from ALL servers
		for (int i = 0; i < _listenNum; i++)
		{
			if (_fds[i].revents & POLLIN)
			{
				int temp_fd = accept(_fds[i].fd, (struct sockaddr *)&clientAddr, &clientLen);
				if (temp_fd != -1)
				{
					close(temp_fd);
				}
			}
		}
		std::cout << "Maximum client limit reached, connections rejected" << std::endl;
		return;
	}

	for (int i = 0; i < _listenNum; i++)
	{
		if (_fds[i].revents & POLLIN)
		{
			newClientPollFd.fd = accept(_fds[i].fd, (struct sockaddr *)&clientAddr, &clientLen);
			if (fcntl(newClientPollFd.fd, F_SETFL, O_NONBLOCK) < 0)
			{
				perror("fcntl failed on new client socket");
				close(newClientPollFd.fd);
				return;
			}
			if (newClientPollFd.fd == -1)
			{
				perror("accept failed");
				return; // to continue the previous
			}
			newClientPollFd.events = POLLIN;
			newClientPollFd.revents = 0;
			_fds.push_back(newClientPollFd);
			_requestsMap[newClientPollFd.fd] = "";
			_httpRequestsMap[newClientPollFd.fd] = HttpRequest();
			_lastActivityMap[newClientPollFd.fd] = time(NULL);
			_clientToServerMap[newClientPollFd.fd] = i;

			_connections[_fds.size() - 1] = Connection(newClientPollFd.fd, _fds.size() - 1);
			std::cout << "Connection was created: " << _fds.size() - 1 << " PollFd index: " << _fds.size() - 1 << std::endl;
			std::cout << "\nNew client connected: FD " << newClientPollFd.fd << " to " << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << std::endl;
		}
	}
}

void Server::checkClientsForData(char **envp)
{
	char buffer[4096];

	for (size_t i = _listenNum; i < _fds.size() && _pollinNum > 0; i++)
	{
		if (_fds[i].revents & POLLIN)
		{
			ssize_t bytes = recv(_fds[i].fd, buffer, sizeof(buffer) - 1, 0);
			// std::cerr << "bytes read" << bytes << std::endl;
			if (bytes <= 0)
			{
				if (bytes == 0)
				{
					continue;
				}
				else
				{

					std::cerr << "Client disconnected: FD " << _fds[i].fd << std::endl;
					close(_fds[i].fd);

					_requestsMap.erase(_fds[i].fd);
					_httpRequestsMap.erase(_fds[i].fd);
					_lastActivityMap.erase(_fds[i].fd);
					_clientToServerMap.erase(_fds[i].fd);
					_fds[i] = _fds.back();
					_fds.pop_back();
					i--;
				}
			}
			else
			{
				buffer[bytes] = '\0';
				_requestsMap[_fds[i].fd] += buffer;

				int initStatusCode = 200;
				_httpRequestsMap[_fds[i].fd].ProcessRawData(buffer, bytes, &initStatusCode);
				_lastActivityMap[_fds[i].fd] = time(NULL);
				if (_httpRequestsMap[_fds[i].fd].isComplete())
				{
					std::cout << "keepAlive: " << std::endl;

					bool keepAlive = startProcess(envp,
												  _httpRequestsMap[_fds[i].fd].getDecodedData().c_str(),
												  _fds[i].fd, initStatusCode, i);

					if (_fds[i].revents & POLLOUT)
					{
						std::cout << "Preparing to send response to FD " << _fds[i].fd << std::endl;
						_connections[i].keepAlive = keepAlive;
						//_connections[i].isSending = _isSending[i];
					}
				}
			}
		}
		if (_fds[i].revents & POLLOUT)
		{
			std::cout << "Sending response to FD " << _fds[i].fd << std::endl;

			Connection &conn = _connections[i];
			size_t to_send = std::min((size_t)60000, conn.responseBuffer.size() - conn.bytesSent);

			if (to_send > 0)
			{
				ssize_t sent = send(_fds[i].fd,
									conn.responseBuffer.c_str() + conn.bytesSent, // ← FROM bytesSent OFFSET
									to_send, 0);								  // ← ONLY REMAINING BYTES

				if (sent > 0)
				{
					_connections[i].bytesSent += sent;
					std::cout << "Sent " << sent << " bytes, total sent: " << _connections[i].bytesSent
							  << "/" << _connections[i].responseBuffer.size() << std::endl;
				}
				else if (sent < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
				{
					// Buffer still full, wait for next POLLOUT
					std::cout << "Send blocked, waiting for next POLLOUT" << std::endl;
				}
				else
				{
					// Error
					perror("send failed");
					// Handle error - close connection
				}
			}

			// Check if all data sent
			if (_connections[i].bytesSent == _connections[i].responseBuffer.size())
			{
				std::cout << "Response fully sent to FD " << _fds[i].fd << std::endl;
				_connections[i].bytesSent = 0;
				_connections[i].responseBuffer.clear();
				_connections[i].isSending = false;
				_fds[i].events &= ~POLLOUT; // Stop monitoring POLLOUT
				_fds[i].events |= POLLIN;	// Back to reading
			}
		}

		if (!_httpRequestsMap[_fds[i].fd].isComplete() || _connections[i].isSending)
			continue;

		if (_connections[i].keepAlive)
		{
			std::cerr << "Keeping connection alive for FD " << _fds[i].fd << std::endl;
			_requestsMap[_fds[i].fd] = "";
			_httpRequestsMap[_fds[i].fd] = HttpRequest();
			_lastActivityMap[_fds[i].fd] = time(NULL);
			_connections[i].isSending = false;
			_fds[i].events = POLLIN;
		}
		else
		{
			std::cerr << "Closing connection for FD " << _fds[i].fd << std::endl;
			close(_fds[i].fd);

			_requestsMap.erase(_fds[i].fd);
			_httpRequestsMap.erase(_fds[i].fd);
			_lastActivityMap.erase(_fds[i].fd);
			_clientToServerMap.erase(_fds[i].fd);
			_fds[i].events = POLLIN;
			_connections.erase(i);
			_fds[i] = _fds.back();
			_fds.pop_back();
			i--;
			_pollinNum--;
		}
	}
}

void Server::checkForTimeouts()
{
	time_t currentTime = time(NULL);

	for (size_t i = _listenNum; i < _fds.size(); i++)
	{
		int fd = _fds[i].fd;

		if (_lastActivityMap.find(fd) != _lastActivityMap.end())
		{
			time_t lastActivity = _lastActivityMap[fd];
			if (currentTime - lastActivity >= TIMEOUT_SECONDS)
			{
				std::cout << "Client FD " << fd << " timed out after " << TIMEOUT_SECONDS << " seconds" << std::endl;
				close(fd);

				_requestsMap.erase(fd);
				_httpRequestsMap.erase(fd);
				_lastActivityMap.erase(fd);
				_clientToServerMap.erase(fd);
				if (i < (_fds.size() - 1))
					_fds[i] = _fds.back();

				_fds.pop_back();
				i--;
			}
		}
	}
}

void Server::serverListen(char **envp)
{
	while (true)
	{
		_pollinNum = poll(_fds.data(), _fds.size(), 5000);

		if (_pollinNum == -1)
		{
			perror("poll failed");
			exit(1);
		}
		else if (_pollinNum == 0)
		{
			checkForTimeouts();
		}
		else
		{

			checkAndConnectNewClient();
			checkClientsForData(envp);
		}

		checkForTimeouts();
	}
}

void Server::setConfig(ServerBlock &config)
{
	this->_config = config;
}

bool Server::startProcess(char **envp, const char *buffer, int clientFd, int initStatusCode, int i)
{
	std::string check = buffer;
	Routing::RequestInfo reqInfo;
	reqInfo.Initialize();
	int id = _clientToServerMap[clientFd];
	std::string response;
	reqInfo._customHeaders.clear();
	reqInfo._errorPages.clear();
	reqInfo._fields.clear();
	reqInfo._statusCode = initStatusCode;
	if (check.find("favicon.ico") != std::string::npos)
	{
		reqInfo._statusCode = 204;
		reqInfo._connection = "close";
	}
	if (reqInfo._statusCode == 200)
		processData(check, envp, reqInfo);
	if (reqInfo._statusCode == 200)
		findConfigs(this->_configs[id], reqInfo);
	if (reqInfo._statusCode == 200)
	{
		checkRequestValid(reqInfo);
	}
	if (reqInfo._statusCode >= 400 || reqInfo._statusCode == 204)
	{
		response = sendErrorResponse(reqInfo._statusCode, reqInfo);
	}
	else if (reqInfo._useCGI)
	{
		startCGI(reqInfo, clientFd, buffer);
	}
	else
	{
		if (reqInfo._upload && reqInfo._method == "POST")
			handleUpload(reqInfo, buffer);
		else if (reqInfo._method == "DELETE")
		{
			deletefile(reqInfo);
		}
		else if (reqInfo._useAutoIndex && reqInfo._index.empty())
		{
			std::vector<std::string> files = getData(reqInfo._path);
			std::string full_response = vectoStr(files, reqInfo._path);
			response = makeResponse(full_response, reqInfo);
		}
		else
		{
			std::cout << "Executing static for path: " << reqInfo._path << std::endl;
			std::string response_static = executeStatic(reqInfo._path, reqInfo);
			// std::cout << "Static resp: " << response_static << std::endl;
			response = makeResponse(response_static, reqInfo);
		}
		if (reqInfo._statusCode >= 400)
		{
			response = sendErrorResponse(reqInfo._statusCode, reqInfo);
		}
	}
	if (reqInfo._statusCode >= 400)
	{
		response = sendErrorResponse(reqInfo._statusCode, reqInfo);
	}
	if (!reqInfo._useCGI || reqInfo._statusCode >= 400)
	{
		if (response.empty())
		{
			response = makeResponse(response, reqInfo);
		}
		std::cout << "Response prepared for FD " << clientFd << " with status code " << reqInfo._statusCode << std::endl;
		// std::cerr << response << std::endl;
		//  Try to send immediately
		size_t sent = send(clientFd, response.c_str(), response.size(), 0);

		if (sent == response.size())
		{
			// All sent immediately - no need for POLLOUT
			std::cout << "Response sent immediately to FD " << clientFd << std::endl;
			_connections[i].isSending = false;
			_fds[i].events = POLLIN; // Back to reading
		}
		else if (sent > 0)
		{
			// Partial send - store remaining data for POLLOUT
			std::cout << "Partial send (" << sent << "/" << response.size() << "), setting POLLOUT" << std::endl;
			_connections[i].responseBuffer.swap(response);
			_connections[i].bytesSent = sent;
			_connections[i].isSending = true;
			_fds[i].events |= POLLOUT;
		}
		else if (sent == 0)
		{
			// Send would block - store full response for POLLOUT
			std::cout << "Send blocked, setting POLLOUT" << std::endl;
			_connections[i].responseBuffer.swap(response);
			_connections[i].bytesSent = 0;
			_connections[i].isSending = true;
			_fds[i].events |= POLLOUT;
		}
		else
		{
			// Error
			perror("Send failed");
			_connections[i].isSending = false;
			return (false);
			// Handle error
		}

		_connections[i].keepAlive = (reqInfo._connection == "keep-alive");
	}
	if (reqInfo._connection != "keep-alive" && !_connections[i].isSending)
	{
		std::cout << "Closing connection for FD " << clientFd << std::endl;
		close(clientFd);
		return (false);
	}
	return (true);
}

void Server::startCGI(Routing::RequestInfo &reqInfo, int clientFd, const char *buffer)
{
	if (reqInfo._statusCode != 200)
	{
		std::string response = sendErrorResponse(reqInfo._statusCode, reqInfo);
		send(clientFd, response.c_str(), response.size(), 0);
		if (reqInfo._connection != "keep-alive")
		{
			close(clientFd);
		}
		return;
	}
	int fd_pipe_in[2];
	int fd_pipe_out[2];
	pipe(fd_pipe_in);
	pipe(fd_pipe_out);
	pid_t pid = fork();
	if (pid == 0)
	{
		close(fd_pipe_in[1]);
		dup2(fd_pipe_in[0], STDIN_FILENO);
		close(fd_pipe_in[0]);
		close(fd_pipe_out[0]);
		dup2(fd_pipe_out[1], STDOUT_FILENO);
		close(fd_pipe_out[1]);
		_handlerCGI.handleCGI(reqInfo);
		exit(1);
	}
	else
	{
		close(fd_pipe_in[0]);
		close(fd_pipe_out[1]);
		if (reqInfo._cLength > 0)
		{
			sendPostData(buffer, fd_pipe_in[1]);
		}
		close(fd_pipe_in[1]);
		size_t n;
		char buf[1024];
		std::string full_response;
		while ((n = read(fd_pipe_out[0], buf, sizeof(buf) - 1)) > 0)
		{
			buf[n] = '\0';
			full_response.append(buf);
		}
		int status;
		waitpid(pid, &status, 0);
		if (WIFEXITED(status))
		{
			int exit_code = WEXITSTATUS(status);
			if (exit_code != 0)
			{
				reqInfo._statusCode = 500;
			}
		}
		std::string response;
		if (reqInfo._statusCode == 200)
		{
			response = makeResponse(full_response, reqInfo);
		}
		else
		{
			response = sendErrorResponse(reqInfo._statusCode, reqInfo);
		}
		send(clientFd, response.c_str(), response.size(), 0);
		close(fd_pipe_out[0]);
	}
}

ServerBlock *Server::findServer(Routing::RequestInfo &reqInfo, std::vector<ServerBlock> &configs, int port)
{
	std::string host = reqInfo._host;
	for (size_t i = 0; i < configs.size(); i++)
	{
		ServerBlock &config = configs[i];

		if (config.GetPort() != port)
			continue;

		for (size_t j = 0; j < config.GetServerNames().size(); j++)
		{
			std::cout << "Checking server_name: '" << config.GetServerNames()[j] << "' against host: '" << host << "'" << std::endl;

			if (config.GetServerNames()[j] == host)
			{
				std::cout << "Found matching server: " << config.GetServerNames()[j] << " on port " << port << std::endl;
				return &config;
			}
		}
	}

	for (size_t i = 0; i < configs.size(); i++)
	{
		ServerBlock &config = configs[i];

		if (config.GetPort() == port && config.GetServerNames().empty())
		{
			std::cout << "Using default server on port " << port << std::endl;
			return &config;
		}
	}

	return NULL;
}
