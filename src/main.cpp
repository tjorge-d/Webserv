#include "../includes/Webserv.h"
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

void delete_servers(std::map<int, ListeningSocket *> &servers)
{
	for (std::map<int, ListeningSocket *>::iterator i = servers.begin(); i != servers.end(); i++)
		delete i->second;
}

void pending_clients(std::map<int, Client *> &clients, EventHandler &events)
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

			default:;
			};
		}
		catch (const std::exception &e)
		{
			std::map<int, Client *>::iterator delete_i = i;
			i++;
			std::cerr << "Error : " << e.what() << std::endl;
			std::cout << "Deleting client " << delete_i->second->getFD() << "..." << std::endl;
			events.deleteClient(delete_i->second->getFD());
			if (i == clients.end())
				break;
		};
	}
}

std::map<int, ListeningSocket *> create_servers(std::vector<ServerBlock> &servers_info)
{
	std::map<int, ListeningSocket *> servers;
	std::cout << "Domain: " << servers_info.back().port << "\n";
	std::cout << "Domain: " << servers_info.front().port << "\n";
	for (std::vector<ServerBlock>::iterator i = servers_info.begin(); i != servers_info.end(); ++i)
	{
		ListeningSocket *new_server = new ListeningSocket(INADDR_ANY, i->port, SOCKET_BACKLOG);
		servers[new_server->getFD()] = new_server;
	}
	return (servers);
}

int main(int argc, char **argv)
{
	try
	{
		// Parsing the arguments (expecting a config_file)
		if (argc != 2)
			throw ParserException("Invalid number of arguments.");
		ServerInfo	*config = config_parser(argv[1]);
		(void)argv;
		(void)argc;

		// Setting up signals to exit the server loop
		signal(SIGINT, stop_signal);
		signal(SIGQUIT, freeze_signal);
		signal(SIGTSTP, print_signal);

		// Initializing servers
		std::cout << "domain: " << config->servers.back().port << "\n";
		std::map<int, ListeningSocket *>	servers = create_servers(config->servers);
		std::map<int, Client *> 			clients;
		EventHandler 						events(servers, clients, MAX_CONNECTIONS);

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
					print = false;
				}
			}
			catch (const std::exception &e)
			{
				std::cerr << e.what() << '\n';
			}
		}
		delete_clients(clients);
		delete_servers(servers);
		delete config;
	}
	catch (const std::exception &e)
	{
		std::cerr << "Error : " << e.what() << std::endl;
	}
}