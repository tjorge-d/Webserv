#include "../includes/Webserv.h"
#include "../includes/Client.hpp"
#include <arpa/inet.h>

bool running = true;
bool freeze = false;
bool print = false;

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

	for (std::vector<ServerBlockInfo>::iterator i = server_info.server_blocks.begin(); \
	i != server_info.server_blocks.end(); ++i)
	{
		ServerBlock *new_server_block = new ServerBlock(*(i), server_info);
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
				std::cout << std::endl << "Client " << i->second->getFD() << " (Recieving)" << std::endl;
				i->second->recieveRequestChunk();
				break;

			case SENDING_HEADER:
				std::cout << std::endl << "Client " << i->second->getFD() << " (Sending Header)" << std::endl;
				i->second->sendHeaderChunk();
				break;

			case SENDING_BODY:
				std::cout << std::endl << "Client " << i->second->getFD() << " (Sending Body)" << std::endl;
				i->second->sendBodyChunk();
				break;

			case DONE:{
				std::map<int, Client *>::iterator delete_i = i--;
				events.deleteClient(delete_i->second->getFD());
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
		EventHandler	events(server_blocks, clients, MAX_CONNECTIONS);

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