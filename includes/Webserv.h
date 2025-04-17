#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include <errno.h>
# include <string.h>
# include <signal.h>
# include <map>
# include <iostream>
# include <string>
# include <fstream>
# include <cstring>
# include <iomanip>
# include <cstdlib>
# include <vector>
# include <sstream>
# include <climits>

# define SOCKET_BACKLOG		5 // Max queue size for listening sockets
# define MAX_CONNECTIONS	10 // Max connections to the server
# define BODY_SIZE_MAX		2147483647; //2 Gb max

struct DomainBlockInfo{
	
	bool							autoindex;
	std::vector<std::string>		allowed_services;
	std::string						domain_location;
	std::string						root_directory;
	std::string						index_file;
};

struct ServerBlockInfo{
	
	int								port;
	std::string						server_name;
	std::string						redirect_directory;
	std::map<int, std::string>		error_codes;
	std::vector<DomainBlockInfo>	domain;
};

struct HttpInfo{

	int								client_max_body_size;
	std::vector<ServerBlockInfo>	server_blocks;
};

class ParserException : public std::runtime_error{

	public:
	ParserException (std::string error);
};

HttpInfo *config_parser(char *file_path);

#endif