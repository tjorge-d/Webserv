#include "../includes/Client.hpp"

// CONSTRUCTORS & DESTRUCTORS

Client::Client(int fd, EventHandler &events, ServerBlock &serverBlock): 
fd(fd),
events(events),
serverBlock(serverBlock),
sessionId(),
request(),
response(),
postFile(),
connected(true),
recievingHeader(true),
recievingBody(false),
state(WAITING_TO_RECIEVE),
lastActivity(time(NULL))
{
	std::cout << "Client constructor called\n";
	generateSessionId();
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

void	Client::setRequestStatus(int code)
{this->response.statusCode = code;}

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

	if (response.statusCode != OK)
		setConnection(false);
	response.setSessionId(this->sessionId);
	response.setPath(request.path);
	response.currentCookie = request.cookie;
	response.createResponse();

	state = WAITING_TO_SEND;
	response.bytesSent = 0;

	// Changes the client event to trigger when ready to write to
	events.modifyClient(fd, EPOLLOUT | EPOLLRDHUP | EPOLLET);
}

int	Client::recieveRequestChunk()
{
	// Protects the client from recieving if not necessary
	if(state != RECIEVING_REQUEST){
		throw ClientException("Invalid client state to call recieveResponseChunk()", fd);
	}

	// Update activity timestamp
	updateActivity();

	// Stores the data from the client fd in a buffer
	char	buffer[CHUNK_SIZE];
	int		bytes = recv(fd, buffer, CHUNK_SIZE, 0);
	if(bytes == -1 && errno != EAGAIN && errno != EWOULDBLOCK){
		throw ClientException("Failed to recieve a request", fd);
	}

	
	// Apppends the filled buffer to _request
	if(recievingHeader){
		std::cout << "HEADEEEEER\n" << buffer << std::endl; 
		appendToRequest(buffer, bytes);
	}

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
				response.statusCode = CONTENT_TOO_LARGE;
				if (serverBlock.getErrorPages().find(CONTENT_TOO_LARGE) != serverBlock.getErrorPages().end())
					response.filePath = serverBlock.getInfo().server_root + serverBlock.getErrorPages()[CONTENT_TOO_LARGE];
				recievingBody = false;
				bytes = -1;
			}
			else if (request.bodySize >= request.contentLenght){
				recievingBody = false;
				request.parseRequestBody();

				postFile.open(request.postFileName.c_str(), std::ios::out);
				postFile.write(request.body.c_str(), request.body.size());
				postFile.close();
				response.filePath = serverBlock.getInfo().server_root + extracted_path + "parabens.html";
				std::cout << "response.filePath = " << response.filePath << std::endl;
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
			response.statusCode = request.parseRequestHeader(it + 4);
			recievingHeader = false;
			// if (response.statusCode != OK){
			// 	response.basicClientResponse(response.statusCode);
			// 	setConnection(false);
			// }
			// Should replace setConnection function called in simpleHTTP (already done)
			if (request.headerInfo.count("Connection") && request.headerInfo["Connection"] == "close")
				response.connection = "close";
			else
				response.connection = "keep-alive";

			// ------------------------------------THIS NEEDS TO BE DONE SOMEWHERE ELSE ---------------------------------------------------
			// extract location, return extract, replace request.path locations /dev/flick_esfand.gif becomes /dev/, /upload.html becomes /
			if (request.path.find("/") == request.path.rfind("/"))
				extracted_path = "/";
			else
				extracted_path = request.path.substr(request.path.find('/'),
					request.path.find('/', request.path.find('/') + 1) - request.path.find('/') + 1);
			std::cout << "extracted location = " << extracted_path << std::endl;
			// end of extraction, need to test special cases but overall should function correctly
			if (!serverBlock.getInfo().locations.count(extracted_path)){

				response.statusCode = NOT_FOUND;
				return ;
			}
			std::cout << "request path before =" << request.path << std::endl;
			if (*(request.path.end() - 1) == '/')
				request.path = serverBlock.getInfo().server_root + serverBlock.getInfo().locations[extracted_path].location;
/* 			else if (request.path.find('.') != std::string::npos)
				request.path = serverBlock.getInfo().server_root + serverBlock.getInfo().locations[extracted_path].location 
				 + request.path; */
			else
				request.path = serverBlock.getInfo().server_root + request.path;
			std::cout << "request path after =" << request.path << std::endl;
			// ------------------------------------THIS NEEDS TO BE DONE SOMEWHERE ELSE ---------------------------------------------------
				handleMethod();
			}
		//make exception for error 431 "Request Header Fields Too Large"
		/* basicClientResponse("Request header fields too large.", getStatus(REQUEST_HEADER_FIELDS_TOO_LARGE));
		setConnection(false); */
	}
}

