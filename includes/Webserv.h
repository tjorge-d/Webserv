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
# define MAX_CONNECTIONS	2 // Max connections to the server
# define BODY_SIZE_MAX		2147483647; //2 Gb max

struct LocationBlockInfo{
	
	bool												autoindex;
	std::vector<std::string>							allowed_services;
	std::string											location;
	std::string											root_directory;
	std::string											index_file;
};

struct ServerBlockInfo{
	
	int													port;
	std::string											server_name;
	std::string											redirect_directory;
	std::map<int, std::string>							error_codes;
	std::map<std::string, LocationBlockInfo>			locations;
};

struct ParserInfo{

	ServerBlockInfo						current_server_block;
	LocationBlockInfo					current_location_block;
	std::string							start;
	std::string							line;
	std::string							newline;
	std::string							current_start;
	std::string							acquired_services;
	std::string							acquired_max_body_size;
	std::ifstream						config_file;
	bool 								max_size_acquired;
	bool 								server_setup_mode;
	bool 								location_setup_mode;
};

struct HttpInfo{

	int													client_max_body_size;
	std::map<std::string, std::string>					failsafe_error_codes;
	std::map<std::string, ServerBlockInfo>				server_blocks;
	ParserInfo											parser_info;
};

struct MultiFormData{
	std::map<std::string, std::string>					headers;
	std::string											content;
};

class ParserException : public std::runtime_error{

	public:
	ParserException (HttpInfo *server, std::string error);
};

HttpInfo *config_parser(char *file_path, int argc);
void parseLocationBlock(HttpInfo *Server);
void parseServerBlock(HttpInfo *Server);
void parseServerInfo(HttpInfo *Server);
std::string trim(const std::string &s);

#endif