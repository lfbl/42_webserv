/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lfabel <lfabel@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/23 09:29:31 by lfabel            #+#    #+#             */
/*   Updated: 2025/09/25 11:08:14 by lfabel           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/HttpRequest.hpp"
#include <stdlib.h>

HttpRequest::HttpRequest() : _state(RECEIVING_HEADERS), _contentLength(0), _bodyStart(0), 
						_isChunked(false), _currentChunkSize(0), _currentChunkBytesRead(0)
{
}

std::string envs[][2] = 
{
	{"", "REQUEST_METHOD="}, 
	{"", "PATH="},
	{"", "QUERY_STRING="},
	{"", "VERSION="},
	{"Content-Type:", "CONTENT_TYPE="},
	{"Content-Length:", "CONTENT_LENGTH="},
	{"Host:", "HOST="},
	{"boundary=", "BOUNDARY="},
	{"User-Agent:", "HTTP_USER_AGENT="},
	{"Connection:", "CONNECTION="},
	{"Content-Disposition;", "CONTENT_DISPOSITION="},
	{"filename=", "FILENAME="},
};

void  processData(std::string input, char **envp, Routing::RequestInfo& reqInfo)
{
	HttpRequest httprequest;
	if (envp != NULL)
	{
		std::list<std::string> env;
		for (int i = 0; envp[i]; i++)
		{
			env.push_back(envp[i]);
		}
	}
	httprequest.processInput(input, reqInfo);
}

void HttpRequest::processInput(std::string input, Routing::RequestInfo& reqInfo)
{
	checkForm(input, reqInfo);
	if (reqInfo._statusCode != 200)
	{
		return;
	}
	int status = takeFirstLine(input);
	if (status != 200)
	{
		reqInfo._statusCode = status;
		return;
	}
	size_t pos2 = this->_env[envs[2][1]].find("status=");
	if (pos2 != std::string::npos)
	{
		std::string status = this->_env[envs[2][1]].substr(7, 3);
		int statusCode = std::atoi(status.c_str());
		if (statusCode >= 100 && statusCode <= 599)
			reqInfo._statusCode = statusCode;
	}
	for(int i = 4; i < 10; i++)
	{
		size_t foundPos = input.find(envs[i][0]);
		if (foundPos == std::string::npos)
		{
			this->_env[envs[i][1]] = "";
			continue;
		}
		size_t start_pos = input.find(envs[i][0]) + envs[i][0].length();
		start_pos = input.find(" ", start_pos);
		if (start_pos != std::string::npos)
			addToEnv(start_pos, input, i, foundPos);
	}
	this->envToStruct(reqInfo);
	if(searchBoundary(reqInfo, input) == 1)
	{
		reqInfo._boundary = this->_env[envs[7][1]];
	}
	this->envToStruct(reqInfo);
	std::string path = reqInfo._path;
	reqInfo._path = URIDecode(path);
	checkDecoded(reqInfo);
	if (reqInfo._statusCode != 200)
	{
		return;
	}
	if (input.find("application/x-www-form-urlencoded") != std::string::npos && reqInfo._method == "POST")
		decodeBody(input,reqInfo);
	parseCustomHeaders(input, reqInfo);
}

int HttpRequest::takeFirstLine(std::string input)
{
	std::stringstream ss(input);
	std::string first_line;
	std::getline(ss, first_line);
	std::istringstream line_ss(first_line);
	std::string method, path, version;
	line_ss >> method >> path >> version;
	if (method.empty() || path.empty() || version.empty())
	{
		return (400);
	}
	this->_env[envs[0][1]] = method;
	path = extractPathFromUrl(path);
	if (path.find("?") != std::string::npos)
	{
		if (path.length() > 2000)
		{
			return (414);
		}
		std::string query;
		size_t pos = path.find("?");
		query = path.substr(pos + 1, path.length() - (pos + 1));
		path = path.substr(0, pos);
		this->_env[envs[2][1]] = query;
	}
	this->_env[envs[1][1]] = path;
	this->_env[envs[3][1]] = version;
	return (200);
}

void HttpRequest::addToEnv(size_t start_pos, std::string input, int i, size_t foundPos)
{
	size_t line_end = input.find("\r\n", foundPos);
	if (line_end == std::string::npos)
	{
		line_end = input.find("\n", foundPos);
	}
	if (line_end == std::string::npos)
	{
		line_end = input.length();  // End of input
	}
	if (start_pos + 1 >= line_end || start_pos >= input.length())
	{
		this->_env[envs[i][1]] = "";
		return;
	}
	std::string value = input.substr(start_pos + 1, line_end - (start_pos + 1));
	value.erase(0, value.find_first_not_of(" \t"));
	value.erase(value.find_last_not_of(" \t\r\n") + 1);
	
	if (i == 7)
	{
		size_t semicolon = value.find(';');
		if (semicolon != std::string::npos)
		{
			value = value.substr(0, semicolon);
		}
	} else if (i == 9)
	{
		size_t comma = value.find(',');
		if (comma != std::string::npos)
			value = value.substr(0, comma);
	}
	
	this->_env[envs[i][1]] = value;
}

void HttpRequest::envToStruct(Routing::RequestInfo& reqInfo)
{
	reqInfo._path = this->_env[envs[1][1]];

	reqInfo._method = this->_env[envs[0][1]];
	reqInfo._queryString = this->_env[envs[2][1]];
	reqInfo._protocol = this->_env[envs[3][1]];
	reqInfo._cType = this->_env[envs[4][1]];
	contentTypeValidation(reqInfo);
	if (reqInfo._statusCode != 200)
		return;
	size_t value = 0;
	std::stringstream ss(this->_env[envs[5][1]]);
	ss >> value;
	reqInfo._cLength = value;
	reqInfo._host = this->_env[envs[6][1]];
	reqInfo._boundary = this->_env[envs[7][1]];
	reqInfo._httpUserAgent = this->_env[envs[8][1]];
	reqInfo._connection = this->_env[envs[9][1]];
}

int	HttpRequest::searchBoundary(Routing::RequestInfo& reqInfo, std::string input)
{
	size_t foundPos = input.find(envs[7][0]);
	if (foundPos == std::string::npos)
	{
		this->_env[envs[7][1]] = "";
		return (0);
	}
	size_t start_pos = input.find(envs[7][0]);
	start_pos = input.find("=", start_pos);
	reqInfo._boundary = input.substr(start_pos + 1, input.find("\r\n", foundPos) - (start_pos + 1));
	return (1);
}

void	HttpRequest::skipTillBoundary(std::string input, Routing::RequestInfo& reqInfo)
{
	for(int i = 8; i < 10; i++)
	{
		size_t foundPos = input.find(envs[i][0]);
		if (foundPos == std::string::npos)
		{
			this->_env[envs[i][1]] = "";
			continue;
		}
		size_t start_pos = input.find(envs[i][0]) + envs[i][0].length();
		addToEnv(start_pos, input, i, foundPos);
	}
	std::string boundary = "--" + this->_env[envs[7][1]];
	size_t start = input.find(boundary);
	input.erase(0, start);
	start = find_body(input, 0);
	input.erase(0, start);
	size_t end = input.find(boundary);
	if (end == std::string::npos)
		end = input.size();
	input.erase(end);
	reqInfo._data = input;
	return;
}

void findConfigs(ServerBlock& config, Routing::RequestInfo& reqInfo)
{
	int nb = -1;
	int matching = 0;
	std::string path = reqInfo._path;
	setBase(reqInfo, config);
	std::vector<ServerBlock::Location> locations;
	locations = config.GetLocations();
	for (size_t i = 0; i < locations.size() ;i++)
	{
		locations[i].SetConfig(&config);
		int tmp = cmpPaths(path, locations[i].path);
		if (tmp > matching)
		{
			matching = tmp;
			nb = i;
		}
	}
	if (nb == -1)
	{
		nb = 0;
		for (size_t i = 0; i < locations.size(); i++)
		{
			if (locations[i].path == "/")
			{
				nb = i;
				break;
			}
		}
	}
	if (locations[nb].path == "/upload")
		reqInfo._upload = true;
	setConfigs(locations[nb], reqInfo);
	if (locations[nb].bodySize != -1)
	{
		reqInfo._max_body_size = locations[nb].bodySize;
	}
	else
	{
		reqInfo._max_body_size = config.GetBodySize();
	}
}

void setConfigs(ServerBlock::Location& locations, Routing::RequestInfo& reqInfo)
{
	reqInfo._root = locations.root;
	reqInfo._cgiEnabled = locations.cgiEnable;
	if (reqInfo._path.find(".cgi") != std::string::npos && reqInfo._cgiEnabled)
	{
			reqInfo._useCGI = true;
			reqInfo._path_cgi = "/usr/bin/sh";
	}
	for (std::vector<ServerBlock::CGIType>::iterator it = locations.cgiHandlers.begin(); it != locations.cgiHandlers.end(); it++)
	{
		if (reqInfo._path.find(it->extension) != std::string::npos)
		{
			reqInfo._useCGI = true;
			reqInfo._path_cgi = it->path;
			break;
		}
	}
	std::vector<ServerBlock::ErrorPage> errorPages = locations.GetErrorPages();
	for (size_t i = 0; i < errorPages.size(); ++i)
	{
		ServerBlock::ErrorPage errorPage;
		errorPage.error = errorPages[i].error;
		errorPage.page = errorPages[i].page;
		reqInfo._errorPages.push_back(errorPage);
	}
	for (int i = 0; i < 3; i++)
	{
		reqInfo._methodsAllowed[i] = locations.methods[i];
	}
	reqInfo._autoIndex = locations.autoindex;
	if (reqInfo._autoIndex == 1 && reqInfo._path == "/")
		reqInfo._useAutoIndex = 1;
	if (!locations.defFiles.empty())
		reqInfo._index = locations.defFiles[0];
	reqInfo._uploadEnable = locations.uploadEnable;
	reqInfo._uploadPath = locations.uploadPath;
	if (locations.redirect)
	{
		reqInfo._path = locations.redir.page;
		reqInfo._redirectLocation = locations.redir.page;
		reqInfo._statusCode = locations.redir.error;
	}
	std::string path_save = reqInfo._path;
	size_t toCut = 0;
	std::stringstream ss;
	ss << path_save;
	if (path_save.find("/") != std::string::npos)
	{
	std::string pathpart;
	std::string toCmp;
	std::getline(ss, pathpart, '/');
	while (std::getline(ss, pathpart, '/'))
	{
		std::cerr << pathpart << std::endl;
		toCmp += "/" + pathpart;
		if (locations.path.find(toCmp) != std::string::npos)
		{
		toCut = toCmp.length();
		}
		else
		break;
	}
	}
	if (locations.exact)
	{
		if (path_save.length() > locations.path.length())
		{
			path_save = path_save.substr(locations.path.length() - 1);
			reqInfo._path = locations.root + '/' + path_save;	
			reqInfo._path = removeDoubleSlashes(reqInfo._path);

		}
		else
			reqInfo._path = locations.root;
	}
	else
	{
		path_save = path_save.substr(toCut, path_save.length() - toCut);
		if (reqInfo._upload)
		{
			std::cerr << reqInfo._uploadPath << " " << path_save << std::endl;
			reqInfo._path = reqInfo._uploadPath + '/' + path_save;
		}
		else
			reqInfo._path = reqInfo._root + '/' + path_save;
		reqInfo._path = removeDoubleSlashes(reqInfo._path);
	}
	checkPathInfo(reqInfo);
}


void setBase(Routing::RequestInfo& reqInfo, ServerBlock& config)
{
	reqInfo._max_body_size = config.GetBodySize();
}

void handleUpload(Routing::RequestInfo& reqInfo, const char *buffer)
{

	if (reqInfo._cLength > reqInfo._max_body_size)
	{
		reqInfo._statusCode = 413;
		return;
	}
	std::string input = buffer;
	if (!reqInfo._boundary.empty())
	{
		std::string boundary = "--" + reqInfo._boundary;
		std::string last_boundary = "--" + reqInfo._boundary + "--";
		size_t pos_next_boundary = input.find(boundary);
		if (pos_next_boundary == std::string::npos)
		{
			reqInfo._statusCode = 87;
			return;
		}
		size_t pos_last_boundary = input.find(last_boundary);
		if (pos_last_boundary == std::string::npos)
			pos_last_boundary = input.size();
		while (pos_next_boundary != pos_last_boundary)
		{
			std::string body = input.substr(pos_next_boundary);
			size_t start = find_body(body, 0);
			if (start == std::string::npos)
			{
				reqInfo._statusCode = 444;
				return;
			}
			std::string header = body.substr(0, start);
			std::string line;
			std::istringstream ss(header);
			while (std::getline(ss, line, ';')) {
				line.erase(0, line.find_first_not_of(" \t"));
				size_t sep = line.find('=');
				if (sep != std::string::npos) {
					std::string key = line.substr(0, sep);
					std::string value = line.substr(sep + 1);
					value.erase(0, value.find_first_not_of("\""));
					value.erase(value.find_last_of("\""));
					if (key.find("filename") != std::string::npos)
						reqInfo._filename = value;
					else if (key.find("name") != std::string::npos)
						reqInfo._contentDisposition = value;
				}
			}
			body.erase(0, start);
			size_t end = body.find(boundary);
			if (end == std::string::npos)
				end = body.size();
			body.erase(end);
			if (reqInfo._filename.empty())
			{
				reqInfo._fields.push_back(body);
			}
			else
			{
				std::string filename = reqInfo._filename;
				std::string filepath = reqInfo._uploadPath + "/" + filename;
				std::ofstream file;
				file.open(filepath.c_str());
				if (!file.is_open())
				{
					reqInfo._statusCode = 500;
					return;
				}
				file << body << std::endl;
				file.close();
			}
			if (pos_next_boundary == pos_last_boundary)
				break;
			pos_next_boundary = input.find(boundary, pos_next_boundary + 3);
			if (pos_next_boundary == std::string::npos)
				break;
		}
	}
	else
	{
		const char* headerEnd = strstr(buffer, "\r\n\r\n");
		size_t bodyPos = headerEnd - buffer + 4;
		if (bodyPos == std::string::npos)
		{
			reqInfo._statusCode = 400;
			return;
		}
		size_t bodyLength = reqInfo._cLength;
		std::string filename = reqInfo._path;
		std::ofstream file;
		file.open(filename.c_str(), std::ios::out | std::ios::binary);
		if (!file.is_open())
		{
			reqInfo._statusCode = 500;
			return;
		}
		file.write(buffer + bodyPos, bodyLength);
		file.close();
	}
	reqInfo._statusCode = 201;
}

void deletefile(Routing::RequestInfo& reqInfo)
{
	int status = remove(reqInfo._path.c_str());
	if (status != 0)
	{
		reqInfo._statusCode = status;
		//std::cerr << "Error deleting File" << this->_path.c_str() <<std::endl;
	} else{
		std::cout << "204 No Content \r\n" << std::	endl;
	}
}


//-----------------------Server for chunked transfer encoding-----------------------
bool HttpRequest::ProcessRawData(const char* chunk, size_t length, int* statusCode) {
// Reject additional data if request is already complete
	if (_state == COMPLETE || _state == ERROR) {
		return false;
	}

	_rawData.append(chunk, length);

	if (_state == RECEIVING_HEADERS) {
		size_t headerEnd = _rawData.find("\r\n\r\n");
		if (headerEnd != std::string::npos) {
			_bodyStart = headerEnd + 4;
			
			// Check for Transfer-Encoding: chunked
			size_t tePos = _rawData.find("Transfer-Encoding:");
			if (tePos != std::string::npos && tePos < headerEnd) {
				size_t valStart = tePos + 18;
				while (valStart < headerEnd && isspace(_rawData[valStart])) valStart++;
				size_t valEnd = _rawData.find("\r\n", valStart);
				std::string teValue = _rawData.substr(valStart, valEnd - valStart);
				if (teValue.find("chunked") != std::string::npos) {
					_isChunked = true;
					_state = RECEIVING_CHUNK_SIZE;
					return processChunkedData();
				}
			}
			
			// Check for Content-Length
			size_t clPos = _rawData.find("Content-Length:");
			if (clPos != std::string::npos && clPos < headerEnd) {
				size_t valStart = clPos + 15;
				while (valStart < _rawData.size() && isspace(_rawData[valStart])) valStart++;
				size_t valEnd = _rawData.find("\r\n", valStart);
				std::string clValue = _rawData.substr(valStart, valEnd - valStart);
				try {
					std::istringstream iss(clValue);
					unsigned long val;
					if (!(iss >> val)) {
						throw std::exception();
					}
					_contentLength = val;
				} catch (const std::exception& e) {
					*statusCode = 400;
					_contentLength = 0;
				}
				_state = RECEIVING_BODY;
				if (_rawData.size() >= _bodyStart + _contentLength) {
					_state = COMPLETE;
					return true;
				}
			} else if (!_isChunked) {
				// No Content-Length and not chunked, request is complete
				_state = COMPLETE;
				return true;
			}
		}
	}

	if (_state == RECEIVING_BODY && _contentLength > 0) {
		if (_rawData.size() >= _bodyStart + _contentLength) {
			_state = COMPLETE;
			return true;
		}
}

	if (_isChunked && (_state == RECEIVING_CHUNK_SIZE || _state == RECEIVING_CHUNK_DATA || _state == RECEIVING_CHUNK_TRAILER)) {
		return processChunkedData();
	}

	return false;
}

bool HttpRequest::isComplete() const {
	return _state == COMPLETE;
}

std::string HttpRequest::getParsedBody() const {
	if (!_isChunked) {
		// For non-chunked requests, return the body directly
		if (_rawData.size() > _bodyStart) {
			return _rawData.substr(_bodyStart);
		}
		return "";
}

// For chunked requests, return the decoded body
return _decodedBody;
}

std::string HttpRequest::getDecodedData() const {
	if (!_isChunked) {
		// For non-chunked requests, return raw data as-is
		return _rawData;
	}

	// For chunked requests, return headers + decoded body
	size_t headerEnd = _rawData.find("\r\n\r\n");
	if (headerEnd != std::string::npos) {
		std::string headers = _rawData.substr(0, headerEnd + 4);
		return headers + _decodedBody;
	}

	return _rawData; // Fallback if headers not found
}

bool HttpRequest::processChunkedData() {
	size_t pos = _bodyStart;

	while (pos < _rawData.size()) {
		if (_state == RECEIVING_CHUNK_SIZE) {
			// Look for chunk size line ending with \r\n
			size_t crlfPos = _rawData.find("\r\n", pos);
			if (crlfPos == std::string::npos) {
				return false; // Need more data
			}
			
			// Parse chunk size (hexadecimal)
			std::string chunkSizeStr = _rawData.substr(pos, crlfPos - pos);
			try {
				// Convert hex string to unsigned long in C++98
				unsigned long val = 0;
				for (size_t i = 0; i < chunkSizeStr.length(); i++) {
					char c = chunkSizeStr[i];
					if (c >= '0' && c <= '9')
						val = (val << 4) | (c - '0');
					else if (c >= 'a' && c <= 'f')
						val = (val << 4) | (c - 'a' + 10);
					else if (c >= 'A' && c <= 'F')
						val = (val << 4) | (c - 'A' + 10);
					else
						throw std::exception();
				}
				_currentChunkSize = val;
			} catch (const std::exception& e) {
				_state = ERROR;
				return false;
			}
			_currentChunkBytesRead = 0;
			
			pos = crlfPos + 2; // Move past \r\n
			_bodyStart = pos; // Update body start for next iteration
			
			if (_currentChunkSize == 0) {
				// Final chunk, transition to trailer state
				_state = RECEIVING_CHUNK_TRAILER;
			} else {
				_state = RECEIVING_CHUNK_DATA;
			}
		}
		else if (_state == RECEIVING_CHUNK_DATA) {
			// We need chunk data + \r\n
			size_t neededBytes = _currentChunkSize + 2;
			size_t availableBytes = _rawData.size() - pos;
			
			if (availableBytes < neededBytes) {
				return false; // Need more data
			}
			
			// Extract chunk data (without the trailing \r\n) and add to decoded body
			_decodedBody += _rawData.substr(pos, _currentChunkSize);
			
			// Move past chunk data + \r\n
			pos += neededBytes;
			_bodyStart = pos; // Update body start for next iteration
			_state = RECEIVING_CHUNK_SIZE;
		}
		else if (_state == RECEIVING_CHUNK_TRAILER) {
			// Look for final \r\n after the 0 chunk
			if (pos + 1 < _rawData.size() && _rawData.substr(pos, 2) == "\r\n") {
				_state = COMPLETE;
				return true;
			}
			return false; // Need more data for final \r\n
		}
	}

	return false;
}
