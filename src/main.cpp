# include "../includes/Webserv.h"

void client_process()
{

}

int main(int argc, char **argv)
{
	try
	{
		(void)argv;
		(void)argc;
		int client_fd;
		char a[1000];
		int	max_connections = 10;
		// Server configuration parse

		// Server initialization
		ListeningSocket server(INADDR_ANY, 8080, max_connections);

		// Server loop
		while (1)
		{
			client_fd = accept(server.getFD(), NULL, NULL);
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