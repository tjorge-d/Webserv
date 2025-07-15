#include "../includes/Webserv.h"
#include "../includes/Client.hpp"
#include <arpa/inet.h>

bool running = true;
bool freeze = false;
bool print = false;

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

void stop_signal(int signal)
{
	(void)signal;
	running = false;
}

void freeze_signal(int signal)
{
	(void)signal;
	if (freeze)
		freeze = false;
	else
		freeze = true;
}

void print_signal(int signal)
{
	(void)signal;
	print = true;
}

void delete_clients(std::map<int, Client *> &clients)
{
	for (std::map<int, Client *>::iterator i = clients.begin(); i != clients.end(); i++)
		delete i->second;
}

void delete_server_blocks(std::map<int, ServerBlock*> &server_blocks)
{
	for (std::map<int, ServerBlock*>::iterator i = server_blocks.begin(); i != server_blocks.end(); i++)
		delete i->second;
}

std::map<int, ServerBlock*> create_server_blocks(HttpInfo &server_info)
{
	std::map<int, ServerBlock*> server_blocks;

	for (std::map<std::string, ServerBlockInfo>::iterator i = server_info.server_blocks.begin(); \
	i != server_info.server_blocks.end(); ++i)
	{
		ServerBlock *new_server_block = new ServerBlock(i->second, server_info.client_max_body_size);
		server_blocks[new_server_block->getListenerFD()] = new_server_block;
	}
	return (server_blocks);
}

void pending_clients(std::map<int, Client*> &clients, EventHandler &events)
{
	for (std::map<int, Client *>::iterator i = clients.begin(); i != clients.end(); i++)
	{
		try
		{
			switch (i->second->getState())
			{
			case RECIEVING_REQUEST:
				// std::cout << std::endl << "Client " << i->second->getFD() << " (Recieving)" << std::endl;
				i->second->recieveRequestChunk();
				break;

			case SENDING_HEADER:
				// std::cout << std::endl << "Client " << i->second->getFD() << " (Sending Header)" << std::endl;
				i->second->sendHeaderChunk();
				break;

			case SENDING_BODY:
				// std::cout << std::endl << "Client " << i->second->getFD() << " (Sending Body)" << std::endl;
				// build header?
				i->second->sendBodyChunk();
				break;

			case DONE:{
				std::map<int, Client *>::iterator delete_i = i--;
				events.deleteClient(delete_i->second->getFD());
				i = clients.begin();
				break;}

			default:;
			};
		}
		catch (const std::exception &e)
		{
			std::map<int, Client *>::iterator delete_i = i--;
			std::cerr << "Error : " << e.what() << std::endl;
			std::cout << "Deleting client " << delete_i->second->getFD() << "..." << std::endl;
			events.deleteClient(delete_i->second->getFD());
		};
	}
}

int main(int argc, char **argv)
{
	try
	{
		// Parsing the arguments (expecting a config_file)
		HttpInfo *config = config_parser(argv[1], argc);
		(void)argv;
		(void)argc;

		// Setting up signals to exit the server loop
		signal(SIGINT, stop_signal);
		signal(SIGQUIT, freeze_signal);
		signal(SIGTSTP, print_signal);

		// Creating a map of server blocks identified by the FD of their listening socket
		std::map<int, ServerBlock*>	server_blocks = create_server_blocks(*config);
		
		// Creating a map of clients identified by their FD 
		std::map<int, Client*>	clients;

		// Creating the server event handler 
		EventHandler	events(clients, server_blocks, MAX_CONNECTIONS);

		// Server loop
		while (running)
		{
			try
			{
				// Waits for events
				events.waitEvents(0);

				// Event loop
				events.checkEvents();

				// Reads/Writes pending requests/responses
				pending_clients(clients, events);

				// debugging tools
				while (freeze)
				{
					;
				}
				if (print)
				{
					std::cout << "Clients connected: " << events.getConnections() << std::endl;
					std::cout << "Clients map size: " << clients.size() << std::endl;
					print = false;
				}
			}
			catch (const std::exception &e)
			{
				std::cerr << e.what() << '\n';
			}
		}
		delete_clients(clients);
		delete_server_blocks(server_blocks);
		delete config;
	}
	catch (const std::exception &e)
	{
		std::cerr << "Error : " << e.what() << std::endl;
	}
}