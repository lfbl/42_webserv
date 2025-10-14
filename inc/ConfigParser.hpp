/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: idvinov <idvinov@student.42.fr>            #+#  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025-08-12 14:55:07 by idvinov           #+#    #+#             */
/*   Updated: 2025-08-12 14:55:07 by idvinov          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGP_HPP
#define CONFIGP_HPP

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <set>
#include <sstream>
#include "../inc/ServerBlock.hpp"

enum Method
{
	GET = 0,
	POST = 1,
	DELETE = 2
};

class ConfigParser
{
public:
private:
	static const int validCodes[];
	static const std::string validCgiExtensions[];
	static const int stringMaxLength = 4096;
	std::vector<ServerBlock> serverBlock;
	std::set<std::string> mainDirectives;
	std::set<std::string> serverDirectives;
	std::set<std::string> locationDirectives;
	ServerBlock FillServerBlock(std::ifstream &file, std::string &text);
	void ListenVar(std::string &text, ServerBlock &serverBlock);
	std::vector<std::string> DefFile(std::string &text);
	unsigned long int BodyVar(std::string &text);
	ServerBlock::ErrorPage ErrorPageVar(std::string &text, std::string keyword);
	void ErrorPageVar(std::string &text, std::vector<ServerBlock::ErrorPage> &errorPages);
	bool FillLocation(std::ifstream &file, std::string &text, ServerBlock &serverBlock);
	void MethodsLocVar(std::string &text, ServerBlock::Location &location);
	bool EnableLocVar(std::string &text, std::string keyword);
	void WriteMultiple(std::string &text, std::vector<std::string> &vec, std::string keyword);
	std::string CheckDirective(std::string &text, std::set<std::string> &directives);
	std::string ExtractVal(std::string &text, const std::string &keyword, bool oneVar, bool semiCol, bool delFPath);
    void ExtractServerName(std::string &text, const std::string &keyword, std::vector<std::string> &serverNames);
    bool CheckIP(const std::string &ip);
	bool CheckPort(const int port);
	bool CheckDirectory(std::string keyword, const std::string &dir);
	bool CheckFile(std::string keyword, const std::string &path);

	bool IsValidHttpCode(int code);
	void PrintServerBlock(ServerBlock &serverBlock);
	void PrintLocationBlock(ServerBlock::Location &location);

public:
	ConfigParser();
	~ConfigParser();
	std::vector<ServerBlock> Parse(std::string configName);
	void CheckVariables(ServerBlock &serverBlock);

	class ConfigErrorException : public std::exception
	{
	private:
		std::string message;

	public:
		ConfigErrorException();
		ConfigErrorException(std::string message);
		virtual ~ConfigErrorException() throw();
		virtual const char *what() const throw();
	};
};

#endif
