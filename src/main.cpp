# include "../includes/Webserv.h"

bool running = true;
bool debug = false;

void	stop_signal(int signal)
{
	(void)signal;
	running = 0;
}

void	debug_signal(int signal)
{
	(void)signal;
	if(debug)
		debug = 0;
	else
		debug = 1;
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

void	pending_clients(std::map<int, Client*> &clients)
{
	int		client_fd;
	for(std::map<int, Client*>::iterator i = clients.begin(); i != clients.end(); i++)
	{
		client_fd = i->second->getFD();
		switch(i->second->getState())
		{
			case RECIEVING_REQUEST:
			case CLEANING_INVALID_REQUEST:
				std::cout << "\nRecieving from client " << client_fd << " ...\n";
				i->second->recieveRequestChunk();
				break;

			case SENDING_HEADER:
				std::cout << "\nSending the Header to client " << client_fd << " ...\n";
				i->second->sendHeaderChunk();
				break;

			case SENDING_BODY:
				std::cout << "\nSending the Body to client " << client_fd << " ...\n";
				i->second->sendBodyChunk();
				break;

			default:;
		};
	}
}

int main(int argc, char **argv)
{
	try
	{
		int							max_connections = 5;
		int							timeout = 1000;

		(void)argv;
		(void)argc;
		// Server configuration parse
		//configuration_parser() -> TODO
		
		// Setting up signals to exit the server loop
		running = 1;
		debug = 0;
		signal(SIGINT, stop_signal);
		signal(SIGQUIT, debug_signal);
		//signal(SIGQUIT, SIG_IGN);

		// Initializing variables
		ListeningSocket* a = new ListeningSocket(INADDR_ANY, 8080, max_connections);
		ListeningSocket* b = new ListeningSocket(INADDR_ANY, 6969, max_connections);
		ListeningSocket* c = new ListeningSocket(INADDR_ANY, 4200, max_connections);

		std::map<int, ListeningSocket*> gateways;
		gateways.insert(std::pair<int, ListeningSocket*>(a->getFD(), a));
    	gateways.insert(std::pair<int, ListeningSocket*>(b->getFD(), b));
		gateways.insert(std::pair<int, ListeningSocket*>(c->getFD(), c));



		std::map<int, Client*>	clients;
		EventHandler			events(gateways, clients, max_connections);
		
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
				pending_clients(clients);

				while(debug){;}
			}
			catch(const std::exception& e)
			{
				std::cerr << e.what() << '\n';
			}
		}
		delete_clients(clients);
		delete_gateways(gateways);
	}
	catch(const std::exception &e)
	{
		std::cerr << "Error : " << e.what() << std::endl;
	}
}