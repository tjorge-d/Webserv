#include "../includes/Client.hpp"

// CONSTRUCTORS & DESTRUCTORS

Client::Client(int fd, EventHandler &events, ServerBlock &serverBlock): 
fd(fd),
events(events),
serverBlock(serverBlock),
request(),
response(),
connected(1),
recievingHeader(1),
recievingBody(0),
state(WAITING_TO_RECIEVE)
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
{return (fd);}

client_state	Client::getState() const
{return (state);}

HttpResponse const	&Client::getResponse() const
{return (response);}


// SETTERS

void	Client::setConnection(bool connection)
{this->connected = connection;}

void	Client::setState(client_state state)
{this->state = state;}


// MEMBER FUNCTIONS

bool	Client::isConnected() const
{return(connected);}

void	Client::closeClient()
{
	// Safely closes the Client fd
	if(fd >= 0)
	{
		if(close(fd) == -1)
			throw ClientErrorException("Failed to close fd", fd);
		fd = -1;
	}
}

void	Client::recieveMode()
{
	// When an unconnected client finishes its loop prevents him from starting a new one
	if (!connected)
	{
		state = DONE;
		return;
	}

	// Resets the client attributes to a recieving starting point
	state = WAITING_TO_RECIEVE;
	recievingHeader = 1;
	recievingBody = 0;
	request.reset();
	response.reset();

	// Changes the client event to trigger when ready to be read from
	events.modifyClient(fd, EPOLLIN | EPOLLRDHUP | EPOLLET);
}

void	Client::sendMode()
{
	state = WAITING_TO_SEND;
	response.bytesSent = 0;

	// Changes the client event to trigger when ready to write to
	events.modifyClient(fd, EPOLLOUT | EPOLLRDHUP | EPOLLET);
}

void	Client::basicClientResponse(std::string msg, std::string status)
{
	int			size = msg.size();
	std::stringstream s;
	s << size;
	std::string n = s.str();

	int	seconds = 10;
	std::stringstream sec;
	sec << seconds;

	std::string response_str;
	response_str += "HTTP/1.1 " + status + "\r\n"
	"Content-Type: text/plain\r\n"
	"Content-Length: " + n + "\r\n"
	"Retry-After: " + sec.str() + "\r\n"
	"Connection: close\r\n"
	"\r\n" + msg;

    // Convert the string to a vector<char>
    response.header = std::vector<char>(response_str.begin(), response_str.end());
	response.headerSize = response.header.size();
	response.contentLenght = 0;
	sendMode();
}

void	Client::parseRequestHeader(std::vector<char>::iterator header_end)
{
	//PARSES THE REQUEST -> todo
	//	Information needed:
	//		_request.method
	//		_request.path
	//		_request.contentLenght
	//		_response.status
	//		_response.contentType
	//		_response.contentLenght;
	//		_response.connection;
	//		_response.headerSize; >> After building the header to be sent
	//		_response.filePath;
	//		_response.fileStats;
	//		_response.fileStream;
	
	// INVALID HEADER CONDITION -> todo
	if(0)
	{
		
	}
	recievingHeader = false;

	std::cout << "The request header is valid" << std::endl;

	// Fetches the header information and cleans the parsed header from the buffer
	fetchRequestInfo();
	request.eraseBufferRange(request.buffer.begin(), header_end);

	if (request.method == "GET")
		response.simpleHTTP("./var/www/dev" + request.path);
	else if(request.method == "POST")
	{
		recievingBody = true;
		request.bodySize = request.buffer.size();
		response.simpleHTTP("./var/www/dev/parabens.html");
		// parsePost();
		postFile.open("./var/www/sussy_files/file", std::ios::out);
		postFile.write(request.buffer.data(), request.bodySize);
	}
	std::cout << "Body Size: " << request.bodySize << std::endl;
}

