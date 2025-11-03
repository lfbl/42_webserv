/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lfabel <lfabel@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/23 09:29:29 by lfabel            #+#    #+#             */
/*   Updated: 2025/09/25 10:30:35 by lfabel           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include "../inc/CGIHandler.hpp"
#include "../inc/Response.hpp"
#include "../inc/ServerBlock.hpp"
#include "../inc/Routing.hpp"
#include <fstream>
#include <iostream>
#include <dirent.h>
#include <vector>
#include <set>
#include <list>
#include <sys/stat.h>
# include <stdio.h>
class CGIHandler;

class HttpRequest 
{
public:
	enum RequestState {
		RECEIVING_HEADERS,
		RECEIVING_BODY,
		RECEIVING_CHUNK_SIZE,
		RECEIVING_CHUNK_DATA,
		RECEIVING_CHUNK_TRAILER,
		COMPLETE,
		ERROR
	};
	HttpRequest();
	bool ProcessRawData(const char* chunk, size_t length, int* statusCode);
	bool isComplete() const;

	// Debug methods
	std::string& getRawData() { return _rawData; }
	std::string getDecodedData() const; // Returns headers + decoded body for chunked requests
	RequestState getState() const { return _state; }
	std::string getParsedBody() const;
	void	addToEnv(size_t start_pos, std::string input, int i, size_t foundPos);
	void	processInput(std::string input, Routing::RequestInfo& reqInfo);
	int		takeFirstLine(std::string input);
	void	check_body(CGIHandler& handlerCGI);
	void	check_header(CGIHandler& handlerCGI);
	int		searchBoundary(Routing::RequestInfo& reqInfo, std::string input);
	void	skipTillBoundary(std::string input, Routing::RequestInfo& reqInfo);
	void setSystemEnv();
	void envToStruct(Routing::RequestInfo& reqInfo);

private:
	bool processChunkedData();

	std::map<std::string, std::string> _env;
	std::string _rawData;
	RequestState _state;
	size_t _contentLength;
	size_t _bodyStart;
	bool _isChunked;
	size_t _currentChunkSize;
	size_t _currentChunkBytesRead;
	std::string _decodedBody; // send this to CGIHandler
};

void sendPostData(std::string buffer, int fd);
void checkRequestValid(Routing::RequestInfo& reqInfo);
void  processData(std::string &input, char **envp, Routing::RequestInfo& reqInfo);
void findConfigs(ServerBlock& config, Routing::RequestInfo& reqInfo);
void checkPath(Routing::RequestInfo& reqInfo);
void setConfigs(ServerBlock::Location& locations, Routing::RequestInfo& reqInfo);
void checkPathInfo(Routing::RequestInfo& reqInfo);
void check_header(Routing::RequestInfo& reqInfo, const std::string input);
void checkForm(std::string input, Routing::RequestInfo& reqInfo);
void handleUpload(Routing::RequestInfo& reqInfo, const char *buffer);
void setBase(Routing::RequestInfo& reqInfo, ServerBlock& config);
std::string removeDoubleSlashes(const std::string& path);
int countHeader(const std::string input, Routing::RequestInfo& reqInfo, std::string toSearch);
std::string URIDecode(std::string path);
void checkDecoded(Routing::RequestInfo& reqInfo);
int cmpPaths(std::string path1, std::string path2);
void parseCustomHeaders(std::string input, Routing::RequestInfo& reqInfo);
void toUpperCase(std::string& input);
void toLowerCase(std::string& input);
void contentTypeValidation(Routing::RequestInfo& reqInfo);
void decodeBody(std::string input, Routing::RequestInfo& reqInfo);
std::string extractPathFromUrl(std::string path);
void deletefile(Routing::RequestInfo& reqInfo);

#endif