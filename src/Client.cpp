

#include "../includes/Client.hpp"

#include "../includes/Logger.hpp"

// CONSTRUCTORS & DESTRUCTORS

Client::Client(int fd, EventHandler &events, ServerBlock &serverBlock) :

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

    Logger::log(INFO, "New client connected: FD " + intToString(fd));

    generateSessionId();
}

Client::~Client()

{

    Logger::log(INFO, "Client disconnected: FD " + intToString(fd));

    closeClient();
}

// GETTERS

int Client::getFD() const

{
    return (fd);
}

client_state Client::getState() const

{
    return (state);
}

HttpResponse const &Client::getResponse() const

{
    return (response);
}

// SETTERS

void Client::setConnection(bool connection)

{
    this->connected = connection;
}

void Client::setState(client_state state)

{
    this->state = state;
}

void Client::setRequestStatus(int code)

{
    this->response.statusCode = code;
}

// MEMBER FUNCTIONS

bool Client::isConnected() const

{
    return (connected);
}

void Client::closeClient()

{

    // Safely closes the Client fd

    if (fd >= 0)

    {

        if (close(fd) == -1)

            throw ClientErrorException("Failed to close fd", fd);

        fd = -1;
    }
}

void Client::recieveMode()

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

void Client::sendMode()

{

    if (response.statusCode != OK)

        setConnection(false);

    response.setSessionId(this->sessionId);

    // THEME COOKIE LOGIC: If theme=dark and .html requested, try _alt.html

    std::string pathToServe = request.path;

    if (!request.cookie.empty() && request.cookie.find("theme=dark") != std::string::npos)
    {

        size_t extPos = pathToServe.rfind(".html");

        if (extPos != std::string::npos)
        {

            std::string altPath = pathToServe.substr(0, extPos) + "_alt.html";

            struct stat buffer;

            if (stat(altPath.c_str(), &buffer) == 0)
            {

                pathToServe = altPath;
            }
        }

        response.setPath(pathToServe);

        response.filePath = pathToServe;
    }

    response.currentCookie = request.cookie;

    response.createResponse();

    state = WAITING_TO_SEND;

    response.bytesSent = 0;

    // Changes the client event to trigger when ready to write to

    events.modifyClient(fd, EPOLLOUT | EPOLLRDHUP | EPOLLET);
}

int Client::recieveRequestChunk()

{

    // Protects the client from recieving if not necessary

    if (state != RECIEVING_REQUEST)
    {

        throw ClientException("Invalid client state to call recieveResponseChunk()", fd);
    }

    // Update activity timestamp

    updateActivity();

    // Stores the data from the client fd in a buffer

    char buffer[CHUNK_SIZE];

    int bytes = recv(fd, buffer, CHUNK_SIZE, 0);

    if (bytes == -1 && errno != EAGAIN && errno != EWOULDBLOCK)
    {

        throw ClientException("Failed to recieve a request", fd);
    }

    // Apppends the filled buffer to _request

    if (recievingHeader)
    {

        appendToRequest(buffer, bytes);
    }

    // Writes the buffer content onto the POST method path

    else if (recievingBody)

    {

        if (request.isChunked)
        {

            request.chunkBuffer.append(buffer, bytes);

            resolveChunkedBody();
        }

        else
        {

            if (bytes > 0)
            {

                request.appendToBuffer(buffer, bytes);

                request.bodySize += bytes;
            }

            if (request.bodySize > serverBlock.getMaxBodySize())
            {

                response.statusCode = CONTENT_TOO_LARGE;

                if (serverBlock.getErrorPages().find(CONTENT_TOO_LARGE) != serverBlock.getErrorPages().end())

                    response.filePath = serverBlock.getInfo().server_root + serverBlock.getErrorPages()[CONTENT_TOO_LARGE];

                recievingBody = false;

                bytes = -1;
            }

            else if (request.bodySize >= request.contentLenght)
            {

                recievingBody = false;

                request.parseRequestBody();

                postFile.open(request.path.c_str(), std::ios::out);

                postFile.write(request.body.c_str(), request.body.size());

                postFile.close();
            }
        }
    }

    // Behaves accordingly in case of not having anything else to read

    if (bytes < CHUNK_SIZE || !bytes)

    {

        if (recievingHeader)

            throw ClientException("Incomplete request header", fd);

        if (!recievingBody || request.chunkedComplete)
        {

            sendMode();
        }
    }

    return (bytes);
}

