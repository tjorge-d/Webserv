#include "../includes/EventHandler.hpp"

// CONSTRUCTORS & DESTRUCTORS
EventHandler::EventHandler(ListeningSocket &server, std::map<int, Client*> &clients) :
_server(server), _clients(clients)
{
	//std::cout << "EventHandler custom constructor called\n";
	// Creates an epoll instance in the kernel
	_epollFd = epoll_create(1);
	if(_epollFd == -1)
		throw EPollCreationFailure();
	
	// Fills an epoll_event struct telling how the server fd should be dealt with
	struct epoll_event event;
	event.events = EPOLLIN;
	event.data.fd = _server.getFD();

	// Adds the server to the epoll instance in the kernel
	if(epoll_ctl(_epollFd, EPOLL_CTL_ADD, _server.getFD(), &event) != 0)
		throw EPollCTLFailure("add");
	
	_events.resize(10);
	(void)_clients;
}

EventHandler::~EventHandler()
{
	//std::cout << "EventHandler default destructor called\n";
	closeHandler();
}


// GETTERS

std::vector<epoll_event>	EventHandler::getEvents()
{
	return (_events);
}

epoll_event	EventHandler::getEvent(int index)
{
	if(index >= _eventsNumber || index < 0)
		throw EventOutOfBounds();
	return (_events.at(index));
}

int	EventHandler::getEventNumber()
{
	return (_eventsNumber);
}

// SETTERS


// OPERATORS
epoll_event	EventHandler::operator[](int index) const
{
	if(index >= _eventsNumber || index < 0)
		throw EventOutOfBounds();
	return (_events[index]);
}

// MEMBER FUNCTIONS

void	EventHandler::closeHandler()
{
	// Safely closes the epoll fd
	if(_epollFd >= 0)
	{
		if(close(_epollFd) != 0)
			throw EPollCloseFailure();
		_epollFd = -1;
	}
}

void	EventHandler::addClient(int client_fd)
{
	// Adds a flag to the socket making it non-blocking
	fcntl(client_fd, F_SETFL, O_NONBLOCK);

	// Fills an epoll_event struct telling how the fd should be dealt with
	struct epoll_event event;
	event.events = EPOLLIN | EPOLLET;
	event.data.fd = client_fd;

	// Adds the client to the epoll instance in the kernel
	if(epoll_ctl(_epollFd, EPOLL_CTL_ADD, client_fd, &event) != 0)
		throw EPollCTLFailure("add");

	// Adds the client to the std::map _clients

	std::cout << "The client " << event.data.fd << " connected\n";
}

void	EventHandler::removeClient(int client_fd)
{
	// Removes a client from the epoll instance in the Kernel
	if(epoll_ctl(_epollFd, EPOLL_CTL_DEL, client_fd, NULL) != 0)
		throw EPollCTLFailure("remove");
}

void	EventHandler::waitEvents(int timeout)
{
	// Waits for events and fills "_events" with the ready ones
	_eventsNumber = epoll_wait(_epollFd, _events.data(), _events.size(), timeout);
	if(_eventsNumber == -1)
		throw EPollWaitFailure();
}

void	EventHandler::checkEvents()
{
	for(int i = 0; i < _eventsNumber; i++)
	{
		// Handles new connections to the server
		if(_events[i].data.fd == _server.getFD())
		{
			// Accepts a new connection
			int					client_fd;
			struct sockaddr_in	client_addr;
			socklen_t			client_size = sizeof(client_addr);
			client_fd = accept(_server.getFD(), (struct sockaddr *)&client_addr, &client_size);
			if(client_fd == -1)
				throw ConnectionAcceptFailure();
			
			// Adds the new client to the std::map of clients
			Client	*client = new Client(client_fd);
			_clients[client_fd] = client;

			// Adds the new client connection to epoll
			addClient(client_fd);
		}
		// Handles the non server events in epoll 
		else
			handleEvent(_events[i]);
	}
}

void	EventHandler::handleEvent(epoll_event& event)
{
	std::cout << "The client " << event.data.fd << " triggered an event\n";
	int		b_size = 1000;
	char	buffer[b_size];
	buffer[b_size] = '\0';
	int		bytes;
	bytes = recv(event.data.fd, buffer, b_size, 0);
	if(bytes == -1)
		throw RecieveFailure();
	if(bytes == 0)
	{
		// Removes the Client from the server and epoll
		std::cout << "The client " << event.data.fd << " disconnected\n";
		delete _clients[event.data.fd];
		_clients.erase(event.data.fd);
		return;
	}
	std::cout << "The client " << event.data.fd << " sent:\n";
	std::cout << buffer << std::endl;
	send(event.data.fd, "HI\n", 4, 0);
}

// EXCEPTIONS
EventHandler::EPollCreationFailure::EPollCreationFailure() :
runtime_error("Failed to create an epoll instance in the kernel"){}

EventHandler::EPollCloseFailure::EPollCloseFailure() :
runtime_error("Failed to close an epoll instance"){}

EventHandler::EPollCTLFailure::EPollCTLFailure(std::string info) :
runtime_error("Failed to " + info + " an event in the epoll instance"){}

EventHandler::EPollWaitFailure::EPollWaitFailure() :
runtime_error("Failed to wait for events"){}

EventHandler::EventOutOfBounds::EventOutOfBounds() :
runtime_error("The index provided is out of bounds"){}

EventHandler::ConnectionAcceptFailure::ConnectionAcceptFailure() :
runtime_error("Failed to accept a new connection"){}

EventHandler::RecieveFailure::RecieveFailure() :
runtime_error("Failed to recieve data from an event"){}