int	Client::recieveRequestChunk()
{
	// Protects the client from recieving if not necessary
	if(state != RECIEVING_REQUEST)
	throw ClientException("Invalid client state to call recieveResponseChunk()", fd);

	// Stores the data from the client fd in a buffer
	char	buffer[CHUNK_SIZE];
	int		bytes = recv(fd, buffer, CHUNK_SIZE, 0);
	if(bytes == -1)
		throw ClientException("Failed to recieve a request", fd);

	// Writes the buffer content onto the POST method path
	if (recievingBody)
	{
		postFile.write(buffer, bytes);
		request.bodySize += bytes;
		if(request.bodySize > serverBlock.getMaxBodySize())
		{

			// ErrorResponse((int)error, (string)msg);
			// maxBodySizeResponse();
			throw ClientException("The request body has reached the maximum size", fd);
		}
	}

	// Apppends the filled buffer to _request
	if(recievingHeader)
		appendToRequest(buffer, bytes);

	// Behaves accordingly in case of not having anything else to read
	if(bytes < CHUNK_SIZE || !bytes)
	{
		if(recievingHeader)
			throw ClientException("Incomplete request header", fd);
		if(recievingBody)
		{
			postFile.close();
			if (request.bodySize < request.contentLenght)
				throw ClientException("Incomplete request body", fd);
		}
		sendMode();
	}

	return (bytes);
}

// ERASE LATER (FOR TESTING)
void	Client::fetchRequestInfo()
{
    // Convert the request vector to a string
    std::string requestString(request.buffer.begin(), request.buffer.end());

    // Find the first line (request line)
    std::istringstream requestStream(requestString);
    std::string requestLine;
    std::getline(requestStream, requestLine);

    // Split the request line into components
    std::istringstream lineStream(requestLine);
    std::string method, path, version;
    lineStream >> method >> path >> version;

	request.method = method;
	request.path = path;
	std::cout << requestString << std::endl;
}

void	Client::appendToRequest(char* buffer, int size)
{
	// Appends the buffer given as an argument to the request
	request.appendToBuffer(buffer, size);

	if (recievingHeader)
	{
		// Checks if the read data contains the end of a request header
		std::vector<char>::iterator it = request.findHeader(size);

		// Parses the header if found
		if (it != request.buffer.end())
		{
			std::cout << "Request header found" << std::endl;
			parseRequestHeader(it + 4);
		}
	}
}

void	Client::sendHeaderChunk()
{
	// Protects the client from sending if not necessary
	if (state != SENDING_HEADER)
		throw ClientException("Invalid client state to call sendHeaderChunk()", fd);

	// Protects the function from sending invalid memory
	int	chunk_size = CHUNK_SIZE;
	if (CHUNK_SIZE + response.bytesSent > response.headerSize)
		chunk_size = response.headerSize - response.bytesSent;

	// Sends the read chunk to the client
	int	bytes = send(fd, &response.header[response.bytesSent], chunk_size, 0);
	if (bytes == -1)
		throw ClientException("Failed to send a response", fd);
	response.bytesSent += bytes;

	// Changes the state of the client when necessary
	if (response.bytesSent == response.headerSize)
		state = SENDING_BODY;
	if (!response.contentLenght)
		recieveMode();
}

void	Client::sendBodyChunk()
{
	// Protects the client from sending if not necessary
	if (state != SENDING_BODY)
		throw ClientException("Invalid client state to call sendBodyChunk()", fd);

	// Reads from the file stream storing the bytes read
	char	buffer[CHUNK_SIZE];
	response.fileStream.read(buffer, CHUNK_SIZE);
    std::streamsize bytes = response.fileStream.gcount();
	// fail() will trigger if the eof is found, therefore we ignore it if the eof is found
	if (response.fileStream.fail() && !response.fileStream.eof())
        throw ClientException("Failed to read from file", fd);

	// Sends the read chunk to the client
	if (send(fd, buffer, bytes, 0) == -1)
		throw ClientException("Failed to send a response", fd);
	response.bytesSent += bytes;

	// Resets the response status of the client when over
	if (response.bytesSent == response.contentLenght + response.headerSize)
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