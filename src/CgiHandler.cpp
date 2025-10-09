#include "../includes/CgiHandler.hpp"
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

// CONSTRUCTORS & DESTRUCTORS

CgiHandler::CgiHandler(std::string cgiPath, HttpRequest request):
env(),
args(),
cgiPath(cgiPath)
{
	initEnv(request);
}

CgiHandler::~CgiHandler(){
	env.clear();
	args.clear();
}

void    CgiHandler::initEnv(HttpRequest request){
	env["AUTH_TYPE"] = "";
	env["CONTENT_LENGTH"] = request.getHeader("Content-Length");
	env["CONTENT_TYPE"] = request.getHeader("Content-Type");
	env["GATEWAY_INTERFACE"] = "CGI/1.1";
	env["PATH_INFO"] = extractPathInfo(request.path);
	env["PATH_TRANSLATED"] = extractPathTranslated(request.path);
	env["QUERY_STRING"] = extractQuery(request.path);
	env["REMOTE_ADDR"] = request.getHeader("X-Forwarded-For");
	env["REMOTE_HOST"] = "";
	env["REMOTE_IDENT"] = "";
	env["REMOTE_USER"] = "";
	env["REQUEST_METHOD"] = request.method;
	env["SCRIPT_NAME"] = extractScript(request.path);
	
	// Safe host parsing
	std::string host = request.getHeader("Host");
	if (host.empty()) host = "localhost:80";
	
	size_t colon_pos = host.find(":");
	if (colon_pos != std::string::npos) {
		env["SERVER_NAME"] = host.substr(0, colon_pos);
		env["SERVER_PORT"] = host.substr(colon_pos + 1);
	} else {
		env["SERVER_NAME"] = host;
		env["SERVER_PORT"] = "80";
	}
	
	env["SERVER_PROTOCOL"] = request.version;
	env["SERVER_SOFTWARE"] = SERVER_VERSION;
	env["HTTP_COOKIE"] = request.getHeader("Cookie");
}

std::string	CgiHandler::extractScript(std::string path)
{
	// Extract script name from path
	size_t script_start = path.find_last_of("/");
	if (script_start == std::string::npos)
		return path;
	
	size_t query_pos = path.find("?");
	if (query_pos != std::string::npos && query_pos > script_start)
		return path.substr(script_start + 1, query_pos - script_start - 1);
	
	return path.substr(script_start + 1);
}

std::string	CgiHandler::extractPathTranslated(std::string path)
{
	// Extract path before query string, remove script name
	size_t query_pos = path.find("?");
	std::string base_path = (query_pos != std::string::npos) ? path.substr(0, query_pos) : path;
	
	// Security: Prevent path traversal
	if (base_path.find("../") != std::string::npos || base_path.find("..\\") != std::string::npos)
		return "";
	
	return base_path;
}

std::string	CgiHandler::extractPathInfo(std::string path)
{
	// Extract additional path info after script name
	size_t script_pos = path.find(".py");
	if (script_pos == std::string::npos)
		script_pos = path.find(".php");
	
	if (script_pos == std::string::npos)
		return "";
	
	// Find end of script file extension
	script_pos += 3; // .py or .php
	if (script_pos < path.length() && path[script_pos] == 'p') // for .php
		script_pos++;
	
	size_t query_pos = path.find("?");
	if (query_pos != std::string::npos && script_pos < query_pos)
		return path.substr(script_pos, query_pos - script_pos);
	else if (script_pos < path.length())
		return path.substr(script_pos);
	
	return "";
}

std::string	CgiHandler::extractQuery(std::string path)
{
	size_t query_pos = path.find("?");
	if (query_pos == std::string::npos || query_pos == path.length() - 1)
		return "";
	
	std::string query = path.substr(query_pos + 1);
	
	// Security: Basic query string sanitization
	// Remove dangerous characters that could be used for injection
	// for (size_t i = 0; i < query.length(); i++) {
	// 	if (query[i] == ';' || query[i] == '|' || query[i] == '&' || 
	// 		query[i] == '`' || query[i] == '$' || query[i] == '(' || query[i] == ')')
	// 		query[i] = '_';
	// }
	
	return query;
}

int CgiHandler::executeCgi(const std::string& scriptPath, const std::string& interpreter,
                          const std::string& requestBody, std::string& cgiOutput)
{
    int inPipe[2], outPipe[2];
    if (pipe(inPipe) < 0 || pipe(outPipe) < 0)
        return -1;

    pid_t pid = fork();
    if (pid < 0)
        return -1;
    if (pid == 0) {
        dup2(inPipe[0], 0);
        dup2(outPipe[1], 1);
        close(inPipe[1]);
        close(outPipe[0]);

        std::vector<std::string> envVec;
        for (std::map<std::string, std::string>::iterator it = env.begin(); it != env.end(); ++it)
            envVec.push_back(it->first + "=" + it->second);
		
		std::vector<char *> envp;
		for (std::vector<std::string>::iterator it = envVec.begin(); it != envVec.end(); ++it)
			envp.push_back(const_cast<char*>(it->c_str()));
		envp.push_back(NULL);

        char* argv[] = {const_cast<char*>(interpreter.c_str()), const_cast<char*>(scriptPath.c_str()), NULL};

        execve(interpreter.c_str(), argv, &envp[0]);
        exit(1);
    }
    close(inPipe[0]);
    close(outPipe[1]);
    write(inPipe[1], requestBody.c_str(), requestBody.size());
    close(inPipe[1]);
    char buffer[4096];
    ssize_t n;
    cgiOutput.clear();
    while ((n = read(outPipe[0], buffer, sizeof(buffer))) > 0)
        cgiOutput.append(buffer, n);
    close(outPipe[0]);
    waitpid(pid, NULL, 0);
	printf("CGI output:\n%s\n", cgiOutput.c_str());
    return (0);
}