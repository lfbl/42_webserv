#include "CGIHandler.hpp"
#include "Server.hpp"
//#include "ConfigBack.hpp"
#include "ConfigParser.hpp"
#include "ServerBlock.hpp"

int main(int argc, char **argv, char **envp)
{
	if (argc != 2)
	{
		std::cerr  << "Wrong amount of arguments" << std::endl;
		exit(1);
	}

	ConfigParser parser;
	std::vector<ServerBlock> serverBlocks;
	try
	{
		serverBlocks = parser.Parse(argv[1]);
	}
	catch(const ConfigParser::ConfigErrorException& e)
	{
		std::cerr << e.what() << '\n';
		exit(1);
	}
	Server server;
	server.setConfigs(serverBlocks);
	server.Initialize();
	server.serverListen(envp);
}