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
	failsafe_error_codes["429"] = "429 Too Many Requests"; //The user has sent too many requests in a given amount of time (rate limiting).
	failsafe_error_codes["431"] = "431 Request Header Fields Too Large"; //The server is unwilling to process the request because its header fields are too large.
	failsafe_error_codes["500"] = "500 Internal Server Error"; //The server has encountered a situation it does not know how to handle. This error is generic, indicating that the server cannot find a more appropriate 5XX status code to respond with.
	failsafe_error_codes["503"] = "503 Service Unavailable"; //The server is not ready to handle the request.
	failsafe_error_codes["504"] = "504 Gateway Timeout"; //This error response is given when the server is acting as a gateway and cannot get a response in time.
	failsafe_error_codes["505"] = "505 HTTP Version Not Supported"; //The HTTP version used in the request is not supported by the server.
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
	recievingHeader = true;
	recievingBody = false;
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
		basicClientResponse("Bad request.", failsafe_error_codes["400"]);
		setConnection(false);
	}

    // Split the request line into components
    std::istringstream lineStream(requestLine);
    lineStream >> request.method >> request.path >> request.version;

	if (request.version != "HTTP/1.1"){
		basicClientResponse("Unsupported HTTP version.", failsafe_error_codes["505"]);
		setConnection(false);
	}

	std::cout << "The request header is valid" << std::endl;

	// Parse header into a comprehensible map with all its information, might be annoying because
	// all the inforamtion is not clearly declared in a variable in the class but this way any kind
	// of header with any kind of information can be parsed all the same
	while (std::getline(requestStream, requestLine) && requestLine != "\r") {
		if (!requestLine.empty() && requestLine[requestLine.size() - 1] == '\r'){
			requestLine.erase(requestLine.size() - 1);
		}

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
		else if (key == "Transfer-Encoding" && value == "chunked"){
			request.isChunked = true;
		}
	}

	// Should replace setConnection function called in simpleHTTP
	if (request.headerInfo.count("Connection") && request.headerInfo["Connection"] == "close")
		response.connection = "close";
	else
		response.connection = "keep-alive";

	// Clean request buffer
	request.eraseBufferRange(request.buffer.begin(), header_end);

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
		request.bodySize = request.buffer.size();
		//if (!request.isChunked){
		//	request.bodySize = request.buffer.size();
		//	postFile.open("./var/www/sussy_files/file", std::ios::out);
		//	postFile.write(request.buffer.data(), request.bodySize);
		//}
	}
	else if (request.method == "DELETE") {
		if (std::remove(("./var/www/dev" + request.path).c_str()) == 0){
			response.status = "200 OK";
			//Need to revise simpleHTTP function because of response status
			response.simpleHTTP("./var/www/dev/delete_success.html");
		}
		else{
			response.status = "500 Internal Server Error."; //404 is only used for invalid HTMLs, not for failed deletes.
		}
	}
	else if (request.method == "HEAD") {
		response.simpleHTTP("./var/www/dev" + request.path);
		response.contentLenght = 0;
	}
	else {
		basicClientResponse("Method not allowed.", failsafe_error_codes["405"]);
		setConnection(false);
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
	if(bytes == -1 && errno != EAGAIN && errno != EWOULDBLOCK){
		throw ClientException("Failed to recieve a request", fd);
	}

	// Apppends the filled buffer to _request
	if(recievingHeader)
		appendToRequest(buffer, bytes);

	// Writes the buffer content onto the POST method path
	else if (recievingBody)
	{
		if (request.isChunked){
			request.chunkBuffer.append(buffer, bytes);
			resolveChunkedBody();
		}
		else{
			if (bytes > 0){
				request.appendToBuffer(buffer, bytes);
				request.bodySize += bytes;
			}
			if(request.bodySize > serverBlock.getMaxBodySize()){
				basicClientResponse("Content body too large.", failsafe_error_codes["413"]);
				setConnection(false);
			}
			if (request.bodySize >= request.contentLenght){
				recievingBody = false;
				parsePostBody();
				std::cout << "HERE IS THE BODY\n" << request.body << std::endl;
				postFile.open(request.postFileName.c_str(), std::ios::out);
				postFile.write(request.body.c_str(), request.body.size());
				postFile.close();
				response.simpleHTTP("./var/www/dev/parabens.html");
			}
		}
	}

	// Behaves accordingly in case of not having anything else to read
	// std::cout << "Bytes -> " << bytes << " Chunk Size -> " << CHUNK_SIZE << std::endl;
	if(bytes < CHUNK_SIZE || !bytes)
	{
		if(recievingHeader)
			throw ClientException("Incomplete request header", fd);
		if (!recievingBody || request.chunkedComplete){
			std::cout << "SEND MODE\n" << std::endl;
			sendMode();
		}
	}

	return (bytes);
}

void	Client::parsePostBody(){
	std::string	boundaryKey = "boundary=";
	size_t		pos = request.headerInfo["Content-Type"].find(boundaryKey), nextPart;
	std::string	boundary = request.headerInfo["Content-Type"].substr(pos + boundaryKey.size());
	std::string delimiter = "--" + boundary, endDelimiter = delimiter + "--";
	std::string	requestBody(request.buffer.begin(), request.buffer.end());

	pos = 0;

	while ((nextPart = requestBody.find(delimiter, pos)) != std::string::npos){
		if (!requestBody.compare(nextPart, endDelimiter.size(), endDelimiter)) break;

		size_t		partStart = nextPart + delimiter.size() + 2, partEnd = requestBody.find(delimiter, partStart), partHeaderEnd;
		std::string	part = requestBody.substr(partStart, partEnd - partStart);
		if ((partHeaderEnd = part.find("\r\n\r\n")) == std::string::npos) continue;

		std::string headerStr = part.substr(0, partHeaderEnd), content = part.substr(partHeaderEnd + 4);

		std::istringstream					requestBodyStream(headerStr);
		std::string							requestBodyLine;
		std::map<std::string, std::string>	headers;

		while (std::getline(requestBodyStream, requestBodyLine)){
			size_t colon = requestBodyLine.find(":");

			if (colon != std::string::npos){
				std::string name = trim(requestBodyLine.substr(0, colon)), value = trim(requestBodyLine.substr(colon + 1));

				headers[name] = value;
			}
		}

		MultiFormData	tmpStruct;

		tmpStruct.headers = headers;
		tmpStruct.content = content;

		request.formParts.push_back(tmpStruct);
        pos = partEnd;
	}

	for (std::vector<MultiFormData>::iterator it = request.formParts.begin(); it != request.formParts.end(); ++it){
		MultiFormData &part = *it;

		if (part.headers.count("Content-Disposition")){
			std::string	fileNameKey = "filename=";
			pos = part.headers["Content-Disposition"].find(fileNameKey);
	 		request.postFileName = part.headers["Content-Disposition"].substr(pos + fileNameKey.size());
			request.postFileName = "./var/www/sussy_files/" + request.postFileName.substr(1,  request.postFileName.size() - 2);
		}
		if (part.headers.count("Content-Type"))
	 		request.bodyContentType = part.headers["Content-Type"];

		request.body.append(part.content.c_str(), part.content.size());
	}
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
		//make exception for error 431 "Request Header Fields Too Large"
		/* basicClientResponse("Request header fields too large.", failsafe_error_codes["431"]);
		setConnection(false); */
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
	if (response.bytesSent == response.headerSize){
		std::cout << std::endl << "Client " << getFD() << " (Sending Body)" << std::endl;
		state = SENDING_BODY;
	}
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