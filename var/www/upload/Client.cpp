

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
    // if (response.statusCode != OK)
    //     setConnection(false);
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

    // --- CGI HANDLING ---
    std::string path = request.path;
    size_t queryPos = path.find('?');
    std::string pathWithoutQuery = (queryPos != std::string::npos) ? path.substr(0, queryPos) : path;
    if (response.cgi &&
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
                printf("Output: %s\n", cgiOutput.c_str());
            }
            else
            {
                // Malformed CGI output
                printf("Fucked\n");
                response.statusCode = INTERNAL_SERVER_ERROR;
            }
        }
        else
        {
            // CGI execution failed
            printf("Fucked2\n");
            response.statusCode = INTERNAL_SERVER_ERROR;
        }
    }
    // --- END CGI HANDLING ---
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
                std::string path = request.path;
                size_t queryPos = path.find('?');
                std::string pathWithoutQuery = (queryPos != std::string::npos) ? path.substr(0, queryPos) : path;
                printf("Path without query: %s\n", pathWithoutQuery.c_str());
                if (pathWithoutQuery.size() > 3 && pathWithoutQuery.find("/cgi-bin/") != std::string::npos)
                    response.cgi = true;
                if ((request.method == "POST" || request.method == "PUT") && !response.cgi) {
                    postFile.open(request.path.c_str(), std::ios::out);
                    postFile.write(request.body.c_str(), request.body.size());
                    postFile.close();
                }
            }
        }
    }
    // Behaves accordingly in case of not having anything else to read
    if (bytes < CHUNK_SIZE || !bytes)
    {
        if (recievingHeader)
            //throw ClientException("Incomplete request header", fd);
            response.statusCode = BAD_REQUEST;
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
    printf("Header: %s\n", buffer);
    if (recievingHeader)
    {
        // Checks if the read data contains the end of a request header
        std::vector<char>::iterator it = request.findHeader(size);
        // Parses the header if found
        if (it != request.buffer.end())
        {
            printf("Entrei nela\n");
            response.statusCode = request.parseRequestHeader(it + 4);
            Logger::log(INFO, "Request received: " + request.method + " " + request.path + " from FD " + intToString(fd));
  POST /upload.html HTTP/1.1
Host: localhost:8081
User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:143.0) Gecko/20100101 Firefox/143.0
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8
Accept-Language: en-US,en;q=0.5
Accept-Encoding: gzip, deflate, br, zstd
Content-Type: multipart/form-data; boundary=----geckoformboundaryaa5641bad4104a4c682176636defc2e9
Content-Length: 19147
Origin: http://localhost:8081
Connection: keep-alive
Referer: http://localhost:8081/upload.html
Cookie: theme=light; sessionId=voFTub2N1l7tCRqZM38zVc7qfXtj6PTz
Upgrade-Insecure-Requests: 1
Sec-Fetch-Dest: document
Sec-Fetch-Mode: navigate
Sec-Fetch-Site: same-origin
Sec-Fetch-User: ?1
Priority: u=0, i



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
    // if (response.statusCode != OK)
    //     setConnection(false);
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

    // --- CGI HANDLING ---
    std::string path = request.path;
    size_t queryPos = path.find('?');
    std::string pathWithoutQuery = (queryPos != std::string::npos) ? path.substr(0, queryPos) : path;
    if (response.cgi &&
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
                printf("Output: %s\n", cgiOutput.c_str());
            }
            else
            {
                // Malformed CGI output
                printf("Fucked\n");
                response.statusCode = INTERNAL_SERVER_ERROR;
            }
        }
        else
        {
            // CGI execution failed
            printf("Fucked2\n");
            response.statusCode = INTERNAL_SERVER_ERROR;
        }
    }
    // --- END CGI HANDLING ---
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
                std::string path = request.path;
                size_t queryPos = path.find('?');
                std::string pathWithoutQuery = (queryPos != std::string::npos) ? path.substr(0, queryPos) : path;
                printf("Path without query: %s\n", pathWithoutQuery.c_str());
                if (pathWithoutQuery.size() > 3 && pathWithoutQuery.find("/cgi-bin/") != std::string::npos)
                    response.cgi = true;
                if ((request.method == "POST" || request.method == "PUT") && !response.cgi) {
                    postFile.open(request.path.c_str(), std::ios::out);
                    postFile.write(request.body.c_str(), request.body.size());
                    postFile.close();
                }
            }
        }
    }
    // Behaves accordingly in case of not having anything else to read
    if (bytes < CHUNK_SIZE || !bytes)
    {
        if (recievingHeader)
            //throw ClientException("Incomplete request header", fd);
            response.statusCode = BAD_REQUEST;
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
    printf("Header: %s\n", buffer);
    if (recievingHeader)
    {
        // Checks if the read data contains the end of a request header
        std::vector<char>::iterator it = request.findHeader(size);
        // Parses the header if found
        if (it != request.buffer.end())
        {
            printf("Entrei nela\n");
            response.statusCode = request.parseRequestHeader(it + 4);
            Logger::log(INFO, "Request received: " + request.method + " " + request.path + " from FD " + intToString(fd));
  

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
    // if (response.statusCode != OK)
    //     setConnection(false);
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

    // --- CGI HANDLING ---
    std::string path = request.path;
    size_t queryPos = path.find('?');
    std::string pathWithoutQuery = (queryPos != std::string::npos) ? path.substr(0, queryPos) : path;
    if (response.cgi &&
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
                printf("Output: %s\n", cgiOutput.c_str());
            }
            else
            {
                // Malformed CGI output
                printf("Fucked\n");
                response.statusCode = INTERNAL_SERVER_ERROR;
            }
        }
        else
        {
            // CGI execution failed
            printf("Fucked2\n");
            response.statusCode = INTERNAL_SERVER_ERROR;
        }
    }
    // --- END CGI HANDLING ---
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
                std::string path = request.path;
                size_t queryPos = path.find('?');
                std::string pathWithoutQuery = (queryPos != std::string::npos) ? path.substr(0, queryPos) : path;
                printf("Path without query: %s\n", pathWithoutQuery.c_str());
                if (pathWithoutQuery.size() > 3 && pathWithoutQuery.find("/cgi-bin/") != std::string::npos)
                    response.cgi = true;
                if ((request.method == "POST" || request.method == "PUT") && !response.cgi) {
                    postFile.open(request.path.c_str(), std::ios::out);
                    postFile.write(request.body.c_str(), request.body.size());
                    postFile.close();
                }
            }
        }
    }
    // Behaves accordingly in case of not having anything else to read
    if (bytes < CHUNK_SIZE || !bytes)
    {
        if (recievingHeader)
            //throw ClientException("Incomplete request header", fd);
            response.statusCode = BAD_REQUEST;
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
    printf("Header: %s\n", buffer);
    if (recievingHeader)
    {
        // Checks if the read data contains the end of a request header
        std::vector<char>::iterator it = request.findHeader(size);
        // Parses the header if found
        if (it != request.buffer.end())
        {
            printf("Entrei nela\n");
            response.statusCode = request.parseRequestHeader(it + 4);
            Logger::log(INFO, "Request received: " + request.method + " " + request.path + " from FD " + intToString(fd));
  POST /upload.html HTTP/1.1
Host: localhost:8081
User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:143.0) Gecko/20100101 Firefox/143.0
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8
Accept-Language: en-US,en;q=0.5
Accept-Encoding: gzip, deflate, br, zstd
Content-Type: multipart/form-data; boundary=----geckoformboundary253563af37dc8ddb322d7e8737cb769b
Content-Length: 19147
Origin: http://localhost:8081
Connection: keep-alive
Referer: http://localhost:8081/upload.html
Cookie: theme=light; sessionId=voFTub2N1l7tCRqZM38zVc7qfXtj6PTz
Upgrade-Insecure-Requests: 1
Sec-Fetch-Dest: document
Sec-Fetch-Mode: navigate
Sec-Fetch-Site: same-origin
Sec-Fetch-User: ?1
Priority: u=0, i



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
    // if (response.statusCode != OK)
    //     setConnection(false);
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

    // --- CGI HANDLING ---
    std::string path = request.path;
    size_t queryPos = path.find('?');
    std::string pathWithoutQuery = (queryPos != std::string::npos) ? path.substr(0, queryPos) : path;
    if (response.cgi &&
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
                printf("Output: %s\n", cgiOutput.c_str());
            }
            else
            {
                // Malformed CGI output
                printf("Fucked\n");
                response.statusCode = INTERNAL_SERVER_ERROR;
            }
        }
        else
        {
            // CGI execution failed
            printf("Fucked2\n");
            response.statusCode = INTERNAL_SERVER_ERROR;
        }
    }
    // --- END CGI HANDLING ---
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
                std::string path = request.path;
                size_t queryPos = path.find('?');
                std::string pathWithoutQuery = (queryPos != std::string::npos) ? path.substr(0, queryPos) : path;
                printf("Path without query: %s\n", pathWithoutQuery.c_str());
                if (pathWithoutQuery.size() > 3 && pathWithoutQuery.find("/cgi-bin/") != std::string::npos)
                    response.cgi = true;
                if ((request.method == "POST" || request.method == "PUT") && !response.cgi) {
                    postFile.open(request.path.c_str(), std::ios::out);
                    postFile.write(request.body.c_str(), request.body.size());
                    postFile.close();
                }
            }
        }
    }
    // Behaves accordingly in case of not having anything else to read
    if (bytes < CHUNK_SIZE || !bytes)
    {
        if (recievingHeader)
            //throw ClientException("Incomplete request header", fd);
            response.statusCode = BAD_REQUEST;
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
    printf("Header: %s\n", buffer);
    if (recievingHeader)
    {
        // Checks if the read data contains the end of a request header
        std::vector<char>::iterator it = request.findHeader(size);
        // Parses the header if found
        if (it != request.buffer.end())
        {
            printf("Entrei nela\n");
            response.statusCode = request.parseRequestHeader(it + 4);
            Logger::log(INFO, "Request received: " + request.method + " " + request.path + " from FD " + intToString(fd));
  