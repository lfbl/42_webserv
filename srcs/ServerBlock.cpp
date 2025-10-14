/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerBlock.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: idvinov <idvinov@student.42.fr>            #+#  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025-07-10 13:52:12 by idvinov           #+#    #+#             */
/*   Updated: 2025-07-10 13:52:12 by idvinov          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/ServerBlock.hpp"
#include "ServerBlock.hpp"

ServerBlock::ServerBlock()
{
	root = "";
	port = 80;
	ip = "0.0.0.0";
	bodySize = 1000000;
	isAutoindex = true;
	dFiles.push_back("index.html");
	dFiles.push_back("index.htm");
	serverNames.push_back("localhost");
	isDefault = false;
}

ServerBlock::ServerBlock(short unsigned int port,
						 std::string ip, std::string root,
						 unsigned int bodySize,
						 bool isAutoindex,
						 std::vector<std::string> dFile) : port(port), ip(ip), root(root),
														   bodySize(bodySize), isAutoindex(isAutoindex),
														   dFiles(dFile)
{
}

ServerBlock::~ServerBlock()
{
}

bool ServerBlock::operator==(const ServerBlock &other) const
{
	std::cout << "Comparing server blocks " << serverNames.size() << std::endl;
	for (size_t i = 0; i < this->serverNames.size(); i++)
	{
		for (size_t j = 0; j < other.serverNames.size(); j++)
		{
			if (this->serverNames[i] == other.serverNames[j] && this->port == other.port && this->ip == other.ip && this->isDefault == other.isDefault)
				return (true);
		}
	}

	return (false);
}

std::vector<std::string> &ServerBlock::GetServerNames()
{
	return serverNames;
}

void ServerBlock::AddServerName(std::string serverName)
{
	serverNames.push_back(serverName);
}

short unsigned int ServerBlock::GetPort()
{
	return port;
}

bool ServerBlock::IsDefault()
{
	return isDefault;
}

void ServerBlock::SetDefault(bool isDefault)
{
	this->isDefault = isDefault;
}

void ServerBlock::SetPort(short unsigned int port)
{
	this->port = port;
}

const std::string &ServerBlock::GetIP()
{
	return ip;
}

void ServerBlock::SetIP(std::string ip)
{
	this->ip = ip;
}
const std::string &ServerBlock::GetRoot()
{
	return root;
}

void ServerBlock::SetRoot(std::string root)
{
	this->root = root;
}

int ServerBlock::GetBodySize()
{
	return bodySize;
}

void ServerBlock::SetBodySize(unsigned int bodySize)
{
	this->bodySize = bodySize;
}

unsigned int ServerBlock::isAutoindexing()
{
	return isAutoindex;
}

void ServerBlock::Setautoindexing(bool isAutoindex)
{
	this->isAutoindex = isAutoindex;
}

const std::vector<std::string> &ServerBlock::GetDefFile()
{
	return dFiles;
}

void ServerBlock::SetDefFile(std::vector<std::string> defFile)
{
	this->dFiles = defFile;
}

void ServerBlock::AddErrorPage(ErrorPage errorPage)
{
	this->errorPages.push_back(errorPage);
}

void ServerBlock::AddLocation(Location location)
{
	locations.push_back(location);
}

std::vector<ServerBlock::ErrorPage> &ServerBlock::GetErrorPages()
{
	// if (errorPages.empty())
	// {
	// 	return DefaultErrorPages;
	// }
	return this->errorPages;
}

const std::vector<ServerBlock::Location> &ServerBlock::GetLocations()
{
	return locations;
}

const std::vector<ServerBlock::Location>::iterator ServerBlock::GetLocationsItB()
{
	return locations.begin();
}

const std::vector<ServerBlock::Location>::iterator ServerBlock::GetLocationsItE()
{
	return locations.end();
}

ServerBlock::ErrorPage::ErrorPage(unsigned short int error, std::string page) : error(error), page(page) {}
ServerBlock::ErrorPage::ErrorPage() : error(0), page("") {}

void ServerBlock::Location::Initialize()
{
	bodySize = -1;
	path.clear();
	for (int i = 0; i < 3; i++)
		methods[i] = false;
	root.clear();
	redir = ErrorPage(0, "");
	// TODO: Change vecs init
	defFiles.reserve(1);
	autoindex = false;
	cgiEnable = false;
	uploadEnable = false;
	redirect = false;
	exact = false;
	cgiHandlers.reserve(1);
}

ServerBlock::Location::Location()
{
	Initialize();
}

ServerBlock::Location::Location(ServerBlock *serverBlock)
{
	this->serverBlock = serverBlock;
	Initialize();
}
void ServerBlock::Location::AddErrorPage(ErrorPage errorPage)
{
	this->errorPages.push_back(errorPage);
}

std::vector<ServerBlock::ErrorPage> &ServerBlock::Location::GetErrorPages()
{
	// if (errorPages.empty())
	// 	return serverBlock->GetErrorPages();

	return this->errorPages;
}

void ServerBlock::Location::SetConfig(ServerBlock *serverBlock)
{
	this->serverBlock = serverBlock;
}

ServerBlock::CGIType::CGIType() : extension(""), path("") {}

ServerBlock::CGIType::CGIType(std::string extension, std::string path) : extension(extension), path(path) {}