void Client::appendToRequest(char *buffer, int size)

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

            response.statusCode = request.parseRequestHeader(it + 4);

            Logger::log(INFO, "Request received: " + request.method + " " + request.path + " from FD " + intToString(fd));

            recievingHeader = false;

            if (request.headerInfo.count("Connection") && request.headerInfo["Connection"] == "close")

                response.connection = "close";

            else

                response.connection = "keep-alive";

            // extract location, return extract, replace request.path locations /dev/flick_esfand.gif becomes /dev/, /upload.html becomes /

            if (request.path.find("/") == request.path.rfind("/"))

                extracted_path = "/";

            else

                extracted_path = request.path.substr(request.path.find('/'), request.path.find('/', request.path.find('/') + 1) - request.path.find('/') + 1);

            // end of extraction

            if (!serverBlock.getInfo().locations.count(extracted_path))
            {

                response.statusCode = NOT_FOUND;

                return;
            }

            if (*(request.path.end() - 1) == '/')

                request.path = serverBlock.getInfo().server_root + serverBlock.getInfo().locations[extracted_path].location;

            else

                request.path = serverBlock.getInfo().server_root + request.path;

            handleMethod();
        }
    }
}

void Client::handleMethod()

{

    // Get the allowed methods for the current location

    std::vector<std::string> allowedMethods = serverBlock.getInfo().locations[extracted_path].allowed_services;

    // Check if the request method is in the allowed list

    bool methodPermitted = std::find(allowedMethods.begin(), allowedMethods.end(), request.method) != allowedMethods.end();

    if (!methodPermitted)
    {

        response.statusCode = METHOD_NOT_ALLOWED;

        if (serverBlock.getErrorPages().find(METHOD_NOT_ALLOWED) != serverBlock.getErrorPages().end())
        {

            response.filePath = serverBlock.getInfo().server_root + serverBlock.getErrorPages()[METHOD_NOT_ALLOWED];
        }

        else

            response.filePath = "";

        return; // Stop processing the request
    }

    // VERIFY ALLOWED METHODS/SERVICES

    if (request.method == "GET" || request.method == "OPTIONS" || request.method == "TRACE")
    {

        if (request.path == serverBlock.getInfo().server_root + serverBlock.getInfo().locations[extracted_path].location)
        {

            // if (request.cookie.find("theme=dark") != std::string::npos)

            // 	request.path = serverBlock.getInfo().server_root

            // 		+ serverBlock.getInfo().locations[extracted_path].location

            // 		+ serverBlock.getInfo().locations[extracted_path].index_file.substr(0,

            // 			serverBlock.getInfo().locations[extracted_path].index_file.rfind(".html")) + "_alt.html";

            // else

            request.path = serverBlock.getInfo().server_root + serverBlock.getInfo().locations[extracted_path].location

                           + serverBlock.getInfo().locations[extracted_path].index_file;
        }

        std::ifstream fileStream(request.path.c_str());

        if (!fileStream.good())
        {

            response.statusCode = NOT_FOUND;

            if (serverBlock.getErrorPages().find(NOT_FOUND) != serverBlock.getErrorPages().end())
            {

                response.filePath = serverBlock.getInfo().server_root + serverBlock.getErrorPages()[NOT_FOUND];

                response.currentPath = response.filePath;
            }
        }

        else

            response.filePath = request.path;
    }

    else if (request.method == "POST" || request.method == "PUT")

    {

        recievingBody = true;

        request.bodySize = request.buffer.size();

        response.filePath = request.path;
    }

    else if (request.method == "DELETE")
    {

        if (std::remove(request.path.c_str()) == 0)
        {

            response.filePath = "";
        }

        else
        {

            response.statusCode = INTERNAL_SERVER_ERROR;

            if (serverBlock.getErrorPages().find(INTERNAL_SERVER_ERROR) != serverBlock.getErrorPages().end())

                response.filePath = serverBlock.getInfo().server_root + serverBlock.getErrorPages()[INTERNAL_SERVER_ERROR];
        }
    }

    else if (request.method == "HEAD")
    {

        response.filePath = request.path;

        response.contentLenght = 0;
    }

    else
    {

        response.statusCode = METHOD_NOT_ALLOWED;

        if (serverBlock.getErrorPages().find(METHOD_NOT_ALLOWED) != serverBlock.getErrorPages().end())

            response.filePath = serverBlock.getInfo().server_root + serverBlock.getErrorPages()[METHOD_NOT_ALLOWED];
    }

    // --- CGI HANDLING ---

    std::string path = request.path;

    size_t queryPos = path.find('?');

    std::string pathWithoutQuery = (queryPos != std::string::npos) ? path.substr(0, queryPos) : path;

    if (pathWithoutQuery.size() > 3 && pathWithoutQuery.find("/cgi-bin/") != std::string::npos &&

        (pathWithoutQuery.substr(pathWithoutQuery.size() - 3) == ".py" ||

         pathWithoutQuery.substr(pathWithoutQuery.size() - 4) == ".php"))
    {

        std::string requestBody = request.body; // For POST, otherwise empty

        std::string interpreter;

        if (pathWithoutQuery.substr(pathWithoutQuery.size() - 3) == ".py")
        {

            interpreter = "/usr/bin/python3";
        }
        else if (pathWithoutQuery.substr(pathWithoutQuery.size() - 4) == ".php")
        {

            interpreter = "/usr/bin/php";
        }

        std::string cgiOutput;

        CgiHandler cgi(pathWithoutQuery, request); // pid not used here

        Logger::log(INFO, "Executing CGI: " + pathWithoutQuery + " for FD " + intToString(fd));

        int status = cgi.executeCgi(pathWithoutQuery, interpreter, requestBody, cgiOutput);

		std::cout << "status: " << status << std::endl;

        if (!status)
        {

            // Parse CGI output: split headers and body

            size_t headerEnd = cgiOutput.find("\r\n\r\n");

            if (headerEnd != std::string::npos)
            {

				std::cout << "headerEnd != std::string::npos" << std::endl;

                std::string headers = cgiOutput.substr(0, headerEnd + 4);

                std::string body = cgiOutput.substr(headerEnd + 4);

                // Set response headers and body accordingly

                response.header = std::vector<char>(headers.begin(), headers.end());

                response.headerSize = headers.size();

                response.body = body;

                response.contentLenght = body.size();

                response.statusCode = OK; // Or parse from CGI output

                std::string header = "Content-Type: ";

                size_t pos = cgiOutput.find(header);

                size_t start = pos + header.length();

                size_t end = cgiOutput.find("\r\n", start);

                response.contentType = cgiOutput.substr(start, end - start);

                response.cgi = true;

				std::cout << "response.headerSize = " << response.headerSize << std::endl;
				std::cout << "response.contentLength = " << response.contentLenght << std::endl;
				std::cout << "response.body = " << response.body << std::endl;
				std::cout << "response.contentType = " << response.contentType << std::endl;
            }
            else
            {

                // Malformed CGI output

                response.statusCode = INTERNAL_SERVER_ERROR;
            }
        }
        else
        {

            // CGI execution failed

            response.statusCode = INTERNAL_SERVER_ERROR;
        }

		std::cout << "response.statusCode = " << response.statusCode << std::endl;

        return; // Done with CGI
    }

    // --- END CGI HANDLING ---

    response.cgi = false;
}

