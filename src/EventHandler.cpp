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
	failsafe_error_codes["200"] = "200 OK"; //The request succeeded.
	failsafe_error_codes["204"] = "204 No Content"; //There is no content to send for this request, but the headers are useful. 
	failsafe_error_codes["301"] = "301 Moved Permanently"; //The URL of the requested resource has been changed permanently. The new URL is given in the response (necessary?)
	failsafe_error_codes["303"] = "303 See Other"; //The server sent this response to direct the client to get the requested resource at another URI with a GET request.
	failsafe_error_codes["308"] = "308 Permanent Redirect"; //This means that the resource is now permanently located at another URI, specified by the Location response header.
	//This has the same semantics as the 301 Moved Permanently HTTP response code, with the exception that the user agent must not change the HTTP method used: if a POST was used in the first request, a POST must be used in the second request.
	failsafe_error_codes["400"] = "400 Bad Request"; //The server cannot or will not process the request due to something that is perceived to be a client error (e.g., malformed request syntax, invalid request message framing, or deceptive request routing).
	failsafe_error_codes["403"] = "403 Forbidden"; //The client does not have access rights to the content; that is, it is unauthorized, so the server is refusing to give the requested resource. Unlike 401 Unauthorized, the client's identity is known to the server.
	failsafe_error_codes["404"] = "404 Not Found"; //The server cannot find the requested resource. In the browser, this means the URL is not recognized. In an API, this can also mean that the endpoint is valid but the resource itself does not exist.
	failsafe_error_codes["405"] = "405 Method Not Allowed"; //The request method is known by the server but is not supported by the target resource. For example, an API may not allow DELETE on a resource, or the TRACE method entirely.
	failsafe_error_codes["408"] = "408 Request Timeout"; //This response is sent on an idle connection by some servers, even without any previous request by the client. It means that the server would like to shut down this unused connection.
	failsafe_error_codes["409"] = "409 Conflict"; //This response is sent when a request conflicts with the current state of the server. 
	failsafe_error_codes["411"] = "411 Length Required"; //Server rejected the request because the Content-Length header field is not defined and the server requires it.
	failsafe_error_codes["413"] = "413 Content Too Large"; //The request body is larger than limits defined by server. The server might close the connection or return an Retry-After header field.
	failsafe_error_codes["414"] = "414 URI Too Large"; //The URI requested by the client is longer than the server is willing to interpret.
	failsafe_error_codes["429"] = "429 Too Many Requests"; //The user has sent too many requests in a given amount of time (rate limiting).
	failsafe_error_codes["431"] = "431 Request Header Fields Too Large"; //The server is unwilling to process the request because its header fields are too large.
	failsafe_error_codes["500"] = "500 Internal Server Error"; //The server has encountered a situation it does not know how to handle. This error is generic, indicating that the server cannot find a more appropriate 5XX status code to respond with.
	failsafe_error_codes["503"] = "503 Service Unavailable"; //The server is not ready to handle the request.
	failsafe_error_codes["504"] = "504 Gateway Timeout"; //This error response is given when the server is acting as a gateway and cannot get a response in time.
	failsafe_error_codes["505"] = "505 HTTP Version Not Supported"; //The HTTP version used in the request is not supported by the server.
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
		clients[client_fd]->basicClientResponse("Please try again later.", failsafe_error_codes["503"]);
		clients[client_fd]->setConnection(false);
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
		clients[event.data.fd]->setState(RECIEVING_REQUEST);
		std::cout << "The client " << event.data.fd << " is ready to be read from" << std::endl;
	}
	// Checks if the client is ready to write to
	else if (event.events & EPOLLOUT)
	{
		clients[event.data.fd]->setState(SENDING_HEADER);
		std::cout << "The client " << event.data.fd << " is ready to write to" << std::endl;
	}
}

// EXCEPTIONS
EventHandler::EPollException::EPollException(std::string info) :
runtime_error(info){}

EventHandler::EPollErrorException::EPollErrorException(std::string info) :
runtime_error(info + " (" + std::string(strerror(errno)) + ")"){}
