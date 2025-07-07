#include "../includes/HttpResponse.hpp"
#include "../includes/Webserv.h"

// CONSTRUCTORS & DESTRUCTORS

HttpResponse::HttpResponse()
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
	supportedContentType[".html"] = "text/html"; 
	supportedContentType[".txt"] = "text/plain";
	supportedContentType[".css"] = "text/css";
	supportedContentType[".jpg"] = "image/jpeg";
	supportedContentType[".jpeg"] = "image/jpeg";
	supportedContentType[".png"] = "image/png";
	supportedContentType[".gif"] = "image/gif";
	supportedContentType[".svg"] = "image/svg+xml";
	supportedContentType[".ico"] = "image/x-icon";
	supportedContentType[".xml"] = "application/xml";
	supportedContentType[".pdf"] = "application/pdf";
	supportedContentType[".zip"] = "application/zip";
	supportedContentType[".json"] = "application/json";
	supportedContentType[".js"] = "application/javascript";
	supportedContentType[".bin"] = "application/octet-stream";
}

HttpResponse::~HttpResponse()
{
	if (fileStream.is_open())
		fileStream.close();
}

void	HttpResponse::simpleHTTPerror(std::string path, std::string error_response)
{
	filePath = path;
	openRequestedFile();
	//setStatus();
	status = error_response;
	setContentType();
	setContentLength();
	//setConnection();
	connection = "close";
	buildHeader();
}

// MEMBER FUNCTIONS
void	HttpResponse::simpleHTTP(std::string path)
{
	filePath = path;
	openRequestedFile();
	//setStatus();
	status = failsafe_error_codes["200"];
	setContentType();
	setContentLength();
	//setConnection();
	buildHeader();
}

void	HttpResponse::openRequestedFile()
{
	// Protects the function to execute it safely
	if (fileStream.is_open())
		throw ResponseException("A file is already opened");
	
	// Opens the file and retrieves the necessary information
	std::cout << "FILE OPENED -> " << filePath.c_str() << std::endl;
	fileStream.open(filePath.c_str(), std::ios::in);
	if (!fileStream.is_open())
		throw ResponseException("Failed to open the file \"" + filePath + "\"");
	if (!fileStream.good())
	{
		fileStream.close();
		throw ResponseException("Error while opening File");
	}
	if (stat(filePath.c_str(), &fileStats) == -1)
		throw ResponseException("Failed to retrieve the stats of the file \"" + filePath + "\"");
}

void	HttpResponse::setStatus(std::string status)
{
	this->status = status;
}

void	HttpResponse::setContentType()
{
	// Finds the extension of the file
	std::string	extension;
	extension = filePath.substr(filePath.find_last_of('.'));

	// Sets the correct content type for the type of file being sent
	if (supportedContentType.count(extension))
		contentType = supportedContentType[extension];
	else
		contentType = "application/octet-stream";
}

void	HttpResponse::setContentLength()
{
	if (!fileStream.is_open())
		contentLenght = 0;
	else
		contentLenght = fileStats.st_size;
}

void	HttpResponse::setConnection()
{
	connection = "keep-alive";
}

void	HttpResponse::buildHeader()
{
	// Builds the header
	std::stringstream lenght;
	lenght << contentLenght;

	std::string header_str;
	header_str += "HTTP/1.1 " + status + "\r\n";
	header_str += "Cookie: session_id=abc123; theme=dark; lang=en; logged_in=true \r\n";
	header_str += "Content-Type: " + contentType + "\r\n";
	header_str += "Content-Length: " + lenght.str() + "\r\n";
	header_str += "Connection: " + connection + "\r\n\r\n";

	header = std::vector<char>(header_str.begin(), header_str.end());
	headerSize = header_str.size();
	std::cout << "Response:" << std::endl << header_str << std::endl;
}

void	HttpResponse::reset()
{
	status.clear();
	contentType.clear();
	contentLenght = 0;
	connection.clear();

	header.clear();
	headerSize = 0;
	bytesSent = 0;

	filePath.clear();
	if (fileStream.is_open())
		fileStream.close();
}

HttpResponse::ResponseException::ResponseException(std::string info) :
runtime_error(info){}