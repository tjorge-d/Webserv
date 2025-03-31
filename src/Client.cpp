#include "../includes/Client.hpp"

// CONSTRUCTORS & DESTRUCTORS

Client::Client(int fd) : 
_fd(fd),
_connected(true),
_pendingRequests(0),
_chunksNumber(0),
_waitingHeader(0),
_waitingBody(0),
_state(WAITING)
{
	std::cout << "Client constructor called\n";
}

Client::~Client()
{
	std::cout << "Client destructor called\n";
	closeClient();
}


// GETTERS

int	Client::getFD()
{
	return (_fd);
}

state	Client::getState()
{
	return (_state);
}

std::vector<char>	Client::getResponse()
{
	return (_response);
}


// MEMBER FUNCTIONS

void	Client::closeClient()
{
	// Safely closes the Client fd
	if(_fd >= 0)
	{
		if(close(_fd) == -1)
			throw ClientCloseFailure();
		_fd = -1;
	}
}

void	Client::newRequest()
{
	if (_state == WAITING)
	{
		_state = RECIEVING_REQUEST;
		_waitingHeader = 1;
		_waitingBody = 0;
	}
	_pendingRequests++;
	std::cout << "Added a new request to the client " << _fd
	<< "\nTotal requests: " << _pendingRequests << "\n";
}

void	Client::sendMode()
{
	_state = SENDING_RESPONSE;
	_bytesSent = 0;
}

void	Client::maxClientsResponse()
{
	std::string str = "Daqui fala a Marta da teleseguros, por favor tente mais tarde.";
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
    _response = std::vector<char>(response.begin(), response.end());
	_responseSize = _response.size();
	_connected = 0;
	_pendingRequests++;
}

void	Client::appendToRequest(char* str, int size)
{
	// Inserts the string given as an argument at the end of _request
	_request.insert(_request.end(), str, str + size);

	if (_waitingHeader)
	{
		// Checks if the read data contains the end of a request header
		int offset = size + 4 * (_chunksNumber != 0);
		std::vector<char>::iterator it;
		it = std::search(_request.end() - offset, _request.end(), \
			"\r\n\r\n", &"\r\n\r\n"[4]);
		
		// Parses the header if found
		if (it != _request.end())
		{
			// Finds the beggining of the Header and cleans what's behind
			if (_state == CLEANING_INVALID_REQUEST)
			{
				//cleanInvalidRequest(findRequestBegin());
				_state = RECIEVING_REQUEST;
			}			
			parseRequestHeader(it + 4);
			if (!_waitingBody)
				sendMode();
		}
	}
}

void	Client::parseRequestHeader(std::vector<char>::iterator header_end)
{
	std::string str = "BOAAAAAS PESSOAL!";
	int			size = str.size();
	std::stringstream s;
	s << size;
	std::string n = s.str();

	std::string response;
	response += "HTTP/1.1 200 OK\r\n"
	"Content-Type: text/plain\r\n"
	"Content-Length: " + n + "\r\n"
	"Connection: keep-alive\r\n"
	"\r\n" + str;

	// Convert the string to a vector<char>
	_response = std::vector<char>(response.begin(), response.end());
	_responseSize = _response.size();

	// PARSES THE REQUEST -> todo

	// INVALID HEADER CONDITION -> todo
	if (0)
	{}

	_waitingHeader = false;

	// REQUEST METHOD THAT NEEDS A BODY CONDITION -> todo
	if (0)
		_waitingBody = true;

	_request.erase(_request.begin(), header_end);
	_request.shrink_to_fit();
}

int	Client::recieveRequestChunk(int chunk_size)
{
	// Protects the client from recieving if not necessary
	if (_state != RECIEVING_REQUEST && _state != CLEANING_INVALID_REQUEST)
		return (-1);

	// Sets and fills a buffer with data from the client _fd
	char	buffer[chunk_size];
	int		bytes = recv(_fd, buffer, chunk_size, 0);
	if(bytes == -1)
		throw RecieveFailure();

	// Apppends the filled buffer to _request
	appendToRequest(buffer, bytes);
	_chunksNumber++;

	// Behaves accordingly in case of not having anything else to read
	if(bytes < chunk_size || !bytes)
	{
		if(_waitingHeader){}
			// throw InvalidHeader()
		if(_waitingBody && _bytesSent != _responseSize){}
			//parseRequestBody(_request.end());
	}

	return (bytes);
}

int	Client::sendResponseChunk(int chunk_size)
{
	// Protects the client from sending if not necessary
	if (_state != SENDING_RESPONSE)
		return (-1);

	// Prevents sending invalid memory
	if (chunk_size + _bytesSent > _responseSize)
		chunk_size = _responseSize - _bytesSent;

	// Sends the response in chunks
	int	bytes = send(_fd, &_response[_bytesSent], chunk_size, 0);
	if (bytes == -1)
		throw SendFailure();
	_bytesSent += bytes;

	// Resets the response status of the client when over
	if (_bytesSent == _responseSize)
	{
		_pendingRequests--;
		_response.clear();
		_response.shrink_to_fit();
		_responseSize = 0; // Remove later after tests
		_waitingBody = 0;
		if (_pendingRequests)
			_state = RECIEVING_REQUEST;
		else
		{
			_request.clear();
			_request.shrink_to_fit();
			_chunksNumber = 0;
			_state = WAITING;
		}
	}

	return (bytes);
}


// EXCEPTIONS
Client::ClientCloseFailure::ClientCloseFailure() :
runtime_error("A Client failed to close"){}

Client::RecieveFailure::RecieveFailure() :
runtime_error("Failed to recieve data from a client"){}

Client::SendFailure::SendFailure() :
runtime_error("Failed to send data to a client"){}