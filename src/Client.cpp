#include "../includes/Webserv.h"

// CONSTRUCTORS & DESTRUCTORS

Client::Client(int fd, EventHandler &events) :
_fd(fd),
_events(events),
_requestBodySize(0),
_response(),
_connected(1),
_recievingHeader(1),
_recievingBody(0),
_state(WAITING_TO_RECIEVE)
{
	std::cout << "Client constructor called\n";
}

Client::~Client()
{
	std::cout << "Client destructor called\n";
	closeClient();
}


// GETTERS

int	Client::getFD() const
{return (_fd);}

state	Client::getState() const
{return (_state);}

HttpResponse const	&Client::getResponse() const
{return (_response);}


// SETTERS

void	Client::setState(state state)
{_state = state;}


// MEMBER FUNCTIONS

bool	Client::isConnected() const
{return(_connected);}

void	Client::closeClient()
{
	// Safely closes the Client fd
	if(_fd >= 0)
	{
		if(close(_fd) == -1)
			throw ClientErrorException("Failed to close fd", _fd);
		_fd = -1;
	}
}

void	Client::recieveMode()
{
	_state = WAITING_TO_RECIEVE;
	_requestBodySize = 0;
	_recievingHeader = 1;
	_recievingBody = 0;

	_response.resetResponse();

	// Changes the client event to trigger when ready to be read from
	_events.modifyClient(_fd, EPOLLIN | EPOLLRDHUP | EPOLLET);
}

void	Client::sendMode()
{
	_state = WAITING_TO_SEND;
	_response.bytesSent = 0;

	// Changes the client event to trigger when ready to write to
	_events.modifyClient(_fd, EPOLLOUT | EPOLLRDHUP | EPOLLET);
}

void	Client::maxClientsResponse()
{
	std::string str = "Por favor tente mais tarde.";
	int			size = str.size();
	std::stringstream s;
	s << size;
	std::string n = s.str();

	int	seconds = 10;
	std::stringstream sec;
	sec << seconds;

	std::string response;
	response += "HTTP/1.1 503 Service Unavailable\r\n"
	"Content-Type: text/plain\r\n"
	"Content-Length: " + n + "\r\n"
	"Retry-After: " + sec.str() + "\r\n"
	"Connection: close\r\n"
	"\r\n" + str;

    // Convert the string to a vector<char>
    _response.header = std::vector<char>(response.begin(), response.end());
	_response.headerSize = _response.header.size();
	_response.contentLenght = 0;
	_connected = false;
	sendMode();
}

void	Client::parseRequestHeader(std::vector<char>::iterator header_end)
{

	// PARSES THE REQUEST -> todo

	// INVALID HEADER CONDITION -> todo
	if(0)
	{}

	_recievingHeader = false;

	// REQUEST METHOD THAT NEEDS A BODY CONDITION -> todo
	if(0)
		_recievingBody = true;

	std::cout << "The request header is valid" << std::endl;
	_response.simpleHTTP(getPath());
	_request.erase(_request.begin(), header_end);
	_request.shrink_to_fit();

	_requestBodySize = _request.size();
}

int	Client::recieveRequestChunk()
{
	// Protects the client from recieving if not necessary
	if(_state != RECIEVING_REQUEST && _state != CLEANING_INVALID_REQUEST)
	throw ClientException("Invalid client state to call recieveResponseChunk()", _fd);

	// Sets and fills a buffer with data from the client fd
	char	buffer[CHUNK_SIZE];
	int		bytes = recv(_fd, buffer, CHUNK_SIZE, 0);
	if(bytes == -1)
		throw ClientException("Failed to recieve a request", _fd);

	// Cheeks the booty side
	if (_recievingBody)
	{
		_requestBodySize += bytes;
		if(_requestBodySize > MAX_BODY)
			throw ClientException("The request body has reached the maximum size", _fd);
	}

	// Apppends the filled buffer to _request
	appendToRequest(buffer, bytes);

	// Behaves accordingly in case of not having anything else to read
	if(bytes < CHUNK_SIZE || !bytes)
	{
		if(_recievingHeader)
			throw ClientException("Incomplete request header", _fd);
		if(_recievingBody){}
			//parseRequestBody(_request.end());
	}

	return (bytes);
}

