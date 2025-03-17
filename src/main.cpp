# include "../includes/Webserv.h"

void client_process()
{

}

int main(int argc, char **argv)
{
	try
	{
		int 						client_fd;
		std::vector<epoll_event>	events;
		char 						a[1000];
		int							max_connections = 10;
		int							max_events = 10;
		int							timeout = 3;
		
		(void)argv;
		(void)argc;
		
		// Server configuration parse
		//configuration_parser() -> to be worked on

		// Server initialization
		ListeningSocket server(INADDR_ANY, 8080, max_connections);
		EpollHandler	epoll(server.getFD());
		//signal() -> to be worked on

		// Server loop
		while (1)
		{
			try
			{
				epoll.waitForEvents(timeout);
				events = epoll.getEvents();
				/*
				while (events[i])
				{
					if (events[i].fd == server.getFD())
						adicionar evento a epoll
					else
						tratar do evento
				}
				*/
			}
			catch(const std::exception& e)
			{
				std::cerr << e.what() << '\n';
			}
			
			// Accepts a client, fills it's sockaddr_in struct and adds an event
			struct sockaddr_in	client_addr;
			socklen_t			client_size = sizeof(client_addr);
			epoll.addClient(accept(server.getFD(), (struct sockaddr *)&client_addr, &client_size));
			
			recv(client_fd, a, 1000, 0); 
			std::cout << a;
			send(client_fd, a, 1000, 0);
			close(client_fd);
		}

	}
	catch(const std::exception &e)
	{
		std::cerr << "Error : " << e.what() << std::endl;
	}
}