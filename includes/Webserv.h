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

# define HTTP_ACCEPTED_VERSION "HTTP/1.1"
# define SERVER_TYPE_RESPONSE_HEADER "Server:"
# define SERVER_VERSION "webserv/1.0.0"
# define DATE_TYPE_RESPONSE_HEADER "Date:"
# define CONTENT_TYPE_RESPONSE_HEADER "Content-Type:"
# define CONTENT_LENGTH_RESPONSE_HEADER "Content-Length:"
# define LAST_MODIFIED_RESPONSE_HEADER "Last-Modified:"
# define CONNECTION_RESPONSE_HEADER "Conection:"
# define PLAIN_TEXT "text/plain; charset=UTF-8"
# define CLOSE_CONNECTION "close"
# define KEEP_ALIVE_CONNECTION "keep-alive"
# define RESPONSE_LINE_END "\r\n"

enum	StatusCodes{
	OK = 200,
	NO_CONTENT = 204,
	MOVED_PERMANENTLY = 301,
	SEE_OTHER = 303,
	PERMANENTE_REDIRECT = 308,
	BAD_REQUEST = 400,
	FORBIDDEN = 403,
	NOT_FOUND,
	METHOD_NOT_ALLOWED,
	REQUEST_TIMEOUT = 408,
	CONFLICT,
	LENGHT_REQUIRED = 411,
	CONTENT_TOO_LARGE = 413,
	TOO_MANY_REQUESTS = 429,
	REQUEST_HEADER_FIELDS_TOO_LARGE = 431,
	INTERNAL_SERVER_ERROR = 500,
	SERVICE_UNAVAILABLE = 503,
	GATEWAY_TIMEOUT,
	HTTP_VERSION_NOT_SUPPORTED
};

struct LocationBlockInfo{
	
	bool												autoindex;
	std::vector<std::string>							allowed_services;
	std::vector<std::string>							allowed_cgi;
	std::string											location;
	std::string											index_file;
};

struct ServerBlockInfo{
	
	int													port;
	std::string											server_name;
	std::string											server_root;
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
	std::string							acquired_cgi;
	std::string							acquired_max_body_size;
	std::ifstream						config_file;
	bool 								max_size_acquired;
	bool 								server_setup_mode;
	bool 								location_setup_mode;
	bool								error_page_detected;
};

struct HttpInfo{

	int													client_max_body_size;
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
std::string	getStatus(int code);
std::string trim(const std::string &s);

#endif