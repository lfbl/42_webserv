/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lfabel <lfabel@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/23 10:50:08 by lfabel            #+#    #+#             */
/*   Updated: 2025/08/27 16:15:01 by lfabel           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <netinet/in.h>
#include <signal.h>
#include "../inc/CGIHandler.hpp"
#include "../inc/Response.hpp"
#include "../inc/ServerBlock.hpp"
#include <errno.h>
#include <arpa/inet.h>

#define MAX_CLIENTS 2048

struct Connection
{
	int clientFd;
	size_t fdIndex;  // Index in the _fds vector
	std::string responseBuffer;
	bool isSending;
	size_t bytesSent;
	bool keepAlive;
	Connection(int fd, size_t index) : clientFd(fd), fdIndex(index), isSending(false), bytesSent(0), keepAlive(false) {}
	Connection() : clientFd(-1), fdIndex(0), isSending(false), bytesSent(0), keepAlive(false) {}
};

class Server
{
private:
	struct sockaddr_in _serverAddr;
	std::vector<struct pollfd> _fds;
	int _listenNum;								 // Number of listening sockets
	int _pollinNum;								 // Number of POLLIN events
	std::map<int, std::string> _requestsMap;	 // Map to associate file descriptors with HTTP requests
	std::map<int, HttpRequest> _httpRequestsMap; // Map to associate file descriptors with HTTP requests
	std::map<int, time_t> _lastActivityMap;		 // Map to track last activity time for each FD
	std::map<int, int> _clientToServerMap;		 // Map to track which server config each client belongs to
	static const int TIMEOUT_SECONDS = 60;		 // 60 second timeout
	static Server *instance;					 // Static instance for signal handler
	CGIHandler _handlerCGI;
	ServerBlock _config;
	std::vector<ServerBlock> _configs;
	std::vector<bool> _isSending;
	std::vector<int> _bytesSent;
	std::map<size_t, Connection> _connections;
	std::string _responseBuffer;
	size_t _bytesToSend;
	void setSockAddrStruct(in_port_t port, std::string ip);
	void checkAndConnectNewClient();
	void checkClientsForData(char **envp);
	void checkForTimeouts();
	bool isHeaderComplete(const std::string &request) const;
	bool isRequestComplete(const std::string &request) const;
	bool isBodyLengthValid(const std::string &request, const size_t &contentLength) const;
	Server(const Server &other);
	Server &operator=(const Server &other);

public:
	Server();
	// ~Server();
	void Initialize();
	void serverListen(char **envp);
	void server_base(char **envp);
	static void sigint(int signo);
	void setConfig(ServerBlock &config);
	void setConfigs(std::vector<ServerBlock> &config);
	void startCGI(Routing::RequestInfo &reqInfo, int clientFd, const char *buffer);
	bool startProcess(char **envp, const char *buffer, int clientFd, int initStatusCode, int i);
	ServerBlock *findServer(Routing::RequestInfo &reqInfo, std::vector<ServerBlock> &configs, int port);

	bool isKeepAlive(Routing::RequestInfo &reqInfo);
	void handleClient(int clientFd);
};