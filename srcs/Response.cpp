/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lfabel <lfabel@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/23 10:50:19 by lfabel            #+#    #+#             */
/*   Updated: 2025/09/25 11:06:38 by lfabel           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Response.hpp"


// Function to create and initialize the STATUS_CODES map in a C++98 compatible way
static std::map<int, std::string> createStatusCodesMap()
{
    std::map<int, std::string> codes;
    // 1xx: Informational
    codes.insert(std::make_pair(100, "Continue"));
    codes.insert(std::make_pair(101, "Switching Protocols"));
    codes.insert(std::make_pair(102, "Processing"));
    // 2xx: Success
    codes.insert(std::make_pair(200, "OK"));
    codes.insert(std::make_pair(201, "Created"));
    codes.insert(std::make_pair(202, "Accepted"));
    codes.insert(std::make_pair(203, "Non-Authoritative Information"));
    codes.insert(std::make_pair(204, "No Content"));
    codes.insert(std::make_pair(205, "Reset Content"));
    codes.insert(std::make_pair(206, "Partial Content"));
    codes.insert(std::make_pair(207, "Multi-Status"));
    // 3xx: Redirection
    codes.insert(std::make_pair(300, "Multiple Choices"));
    codes.insert(std::make_pair(301, "Moved Permanently"));
    codes.insert(std::make_pair(302, "Found"));
    codes.insert(std::make_pair(303, "See Other"));
    codes.insert(std::make_pair(304, "Not Modified"));
    codes.insert(std::make_pair(305, "Use Proxy"));
    codes.insert(std::make_pair(307, "Temporary Redirect"));
    codes.insert(std::make_pair(308, "Permanent Redirect"));
    // 4xx: Client Error
    codes.insert(std::make_pair(400, "Bad Request"));
    codes.insert(std::make_pair(401, "Unauthorized"));
    codes.insert(std::make_pair(402, "Payment Required"));
    codes.insert(std::make_pair(403, "Forbidden"));
    codes.insert(std::make_pair(404, "Not Found"));
    codes.insert(std::make_pair(405, "Method Not Allowed"));
    codes.insert(std::make_pair(406, "Not Acceptable"));
    codes.insert(std::make_pair(407, "Proxy Authentication Required"));
    codes.insert(std::make_pair(408, "Request Timeout"));
    codes.insert(std::make_pair(409, "Conflict"));
    codes.insert(std::make_pair(410, "Gone"));
    codes.insert(std::make_pair(411, "Length Required"));
    codes.insert(std::make_pair(412, "Precondition Failed"));
    codes.insert(std::make_pair(413, "Payload Too Large"));
    codes.insert(std::make_pair(414, "URI Too Long"));
    codes.insert(std::make_pair(415, "Unsupported Media Type"));
    codes.insert(std::make_pair(416, "Range Not Satisfiable"));
    codes.insert(std::make_pair(417, "Expectation Failed"));
    codes.insert(std::make_pair(418, "I'm a teapot"));
    codes.insert(std::make_pair(421, "Misdirected Request"));
    codes.insert(std::make_pair(422, "Unprocessable Entity"));
    codes.insert(std::make_pair(423, "Locked"));
    codes.insert(std::make_pair(424, "Failed Dependency"));
    codes.insert(std::make_pair(425, "Too Early"));
    codes.insert(std::make_pair(426, "Upgrade Required"));
    codes.insert(std::make_pair(428, "Precondition Required"));
    codes.insert(std::make_pair(429, "Too Many Requests"));
    codes.insert(std::make_pair(431, "Request Header Fields Too Large"));
    codes.insert(std::make_pair(451, "Unavailable For Legal Reasons"));
    // 5xx: Server Error
    codes.insert(std::make_pair(500, "Internal Server Error"));
    codes.insert(std::make_pair(501, "Not Implemented"));
    codes.insert(std::make_pair(502, "Bad Gateway"));
    codes.insert(std::make_pair(503, "Service Unavailable"));
    codes.insert(std::make_pair(504, "Gateway Timeout"));
    codes.insert(std::make_pair(505, "HTTP Version Not Supported"));
    codes.insert(std::make_pair(506, "Variant Also Negotiates"));
    codes.insert(std::make_pair(507, "Insufficient Storage"));
    codes.insert(std::make_pair(508, "Loop Detected"));
    codes.insert(std::make_pair(510, "Not Extended"));
    codes.insert(std::make_pair(511, "Network Authentication Required"));
    return codes;
}

// Static const map initialized with the helper function
static const std::map<int, std::string> STATUS_CODES = createStatusCodesMap();

std::string trim_response(std::string buf, size_t pos)
{
	std::string ret;
	size_t pos_of_start = buf.find("HTTP/1.1");
	if (pos_of_start == std::string::npos)
		pos_of_start = buf.find("Status:");
	if (pos_of_start != std::string::npos)
	{
		return buf.substr(pos);
	}
	ret = buf.substr(pos, buf.size() - pos);
	return (ret);
}

size_t find_body(std::string buf, size_t pos)
{
	size_t posStart = buf.find("\r\n\r\n", pos);
	if (posStart != std::string::npos)
		return(posStart + 4);
	if (posStart == std::string::npos)
		posStart = buf.find("\n\n", pos);
	if (posStart != std::string::npos)
		return(posStart + 2);
	else
		return (std::string::npos);
}

std::string createHeader(const std::string &body, Routing::RequestInfo &reqInfo)
{
	std::string header;
	std::stringstream ss;
	ss << reqInfo._statusCode;
	size_t start_body;
	std::map<int, std::string>::const_iterator it = STATUS_CODES.begin();
	std::string response = "OK";
	while (it != STATUS_CODES.end())
	{
		if (reqInfo._statusCode == it->first)
		{
			response = it->second;
			break;
		}
		it++;
	}
	header += reqInfo._protocol + " " + ss.str() + " " + response + "\r\n";
	if (body.find("Content-Length:") == std::string::npos)
	{
		start_body = find_body(body, 0);
		if (start_body != std::string::npos || !body.empty())
		{
			size_t length = body.length(); // deleted +4
			if (start_body != std::string::npos)
				length -= start_body;
			std::stringstream ss;
			ss << length;
			header += "Content-Length: " + ss.str() + "\r\n";
		}
		else
			header += "Content-Length: 0\r\n";
	}
	if (body.find("Content-Type:") == std::string::npos)
	{
		if (!reqInfo._cType.empty())
			header += "Content-Type: " + reqInfo._cType + "\r\n";
		else
			header += "Content-Type: text/html\r\n";
	}
	if (reqInfo._statusCode >= 300 && reqInfo._statusCode < 400)
	{
		header += "Location: " + reqInfo._redirectLocation + "\r\n";
		if (reqInfo._statusCode >= 301)
			header += "Cache-Control: max-age=31536000\r\n"; // 1 year
	}
	if (reqInfo._connection == "")
		reqInfo._connection = "close";
	header += "Connection: " + reqInfo._connection + "\r\n";
	header += "Server: Webserv42\r\n";
	std::time_t date = std::time(NULL);
	ss.str("");
	ss << std::asctime(std::localtime(&date));
	std::string dateStr = ss.str();
	if (!dateStr.empty() && dateStr[dateStr.length() - 1] == '\n')
	{
		dateStr.erase(dateStr.length() - 1);
	}
	header += "Date: " + dateStr + "\r\n";
	return (header);
}

std::string makeResponse(std::string full_response, Routing::RequestInfo &reqInfo)
{
	std::string header;
	std::string trimed;
	std::string ret;
	
	if (!reqInfo._postBody.empty())
		full_response += "\r\nParsed parameters:\r\n" + reqInfo._postBody;
	header = createHeader(full_response, reqInfo);
	size_t pos = find_body(full_response, 0);
	if (pos == std::string::npos)
		pos = full_response.find("\n\n");
	if (pos == std::string::npos)
	{
		header.append("\r\n");
	}
	trimed = full_response;
	ret = header + trimed;
	return (ret);
}

std::string sendErrorResponse(int status, Routing::RequestInfo &reqInfo)
{
	reqInfo._connection = "close";
	std::map<int, std::string>::const_iterator it = STATUS_CODES.begin();
	std::string response;
	std::stringstream ss;
	while (it != STATUS_CODES.end())
	{
		if (status == it->first)
		{
			ss << it->first;
			std::string body;
			for (size_t i = 0; i < reqInfo._errorPages.size(); i++)
			{
				if (reqInfo._errorPages[i].error == status)
				{
					std::string errorFile = reqInfo._errorPages[i].page;
					if (access(errorFile.c_str(), F_OK) == 0)
					{
						std::ifstream file;
						file.open(errorFile.c_str());
						if (file.is_open())
						{
							std::stringstream buffer;
							buffer << file.rdbuf();
							body = buffer.str();
							file.close();
							break;
						}
					}
				}
			}
			// else
			
			// 	body = "<!DOCTYPE html>\r\n"
			// 		   "<html>\r\n"
			// 		   "<head>\r\n"
			// 		   "<title>" +
			// 		   ss.str() + " " + it->second + "</title>\r\n"
			// 										 "<style>\r\n"
			// 										 "        body { font-family: Arial, sans-serif; margin: 40px; }\r\n"
			// 										 "        .error-container { text-align: center; }\r\n"
			// 										 "        .error-code { font-size: 48px; color: #444; }\r\n"
			// 										 "        .error-text { font-size: 24px; color: #666; }\r\n"
			// 										 "    </style>\r\n"
			// 										 "</head>\r\n"
			// 										 "<body>\r\n"
			// 										 "    <div class=\"error-container\">\r\n"
			// 										 "        <div class=\"error-code\">" +
			// 		   ss.str() + "</div>\r\n"
			// 					  "<div class=\"error-text\">" +
			// 		   it->second + "</div>\r\n"
			// 						"    </div>\r\n"
			// 						"</body>\r\n"
			// 						"</html>\r\n";
			// }
			std::stringstream ss_length;
			ss_length << body.length();
			std::stringstream ss_header;
			ss_header << "HTTP/1.1 " << it->first << " " << it->second << "\r\n";
			if (status == 405)
			{
				ss_header << "Allow: ";
				if (reqInfo._methodsAllowed[0])
					ss_header << "GET";
				if (reqInfo._methodsAllowed[1])
				{
					if (reqInfo._methodsAllowed[0])
						ss_header << ", ";
					ss_header << "POST";
				}
				if (reqInfo._methodsAllowed[2])
				{
					if (reqInfo._methodsAllowed[0] || reqInfo._methodsAllowed[1])
						ss_header << ", ";
					ss_header << "DELETE";
				}
				ss_header << "\r\n";
			}
			ss_header << "Content-Type: text/html\r\n";
			ss_header << "Content-Length: " << ss_length.str() << "\r\n";
			ss_header << "Connection: close\r\n\r\n";
			response = ss_header.str();
			response += body;
			break;
		}
		it++;
	}
	return (response);
}

std::string executeStatic(std::string path)
{
	std::string content;
	std::string File = path;
	if (access(File.c_str(), F_OK) == 0)
	{
		std::ifstream file;
		file.open(File.c_str());
		if (file.is_open())
		{
			std::stringstream buffer;
			buffer << file.rdbuf();
			content = buffer.str();
			file.close();
		}
	}
	return (content);
}

std::vector<std::string> getData(std::string path) // wahrscheinlich besser in dem response creation header part
{
	std::vector<std::string> files;
	DIR *dir = opendir(path.c_str());
	if (dir)
	{
		struct dirent *entry;
		while ((entry = readdir(dir)) != NULL)
		{
			std::string name = entry->d_name;
			if (name != "." && name != "..")
			{
				files.push_back(name);
			}
		}
		closedir(dir);
	}
	return (files);
}

std::string vectoStr(std::vector<std::string> &files, std::string &path)
{
	std::string result = "<!DOCTYPE html>\n<html>\n<head>\n";
	result += "    <title>Index of " + path + "</title>\n";
	result += "    <style>\n";
	result += "        body { font-family: Arial, sans-serif; margin: 20px; }\n";
	result += "        h1 { border-bottom: 1px solid #ccc; padding-bottom: 10px; }\n";
	result += "        table { border-collapse: collapse; width: 100%; }\n";
	result += "        th, td { text-align: left; padding: 8px; }\n";
	result += "        tr:nth-child(even) { background-color: #f2f2f2; }\n";
	result += "        a { text-decoration: none; }\n";
	result += "        a:hover { text-decoration: underline; }\n";
	result += "    </style>\n";
	result += "</head>\n<body>\n";
	result += "    <h1>Index of " + path + "</h1>\n";
	result += "    <table>\n";
	result += "        <tr>\n";
	result += "            <th>Name</th>\n";
	result += "            <th>Size</th>\n";
	result += "        </tr>\n";

	// Parent directory link
	if (path != "/")
	{
		result += "        <tr>\n";
		result += "            <td><a href=\"../\">Parent Directory</a></td>\n";
		result += "            <td>-</td>\n";
		result += "        </tr>\n";
	}
	// Files and directories
	for (std::vector<std::string>::const_iterator it = files.begin(); it != files.end(); ++it)
	{
   	 	const std::string &file = *it;
		result += "        <tr>\n";
		result += "            <td><a href=\"" + file + "\">" + file + "</a></td>\n";
		result += "            <td>-</td>\n";
		result += "        </tr>\n";
	}

	result += "    </table>\n";
	result += "</body>\n</html>";

	return result;
}