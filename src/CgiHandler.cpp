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
	env["CONTENT_LENGHT"] = request.headerInfo["Content-lenght"];
	env["CONTENT_TYPE"] = request.headerInfo["Content-type"];
	env["GATEWAY_INTERFACE"] = "CGI/1.1";
	env["PATH_INFO"] = 
	env["PATH_TRANSLATED"] = 
	env["QUERY_STRING"] = decodeQuery(request);
	env["REMOTE_ADDR"] = request.headerInfo["Host"];
	env["REQUEST_METHOD"] = request.method;
	env["SCRIPT_NAME"] = 
	env["SERVER_NAME"] = request.headerInfo["Host"].substr(0, request.headerInfo["Host"].find(":"));
	env["SERVER_PORT"] = request.headerInfo["Host"].substr(request.headerInfo["Host"].find(":") + 1, request.headerInfo["host"].size());
	env["SERVER_PROTOCOL"] = request.version;
	env["SERVER_SOFTWARE"] = SERVER_VERSION;
	env["HTTP_COOKIE"] = request.headerInfo["Cookie"];
}

std::string	CgiHandler::decodeQuery(HttpRequest request)
{
	std::string	toDecode = request.path.substr(request.path.find("?"), request.path.size());
	
}