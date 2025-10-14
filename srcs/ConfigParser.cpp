/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lfabel <lfabel@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/12 14:59:35 by idvinov           #+#    #+#             */
/*   Updated: 2025/08/27 16:25:25 by lfabel           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/ConfigParser.hpp"
#include "ConfigParser.hpp"

ConfigParser::ConfigParser()
{
	mainDirectives.insert("server");

	serverDirectives.insert("listen");
	serverDirectives.insert("server_name");
	serverDirectives.insert("root");
	serverDirectives.insert("index");
	serverDirectives.insert("error_page");
	serverDirectives.insert("location");
	serverDirectives.insert("default");
	serverDirectives.insert("body_size");

	locationDirectives.insert("methods");
	locationDirectives.insert("autoindex");
	locationDirectives.insert("upload");
	locationDirectives.insert("index");
	locationDirectives.insert("root");
	locationDirectives.insert("return");
	locationDirectives.insert("cgi_enable");
	locationDirectives.insert("cgi_path");
	locationDirectives.insert("cgi_extension");
	locationDirectives.insert("upload_enable");
	locationDirectives.insert("upload_path");
	locationDirectives.insert("cgi_handler");
	locationDirectives.insert("error_page");
	locationDirectives.insert("body_size");
}

const int ConfigParser::validCodes[] = {
	// 1xx
	100, 101, 102, 103,
	// 2xx
	200, 201, 202, 203, 204, 205, 206, 207, 208, 226,
	// 3xx
	300, 301, 302, 303, 304, 305, 307, 308,
	// 4xx
	400, 401, 402, 403, 404, 405, 406, 407, 408, 409,
	410, 411, 412, 413, 414, 415, 416, 417, 418,
	421, 422, 423, 424, 425, 426, 428, 429, 431, 451,
	// 5xx
	500, 501, 502, 503, 504, 505, 506, 507, 508,
	510, 511};

const std::string ConfigParser::validCgiExtensions[] = {
	".php",
	".py",
	".pl",
	".rb",
	".js",
	".jsp",
	".sh",
	".cgi",
	".go",
	".lua"};

ConfigParser::~ConfigParser()
{
}

// std::vector<ServerBlock> ConfigParser::Parse(std::string ConfigParserName)
// {

// 	std::string text;
// 	std::ifstream config((ConfigParserName).c_str());
// 	CheckFile("file not found", ConfigParserName);
// 	if (!config.is_open())
// 		throw ConfigErrorException("Error: Could not open ConfigParser file");
// 	std::vector<ServerBlock> serverBlocks;
// 	while (getline(config, text))
// 	{
// 		if (text.find("#") != std::string::npos)
// 		{
// 			continue;
// 		}
// 		if (text.find("server") != std::string::npos)
// 		{
// 			std::cout << std::endl
// 					  << "Server block: " << std::endl;
// 			serverBlocks.push_back(FillServerBlock(config, text));
// 		}
// 	}
// 	if (serverBlocks.empty())
// 		throw ConfigErrorException("Error: No valid server blocks found in the configuration file");
// 	for (std::vector<ServerBlock>::iterator it = serverBlocks.begin(); it != serverBlocks.end(); ++it)
// 	{
// 		CheckVariables(*it);
// 	}
// 	config.close();
// 	return serverBlocks;
// }

std::vector<ServerBlock> ConfigParser::Parse(std::string ConfigParserName)
{

	std::string text;
	std::ifstream config((ConfigParserName).c_str());
	CheckFile("config", ConfigParserName);
	if (!config.is_open())
		throw ConfigErrorException("Error: Could not open ConfigParser file");
	if (config.peek() == std::ifstream::traits_type::eof())
		throw ConfigErrorException("Error: Configuration file is empty");
	std::vector<ServerBlock> serverBlocks;
	while (getline(config, text))
	{
		text.erase(0, text.find_first_not_of(" \t"));
		if (text.empty() || text.find("#") != std::string::npos)
			continue;

		std::string directive = CheckDirective(text, mainDirectives);
		if (directive == "server")
		{
			serverBlocks.push_back(FillServerBlock(config, text));
			for (size_t i = 0; i < serverBlocks.size() - 1; i++)
			{
				if (serverBlocks[i] == serverBlocks.back())
					throw ConfigErrorException("Error: Duplicate server block with same server_name, ip, port and default_server");
			}
		}
	}
	if (serverBlocks.empty())
		throw ConfigErrorException("Error: No valid server blocks found in the configuration file");
	for (std::vector<ServerBlock>::iterator it = serverBlocks.begin(); it != serverBlocks.end(); ++it)
	{
		CheckVariables(*it);
	}
	config.close();
	return serverBlocks;
}

ServerBlock ConfigParser::FillServerBlock(std::ifstream &file, std::string &text)
{
	unsigned short int scope = 0;
	bool eob = false;
	ServerBlock serverBlock;
	bool blockStarted = false;
	if (text.find("{") != std::string::npos)
	{
		scope++;
		blockStarted = true;
	}
	while (!blockStarted && getline(file, text))
	{
		text.erase(0, text.find_first_not_of(" \t"));

		if (text.empty() || text.find("#") != std::string::npos)
			continue;
		if (text.find_first_not_of(" \t{") != std::string::npos)
		{
			throw ConfigErrorException("Error: Missing '{' for server block");
		}
		if (text.find("{") != std::string::npos)
		{
			scope++;
			blockStarted = true;
			break;
		}
	}
	if (!blockStarted)
		throw ConfigErrorException("Error: Missing '{' for server block");

	while (getline(file, text) && !eob)
	{
		text.erase(0, text.find_first_not_of(" \t"));
		size_t comment = text.find("#");
		if (comment != std::string::npos)
		{
			text.erase(comment, text.length());
		}
		if (text.empty() || text.find("#") != std::string::npos)
			continue;

		std::string directive = CheckDirective(text, serverDirectives);
		if (directive == "listen")
		{
			ListenVar(text, serverBlock);
		}
		else if (directive == "server_name")
		{
			ExtractServerName(text, "server_name", serverBlock.GetServerNames());
		}
		else if (directive == "index")
		{
			serverBlock.SetDefFile(DefFile(text));
		}
		else if (directive == "root")
		{
			serverBlock.SetRoot(ExtractVal(text, "root", true, true, false));
		}
		else if (directive == "body_size")
		{
			serverBlock.SetBodySize(BodyVar(text));
		}
		else if (directive == "error_page")
		{
			ErrorPageVar(text, serverBlock.GetErrorPages());
		}

		else if (directive == "location")
		{
			FillLocation(file, text, serverBlock);
		}
		else if (directive == "default")
		{
			if (text.find_first_not_of(" \tdefault;") != std::string::npos)
				throw ConfigErrorException("Error {default}: Too many arguments");

			serverBlock.SetDefault(true);
		}
		if (text.find("}") != std::string::npos)
		{
			scope--;
			if (scope == 0)
			{
				text.clear();
				break;
			}
		}
	}
	if (scope > 0)
		throw ConfigErrorException("Error: Wrong file structure (block)");
	PrintServerBlock(serverBlock);
	std::cout << "Exiting server block" << std::endl
			  << std::endl;
	return serverBlock;
}

bool ConfigParser::FillLocation(std::ifstream &file, std::string &text, ServerBlock &serverBlock)
{
	unsigned short int scope = 0;
	bool blockStarted = false;
	ServerBlock::Location location(&serverBlock);
	size_t pos = text.find("=");
	if (pos != std::string::npos)
	{
		std::cout << "	Location is exact" << std::endl;
		text.erase(pos, 1);
		location.exact = true;
	}
	std::string val = ExtractVal(text, "location", true, false, true);

	val.insert(0, "/");
	location.path = val.substr(0, val.find_first_of(" \t{"));
	if (text.find("{") != std::string::npos)
	{
		scope++;
		blockStarted = true;
	}
	while (!blockStarted && getline(file, text))
	{
		text.erase(0, text.find_first_not_of(" \t"));
		if (text.empty() || text.find("#") == 0)
			continue;
		if (text.find_first_not_of(" \t{") != std::string::npos)
		{
			throw ConfigErrorException("Error: Missing '{' for location  block");
		}
		if (text.find("{") != std::string::npos)
		{
			scope++;
			blockStarted = true;
			break;
		}
	}

	if (!blockStarted)
		throw ConfigErrorException("Error: Missing '{' for location block");

	std::cout << "SCOPE:" << scope << std::endl;
	while (getline(file, text))
	{
		text.erase(0, text.find_first_not_of(" \t"));
		size_t comment = text.find("#");
		if (comment != std::string::npos)
		{
			text.erase(comment, text.length());
		}
		if (text.empty() || text.find("#") != std::string::npos)
			continue;

		std::string directive = CheckDirective(text, locationDirectives);
		if (directive == "methods")
		{
			MethodsLocVar(text, location);
		}
		else if (directive == "root")
		{
			location.root = ExtractVal(text, "root", true, true, false);
			if (location.root.find_last_of("/") != serverBlock.GetRoot().length() - 1)
				location.root += "/";
		}
		else if (directive == "index")
		{
			location.defFiles = DefFile(text);
		}
		else if (directive == "return")
		{
			location.redir = ErrorPageVar(text, "return");
		}
		else if (directive == "autoindex")
		{
			location.autoindex = EnableLocVar(text, "autoindex");
		}
		else if (directive == "cgi_handler")
		{
			std::vector<std::string> vals;
			WriteMultiple(text, vals, "cgi_handler");
			if (vals.size() != 2)
				throw ConfigErrorException("Error {cgi_handler}: Wrong number of arguments");

			location.cgiHandlers.push_back(ServerBlock::CGIType(vals[0], vals[1]));
			location.cgiEnable = true;
		}
		else if (directive == "upload_enable")
		{
			location.uploadEnable = EnableLocVar(text, "upload_enable");
		}
		else if (directive == "upload_path")
		{
			location.uploadPath = ExtractVal(text, "upload_path", true, true, false);
		}
		else if (directive == "error_page")
		{
			ErrorPageVar(text, location.GetErrorPages());
		}
		else if (directive == "body_size")
		{
			location.bodySize = BodyVar(text);
		}
		else if (directive.find("}") != std::string::npos)
		{
			--scope;
			if (scope == 0)
			{
				text.clear();
				break;
			}
		}
	}
	if (scope > 0)
		throw ConfigErrorException("Error: Wrong file structure (block)");
	for (size_t i = 0; i < serverBlock.GetLocations().size(); i++)
	{
		if (serverBlock.GetLocations()[i].path == location.path)
			throw ConfigErrorException("Error {location}: Duplicate location block " + location.path);
	}
	PrintLocationBlock(location);
	serverBlock.AddLocation(location);
	std::cout << "	Exiting location block" << text << std::endl
			  << std::endl;
	return true;
}

void ConfigParser::CheckVariables(ServerBlock &serverBlock)
{
	std::cout << std::endl;
	if (!CheckIP(serverBlock.GetIP()))
		throw ConfigErrorException("Error {listen}: Wrong ip values");
	if (!CheckPort(serverBlock.GetPort()))
		throw ConfigErrorException("Error {listen}: Wrong port values");

	CheckDirectory("root", serverBlock.GetRoot());

	int s = serverBlock.GetErrorPages().size();
	for (int i = 0; i < s; i++)
	{
		ServerBlock::ErrorPage &errorPage = serverBlock.GetErrorPages()[i];
		if (errorPage.error < 100 || errorPage.error > 599 || !IsValidHttpCode(errorPage.error))
		{
			std::stringstream ss;
			ss << errorPage.error;
			throw ConfigErrorException("Error {error_page}: Wrong http code " + ss.str());
		}

		if (errorPage.page[0] == '/')
			errorPage.page = serverBlock.GetRoot() + errorPage.page;

		CheckFile("error_page", errorPage.page);
	}
	for (std::vector<ServerBlock::Location>::iterator it = serverBlock.GetLocationsItB(); it != serverBlock.GetLocationsItE(); ++it)
	{
		it->redirect = false;
		int counter = 1;
		if (it->root.empty())
		{
			it->root = serverBlock.GetRoot();
			--counter;
		}
		if (!it->redir.page.empty())
		{
			it->redirect = true;
			std::cout << "Location: " << it->path << " Redirection set to: " << it->redir.error << " " << it->redir.page << std::endl;
		}

		if (!it->uploadPath.empty())
		{
			CheckDirectory("upload path", it->uploadPath);
			++counter;
		}
		if (it->autoindex)
		{
			++counter;
		}
		if (it->cgiEnable)
		{
			for (std::vector<ServerBlock::CGIType>::iterator it2 = it->cgiHandlers.begin(); it2 != it->cgiHandlers.end(); ++it2)
			{
				if (it2->extension.find(".") != 0)
					throw ConfigErrorException("Error {cgi_handler}: No extension type");
				const std::string *end = validCgiExtensions + sizeof(validCgiExtensions) / sizeof(validCgiExtensions[0]);
				if (std::find(validCgiExtensions, end, it2->extension) == end)
					throw ConfigErrorException("Error {cgi_handler}: Invalid extension type: " + it2->extension);

				CheckFile("CGI path", it2->path);
			}
			++counter;
		}

		if (!it->GetErrorPages().empty())
		{
			s = it->GetErrorPages().size();
			for (int i = 0; i < s; i++)
			{
				ServerBlock::ErrorPage &errorPage = it->GetErrorPages()[i];
				if (errorPage.error < 100 || errorPage.error > 599 || !IsValidHttpCode(errorPage.error))
				{
					std::stringstream ss;
					ss << errorPage.error;
					throw ConfigErrorException("Error {error_page}: Wrong http code " + ss.str());
				}	

				std::string path;
				if (errorPage.page[0] == '/')
					errorPage.page.erase(0, 1);

				if (it->root.empty())
					path = serverBlock.GetRoot() + errorPage.page;
				else
					path = it->root + errorPage.page;

				CheckFile("error_page", path);
				errorPage.page = path;
			}
			++counter;
		}
		else
		{
			it->GetErrorPages() = serverBlock.GetErrorPages();
		}

		for (std::vector<std::string>::iterator it1 = it->defFiles.begin(); it1 != it->defFiles.end(); ++it1)
		{
			std::string path;
			if (it->root[it->root.length() - 1] == '/')
			{
				path = it->root + *it1;
			}
			else
				path = it->root + "/" + *it1;
			CheckFile("location default files", path);
			++counter;
		}

		if (counter > 0 && it->redirect)
			throw ConfigErrorException("Error {location}: 'return', 'upload_path' 'autoindex' and 'root' are mutually exclusive");

		CheckDirectory("location path", it->root);
	}
}

void ConfigParser::ListenVar(std::string &text, ServerBlock &serverBlock)
{
	std::string ips = ExtractVal(text, "listen", true, true, false);

	int i = -1;
	while (ips[++i])
	{
		if (!isdigit(ips[i]) && ips[i] != '.' && ips[i] != ':')
			throw ConfigErrorException("Error {listen}: invalid numbers");
	}
	std::string::size_type colonPos = ips.find(':');
	if (colonPos == std::string::npos)
	{
		if (ips.find('.') != std::string::npos)
			serverBlock.SetIP(ips);
		else
		{
			int a = static_cast<int>(std::atoi(ips.c_str()));
			if (a < 0 || a > 65535)
				throw ConfigErrorException("Error {listen}: Wrong port value");
			serverBlock.SetPort(a);
		}
	}
	else
	{
		std::cout << ips << std::endl;
		if (!ips[colonPos + 1])
			throw ConfigErrorException("Error {listen}: Wrong no value after ':'");

		std::string portS = ips.substr(colonPos + 1, ips.length());
		serverBlock.SetIP(ips.substr(0, colonPos));
		serverBlock.SetPort(static_cast<int>(std::atoi(portS.c_str())));
	}
}

std::vector<std::string> ConfigParser::DefFile(std::string &text)
{
	std::string val = ExtractVal(text, "index", true, true, true);
	// int i = -1;
	std::vector<std::string> defFiles;
	while (!val.empty())
	{
		size_t s = val.find_first_of(" \t");
		defFiles.push_back(val.substr(0, s));
		val.erase(0, s);
		s = val.find_first_not_of(" \t");
		val.erase(0, s);
	}
	return defFiles;
}

void ConfigParser::WriteMultiple(std::string &text, std::vector<std::string> &vec, std::string keyword)
{
	std::string val = ExtractVal(text, keyword, false, true, false);
	vec.clear();
	while (!val.empty())
	{
		size_t s = val.find_first_of(" \t");
		vec.push_back(val.substr(0, s));
		val.erase(0, s);
		s = val.find_first_not_of(" \t");
		val.erase(0, s);
	}
}

unsigned long int ConfigParser::BodyVar(std::string &text)
{
	std::string value, nums;
	int i = -1;
	value = ExtractVal(text, "body_size", true, true, false);
	while (value[++i] && isdigit(value[i]))
		nums += value[i];
	value.erase(0, i);
	unsigned long int bodySize;
	if (nums.empty())
		bodySize = 1000000;
	else
	{
		if (value.find_first_not_of("KMGkmg") != std::string::npos)
			throw ConfigErrorException("Error {body_size}: Wrong symbols");
		if (value.find_first_of("Kk") != std::string::npos)
			bodySize = atoi(nums.c_str()) * 1000;
		else if (value.find_first_of("Mm") != std::string::npos)
			bodySize = atoi(nums.c_str()) * 1000000;
		else if (value.find_first_of("Gg") != std::string::npos)
			bodySize = atoi(nums.c_str()) * 1000000000;
		else
			bodySize = atoi(nums.c_str());
	}
	return bodySize;
}

ServerBlock::ErrorPage ConfigParser::ErrorPageVar(std::string &text, std::string keyword)
{
	std::string val = ExtractVal(text, keyword, false, true, true);
	size_t s = val.find_first_of(" \t");

	std::string numS = val.substr(0, s).c_str();
	for (size_t i = 0; i < numS.length(); i++)
	{
		if (!isdigit(numS[i]))
			throw ConfigErrorException("Error {" + keyword + "}: Wrong status code " + val);
	}
	unsigned short int num = atoi(numS.c_str());
	val.erase(0, s);
	s = val.find_first_not_of(" \t");
	val.erase(0, s);
	s = val.find_first_of(" \t");
	std::string page = val.substr(0, s);
	val.erase(0, s);
	ServerBlock::ErrorPage errorPage(num, page);
	if (val.find_first_not_of(" \t;") != std::string::npos)
		throw ConfigErrorException("Error {" + keyword + ": '" + val + "' }: Too many arguments");

	return errorPage;
}

void ConfigParser::ErrorPageVar(std::string &text, std::vector<ServerBlock::ErrorPage> &errorPages)
{
	std::string line = ExtractVal(text, "error_page", false, true, false);
	std::istringstream iss(line);
	std::vector<int> errorCodes;
	std::string token;
	std::string filePath;
	bool isErrorCode = true;
	bool foundFilePath = false;

	while (iss >> token)
	{
		for (size_t i = 0; i < token.size(); i++)
		{
			if (!isdigit(token[i]))
			{
				isErrorCode = false;
				break;
			}
		}

		if (isErrorCode)
		{
			errorCodes.push_back(std::atoi(token.c_str()));
		}
		else
		{
			if (foundFilePath)
				throw ConfigErrorException("Error {error_page}: Multiple file paths specified: " + filePath + " and " + token);

			if (token.find("/") == std::string::npos && token.find(".") == std::string::npos)
				throw ConfigErrorException("Error {error_page}: Invalid token (not a valid error code or file path): " + token);

			filePath = token;
			foundFilePath = true;
			break;
		}
	}

	if (errorCodes.empty())
		throw ConfigErrorException("Error {error_page}: No valid error codes found");
	if (filePath.empty())
		throw ConfigErrorException("Error {error_page}: No file path specified");

	for (size_t i = 0; i < errorCodes.size(); i++)
		errorPages.push_back(ServerBlock::ErrorPage(errorCodes[i], filePath));
}

void ConfigParser::MethodsLocVar(std::string &text, ServerBlock::Location &location)
{
	std::string val, method;
	val = ExtractVal(text, "methods", false, true, false);
	int i = -1;
	location.methods[0] = false;
	location.methods[1] = false;
	location.methods[2] = false;
	while (!val.empty() && i < 3)
	{
		size_t s = val.find_first_of(" \t");
		method = val.substr(0, s);
		if (method == "GET")
			location.methods[GET] = true;
		else if (method == "POST")
			location.methods[POST] = true;
		else if (method == "DELETE")
			location.methods[DELETE] = true;
		else
			throw ConfigErrorException("Error {methods}: Unknown method '" + method + "'");
		val.erase(0, s);
		s = val.find_first_not_of(" \t");
		val.erase(0, s);
	}
}
bool ConfigParser::EnableLocVar(std::string &text, std::string keyword)
{
	std::string val = ExtractVal(text, keyword, true, true, false);
	if (val.find("on") == 0)
		return true;
	return false;
}

std::string ConfigParser::CheckDirective(std::string &text, std::set<std::string> &directives)
{
	size_t firstSpace = text.find_first_of(" \t;");
	std::string directive = text.substr(0, firstSpace);
	if (directive == "}")
	{
		return directive;
	}
	// std::cout << text << std::endl;
	if (directives.find(directive) == directives.end())
		throw ConfigErrorException("Error: expected directive value, found: " + directive);

	return directive;
}

std::string ConfigParser::ExtractVal(std::string &text, const std::string &keyword, bool oneVar, bool semiCol, bool delFPath)
{
	std::string value;

	std::string::size_type begin = text.find_first_not_of(" \t");
	text.erase(0, begin);
	std::string::size_type afterKeyword = text.find_first_not_of(" \t;", keyword.size());
	if (afterKeyword == std::string::npos)
	{
		throw ConfigErrorException("Error {" + keyword + "}: Syntax error (missing values)");
	}
	std::string::size_type semicolonPos;
	semicolonPos = text.find(";", afterKeyword);
	// if (semiCol && text.find_first_not_of(" \t", semicolonPos) != std::string::npos)
	// 	throw ConfigErrorException("Error {" + keyword + "}: Values after semicolon");

	if (semiCol)
	{
		if (semicolonPos == std::string::npos)
			throw ConfigErrorException("Error {" + keyword + "}: Missing semicolon");

		std::string::size_type afterSemicolon = text.find_first_not_of(" \t", semicolonPos + 1);
		if (afterSemicolon != std::string::npos)
		{
			throw ConfigErrorException("Error {" + keyword + "}: syntax error");
		}
	}

	value = text.substr(afterKeyword, semicolonPos - afterKeyword);
	if (oneVar)
	{
		bool wasSpace = false;
		for (size_t i = 0; i < value.length(); i++)
		{
			if (value[i] == ' ' || value[i] == '\t')
				wasSpace = true;
			else if (wasSpace && (isalnum(value[i])))
			{
				value.clear();
				throw ConfigErrorException("Error {" + keyword + "}: Too many arguments " + value);
			}
		}
		value.erase(std::remove_if(value.begin(), value.end(), isspace), value.end());
	}

	if (delFPath && value[0] == '/')
	{
		value.erase(0, 1);
	}
	for (size_t i = 0; i < value.size(); i++)
	{
		if (!isascii(value[i]))
		{
			throw ConfigErrorException("Error {" + keyword + "}: Invalid character in path (Non-ASCII value)");
		}
	}
	if (value.find('\0') != std::string::npos || value.find("\\0") != std::string::npos)
	{
		throw ConfigErrorException("Error {" + keyword + "}: Null byte detected");
	}
	if (value.length() > stringMaxLength)
	{
		std::stringstream ss;
		ss << stringMaxLength;
		throw ConfigErrorException("Error {" + keyword + "}: Value too long (max " + ss.str() + " characters)");
	}
	if (size_t firstQuote = value.find_first_of("\"") != std::string::npos || value.find("'") != std::string::npos)
	{
		if (value.find("'") != std::string::npos)
			throw ConfigErrorException("Error {" + keyword + "}: Single quotes are not allowed in path");

		size_t secondQuote = value.find_first_of("\"", firstQuote + 1);
		if (secondQuote == std::string::npos)
			throw ConfigErrorException("Error {" + keyword + "}: Mismatched quotes in path");
		else
			value = value.substr(firstQuote, secondQuote - firstQuote);
	}
	return value;
}

void ConfigParser::ExtractServerName(std::string &text, const std::string &keyword, std::vector<std::string> &serverNames)
{
	std::string value;

	std::string::size_type begin = text.find_first_not_of(" \t");
	text.erase(0, begin);
	std::string::size_type afterKeyword = text.find_first_not_of(" \t;", keyword.size());
	if (afterKeyword == std::string::npos)
	{
		throw ConfigErrorException("Error {" + keyword + "}: Syntax error (missing values)");
	}
	std::string::size_type semicolonPos;
	semicolonPos = text.find(";", afterKeyword);
	// if (semiCol && text.find_first_not_of(" \t", semicolonPos) != std::string::npos)
	// 	throw ConfigErrorException("Error {" + keyword + "}: Values after semicolon");

	if (semicolonPos == std::string::npos)
		throw ConfigErrorException("Error {" + keyword + "}: Missing semicolon");

	std::string::size_type afterSemicolon = text.find_first_not_of(" \t", semicolonPos + 1);
	if (afterSemicolon != std::string::npos)
	{
		throw ConfigErrorException("Error {" + keyword + "}: syntax error");
	}

	value = text.substr(afterKeyword, semicolonPos - afterKeyword);
	while (!value.empty())
	{
		size_t s = value.find_first_of(" \t");
		std::string serverName = value.substr(0, s);
		if (serverName.empty())
			throw ConfigErrorException("Error {" + keyword + "}: Empty server name");
		if (serverName.length() > stringMaxLength)
		{
			std::stringstream ss;
			ss << stringMaxLength;
			throw ConfigErrorException("Error {" + keyword + "}: Server name too long (max " + ss.str() + " characters)");
		}
		if (serverName.find("\"") != std::string::npos || serverName.find("'") != std::string::npos)
			throw ConfigErrorException("Error {" + keyword + "}: Quotes are not allowed in server name: " + serverName);
		// if (serverName.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-.") != std::string::npos)
		// 	throw ConfigErrorException("Error {" + keyword + "}: Invalid character in server name: " + serverName);
		if (serverName[0] == '-' || serverName[serverName.length() - 1] == '-')
			throw ConfigErrorException("Error {" + keyword + "}: Server name cannot start or end with a hyphen: " + serverName);
		if (serverName.find("..") != std::string::npos)
			throw ConfigErrorException("Error {" + keyword + "}: Server name cannot contain consecutive dots: " + serverName);

		size_t dotPos = serverName.find('.');
		if (dotPos != std::string::npos)
		{
			std::string tld = serverName.substr(dotPos + 1);
			if (tld.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ.-") != std::string::npos)
				throw ConfigErrorException("Error {" + keyword + "}: Invalid top-level domain in server name: " + tld);
			if (tld.length() < 2 || tld.length() > 63)
				throw ConfigErrorException("Error {" + keyword + "}: Top-level domain length must be between 2 and 63 characters: " + tld);
		}

		serverNames.push_back(serverName);
		std::cout << keyword << ": " << serverNames.back() << std::endl;
		value.erase(0, s);
		s = value.find_first_not_of(" \t");
		value.erase(0, s);
	}
}

bool ConfigParser::CheckIP(const std::string &ip)
{
	size_t i = ip.length();
	int j = 0;
	int d = 0;
	int num = 0;
	for (i = 0; i < ip.length(); ++i)
	{
		if (isdigit(ip[i]))
		{
			num = num * 10 + (ip[i] - '0');
			++j;
		}
		else if (ip[i] == '.')
		{
			if (d < 3 && !ip[i + 1])
				return false;
			if (num < 0 || num > 255)
				return false;
			++d;
			num = 0;
		}
	}
	if (j > 12 || j < 3)
		return false;
	else if (d != 3)
		return false;
	return true;
}

bool ConfigParser::CheckPort(int port)
{
	if (port < 0 || port > 65545)
		return false;
	return true;
}

bool ConfigParser::CheckDirectory(std::string keyword, const std::string &path)
{
	struct stat info;

	if (path.empty())
	{
		throw ConfigErrorException("Error {" + keyword + "}: No value");
	}
	if (stat(path.c_str(), &info) != 0)
	{
		throw ConfigErrorException("Error {" + keyword + "}: Cannot access path: " + path);
		return false;
	}
	if (info.st_mode & S_IFDIR)
	{
		std::cout << keyword << ": " << path << " is a directory." << std::endl;
		return true;
	}
	else
	{
		throw ConfigErrorException("Error {" + keyword + "}: " + path + " exists but is not a directory.");
		return false;
	}
	return false;
}

bool ConfigParser::CheckFile(std::string keyword, const std::string &path)
{
	struct stat info;

	if (stat(path.c_str(), &info) != 0)
	{
		throw ConfigErrorException("Error {" + keyword + "}: Cannot access path: " + path);
	}
	if (access(path.c_str(), R_OK) != 0)
	{
		throw ConfigErrorException("Error {" + keyword + "}: " + path + " is not readable.");
	}
	if (info.st_mode & S_IFREG)
	{
		std::cout << keyword << ": " << path << " is a file." << std::endl;
		return true;
	}

	else
	{
		throw ConfigErrorException("Error {" + keyword + "}: " + path + " exists but is not a regular file.");
	}

	return false;
}

bool ConfigParser::IsValidHttpCode(int code)
{
	for (size_t i = 0; i < sizeof(validCodes) / sizeof(validCodes[0]); ++i)
	{
		if (validCodes[i] == code)
			return true;
	}
	return false;
}

ConfigParser::ConfigErrorException::ConfigErrorException()
{
	message = "Error occured in ConfigParser";
}

ConfigParser::ConfigErrorException::ConfigErrorException(std::string message) : message(message)
{
}

ConfigParser::ConfigErrorException::~ConfigErrorException() throw()
{
}

const char *ConfigParser::ConfigErrorException::what() const throw()
{
	return message.c_str();
}

void ConfigParser::PrintServerBlock(ServerBlock &serverBlock)
{
	std::cout << "Server Block:" << std::endl;
	std::cout << "Server Name: " << serverBlock.GetServerNames().back() << std::endl;
	std::cout << "Port: " << serverBlock.GetPort() << std::endl;
	std::cout << "IP: " << serverBlock.GetIP() << std::endl;
	std::cout << "Root: " << serverBlock.GetRoot() << std::endl;
	std::cout << "Body Size: " << serverBlock.GetBodySize() << std::endl;
	std::cout << "Autoindex: " << (serverBlock.isAutoindexing() ? "On" : "Off") << std::endl;

	std::cout << "Default Files: ";
	for (std::vector<std::string>::const_iterator it = serverBlock.GetDefFile().begin(); it != serverBlock.GetDefFile().end(); ++it)
	{
		const std::string defFile = *it;
		std::cout << defFile << " ";
	}
	std::cout << std::endl;

	std::cout << "Error Pages: ";
	for (std::vector<ServerBlock::ErrorPage>::const_iterator it = serverBlock.GetErrorPages().begin();
		 it != serverBlock.GetErrorPages().end(); ++it)
	{
		const ServerBlock::ErrorPage &errorPage = *it;
		std::cout << errorPage.error << " -> " << errorPage.page << " ";
	}
	std::cout << std::endl;

	std::cout << "Locations: " << serverBlock.GetLocations().size() << std::endl;
}

void ConfigParser::PrintLocationBlock(ServerBlock::Location &location)
{
	std::cout << std::endl
			  << "	" << location.path << std::endl
			  << "	root: " << location.root << std::endl
			  << "	redirection: " << location.redir.error << " " << location.redir.page << std::endl;

	std::cout << 	"Default Files: ";
	for (std::vector<std::string>::iterator it = location.defFiles.begin(); it < location.defFiles.end(); it++)
	{
		const std::string defFile = *it;
		std::cout << defFile << " ";
	}
	std::cout << std::endl;
	std::cout << "	autoindex: " << location.autoindex << std::endl
			  << "	methods (GET, POST, DELETE): " << location.methods[0] << ", " << location.methods[1] << ", " << location.methods[2] << std::endl
			  << "	body size: " << location.bodySize << std::endl
			  << "	error pages: ";
	for (size_t i = 0; i < location.GetErrorPages().size(); ++i)
	{
		std::cout << location.GetErrorPages()[i].error << " " << location.GetErrorPages()[i].page;
		if (i < location.GetErrorPages().size() - 1)
			std::cout << ", ";
	}
	std::cout << std::endl
			  << "	cgi enabled: " << location.cgiEnable << std::endl;
	if (!location.cgiHandlers.empty())
	{
		std::cout << "	cgi extension: " << location.cgiHandlers[0].extension << " " << location.cgiHandlers[0].path;
	}
	else
	{
		std::cout << "	cgi extension: none";
	}
	std::cout << std::endl
			  << "	upload enabled: " << location.uploadEnable << std::endl
			  << "	upload path: " << location.uploadPath << std::endl;
}