void	Client::handleMethod()
{
	//VERIFY ALLOWED METHODS/SERVICES
    if (request.method == "GET" || request.method == "OPTIONS" || request.method == "TRACE"){
		//response.simpleHTTP("./var/www/dev" + request.path);
		if (request.path == serverBlock.getInfo().server_root + serverBlock.getInfo().locations[extracted_path].location){
			if (request.cookie.find("theme=dark") != std::string::npos)
				request.path = serverBlock.getInfo().server_root 
					+ serverBlock.getInfo().locations[extracted_path].location
					+ serverBlock.getInfo().locations[extracted_path].index_file.substr(0, 
						serverBlock.getInfo().locations[extracted_path].index_file.rfind(".html")) + "_alt.html";
			else
				request.path = serverBlock.getInfo().server_root + serverBlock.getInfo().locations[extracted_path].location
						+ serverBlock.getInfo().locations[extracted_path].index_file;
		}
		std::ifstream fileStream(request.path.c_str());

		if (!fileStream.good()){
			response.statusCode = NOT_FOUND;
			if (serverBlock.getErrorPages().find(NOT_FOUND) != serverBlock.getErrorPages().end()){
				response.filePath = serverBlock.getInfo().server_root + serverBlock.getErrorPages()[NOT_FOUND];
				response.currentPath = response.filePath;
			}
		}
		else
			response.filePath = request.path;
		std::cout << "response.filePath = " << response.filePath << std::endl;
	}
	else if(request.method == "POST" || request.method == "PUT")
	{
		recievingBody = true;
		request.bodySize = request.buffer.size();
	}
	else if (request.method == "DELETE") {
		std::cout << "to delete: " << request.path << std::endl;
		if (!std::remove(request.path.c_str())){
			response.filePath = serverBlock.getInfo().server_root + extracted_path + "parabens.html";
			printf("FilePath: %s\n", response.filePath.c_str());
		}
		else{
			response.statusCode = INTERNAL_SERVER_ERROR;
			if (serverBlock.getErrorPages().find(INTERNAL_SERVER_ERROR) != serverBlock.getErrorPages().end())
					response.filePath = serverBlock.getInfo().server_root + serverBlock.getErrorPages()[INTERNAL_SERVER_ERROR];
		}
	}
	else if (request.method == "HEAD") {
		response.filePath = request.path;
		response.contentLenght = 0;
	}
	else {
		response.statusCode = METHOD_NOT_ALLOWED;
		if (serverBlock.getErrorPages().find(METHOD_NOT_ALLOWED) != serverBlock.getErrorPages().end())
					response.filePath = serverBlock.getInfo().server_root + serverBlock.getErrorPages()[METHOD_NOT_ALLOWED];
	}

	// --- CGI HANDLING ---
	std::string path = request.path;
	size_t queryPos = path.find('?');
	std::string pathWithoutQuery = (queryPos != std::string::npos) ? path.substr(0, queryPos) : path;

	if (pathWithoutQuery.size() > 3 && pathWithoutQuery.substr(pathWithoutQuery.size() - 3) == ".py" && pathWithoutQuery.find("/cgi-bin/") != std::string::npos) {
		std::string requestBody = request.body; // For POST, otherwise empty
		std::string interpreter = "/usr/bin/python3"; // Adjust if needed
		std::string cgiOutput;
		CgiHandler cgi(pathWithoutQuery, 0, request); // pid not used here

		int status = cgi.executeCgi(pathWithoutQuery, interpreter, requestBody, cgiOutput);

		if (status == 0) {
			// Parse CGI output: split headers and body
			dprintf(2, "CGI Output:\n%s\n", cgiOutput.c_str());
			size_t headerEnd = cgiOutput.find("\r\n\r\n");
			if (headerEnd != std::string::npos) {
				std::string headers = cgiOutput.substr(0, headerEnd + 4); // include \r\n\r\n
				std::string body = cgiOutput.substr(headerEnd + 4);
				// Set response headers and body accordingly
				response.header = std::vector<char>(headers.begin(), headers.end());
				response.headerSize = headers.size();
				response.body = body;
				response.contentLenght = body.size();
				response.statusCode = 200; // Or parse from CGI output
				std::string header = "Content-Type: ";
				size_t pos = cgiOutput.find(header);
				size_t start = pos + header.length();
				size_t end = cgiOutput.find("\r\n", start);
				response.contentType = cgiOutput.substr(start, end - start);
				printf("CGI Content-Type: %s\n", response.contentType.c_str());
				response.cgi = true;
			} else {
				// Malformed CGI output
				dprintf(2, "Malformed CGI output: %s\n", cgiOutput.c_str());
				response.statusCode = 500;
			}
		} else {
			// CGI execution failed
			dprintf(2, "CGI execution failed: %s\n", cgiOutput.c_str());
			response.statusCode = 500;
		}
		return; // Done with CGI
	}
	// --- END CGI HANDLING ---
	response.cgi = false;

	std::cout << "Body Size: " << request.bodySize << std::endl;
}

