#include "HttpRequest.hpp"


void checkForm(std::string input, Routing::RequestInfo& reqInfo)//hier am liebsten alle checks einbauen
{
	size_t pos= input.find_first_not_of(" \r\t\n");
	if (input.length() > 10485760 && input.find("boundary") == std::string::npos)
	{
		reqInfo._statusCode = 413;
		return;
	}
	if (input.length() <= 0 || pos == std::string::npos)
	{
		reqInfo._statusCode = 400;
		return;
	}
	size_t header_end = input.find("\r\n\r\n");
	size_t more_header = input.find("\r\n\r\n", header_end + 8);
	if (more_header == std::string::npos)
		more_header = header_end;
	if (input.find("boundary") == std::string::npos && (header_end > 8192 || (header_end != std::string::npos && more_header != header_end)))
	{
		reqInfo._statusCode = 431;
		return;
	}
	std::string headers_only = input.substr(0, header_end + 4);
	check_header(reqInfo, headers_only);
	if (reqInfo._statusCode != 200)
		return;
	std::istringstream ss(input);
	std::string line;
	std::getline(ss, line);//to skip first line
	while (std::getline(ss , line))
	{
		if (line.empty() || line == "\r")
		{
			break;
		}
		size_t colon = line.find(":");
		if (colon == std::string::npos)
		{
			reqInfo._statusCode = 400;
			return;
		}
		std::string headerName = line.substr(0, colon);
		if (!headerName.empty())
		{
			for (size_t i = 0; i < headerName.length(); ++i)
			{
				char c = headerName[i];
				if (!isalnum(c) && c != '_' && c != '-')
				{
					reqInfo._statusCode = 400;
					return;
				}
			}
		}
		if (headerName.empty() || line.at(colon + 1) != ' ')
		{
			reqInfo._statusCode = 400;
			return;
		}
	}
}

int countHeader(const std::string input, Routing::RequestInfo& reqInfo, std::string toSearch)//make it work for different parts of the header mayby
{
	(void)reqInfo;
	int count = 0;
	std::string buffer = input;
	size_t pos = buffer.find(toSearch, 0);
	size_t lhostPos = buffer.find("localhost", 0);
	while (pos != std::string::npos)
	{
		if (pos - 1 > 0 && (buffer.at(pos - 1) == '\n' || buffer.at(pos - 1) == ' '))
		{
			if (toSearch != "host" || lhostPos == std::string::npos || (lhostPos != std::string::npos &&  (pos - 5) != lhostPos))
				count++;
		}
		pos++;
		pos = buffer.find(toSearch, pos);
	}
	return (count);
}

void toLowerCase(std::string& input)
{
	for (size_t i = 0; i < input.length(); ++i)
	{
		if (input[i] >= 'A' && input[i] <= 'Z')
		{
			input[i] += 32;
		}
	}
}

void toUpperCase(std::string& input)
{
	for (size_t i = 0; i < input.length(); ++i)
	{
		if (input[i] >= 'a' && input[i] <= 'z')
		{
			input[i] -= 32;
		}
		else if (input[i] == '-')
			input[i] = '_';
	}
}

void check_header(Routing::RequestInfo& reqInfo, const std::string input)
{
	std::string buffer = input;
	toLowerCase(buffer);
	if (buffer.find("http/1.0") == std::string::npos && buffer.find("http/1.1") == std::string::npos && buffer.find("http/2.0") == std::string::npos)
	{
		reqInfo._statusCode = 505;
		return;
	}
	//count amount of headers
    size_t count = 0;
    size_t pos = 0;
    size_t endOfHeaders = buffer.find("\r\n\r\n");
	if (endOfHeaders == std::string::npos) {
		endOfHeaders = buffer.length(); // Falls kein doppeltes \r\n gefunden wird, bis zum Ende zählen
	}
    while ((pos = buffer.find("\r\n", pos)) != std::string::npos && pos < endOfHeaders) {
        count++;
        pos += 2; // Überspringe das aktuelle \r\n
    }
	if (count > 50) { // Limit auf 50 Header
		reqInfo._statusCode = 431; // Request Header Fields Too Large
		return;
	}
	std::set<std::string> singleHeaders;
	singleHeaders.insert("header");
	singleHeaders.insert("user-agent");
	singleHeaders.insert("authorization");
	singleHeaders.insert("location");
	singleHeaders.insert("date");
	singleHeaders.insert("server");
	singleHeaders.insert("from");
	singleHeaders.insert("referer");
	singleHeaders.insert("etag");
	singleHeaders.insert("content-encoding");
	singleHeaders.insert("content-location");
	singleHeaders.insert("content-range");
	singleHeaders.insert("retry-after");
	singleHeaders.insert("expires");
	singleHeaders.insert("last-modified");
	singleHeaders.insert("www-authenticate");
	singleHeaders.insert("proxy-authenticate");
	singleHeaders.insert("content-md5");
	singleHeaders.insert("content-language");
	if (countHeader(buffer, reqInfo, "http") == 0 || countHeader(buffer, reqInfo, "host") != 1)
	{
		reqInfo._statusCode = 400;
		return;
	}
	for (std::set<std::string>::iterator it = singleHeaders.begin() ; it != singleHeaders.end(); it++)
	{
		if (countHeader(buffer, reqInfo, *it) > 1)
		{
			reqInfo._statusCode = 400;
			return;
		}
	}
}

