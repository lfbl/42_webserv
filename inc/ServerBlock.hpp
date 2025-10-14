/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerBlock.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: idvinov <idvinov@student.42.fr>            #+#  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025-08-12 15:58:46 by idvinov           #+#    #+#             */
/*   Updated: 2025-08-12 15:58:46 by idvinov          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERBLOCK_HPP
#define SERVERBLOCK_HPP

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

class ServerBlock
{
public:
	struct ErrorPage
	{
		unsigned short int error;
		std::string page;
		ErrorPage();
		ErrorPage(unsigned short int error, std::string page);
	};
	struct CGIType
	{
		std::string extension;
		std::string path;
		CGIType();
		CGIType(std::string extension, std::string path);
	};
	struct Location
	{
	private:
		std::vector<ErrorPage> errorPages;
		ServerBlock *serverBlock;
	public:
		std::string path;
		std::string root;
		ErrorPage redir;
		std::vector<std::string> defFiles;
		std::string uploadPath;
		std::string cgiExtension;
		std::vector<CGIType> cgiHandlers;

		long int bodySize;
		bool autoindex;
		bool methods[3];
		bool cgiEnable;
		bool uploadEnable;
		bool redirect;
		bool exact;
		Location();
		Location(ServerBlock *serverBlock);
		void Initialize();
		void AddErrorPage(ErrorPage errorPage);
		std::vector<ErrorPage> &GetErrorPages();
		void SetConfig(ServerBlock *serverBlock);
	};

private:
	std::vector<std::string> serverNames;
	short unsigned int port;
	std::string ip;
	std::string root;
	unsigned long int bodySize;
	bool isAutoindex;
	bool isDefault;
	std::vector<std::string> dFiles;
	std::vector<ErrorPage> errorPages;
	std::vector<Location> locations;
	std::vector<std::pair<std::string, std::string> > cgiHandlers;

public:
	ServerBlock();
	ServerBlock(short unsigned int port,
				std::string ip, std::string root,
				unsigned int bodySize,
				bool isAutoindex,
				std::vector<std::string> dFile);
	~ServerBlock();
	void SetPort(short unsigned int port);
	void SetIP(std::string ip);
	void SetRoot(std::string root);
	void SetBodySize(unsigned int bodySize);
	void Setautoindexing(bool isAutoindex);
	void SetDefFile(std::vector<std::string> defFile);
	void SetDefault(bool isDefault);
	void AddErrorPage(ErrorPage errorPage);
	void AddLocation(Location location);
	void AddServerName(std::string serverName);
	std::vector<std::string> &GetServerNames();
	short unsigned int GetPort();
	const std::string &GetIP();
	const std::string &GetRoot();
	bool IsDefault();
	int GetBodySize();
	unsigned int isAutoindexing();
	const std::vector<std::string> &GetDefFile();
	std::vector<ErrorPage> &GetErrorPages();
	const std::vector<Location> &GetLocations();
	const std::vector<Location>::iterator GetLocationsItB();
	const std::vector<Location>::iterator GetLocationsItE();
	bool operator==(const ServerBlock &other) const;
};
// static std::vector<ServerBlock::ErrorPage> DefaultErrorPages = {
	// ServerBlock::ErrorPage(400, "static/errors/400.html"),
	// ServerBlock::ErrorPage(401, "static/errors/401.html"),
	// ServerBlock::ErrorPage(403, "static/errors/403.html"),
	// ServerBlock::ErrorPage(404, "static/errors/404.html"),
	// ServerBlock::ErrorPage(405, "static/errors/405.html"),
	// ServerBlock::ErrorPage(408, "static/errors/408.html"),
	// ServerBlock::ErrorPage(413, "static/errors/413.html"),
	// ServerBlock::ErrorPage(414, "static/errors/414.html"),
	// ServerBlock::ErrorPage(500, "static/errors/500.html"),
	// ServerBlock::ErrorPage(501, "static/errors/501.html"),
	// ServerBlock::ErrorPage(502, "static/errors/502.html"),
	// ServerBlock::ErrorPage(503, "static/errors/503.html"),
	// ServerBlock::ErrorPage(504, "static/errors/504.html"),
	// ServerBlock::ErrorPage(505, "static/errors/505.html")
// };

#endif