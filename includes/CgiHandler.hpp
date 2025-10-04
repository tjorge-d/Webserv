#ifndef CGIHANDLER_HPP
# define CGIHANDLER_HPP

# include <vector>
# include <map>
# include <iostream>
#include "HttpRequest.hpp"

enum    metaVar{
	AUTH_TYPE = 0,
	CONTENT_LENGHT,
	CONTENT_TYPE,
	GATEWAY_INTERFACE,
	PATH_INFO,
	PATH_TRANSLATED,
	QUERY_STRING,
	REMOTE_ADDR,
	REQUEST_METHOD,
	SCRIPT_NAME,
	SERVER_NAME,
	SERVER_PORT,
	SERVER_PROTOCOL,
	SERVER_SOFTWARE,
	HTTP_COOKIE
};

class HttpRequest;

class CgiHandler
{
	private:
		std::map<std::string, std::string>  env;
		std::vector<std::string>            args;
		std::string                         cgiPath;
		pid_t                               cgiPid;

	public:
		CgiHandler(std::string cgiPath, pid_t cgiPid, HttpRequest request);
		~CgiHandler();

		void		initEnv(HttpRequest request);
		std::string	extractPathInfo(std::string path);
		std::string	extractPathTranslated(std::string path);
		std::string	extractQuery(std::string path);
		std::string	extractScript(std::string path);
		int			executeCgi(const std::string& scriptPath, const std::string& interpreter,
		              const std::string& requestBody, std::string& cgiOutput);
};

#endif