std::string extractPathFromUrl(std::string path)
{
	std::string ret;
	size_t pos;
	if ((pos = path.find("://")) != std::string::npos)
	{
		size_t path_start = path.find("/", pos + 3);
		if (path_start != std::string::npos)
		{
			ret = path.substr(path_start);
		}
		else 
			ret = "/";
	}
	else
		ret = path;
	return (ret);
}

std::string removeDoubleSlashes(const std::string& path)
{
	std::string result;
	bool lastWasSlash = false;
	for (size_t i = 0; i < path.length(); ++i) {
		if (path[i] == '/') {
			if (!lastWasSlash) {
				result += path[i];
				lastWasSlash = true;
			}
		} else {
			result += path[i];
			lastWasSlash = false;
		}
	}
	return result;
}

void checkPathInfo(Routing::RequestInfo& reqInfo)
{
	std::string fullPath = reqInfo._path;
	std::string currentPath = "";
	std::istringstream ss(fullPath);
	std::string segment;
	struct stat info;
	while (std::getline(ss, segment, '/'))
	{
		if (segment.empty()) continue;
		currentPath += segment;
		if (access(currentPath.c_str(), F_OK) == 0)
		{    
			if (stat(currentPath.c_str(), &info) == 0 && !S_ISDIR(info.st_mode))
			{
				reqInfo._pathInfo = fullPath.substr(currentPath.length());
				reqInfo._path = fullPath.substr(0, currentPath.length());
				return;
			}
		}
		currentPath += "/";
	}
}

void checkPath(Routing::RequestInfo& reqInfo)
{
	if (reqInfo._statusCode >= 400)
		return;
	std::string path = reqInfo._path;
	struct stat info;
	if (path.find("upload") != std::string::npos)
		return;
	if (stat(path.c_str(), &info) == -1)
	{
		reqInfo._statusCode = 404;
		return;
	}
	if (reqInfo._useCGI && access(path.c_str(), X_OK) != 0)
	{
		reqInfo._statusCode = 403;
		reqInfo._connection = "close";
		return;
	}
	size_t size = path.length();
	if (path.find(".html") != std::string::npos)
		return;
	if (S_ISDIR(info.st_mode) && path.at(size - 1) != '/')
	{
		path += reqInfo._index;
		if (access(path.c_str(), F_OK) == 0)
		{
			reqInfo._path = path;
		}
		return;
	}
	if (path.at(size - 1) == '/' && !reqInfo._index.empty())
	{
		//reqInfo._statusCode = 301;
		path += reqInfo._index;
		if (access(path.c_str(), F_OK) == 0)
		{
			reqInfo._path = path;
		}
		
	}
	if (reqInfo._autoIndex)
	{
		reqInfo._useAutoIndex = true;
		return;
	}
	//reqInfo._statusCode = 403;
}

void checkRequestValid(Routing::RequestInfo& reqInfo)
{
	if (reqInfo._statusCode != 200)
		return;
	if (reqInfo._cLength > reqInfo._max_body_size)
	{
		std::cout << "Request body too large: " << reqInfo._cLength << " > " << reqInfo._max_body_size << std::endl;
		reqInfo._statusCode = 413;
		return;
	}
	std::string method = reqInfo._method;
	if (method != "GET" && method != "POST" && method != "DELETE" && method != "PUT" && method != "PATCH"  && method != "HEAD" && method != "OPTIONS" && method != "CONNECT")
		reqInfo._statusCode = 501;
	else if (method != "GET" && method != "POST" && method != "DELETE")
		reqInfo._statusCode = 405;
	else if (method == "GET" && !reqInfo._methodsAllowed[0])
	{
		reqInfo._statusCode = 405;
	}
	else if (method == "POST" && !reqInfo._methodsAllowed[1])
		reqInfo._statusCode = 405;
	else if (method == "DELETE" && !reqInfo._methodsAllowed[2])
		reqInfo._statusCode = 405;
	else if (method == "UPLOAD")
		reqInfo._statusCode = 405;
	if (reqInfo._statusCode != 200)
		return;
	checkPath(reqInfo);
}

std::string URIDecode(std::string path)
{
	std::string result;
	for (size_t i = 0; i < path.length(); i++)
	{
		if (path.at(i) == '%' && i + 2 < path.length() && path.at(i + 1) == '2' && path.at(i + 2) == 'F')
		{
			for (size_t j = 0; j < 2; j++)
			{
				result += path.at(i);
				i++;
			}
			continue;
		}
		if (path.at(i) == '%' && i+2 < path.length())
		{
			char hex[3] = { path[i + 1], path[i + 2], '\0'};
			int value;
			std::istringstream iss(hex);
			iss >> std::hex >> value;
			result += static_cast<char>(value);
			i+= 2;
		}
		// else if (path.at(i) == '+')
		// 	result += ' ';
		else if (path.at(i) == '&')
			result += "\r\n";
		else
			result += path.at(i);
	}
	return (result);
}

