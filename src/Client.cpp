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
	recievingHeader = false;

	// Convert the request vector to a string
    std::string requestString(request.buffer.begin(), request.buffer.end());

    // Find the first line (request line)
    std::istringstream requestStream(requestString);
    std::string requestLine;
    if (!std::getline(requestStream, requestLine) || requestLine.empty()){
		throw ClientErrorException("Malformed request line", fd);
	}

    // Split the request line into components
    std::istringstream lineStream(requestLine);
    lineStream >> request.method >> request.path >> request.version;

	std::cout << requestString << std::endl;

	if (request.version != "HTTP/1.1"){
		throw ClientErrorException("Unsupported HTTP version", fd);
	}

	std::cout << "The request header is valid" << std::endl;

	// Parse header into a comprehensible map with all its information, might be annoying because
	// all the inforamtion is not clearly declared in a variable in the class but this way any kind
	// of header with any kind of information can be parsed all the same
	while (std::getline(requestStream, requestLine) && requestLine != "\r") {
		if (!requestLine.empty() && requestLine[requestLine.size() - 1] == '\r'){
			requestLine.erase(requestLine.size() - 1);
		}

		std::cout << "Testing -> " <<requestLine << std::endl;

		std::size_t colon = requestLine.find(':');
		if (colon == std::string::npos)
			continue;

		std::string key = requestLine.substr(0, colon);
		std::string value = requestLine.substr(colon + 1);

		key = trim(key);
		value = trim(value);

		request.headerInfo[key] = value;

		if (key == "Content-Length")
			request.contentLenght = std::atoi(value.c_str());
		else if (key == "Transfer-Encoding" && value == "chunked")
			request.isChunked = true;
	}

	// Should replace setConnection function called in simpleHTTP
	if (request.headerInfo.count("Connection") && request.headerInfo["Connection"] == "close")
		response.connection = "close";
	else
		response.connection = "keep-alive";

	// Clean request buffer
	request.eraseBufferRange(request.buffer.begin(), header_end);

	std::cout << "Parsed method: " << request.method << std::endl;
	std::cout << "Parsed path: " << request.path << std::endl;

	handleMethod();
}

void	Client::handleMethod()
{
    if (request.method == "GET" || request.method == "OPTIONS" || request.method == "TRACE"){
		response.simpleHTTP("./var/www/dev" + request.path);
	}
	else if(request.method == "POST" || request.method == "PUT")
	{
		recievingBody = true;
		if (!request.isChunked){
			request.bodySize = request.buffer.size();
			postFile.open("./var/www/sussy_files/file", std::ios::out);
			postFile.write(request.buffer.data(), request.bodySize);
		}
	}
	else if (request.method == "DELETE") {
		if (std::remove(("./var/www/dev" + request.path).c_str()) == 0){
			response.status = "200 OK";
			//Need to revise simpleHTTP function because of response status
			response.simpleHTTP("./var/www/dev/delete_success.html");
		}
		else{
			response.status = "404 Not Found";
			//Need specific html for delete fail
		}
	}
	else if (request.method == "HEAD") {
		response.simpleHTTP("./var/www/dev" + request.path);
		response.contentLenght = 0;
	}
	else {
		throw ClientErrorException("Unsupported HTTP method: " + request.method, fd);
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

	// Apppends the filled buffer to _request
	if(recievingHeader)
		appendToRequest(buffer, bytes);

	// Writes the buffer content onto the POST method path
	if (recievingBody)
	{
		if (request.isChunked){
			request.chunkBuffer.append(buffer, bytes);
			resolveChunkedBody();
		}
		else{
			postFile.write(buffer, bytes);
			request.bodySize += bytes;
			if(request.bodySize > serverBlock.getMaxBodySize()){
				throw ClientException("The request body has reached the maximum size", fd);
			}
			if (request.bodySize >= request.contentLenght){
				recievingBody = false;
				postFile.close();
				response.simpleHTTP("./var/www/dev/parabens.html");
			}
		}
	}

	// Behaves accordingly in case of not having anything else to read
	if(bytes < CHUNK_SIZE || !bytes)
	{
		if(recievingHeader)
			throw ClientException("Incomplete request header", fd);
		if(recievingBody && !request.isChunked && request.bodySize < request.contentLenght){
				throw ClientException("Incomplete request body", fd);
		}
		if (!recievingBody || request.chunkedComplete)
			sendMode();
	}

	return (bytes);
}

void	Client::resolveChunkedBody(){
	while (true)
	{
		std::size_t pos = request.chunkBuffer.find("\r\n");
		if (pos == std::string::npos)
			break;

		std::string sizeLine = request.chunkBuffer.substr(0, pos);
		int chunkSize = 0;

		std::istringstream ss(sizeLine);
		ss >> std::hex >> chunkSize;
		if (ss.fail())
			throw ClientException("Malformed chunk size", fd);

		if (chunkSize == 0)
		{
			if (request.chunkBuffer.size() < pos + 4)
				break; // need more data
			request.chunkedComplete = true;
			break;
		}

		if (request.chunkBuffer.size() < pos + 2 + chunkSize + 2)
			break; // full chunk not yet received

		request.body += request.chunkBuffer.substr(pos + 2, chunkSize);

		request.chunkBuffer.erase(0, pos + 2 + chunkSize + 2);
	}

	if (request.chunkedComplete)
	{
		recievingBody = false;
		postFile.open("./var/www/sussy_files/file", std::ios::out);
		postFile.write(request.body.c_str(), static_cast<int>(request.body.size()));
		response.simpleHTTP("./var/www/dev/parabens.html");
	}
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