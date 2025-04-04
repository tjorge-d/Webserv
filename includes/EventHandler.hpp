#ifndef EVENTHANDLER_HPP
# define EVENTHANDLER_HPP
# include <stdio.h>
# include <unistd.h>
# include <iostream>
# include <string.h>
# include <fstream>
# include <sys/epoll.h>
# include <netinet/in.h>
# include <vector>
# include <fcntl.h>
# include <map>
# include "Webserv.h"

class Client;

class EventHandler
{
	private:
		// ATTRIBUTES
		std::map<int, ListeningSocket*>	&_servers;
		std::map<int, Client*>			&_clients;
		int								_connections;
		int								_maxConnections;
		int								_epollFd;
		int								_eventsNumber;
		std::vector<epoll_event>		_events;

	public:
		// CONSTRUCTORS/DESTRUCTORS
		EventHandler(std::map<int, ListeningSocket*> &servers, std::map<int, Client*> &clients, int maxConnections);
		~EventHandler() ;
		
		// GETTERS
		std::vector<epoll_event>	getEvents();
		epoll_event					getEvent(int index);
		int							getEventNumber();

		// OPERATORS
		epoll_event		operator[](int index) const;

		// MEMBER FUNCTIONS
		// Safely closes the epoll fd
		void	safeClose();

		// Waits for events and fills "_events" with ready ones
		void	waitEvents(int timeout);

		// Checks the triggered events
		void	checkEvents();

		// Handles client events
		void	handleClientEvent(epoll_event& event);

		// Handles server events
		void	handleServerEvent(int server_fd);

		// Adds a client to the epoll instance in the Kernel
		void	addClient(int client_fd);

		// Removes a client from the epoll instance in the Kernel (doesn't close the fd)
		void	removeClient(int client_fd);

		// Modifies a client event
		void	modifyClient(int client_fd, uint32_t flags);

	// EXCEPTIONS

	class	EPollException : public std::runtime_error
	{
		public :
			EPollException(std::string info);
	};

	class	EPollErrorException : public std::runtime_error
	{
		public :
			EPollErrorException(std::string info);
	};

};

#endif