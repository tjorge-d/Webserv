# include "../includes/Webserv.h"
#include <arpa/inet.h> 
bool running = true;
bool freeze = false;
bool print = false;

void	stop_signal(int signal)
{
	(void)signal;
	running = false;
}

void	freeze_signal(int signal)
{
	(void)signal;
	if(freeze)
		freeze = false;
	else
		freeze = true;
}

void	print_signal(int signal)
{
	(void)signal;
	print = true; 
}

void	delete_clients(std::map<int, Client*> &clients)
{
	for(std::map<int, Client*>::iterator i = clients.begin(); i != clients.end(); i++)
		delete i->second;
}

void	delete_gateways(std::map<int, ListeningSocket*> &gateways)
{
	for(std::map<int, ListeningSocket*>::iterator i = gateways.begin(); i != gateways.end(); i++)
		delete i->second;
}

void	pending_clients(std::map<int, Client*> &clients, EventHandler &events)
{
	for(std::map<int, Client*>::iterator i = clients.begin(); i != clients.end(); i++)
	{
		try
		{
			switch(i->second->getState())
			{
				case RECIEVING_REQUEST:
				case CLEANING_INVALID_REQUEST:
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
		catch(const std::exception &e)
		{
			std::cerr << "Error : " << e.what() << std::endl;
			std::cout << "Deleting client " << i->second->getFD() << "..." << std::endl;
			events.deleteClient(i->second->getFD());
		}
	}
}

int main(int argc, char **argv)
{
	try
	{
		int							max_connections = 5;
		int							timeout = 0;
		(void)argv;
		(void)argc;

		// Server configuration parse
		//configuration_parser() -> TODO
		
		// Setting up signals to exit the server loop
		signal(SIGINT, stop_signal);
		signal(SIGQUIT, freeze_signal);
		signal(SIGTSTP, print_signal);

		// Initializing servers
		std::map<int, ListeningSocket*> servers;
		ListeningSocket* a = new ListeningSocket(INADDR_ANY, 8080, max_connections);	
		servers.insert(std::pair<int, ListeningSocket*>(a->getFD(), a));
		ListeningSocket* b = new ListeningSocket(INADDR_ANY, 6969, max_connections);
    	servers.insert(std::pair<int, ListeningSocket*>(b->getFD(), b));
		ListeningSocket* c = new ListeningSocket(INADDR_ANY, 4200, max_connections);
		servers.insert(std::pair<int, ListeningSocket*>(c->getFD(), c));

		std::map<int, Client*>	clients;
		EventHandler			events(servers, clients, max_connections);
		
		// Server loop
		while(running)
		{
			try
			{
				// Waits for events 
				events.waitEvents(timeout);

				// Event loop
				events.checkEvents();

				// Reads/Writes pending requests/responses
				pending_clients(clients, events);

				// debugging tools
				while(freeze){;}
				if (print)
				{
					std::cout << "Clients connected: " << events.getConnections() << std::endl;
					print = false;
				}
			}
			catch(const std::exception& e)
			{
				std::cerr << e.what() << '\n';
			}
		}
		delete_clients(clients);
		delete_gateways(servers);
	}
	catch(const std::exception &e)
	{
		std::cerr << "Error : " << e.what() << std::endl;
	}
}