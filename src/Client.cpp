#include "../includes/Client.hpp"

// CONSTRUCTORS & DESTRUCTORS

Client::Client(int fd) : 
_fd(fd),
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
}

void	Client::appendToRequest(char* str, int size)
{
	// Inserts the string given as an argument at the end of _request
	_request.insert(_request.end(), str, str + size);

	if (_waitingHeader)
	{
		// Checks if the read data contains the end of a request header
		int off_set = size + 4 * (_chunksNumber != 0);
		std::vector<char>::iterator it;
		it = std::search(_request.end() - off_set, _request.end(), \
			"\r\n\r\n", &"\r\n\r\n"[4]);

		// Finds the beggining of the Header and cleans what's behind
		if (_state == CLEANING_INVALID_REQUEST)
		{
			//cleanInvalidRequest(findRequestBegin());
			_state = RECIEVING_REQUEST;
		}

		// Parses the header if found
		if (it != _request.end())
			parseRequestHeader(it + 4);
	}
}

void	Client::parseRequestHeader(std::vector<char>::iterator header_end)
{
	// Parses the request
	_response = _request;
	_request.clear();
	_responseSize = _response.size();
	_bytesSent = 0;

	// INVALID HEADER CONDITION
	if (0)
	{
		// Erases the invalid header from _request
		std::vector<char>::iterator it;
		it = std::search(_request.begin(), _request.end(), \
			"\r\n\r\n", &"\r\n\r\n"[4]);
		_pendingRequests--;
		//	throw InvalidHeader()
	}
	_waitingHeader = false;
	_request.erase(_request.begin(), header_end);
}

int	Client::recieveRequestChunk(int chunk_size)
{
	// Protects the client from recieving if not necessary
	if (_state != RECIEVING_REQUEST && _state != CLEANING_INVALID_REQUEST)
		return (-1);

	// Sets and fills a buffer with data from the client _fd
	char	buffer[chunk_size];
	int		bytes = recv(_fd, buffer, chunk_size, 0);
	//if(bytes == -1)
	//	throw RecieveFailure();
	std::cout << "bytes: " << bytes << " bytes\n";

	// Apppends the filled buffer to _request
	appendToRequest(buffer, bytes);
	_chunksNumber++;

	// Behaves accordingly in case of not having anything else to read
	if(bytes < chunk_size || !bytes)
	{
		// Initializing SENDING_RESPONSE mode (should be done after a valid header parsing)
		_state = SENDING_RESPONSE;
		_response = _request;
		_request.clear();
		_responseSize = _response.size();
		_bytesSent = 0;
		_chunksNumber = 0;
		///////////////////////////////////

		if(_waitingHeader){}
			// throw InvalidHeader()
		if(_waitingBody) {}
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
	//if (bytes == -1)
	//throw SendFailure();
	_bytesSent += bytes;
	std::cout << "Sending chunk: " << chunk_size << " bytes\n";
	std::cout << "Bytes sent so far: " << _bytesSent << "\n";
	std::cout << "Response size: " << _responseSize << "\n";

	// Resets the response status of the client when over
	if (_bytesSent == _responseSize)
	{
		_pendingRequests--;
		_response.clear();
		_bytesSent = 0;
		if (_pendingRequests)
			_state = RECIEVING_REQUEST;
		else
			_state = WAITING;
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