// ERASE LATER (FOR TESTING)
std::string	Client::getPath()
{
    // Convert the request vector to a string
    std::string requestString(_request.begin(), _request.end());

    // Find the first line (request line)
    std::istringstream requestStream(requestString);
    std::string requestLine;
    std::getline(requestStream, requestLine);

    // Split the request line into components
    std::istringstream lineStream(requestLine);
    std::string method, path, version;
    lineStream >> method >> path >> version;

    // Ensure the method is GET

	std::cout << requestString << std::endl;
    return ("./var/www/dev" + path);
}

void	Client::appendToRequest(char* str, int size)
{
	// Inserts the string given as an argument at the end of _request
	_request.insert(_request.end(), str, str + size);

	if (_recievingHeader)
	{
		// Checks if the read data contains the end of a request header
		int offset = size + 4 * (static_cast<int>(_request.size()) >= size + 4);
		std::vector<char>::iterator it;
		it = std::search(_request.end() - offset, _request.end(), \
			"\r\n\r\n", &"\r\n\r\n"[4]);
		
		// Parses the header if found
		if (it != _request.end())
		{
			std::cout << "Request header found" << std::endl;
			if (_state == CLEANING_INVALID_REQUEST)
			{
				// Finds the beggining of the Header and cleans what's behind
				//cleanInvalidRequest(findRequestBegin());
				_state = RECIEVING_REQUEST;
			}
			parseRequestHeader(it + 4);
			if (!_recievingBody)
				sendMode();
		}
	}
}

void	Client::sendHeaderChunk()
{
	// Protects the client from sending if not necessary
	if (_state != SENDING_HEADER)
		throw ClientException("Invalid client state to call sendHeaderChunk()", _fd);

	// Protects the function from sending invalid memory
	int	chunk_size = CHUNK_SIZE;
	if (CHUNK_SIZE + _response.bytesSent > _response.headerSize)
		chunk_size = _response.headerSize - _response.bytesSent;

	// Sends the read chunk to the client
	int	bytes = send(_fd, &_response.header[_response.bytesSent], chunk_size, 0);
	if (bytes == -1)
		throw ClientException("Failed to send a response", _fd);
	_response.bytesSent += bytes;

	// Changes the state of the client when necessary
	if (_response.bytesSent == _response.headerSize)
		_state = SENDING_BODY;
	if (!_response.contentLenght)
		recieveMode();
}

void	Client::sendBodyChunk()
{
	// Protects the client from sending if not necessary
	if (_state != SENDING_BODY)
		throw ClientException("Invalid client state to call sendBodyChunk()", _fd);

	// Reads from the file stream storing the bytes read
	char	buffer[CHUNK_SIZE];
	_response.fileStream.read(buffer, CHUNK_SIZE);
    std::streamsize bytes = _response.fileStream.gcount();
	// fail() will trigger if the eof is found, therefore we ignore it if the eof is found
	if (_response.fileStream.fail() && !_response.fileStream.eof())
        throw ClientException("Failed to read from file", _fd);

	// Sends the read chunk to the client
	if (send(_fd, buffer, bytes, 0) == -1)
		throw ClientException("Failed to send a response", _fd);
	_response.bytesSent += bytes;

	// Resets the response status of the client when over
	if (_response.bytesSent == _response.contentLenght + _response.headerSize)
		recieveMode();
}


// EXCEPTIONS
Client::ClientException::ClientException(std::string info, int fd) :
runtime_error(createMessage(info, fd)){}
std::string Client::ClientException::createMessage(std::string info, int fd)
{
	std::ostringstream oss;
	oss << info << " (Client " << fd << ": " + std::string(strerror(errno)) + ")";
	return oss.str();
}

Client::ClientErrorException::ClientErrorException(std::string info, int fd) :
runtime_error(createMessage(info, fd)){}
std::string Client::ClientErrorException::createMessage(std::string info, int fd)
{
	std::ostringstream oss;
	oss << info << " (Client " << fd << ")";
	return oss.str();
}