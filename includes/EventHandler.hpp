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

class EventHandler
{
	private:
		// ATTRIBUTES
		int							_epollFd;
		int							_serverFd;
		int							_eventsNumber;
		std::vector<epoll_event>	_events;


		// CONSTRUCTORS/DESTRUCTORS
		EventHandler();

	public:
		// ATTRIBUTES
		// CONSTRUCTORS/DESTRUCTORS
		EventHandler(int serverFd);
		EventHandler(const EventHandler &a);
		~EventHandler() ;
		
		// GETTERS
		std::vector<epoll_event> getEvents();
		epoll_event	getEvent(int index);

		// OPERATORS
		EventHandler&	operator=(const EventHandler &copy);
		epoll_event	operator[](int index) const;

		// MEMBER FUNCTIONS
		// Safely closes the Class
		void	closeHandler();
		
		// Adds a client to the epoll instance in the Kernel
		void	addClient(int client_fd);
		
		// Removes a client from the epoll instance in the Kernel (doesn't close the fd)
		void	removeClient(int client_fd);

		// Waits for events, fills "_events" with ready ones and returns its number 
		void	waitEvents(int timeout);

		void	checkEvents();

		void	handleEvent(epoll_event& event);
		
	// EXCEPTIONS
	class	EPollCreationFailure : public std::runtime_error
	{
		public :
			EPollCreationFailure();
	};

	class	EPollCloseFailure : public std::runtime_error
	{
		public :
			EPollCloseFailure();
	};

	class	EPollCTLFailure : public std::runtime_error
	{
		public :
			EPollCTLFailure(std::string info);
	};

	class	EPollWaitFailure : public std::runtime_error
	{
		public :
			EPollWaitFailure();
	};

	class	EventOutOfBounds : public std::runtime_error
	{
		public :
			EventOutOfBounds();
	};

	class	ConnectionAcceptFailure : public std::runtime_error
	{
		public :
			ConnectionAcceptFailure();
	};

	class	RecieveFailure : public std::runtime_error
	{
		public :
			RecieveFailure();
	};		
};

#endif