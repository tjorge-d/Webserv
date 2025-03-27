# include "../includes/Webserv.h"

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

void	pending_clients(std::map<int, Client*> &clients)
{
	state	c_state;
	int		chunk_size = 50;
	for(std::map<int, Client*>::iterator i = clients.begin(); i != clients.end(); i++)
	{
		c_state = i->second->getState();
		if (c_state == RECIEVING_REQUEST || c_state == CLEANING_INVALID_REQUEST)
		{
			std::cout << "The client " << i->second->getFD() << " is Recieving\n";
			i->second->recieveRequestChunk(chunk_size);
		}
		else if (c_state == SENDING_RESPONSE)
		{
			std::cout << "The client " << i->second->getFD() << " is Sending\n";
			i->second->sendResponseChunk(chunk_size);
		}
	}
}

int main(int argc, char **argv)
{
	try
	{
		int							max_connections = 3;
		int							timeout = 1000;

		(void)argv;
		(void)argc;
		// Server configuration parse
		//configuration_parser() -> TODO
		
		// Setting up signals to exit the server loop
		running = 1;
		debug = 1;
		signal(SIGINT, stop_signal);
		signal(SIGQUIT, debug_signal);
		//signal(SIGQUIT, SIG_IGN);

		// Server initialization
		ListeningSocket 		server(INADDR_ANY, 6969, max_connections);
		std::map<int, Client*>	clients;
		EventHandler			events(server, clients);
		
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
				std::cerr << errno << '\n';
			}
		}
		delete_clients(clients);
	}
	catch(const std::exception &e)
	{
		std::cerr << "Error : " << e.what() << std::endl;
	}
}