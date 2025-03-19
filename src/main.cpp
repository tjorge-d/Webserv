# include "../includes/Webserv.h"

void client_process()
{

}

int main(int argc, char **argv)
{
	try
	{
		int							max_connections = 10;
		int							timeout = -1;

		(void)argv;
		(void)argc;
		
		// Server configuration parse
		//configuration_parser() -> to be worked on

		// Server initialization
		ListeningSocket server(INADDR_ANY, 8080, max_connections);
		EventHandler	events(server.getFD());
		
		//signal() -> to be worked on

		// Server loop
		while (1)
		{
			try
			{
				events.waitEvents(timeout);
				events.checkEvents();
				std::cout << "A";
			}
			catch(const std::exception& e)
			{
				std::cerr << e.what() << '\n';
			}
		}

	}
	catch(const std::exception &e)
	{
		std::cerr << "Error : " << e.what() << std::endl;
	}
}