#ifndef EPOLLHANDLER_HPP
# define EPOLLHANDLER_HPP
# include <stdio.h>
# include <unistd.h>
# include <iostream>
# include <string.h>
# include <fstream>
# include <sys/epoll.h>
# include <vector>
# include <fcntl.h>

class EpollHandler
{
	private:
		// ATTRIBUTES
		int							_epollFd;
		int							_serverFd;
		int							_eventsNumber;
		std::vector<epoll_event>	_events;


		// CONSTRUCTORS/DESTRUCTORS
		EpollHandler();

	public:
		// ATTRIBUTES
		// CONSTRUCTORS/DESTRUCTORS
		EpollHandler(int serverFd);
		EpollHandler(const EpollHandler &a);
		~EpollHandler() ;
		
		// GETTERS
		std::vector<epoll_event> getEvents();
		
		epoll_event	getEvent(int index);

		// OPERATORS
		EpollHandler&	operator=(const EpollHandler &copy);

		// MEMBER FUNCTIONS
		// Safely closes the Class
		void	closeHandler();
		
		// Adds a client to the epoll instance in the Kernel
		void	addClient(int client_fd);
		
		// Removes a client from the epoll instance in the Kernel (doesn't close the fd)
		void	removeClient(int client_fd);

		// Waits for events, fills "_events" with ready ones and returns its number 
		int		waitForEvents(int );

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
};

#endif