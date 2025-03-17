#include "../includes/EpollHandler.hpp"

// CONSTRUCTORS & DESTRUCTORS
EpollHandler::EpollHandler()
{
	//std::cout << "EpollHandler default constructor called\n";
}

EpollHandler::EpollHandler(int serverFd) : _serverFd(serverFd)
{
	//std::cout << "EpollHandler custom constructor called\n";
	// Creates an epoll instance in the kernel
	_epollFd = epoll_create(1);
	if (_epollFd == -1)
		throw EPollCreationFailure();
	
	// Fills an epoll_event struct telling how the server fd should be dealt with
	struct epoll_event event;
	event.events = EPOLLIN;
	event.data.fd = _serverFd;

	// Adds the server to the epoll instance in the kernel
	if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, _serverFd, &event) != 0)
		throw EPollCTLFailure("add");
}

EpollHandler::EpollHandler(const EpollHandler &copy)
{
	//std::cout << "EpollHandler copy constructor called\n";
	*this = copy;
}

EpollHandler::~EpollHandler()
{
	//std::cout << "EpollHandler default destructor called\n";
	closeHandler();
}

// GETTERS

std::vector<epoll_event>	EpollHandler::getEvents()
{
	return (_events);
}

epoll_event	EpollHandler::getEvent(int index)
{
	if (index >= _eventsNumber)
		throw EventOutOfBounds();
	return (_events.at(index));
}

// SETTERS


// OPERATORS

EpollHandler&	EpollHandler::operator=(const EpollHandler &copy)
{
	//std::cout << "EpollHandler copy assignment operator called\n";
	if (this != &copy)
	{
		closeHandler();
		_epollFd = copy._epollFd;
	}
	return (*this);
}

// MEMBER FUNCTIONS

void	EpollHandler::closeHandler()
{
	// Safely closes the epoll fd
	if (_epollFd >= 0)
	{
		if (close(_epollFd) != 0)
			throw EPollCloseFailure();
		_epollFd = -1;
	}
}

void	EpollHandler::addClient(int client_fd)
{
	// Adds a flag to the socket making it non-blocking
	fcntl(client_fd, F_SETFL, O_NONBLOCK);

	// Fills an epoll_event struct telling how the fd should be dealt with
	struct epoll_event event;
	event.events = EPOLLIN | EPOLLET;
	event.data.fd = client_fd;

	// Adds the client to the epoll instance in the kernel
	if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, client_fd, &event) != 0)
		throw EPollCTLFailure("add");
}

void	EpollHandler::removeClient(int client_fd)
{
	// Removes a client from the epoll instance in the Kernel
	if(epoll_ctl(_epollFd, EPOLL_CTL_DEL, client_fd, NULL) != 0)
		throw EPollCTLFailure("remove");
}

int		EpollHandler::waitForEvents(int timeout)
{
	// Waits for events and fills "_events" with the ready ones
	_eventsNumber = epoll_wait(_epollFd, _events.data(), _events.size(), timeout);
	if (_eventsNumber == -1)
		throw EPollWaitFailure();
}


// EXCEPTIONS
EpollHandler::EPollCreationFailure::EPollCreationFailure() :
runtime_error("Failed to create an epoll instance in the kernel"){}

EpollHandler::EPollCloseFailure::EPollCloseFailure() :
runtime_error("Failed to close an epoll instance"){}

EpollHandler::EPollCTLFailure::EPollCTLFailure(std::string info) :
runtime_error("Failed to " + info + " an event in the epoll instance"){}

EpollHandler::EPollWaitFailure::EPollWaitFailure() :
runtime_error("Failed to wait for events"){}

EpollHandler::EventOutOfBounds::EventOutOfBounds() :
runtime_error("The index provided is out of bounds"){}
