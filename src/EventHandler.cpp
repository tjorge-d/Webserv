#include "../includes/EventHandler.hpp"

// CONSTRUCTORS & DESTRUCTORS
EventHandler::EventHandler(std::map<int, ServerBlock*> &server_blocks, std::map<int, Client*> &clients, int maxConnections) :
serverBlocks(server_blocks),
clients(clients),
connections(0),
maxConnections(maxConnections)
{
	//std::cout << "EventHandler custom constructor called" << std::endl;
	// Creates an epoll instance in the kernel
	epollFd = epoll_create(1);
	if(epollFd == -1)
		throw EPollErrorException("Failed to create an epoll instance in the kernel");
	
	// Adds the ports of each server block to events in order to accept client connections
	for(std::map<int, ServerBlock*>::iterator i = serverBlocks.begin(); i != serverBlocks.end(); i++)
	{
		// Fills an epoll_event struct to configure the event of the port
		struct epoll_event event;
		event.events = EPOLLIN;
		event.data.fd = i->second->getListenerFD();

		// Adds the gateway to the epoll instance in the kernel
		if(epoll_ctl(epollFd, EPOLL_CTL_ADD, event.data.fd, &event) != 0)
			throw EPollErrorException("Failed to add a server event to epoll");
	}
	events.resize(maxConnections);
}

EventHandler::~EventHandler()
{
	//std::cout << "EventHandler default destructor called" << std::endl;
	safeClose();
}


// GETTERS

int	EventHandler::getConnections()
{ return(connections); }

// MEMBER FUNCTIONS

void	EventHandler::safeClose()
{
	if(epollFd >= 0)
	{
		if(close(epollFd) != 0)
			throw EPollErrorException("Failed to close an epoll instance");
		epollFd = -1;
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
	if(epoll_ctl(epollFd, EPOLL_CTL_ADD, client_fd, &event) != 0)
	{
		delete clients[client_fd];
		clients.erase(client_fd);
		throw EPollErrorException("Failed to add a client event to epoll");
	}

	// Fills a "Max Clients" response if the server is full
	if (connections == maxConnections)
	{
		//HttpResponse response = clients[client_fd]->getResponse(); a horrible attempt at trying to return an error html
		//response.simpleHTTP("./var/www/dev/500.html");
		clients[client_fd]->setRequestStatus(SERVICE_UNAVAILABLE);
		std::cout << "THIS GO FUCKED UP -> " << std::endl;
		//clients[client_fd]->basicClientResponse(503);
		//clients[client_fd]->setConnection(false);
	}
	else
		connections++;
	
	std::cout << "The client " << client_fd << " connected" << std::endl;
}

void	EventHandler::removeClient(int client_fd)
{
	// Removes a client from the epoll instance in the Kernel
	if(epoll_ctl(epollFd, EPOLL_CTL_DEL, client_fd, NULL) != 0)
		throw EPollErrorException("Failed to delete a client event from epoll");
}

void	EventHandler::deleteClient(int	client_fd)
{
	if (clients[client_fd]->isConnected())
			connections--;

	delete clients[client_fd];
	clients.erase(client_fd);
	std::cout << "The client " << client_fd << " disconnected" << std::endl;
}

void	EventHandler::modifyClient(int client_fd, uint32_t flags)
{
	// Fills an epoll_event struct telling how the fd should be dealt with
	struct epoll_event	event;
	event.events = flags;
	event.data.fd = client_fd;

	// Modifies the client in the epoll instance
	if(epoll_ctl(epollFd, EPOLL_CTL_MOD, client_fd, &event) != 0)
		throw EPollErrorException("Failed to modify a client event in epoll");
}

void	EventHandler::waitEvents(int timeout)
{
	// Waits for events and fills "_events" with the ready ones
	eventsNumber = epoll_wait(epollFd, events.data(), events.size(), timeout);
	if(eventsNumber == -1)
		throw EPollErrorException("Failed to wait for the events");
}

void	EventHandler::checkEvents()
{
	for(int i = 0; i < eventsNumber; i++)
	{
		try
		{
			// Checks if the event comes from a server or a client and handles it
			if(serverBlocks.count(events[i].data.fd))
				handleServerEvent(events[i].data.fd);
			else
				handleClientEvent(events[i]);
		}
		catch(const std::exception& e)
		{
			std::cerr << e.what() << std::endl;
			if (!serverBlocks.count(events[i].data.fd))
				deleteClient(events[i].data.fd);
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
	if (clients.count(client_fd))
	{
		std::cout << "The Client " << client_fd << " is already connected!" << std::endl;
		return ;
	}

	// Adds the new client to the map of clients and to epoll
	Client	*client = new Client(client_fd, *this, *serverBlocks[server_fd]);
	clients[client_fd] = client;
	addClient(client_fd);
}

void	EventHandler::handleClientEvent(epoll_event& event)
{
	std::cout << "\nThe client " << event.data.fd << " triggered an event" << std::endl;
	
	// Checks if the event was triggered because of disconnection
	if (event.events & EPOLLRDHUP)
	{
		deleteClient(event.data.fd);
		return ;
	}
	// Checks if the client is ready to be read from
	else if (event.events & EPOLLIN)
	{
		std::cout << std::endl << "Client " << event.data.fd << " (Recieving)" << std::endl;
		clients[event.data.fd]->setState(RECIEVING_REQUEST);
		std::cout << "The client " << event.data.fd << " is ready to be read from" << std::endl;
	}
	// Checks if the client is ready to write to
	else if (event.events & EPOLLOUT)
	{
		std::cout << std::endl << "Client " << event.data.fd << " (Sending Header)" << std::endl;
		clients[event.data.fd]->setState(SENDING_HEADER);
		std::cout << "The client " << event.data.fd << " is ready to write to" << std::endl;
	}
}

// EXCEPTIONS
EventHandler::EPollException::EPollException(std::string info) :
runtime_error(info){}

EventHandler::EPollErrorException::EPollErrorException(std::string info) :
runtime_error(info + " (" + std::string(strerror(errno)) + ")"){}
