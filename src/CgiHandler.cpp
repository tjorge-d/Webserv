#include "../includes/CgiHandler.hpp"

// CONSTRUCTORS & DESTRUCTORS

CgiHandler::CgiHandler(std::string cgiPath, pid_t cgiPid, HttpRequest request):
env(),
args(),
cgiPath(cgiPath),
cgiPid(cgiPid)
{
	std::cout << "CGI constructor called\n";
	initEnv(request);
}

CgiHandler::~CgiHandler(){
	env.clear();
	args.clear();
}

void    CgiHandler::initEnv(HttpRequest request){
	env["AUTH_TYPE"] = "";
	env["CONTENT_LENGHT"] = request.headerInfo["content-lenght"];
	env["CONTENT_TYPE"] = request.headerInfo["content-type"];
	env["GATEWAY_INTERFACE"] = "CGI/1.1";
	env["PATH_INFO"] = 
	env["PATH_TRANSLATED"] = 
	env["QUERY_STRING"] = decodeQuery(request);
	env["REMOTE_ADDR"] = request.headerInfo["host"];
	env["REMOTE_HOST"] = 
	env["REMOTE_IDENT"] = 
	env["REMOTE_USER"] = 
	env["REQUEST_METHOD"] = 
	env["SCRIPT_NAME"] = 
	env["SERVER_NAME"] = request.headerInfo["host"].substr(0, request.headerInfo["host"].find(":"));
	env["SERVER_PORT"] = request.headerInfo["host"].substr(request.headerInfo["host"].find(":") + 1, request.headerInfo["host"].size());
	env["SERVER_PROTOCOL"] = 
	env["SERVER_SOFTWARE"] = 
	env["HTTP_COOKIE"] = 
}