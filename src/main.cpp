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

void	debug_msg(std::string msg)
{
	if(debug)
		std::cout << msg << std::endl;
}

void	delete_clients(std::map<int, Client*> &clients)
{
	for(std::map<int, Client*>::iterator i = clients.begin(); i != clients.end(); i++)
		delete i->second;
}

void	pending_clients(std::map<int, Client*> &clients)
{
	state	c_state;
	for(std::map<int, Client*>::iterator i = clients.begin(); i != clients.end(); i++)
	{
		c_state = i->second->getState();

		if (c_state == RECIEVING_REQUEST || c_state == CLEANING_INVALID_REQUEST)
			i->second->recieveRequestChunk(8);
		else if (c_state == SENDING_RESPONSE)
			i->second->sendResponseChunk(8);
	}
}

int main(int argc, char **argv)
{
	try
	{
		int							max_connections = 10;
		int							timeout = 1000;

		(void)argv;
		(void)argc;
		// Server configuration parse
		//configuration_parser() -> TODO
		
		// Server initialization
		ListeningSocket 		server(INADDR_ANY, 6969, max_connections);
		std::map<int, Client*>	clients;
		EventHandler			events(server, clients);
		
		// Setting up signals to exit the server loop
		running = 1;
		debug = 0;
		signal(SIGINT, stop_signal);
		signal(SIGQUIT, debug_signal);

		// Server loop
		while(running)
		{
			try
			{
				// Waits for events 
				debug_msg("WAITING...");
				events.waitEvents(timeout);

				// Event loop
				debug_msg("CHECKING EVENTS...");
				events.checkEvents();

				// Reads/Writes pending requests/responses
				debug_msg("CHECKING EVENTS...");
				pending_clients(clients);

				//while(debug)
				//{;}
				// Pending
			}
			catch(const std::exception& e)
			{
				std::cerr << e.what() << '\n';
				std::cerr << errno << '\n';
			}
		}
		debug_msg("TERMINATING...");
		delete_clients(clients);
	}
	catch(const std::exception &e)
	{
		std::cerr << "Error : " << e.what() << std::endl;
	}
}