void Client::resolveChunkedBody()
{

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
    }
}

void Client::sendHeaderChunk()

{

    // Protects the client from sending if not necessary

    if (state != SENDING_HEADER)

        throw ClientException("Invalid client state to call sendHeaderChunk()", fd);

    // Protects the function from sending invalid memory

    int chunk_size = CHUNK_SIZE;

    if (CHUNK_SIZE + response.bytesSent > response.headerSize)

        chunk_size = response.headerSize - response.bytesSent;

    // Sends the read chunk to the client

    int bytes = send(fd, &response.header[response.bytesSent], chunk_size, 0);

    if (bytes == -1)

        throw ClientException("Failed to send a response", fd);

    response.bytesSent += bytes;

    // Changes the state of the client when necessary

    if (response.bytesSent == response.headerSize)
    {

        state = SENDING_BODY;

        if (response.statusCode != OK)
        {

            if (send(fd, getStatus(response.statusCode).c_str(), getStatus(response.statusCode).size(), 0) == -1)

                throw ClientException("Failed to send a response", fd);

            Logger::log(INFO, "Response sent: " + intToString(response.statusCode) + " for " + request.method + " " + request.path + " from FD " + intToString(fd));

            recieveMode();
        }
    }
}

void Client::sendBodyChunk()

{

    // Protects the client from sending if not necessary

    if (state != SENDING_BODY)

        throw ClientException("Invalid client state to call sendBodyChunk()", fd);

    if (!response.body.empty())
    {

        // Send body from memory (CGI or dynamic response)

        size_t bytes = (response.contentLenght + response.headerSize) - response.bytesSent;

        if (bytes > CHUNK_SIZE)

            bytes = CHUNK_SIZE;

        if (send(fd, response.body.data() + (response.bytesSent - response.headerSize), bytes, 0) == -1)

            throw ClientException("Failed to send response body", fd);

        response.bytesSent += bytes;
    }

    else
    {

        // Reads from the file stream storing the bytes read

        char buffer[CHUNK_SIZE];

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

    {

        Logger::log(INFO, "Response sent: " + intToString(response.statusCode) + " for " + request.method + " " + request.path + " from FD " + intToString(fd));

        recieveMode();
    }
}

void Client::generateSessionId()
{

    static const char charset[] =

        "0123456789"

        "abcdefghijklmnopqrstuvwxyz"

        "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    static const size_t charsetSize = sizeof(charset) - 1;

    size_t length = 32;

    for (size_t i = 0; i < length; ++i)
    {

        this->sessionId += charset[std::rand() % charsetSize];
    }
}

// EXCEPTIONS

Client::ClientException::ClientException(std::string info, int fd) :

                                                                     runtime_error(createMessage(info, fd))
{
}

std::string Client::ClientException::createMessage(std::string info, int fd)

{

    std::ostringstream oss;

    oss << info << " (Client " << fd << ": " + std::string(strerror(errno)) + ")";

    return oss.str();
}

Client::ClientErrorException::ClientErrorException(std::string info, int fd) :

                                                                               runtime_error(createMessage(info, fd))
{
}

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

    return (time(NULL) - lastActivity > timeoutSeconds);
}