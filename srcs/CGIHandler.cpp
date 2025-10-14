/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lfabel <lfabel@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/23 10:46:30 by lfabel            #+#    #+#             */
/*   Updated: 2025/08/27 16:19:41 by lfabel           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/CGIHandler.hpp"

void	CGIHandler::handleCGI(Routing::RequestInfo& reqInfo)
{
	getAll(reqInfo);
    char* args[] = { 
        const_cast<char*>(reqInfo._path_cgi.c_str()),  // "/usr/bin/python3"
        const_cast<char*>(reqInfo._path.c_str()),     // "data/www/cgi-bin/test.py" 
        NULL 
    };	// checkFile();
   std::vector<std::string> env_strings;
    
    // HTTP-spezifische Variablen
    env_strings.push_back("REQUEST_METHOD=" + reqInfo._method);
    env_strings.push_back("QUERY_STRING=" + reqInfo._queryString);
    env_strings.push_back("CONTENT_TYPE=" + reqInfo._cType); 

    std::ostringstream contentLengthStream;
    contentLengthStream << reqInfo._cLength;
    env_strings.push_back("CONTENT_LENGTH=" + contentLengthStream.str());
    
    // Script-spezifische Variablen  
    env_strings.push_back("SCRIPT_NAME=" + reqInfo._path_cgi);
    env_strings.push_back("SCRIPT_FILENAME=" + reqInfo._path);
    env_strings.push_back("PATH_INFO=" + reqInfo._pathInfo);
    env_strings.push_back("PATH_TRANSLATED=" + reqInfo._path);
    
    // Server-spezifische Variablen
    env_strings.push_back("SERVER_NAME=" + reqInfo._serverName);
    env_strings.push_back("SERVER_PORT=" + reqInfo._serverPort);
    env_strings.push_back("SERVER_PROTOCOL=" + reqInfo._protocol);
    env_strings.push_back("SERVER_SOFTWARE=webserve/1.0");
    env_strings.push_back("GATEWAY_INTERFACE=CGI/1.1");
    
    // Client-spezifische Variablen
    env_strings.push_back("REMOTE_ADDR=" + reqInfo._remoteAddr);
    env_strings.push_back("REMOTE_HOST=" + reqInfo._remoteAddr);
    env_strings.push_back("HTTP_USER_AGENT=" + reqInfo._httpUserAgent);
	for (std::vector<std::string>::iterator it = reqInfo._customHeaders.begin(); it < reqInfo._customHeaders.end(); it++)
	{
		env_strings.push_back(*it);
	}
  //  env_strings.push_back("HTTP_HOST=" + reqInfo._serverName + ":" + reqInfo._serverPort);
	std::vector<char*> envs;
	for (size_t i = 0; i < env_strings.size(); i++)
	    envs.push_back(const_cast<char*>(env_strings[i].c_str()));
	envs.push_back(NULL);
	std::string execute;
	//if (!reqInfo._path_cgi.empty() && reqInfo._path_cgi != ".cgi")
		execute = reqInfo._path_cgi;
	//else
	//	execute = reqInfo._path;
	execve(execute.c_str(), args, envs.data());
	perror("execve failed");
	reqInfo._statusCode = 500;
	exit(1);
}

CGIHandler::CGIHandler()
{
	this->_map_env.clear();
	this->_pid = -1;
	this->_method = "";
	this->_path = "";
	this->_statusCode = 200;
}

void CGIHandler::copy(const CGIHandler& other)
{
	this->_map_env = other._map_env;
	this->_pid = other._pid;
	this->_path = other._path;
	this-> _statusCode = other._statusCode;
	this->_method = other._method;
}

CGIHandler& CGIHandler::operator=(const CGIHandler& other)
{
	if (this != &other)
	{
		this->_map_env = other._map_env;
		this->_pid = other._pid;
		this->_path = other._path;
		this-> _statusCode = other._statusCode;
		this->_method = other._method;
	}
	return (*this);
}

CGIHandler::~CGIHandler()
{
}

int CGIHandler::getStatus()
{
	return (this->_statusCode);
}

std::string CGIHandler::getMap(std::string str)
{
	std::map<std::string, std::string>::iterator it = this->_map_env.find(str);
	if (it != this->_map_env.end()) {
		return it->second;
	}
	return "";
}

std::string CGIHandler::get_path()
{
	return (this->_path);
}

void CGIHandler::set_cgi(std::map<std::string, std::string>& map, std::string envs[][2])
{
	// this->_map_env = map;	
	this->_path = map[envs[1][1]];
	this->_method = map[envs[0][1]];
}	

void CGIHandler::setPid(int pid)
{
	this->_pid = pid;
}

void CGIHandler::setStatus(int status)
{
	this->_statusCode = status;
}

void CGIHandler::set_cType(std::string type)
{
	this->_map_env["CONTENT_TYPE="] = type;
}

void CGIHandler::deletefile()
{
	int status = remove(this->_path.c_str());
	if (status != 0)
	{
		//std::cerr << "Error deleting File" << this->_path.c_str() <<std::endl;
	} else{
		std::cout << "204 No Content \r\n" << std::	endl;
	}
}

std::string CGIHandler::getMethod() const 
{
	return (this->_method);
}

std::string CGIHandler::getIndex() const
{
	return (this->_index);
}

void CGIHandler::setPath(std::string path)
{
	this->_path = path;	
}

void CGIHandler::getAll(Routing::RequestInfo& reqInfo)
{
	this->_pid = -1;
	this->_method = reqInfo._method;
	this->_path = reqInfo._path;
	this->_statusCode = reqInfo._statusCode;
	this->_path_cgi = reqInfo._path_cgi;
	this->_extensionCGI = reqInfo._extensionCGI;
	this->_queryString = reqInfo._data;
}

/*	std::vector<std::string> env_strings;
	env_strings.push_back("REQUEST_METHOD=" + reqInfo._method);
	env_strings.push_back("QUERY_STRING=" + reqInfo._queryString);
	env_strings.push_back("REMOTE_ADDR=" + reqInfo._remoteAddr);
	//env_strings.push_back("CGI_Extension=" + reqInfo._extensionCGI)
	env_strings.push_back("HTTP_USER_AGENT=" + reqInfo._httpUserAgent);
	env_strings.push_back("SERVER_PROTOCOL=" + reqInfo._protocol);
	env_strings.push_back("PATH_INFO=" + reqInfo._path_cgi);
	env_strings.push_back("SERVER_NAME=" + reqInfo._serverName);
	env_strings.push_back("SERVER_PORT=" + reqInfo._serverPort);
	env_strings.push_back("GATEWAY_INTERFACE=" + reqInfo._gatewayInterface);
	env_strings.push_back("SCRIPT_NAME=" + reqInfo._path);
	env_strings.push_back("SCRIPT_FILENAME=" + reqInfo._filename);
	env_strings.push_back("CONTENT_TYPE=" + reqInfo._cType);
	env_strings.push_back("CONTENT_LENGTH=" + std::to_string(reqInfo._cLength));//id bout this
	env_strings.push_back("SERVER_SOFTWARE=webserve/1.0");
*/