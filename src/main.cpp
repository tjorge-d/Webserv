#include "../includes/Webserv.h"
#include "../includes/Server.hpp"
#include "../includes/Logger.hpp"
#include <arpa/inet.h>

std::string	getStatus(int code){
	static std::map<int, std::string>	failsafe_error_codes;

	failsafe_error_codes[200] = "OK"; //The request succeeded.
	failsafe_error_codes[204] = "No Content"; //There is no content to send for this request, but the headers are useful.
	failsafe_error_codes[301] = "Moved Permanently"; //The URL of the requested resource has been changed permanently. The new URL is given in the response (necessary?)
	failsafe_error_codes[303] = "See Other"; //The server sent this response to direct the client to get the requested resource at another URI with a GET request.
	failsafe_error_codes[308] = "Permanent Redirect"; //This means that the resource is now permanently located at another URI, specified by the Location response header.
	//This has the same semantics as the 301 Moved Permanently HTTP response code, with the exception that the user agent must not change the HTTP method used: if a POST was used in the first request, a POST must be used in the second request.
	failsafe_error_codes[400] = "Bad Request"; //The server cannot or will not process the request due to something that is perceived to be a client error (e.g., malformed request syntax, invalid request message framing, or deceptive request routing).
	failsafe_error_codes[403] = "Forbidden"; //The client does not have access rights to the content; that is, it is unauthorized, so the server is refusing to give the requested resource. Unlike 401 Unauthorized, the client's identity is known to the server.
	failsafe_error_codes[404] = "Not Found"; //The server cannot find the requested resource. In the browser, this means the URL is not recognized. In an API, this can also mean that the endpoint is valid but the resource itself does not exist.
	failsafe_error_codes[405] = "Method Not Allowed"; //The request method is known by the server but is not supported by the target resource. For example, an API may not allow DELETE on a resource, or the TRACE method entirely.
	failsafe_error_codes[408] = "Request Timeout"; //This response is sent on an idle connection by some servers, even without any previous request by the client. It means that the server would like to shut down this unused connection.
	failsafe_error_codes[409] = "Conflict"; //This response is sent when a request conflicts with the current state of the server. 
	failsafe_error_codes[411] = "Length Required"; //Server rejected the request because the Content-Length header field is not defined and the server requires it.
	failsafe_error_codes[413] = "Content Too Large"; //The request body is larger than limits defined by server. The server might close the connection or return an Retry-After header field.
	failsafe_error_codes[429] = "Too Many Requests"; //The user has sent too many requests in a given amount of time (rate limiting).
	failsafe_error_codes[431] = "Request Header Fields Too Large"; //The server is unwilling to process the request because its header fields are too large.
	failsafe_error_codes[500] = "Internal Server Error"; //The server has encountered a situation it does not know how to handle. This error is generic, indicating that the server cannot find a more appropriate 5XX status code to respond with.
	failsafe_error_codes[503] = "Service Unavailable"; //The server is not ready to handle the request.
	failsafe_error_codes[504] = "Gateway Timeout"; //This error response is given when the server is acting as a gateway and cannot get a response in time.
	failsafe_error_codes[505] = "HTTP Version Not Supported"; //The HTTP version used in the request is not supported by the server.
	
	return failsafe_error_codes[code];
}

int main(int argc, char **argv)
{
	std::srand(std::time(NULL)); //THIS NEEDS TO BE HERE, IT SETS UP THE SEED AT PROGRAM START THAT IS USED TO GENERATE A SESSIONID FOR ALL CLIENTS

	Logger::init("webserv.log", true, INFO);

	try
	{
		// Parse configuration
		HttpInfo *config = config_parser(argv[1], argc);
		(void)argv;
		(void)argc;

		// Create and run server
		Server server(config);
		server.run();
	}
	catch (const std::exception &e)
	{
		Logger::log(ERROR, "Fatal error: " + std::string(e.what()));
		std::cerr << "Error: " << e.what() << std::endl;
		Logger::close();
		return 1;
	}

	Logger::close();
	return 0;
}