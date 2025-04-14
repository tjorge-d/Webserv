#include "../includes/Webserv.h"

// CONSTRUCTORS & DESTRUCTORS
EventHandler::EventHandler(std::map<int, ListeningSocket*> &servers, std::map<int, Client*> &clients, int maxConnections) :
_servers(servers),
_clients(clients),
_connections(0),
_maxConnections(maxConnections)
{
	//std::cout << "EventHandler custom constructor called\n";
	// Creates an epoll instance in the kernel
	_epollFd = epoll_create(1);
	if(_epollFd == -1)
		throw EPollErrorException("Failed to create an epoll instance in the kernel");
	
	// Configures the ports of each server
	for(std::map<int, ListeningSocket*>::iterator i = _servers.begin(); i != _servers.end(); i++)
	{
		// Fills an epoll_event struct to configure the event of the port
		struct epoll_event event;
		event.events = EPOLLIN;
		event.data.fd = i->second->getFD();

		// Adds the gateway to the epoll instance in the kernel
		if(epoll_ctl(_epollFd, EPOLL_CTL_ADD, event.data.fd, &event) != 0)
			throw EPollErrorException("Failed to add a server event to epoll");
	}
	_events.resize(_maxConnections);
}

EventHandler::~EventHandler()
{
	//std::cout << "EventHandler default destructor called\n";
	safeClose();
}


// GETTERS

std::vector<epoll_event>	EventHandler::getEvents()
{ return (_events); }

epoll_event	EventHandler::getEvent(int index)
{
	if(index >= _eventsNumber || index < 0)
		throw EPollException("Index Out of bounds");
	return (_events.at(index));
}

int	EventHandler::getEventNumber()
{ return (_eventsNumber); }

int	EventHandler::getConnections()
{ return(_connections); }

// MEMBER FUNCTIONS

void	EventHandler::safeClose()
{
	if(_epollFd >= 0)
	{
		if(close(_epollFd) != 0)
			throw EPollErrorException("Failed to close an epoll instance");
		_epollFd = -1;
	}
}

void	EventHandler::addClient(int client_fd)
{
	// Adds a flag to the socket making it non-blocking
	fcntl(client_fd, F_SETFL, O_NONBLOCK);

	// Fills an epoll_event struct telling how the fd should be dealt with
	struct epoll_event event;
	event.events = EPOLLIN | EPOLLRDHUP | EPOLLET;
	event.data.fd = client_fd;

	// Adds the client to the epoll instance in the kernel
	if(epoll_ctl(_epollFd, EPOLL_CTL_ADD, client_fd, &event) != 0)
	{
		delete _clients[client_fd];
		_clients.erase(client_fd);
		throw EPollErrorException("Failed to add a client event to epoll");
	}

	// Fills a "Max Clients" response if the server is full
	if (_connections == _maxConnections)
		_clients[client_fd]->maxClientsResponse();
	else
		_connections++;
	
	std::cout << "The client " << client_fd << " connected\n";
}

void	EventHandler::removeClient(int client_fd)
{
	// Removes a client from the epoll instance in the Kernel
	if(epoll_ctl(_epollFd, EPOLL_CTL_DEL, client_fd, NULL) != 0)
		throw EPollErrorException("Failed to delete a client event from epoll");
}

void	EventHandler::deleteClient(int	client_fd)
{
	if (_clients[client_fd]->isConnected())
			_connections--;

	delete _clients[client_fd];
	_clients.erase(client_fd);
	std::cout << "The client " << client_fd << " disconnected\n";
}

void	EventHandler::modifyClient(int client_fd, uint32_t flags)
{
	// Fills an epoll_event struct telling how the fd should be dealt with
	struct epoll_event	event;
	event.events = flags;
	event.data.fd = client_fd;

	// Modifies the client in the epoll instance
	if(epoll_ctl(_epollFd, EPOLL_CTL_MOD, client_fd, &event) != 0)
		throw EPollErrorException("Failed to modify a client event in epoll");
}

void	EventHandler::waitEvents(int timeout)
{
	// Waits for events and fills "_events" with the ready ones
	_eventsNumber = epoll_wait(_epollFd, _events.data(), _events.size(), timeout);
	if(_eventsNumber == -1)
		throw EPollErrorException("Failed to wait for the events");
}

void	EventHandler::checkEvents()
{
	for(int i = 0; i < _eventsNumber; i++)
	{
		try
		{
			// Checks if the event comes from a server or a client and handles it
			if(_servers.count(_events[i].data.fd))
				handleServerEvent(_events[i].data.fd);
			else
				handleClientEvent(_events[i]);
		}
		catch(const std::exception& e)
		{
			std::cerr << e.what() << std::endl;
			if (!_servers.count(_events[i].data.fd))
				deleteClient(_events[i].data.fd);
		}
		
	}
}

void	EventHandler::handleServerEvent(int server_fd)
{
	// Accepts a new connection
	int					client_fd;
	struct sockaddr_in	client_addr;
	socklen_t			client_size = sizeof(client_addr);
	client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_size);
	if(client_fd == -1)
		throw EPollErrorException("Failed to accept a client connection");

	// Checks if the client is already connected
	if (_clients.count(client_fd))
	{
		std::cout << "The Client " << client_fd << " is already connected!" << std::endl;
		return ;
	}

	// Adds the new client to the map of clients and to epoll
	Client	*client = new Client(client_fd, *this);
	_clients[client_fd] = client;
	addClient(client_fd);
}

void	EventHandler::handleClientEvent(epoll_event& event)
{
	std::cout << "\nThe client " << event.data.fd << " triggered an event\n";
	
	// Checks if the event was triggered because of disconnection
	if (event.events & EPOLLRDHUP)
	{
		deleteClient(event.data.fd);
		return ;
	}
	// Checks if the client is ready to be read from
	else if (event.events & EPOLLIN)
	{
		_clients[event.data.fd]->setState(RECIEVING_REQUEST);
		std::cout << "The client " << event.data.fd << " is ready to be read from\n";
	}
	// Checks if the client is ready to write to
	else if (event.events & EPOLLOUT)
	{
		_clients[event.data.fd]->setState(SENDING_HEADER);
		std::cout << "The client " << event.data.fd << " is ready to write to\n";
	}
}

// EXCEPTIONS
EventHandler::EPollException::EPollException(std::string info) :
runtime_error(info){}

EventHandler::EPollErrorException::EPollErrorException(std::string info) :
runtime_error(info + " (" + std::string(strerror(errno)) + ")"){}
