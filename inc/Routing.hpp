/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Routing.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lfabel <lfabel@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/17 10:35:02 by lfabel            #+#    #+#             */
/*   Updated: 2025/09/25 10:24:39 by lfabel           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ROUTING_HPP
#define ROUTING_HPP

#include <string>
#include <vector>
#include "init.hpp"
#include "../inc/ServerBlock.hpp"
class Routing
{
	public:
		struct RequestInfo
		{
			void Initialize();

			int									_statusCode;
			size_t								_cLength;
			std::string							_method;
			std::string							_path;
			std::string							_pathInfo;
			std::string 						_filename;
			std::string							_queryString;
			std::string							_boundary;
			std::string							_data;
			std::string							_cType;
			std::string							_host;
			std::string							_protocol;

			bool								_autoIndex;
			bool								_useAutoIndex;
			bool								_cgiEnabled;
			bool								_uploadEnable;
			bool								_methodsAllowed[3];
			bool 								_useCGI;
			bool								_upload;
			unsigned int 						_max_body_size;
			std::string							_validPaths;
			std::string 						_path_cgi;
			std::string							_extensionCGI;
			std::string							_uploadPath;
			std::string							_root;
			std::string							_index;
			std::string							_gatewayInterface;
			std::string							_serverPort;
			std::string							_serverName;
			std::string							_remoteAddr;
			std::string							_httpUserAgent;
			std::string							_contentDisposition;
			std::string							_connection;
			std::string							_redirectLocation;
			std::string							_postBody;
			std::vector<std::string>			_customHeaders;
			std::vector<std::string>			_fields;
			std::vector<ServerBlock::ErrorPage>	_errorPages;
			std::map<int, std::string>			_STATUS_CODES;
		};
};

#endif