void checkDecoded(Routing::RequestInfo& reqInfo)
{
	std::string path = reqInfo._path;
	if (path == "/")
		return;
	if (path.length() > 512) {
		reqInfo._statusCode = 414;
		return;
	}
	if (path.find('%') != std::string::npos)
		path = URIDecode(path);
	reqInfo._path = path;
	if (path.length() > 512) {
		reqInfo._statusCode = 414;
		return;
	}
	std::istringstream iss(path);
	std::string segment;
	while (std::getline(iss, segment, '/')) {
		if (segment.empty())
			break;
		if (segment == "..")
		{
			reqInfo._statusCode = 403;
			return;
		}
	}
	if (path.find("/..") != std::string::npos || path.find("%2e%2e") != std::string::npos)
	{
		reqInfo._statusCode = 403;
		return;
	}
	for(int i = 0; i < 32; i++) {
		char c = i;
		if (path.find(c) != std::string::npos) {
			reqInfo._statusCode = 400;
			return;
		}
	}
}

void parseCustomHeaders(std::string input, Routing::RequestInfo& reqInfo)
{
	size_t pos = 0;
	toLowerCase(input);
	std::string line;
	std::istringstream ss(input.substr(pos));
	while(std::getline(ss, line))
	{
		if (line == "\r" || line.empty())
			break;
		if ((line.find("x-")) == 0)
		{
			std::string name, value;
			size_t endPos = line.find("\r");
			size_t colon = line.find(':');
			size_t pos_value = colon + 2;
			name = line.substr(0, colon);
			value = line.substr(pos_value , endPos - pos_value);
			if (value.at(0) == ' ')
				value = value.substr(1);
			toUpperCase(name);
			std::string ret = "HTTP_" + name + "=" + value;
			reqInfo._customHeaders.push_back(ret);
		}
	}
}

int cmpPaths(std::string path1, std::string path2)
{
	int nb = 0;
	if (path1.at(path1.length() - 1) == '/')
		path1 = path1.substr(0, path1.length() - 1);
	if (path1.length() < path2.length())
		return (0);
	int border = path2.length();
	std::string pathMatch = path1.substr(0, border);
	if (pathMatch == path2)
	{
		nb = border;
	}
	return (nb);
}

void	sendPostData(std::string buf, int fd)
{
	size_t pos = buf.find("\r\n\r\n");
	if (pos != std::string::npos)
	{
		pos = pos + 4;
		std::string str = buf.substr(pos);
		write(fd, str.c_str(), str.size());
	}
}


void contentTypeValidation(Routing::RequestInfo& reqInfo)
{
	std::string cType = reqInfo._cType;
	size_t pos = cType.find(";");
	if (pos != std::string::npos)
		cType = cType.substr(0, pos);
	if (reqInfo._cType.empty())
		return;
	std::set<std::string> allowedTypes;
	// Text types
	allowedTypes.insert("text/html");
	allowedTypes.insert("text/plain");
	allowedTypes.insert("text/css");
	allowedTypes.insert("text/javascript");
	allowedTypes.insert("text/xml");
	allowedTypes.insert("text/csv");

	// Application types
	allowedTypes.insert("application/json");
	allowedTypes.insert("application/xml");
	allowedTypes.insert("application/javascript");
	allowedTypes.insert("application/pdf");
	allowedTypes.insert("application/zip");
	allowedTypes.insert("application/gzip");
	allowedTypes.insert("application/octet-stream");

	// Form types
	allowedTypes.insert("application/x-www-form-urlencoded");
	allowedTypes.insert("multipart/form-data");
	allowedTypes.insert("multipart/mixed");

	// Image types (für static files)
	allowedTypes.insert("image/jpeg");
	allowedTypes.insert("image/png");
	allowedTypes.insert("image/gif");
	allowedTypes.insert("image/webp");
	allowedTypes.insert("image/svg+xml");

	// Audio/Video (falls unterstützt)
	allowedTypes.insert("audio/mpeg");
	allowedTypes.insert("audio/wav");
	allowedTypes.insert("video/mp4");
	for (std::set<std::string>::iterator it = allowedTypes.begin() ; it != allowedTypes.end(); it++)
	{
		if (cType == *it)
		{
			return;
		}
	}
	reqInfo._statusCode = 400;
}

void decodeBody(std::string input, Routing::RequestInfo& reqInfo)
{
	size_t body = input.find("\r\n\r\n");
	if (body != std::string::npos)
	{
		std::string todecode = input.substr(body + 4);
		reqInfo._postBody = URIDecode(todecode);
	}
}