void	Client::resolveChunkedBody(){

	std::string fileToPost;

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

		fileToPost = serverBlock.getInfo().server_root + "/upload/file";
		postFile.open(fileToPost.c_str(), std::ios::out);
		postFile.write(request.body.c_str(), static_cast<int>(request.body.size()));
		//response.simpleHTTP("./var/www/dev/parabens.html");
		response.filePath = serverBlock.getInfo().server_root + extracted_path + "parabens.html";
	}
}

void	Client::sendHeaderChunk()
{
	// Protects the client from sending if not necessary
	if (state != SENDING_HEADER)
		throw ClientException("Invalid client state to call sendHeaderChunk()", fd);

	// checkar erros e buildar o response header

	// Protects the function from sending invalid memory
	int	chunk_size = CHUNK_SIZE;
	if (CHUNK_SIZE + response.bytesSent > response.headerSize)
		chunk_size = response.headerSize - response.bytesSent;

	std::cout << "BYTES SENT -> " << response.bytesSent << std::endl;
	// Sends the read chunk to the client
	int	bytes = send(fd, &response.header[response.bytesSent], chunk_size, 0);
	if (bytes == -1)
		throw ClientException("Failed to send a response", fd);
	response.bytesSent += bytes;

	// Changes the state of the client when necessary
	if (response.bytesSent == response.headerSize){
		std::cout << std::endl << "Client " << getFD() << " (Sending Body)" << std::endl;
		state = SENDING_BODY;
		if (response.statusCode != OK){
			std::cout << "Sending error message -> " << getStatus(response.statusCode).c_str() << std::endl;
			if (send(fd, getStatus(response.statusCode).c_str(), getStatus(response.statusCode).size(), 0) == -1)
				throw ClientException("Failed to send a response", fd);
			std::cout << "Sent!" << std::endl;
			//setConnection(false);
			recieveMode();
		}
	}
}

void	Client::sendBodyChunk()
{
	// Protects the client from sending if not necessary
	if (state != SENDING_BODY)
		throw ClientException("Invalid client state to call sendBodyChunk()", fd);

	if (!response.body.empty()) {
        // Send body from memory (CGI or dynamic response)
        size_t bytes = (response.contentLenght + response.headerSize) - response.bytesSent;
        if (bytes > CHUNK_SIZE)
			bytes = CHUNK_SIZE;
        if (send(fd, response.body.data() + (response.bytesSent - response.headerSize), bytes, 0) == -1)
            throw ClientException("Failed to send response body", fd);
        response.bytesSent += bytes;
    }
	else {
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
	}
	// Resets the response status of the client when over
	if (response.bytesSent == response.contentLenght + response.headerSize)
		recieveMode();
}

void	Client::generateSessionId(){

	static const char charset[] =
        "0123456789"
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    static const size_t charsetSize = sizeof(charset) - 1;
	size_t length = 32;

    for (size_t i = 0; i < length; ++i) {
        this->sessionId += charset[std::rand() % charsetSize];
    }
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

// Timeout management methods
void Client::updateActivity()
{
	lastActivity = time(NULL);
}

bool Client::isTimedOut() const
{
	return (time(NULL) - lastActivity > TIMEOUT_SECONDS);
}