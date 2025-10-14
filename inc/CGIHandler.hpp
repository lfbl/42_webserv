/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lfabel <lfabel@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/23 10:42:19 by lfabel            #+#    #+#             */
/*   Updated: 2025/08/27 16:20:40 by lfabel           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGI_HANDLER_HPP
#define CGI_HANDLER_HPP

#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <map>
#include <sstream>
#include "../inc/HttpRequest.hpp"
#include "../inc/ServerBlock.hpp"
#include "../inc/Routing.hpp"
#include <vector>

class Routing;

class CGIHandler
{
	private:
		std::map<std::string, std::string>	_map_env;
		int   								_pid;
		std::string 						_path_cgi;
		std::string							_method;
		int									_statusCode;

		std::string							_path;
		std::string							_extensionCGI;
		std::string							_uploadPath;
		std::string							_root;
		std::string							_index;
		std::string							_queryString;
		std::string							_script_name;
		ServerBlock::Location					_location;
		void	checkFile();
	public:
		CGIHandler();
		void copy(const CGIHandler& other);
		CGIHandler& operator=(const CGIHandler& other); 
		~CGIHandler();
		void	handleCGI(Routing::RequestInfo& reqInfo);
		void set_cgi(std::map<std::string, std::string>& map, std::string envs[][2]);
		int getStatus();
		std::string getMap(std::string str);
		std::string get_path();
		bool getCGIEnabled() const;
		bool getAllowedMethods(int i ) const;
		std::string getMethod() const;
		bool getAutoIndex() const;
		std::string getIndex() const;
		void setPath(std::string path);
		void setStatus(int status);
		void setPid(int pid);
		void setCGIEnabled(bool state);
		void set_cType(std::string type);
		void deletefile();
		void getAll(Routing::RequestInfo& reqInfo);
		std::vector<char *> argsToEnv(Routing::RequestInfo& reqInfo);
};

#endif