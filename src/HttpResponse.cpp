#include "../includes/HttpResponse.hpp"
#include "../includes/Webserv.h"

// CONSTRUCTORS & DESTRUCTORS

HttpResponse::HttpResponse()
{
	statusMessages[400] = "Bad request.";
	statusMessages[405] = "Method not allowed.";
	statusMessages[413] = "Content body too large.";
	statusMessages[431] = "Request header fields too large.";
	statusMessages[503] = "Please try again later.";
	statusMessages[505] = "Unsupported HTTP version.";
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

/* void	HttpResponse::simpleHTTPerror(std::string path)
{
	filePath = path;
	openRequestedFile();
	//setStatus();
	status = "200 OK";
	setContentType();
	setContentLength();
	//setConnection();
	buildHeader();
} */

// MEMBER FUNCTIONS

void	HttpResponse::basicClientResponse(int code)
{
	int			size = statusMessages[code].size();
	std::stringstream s;
	s << size;
	std::string n = s.str();

	int	seconds = 10;
	std::stringstream sec;
	sec << seconds;

	std::string response_str;
	response_str += "HTTP/1.1 " + getStatus(code) + "\r\n"
	"Content-Type: text/plain\r\n"
	"Content-Length: " + n + "\r\n"
	"Retry-After: " + sec.str() + "\r\n"
	"Connection: close\r\n"
	"\r\n" + statusMessages[code];

    // Convert the string to a vector<char>
    header = std::vector<char>(response_str.begin(), response_str.end());
	headerSize = header.size();
	contentLenght = 0;
}

void	HttpResponse::simpleHTTP(std::string path)
{
	filePath = path;
	std::cout << "Path -> " << filePath << std::endl;
	if (filePath == "./var/www/dev/")
		filePath += "index.html";
	openRequestedFile();
	//setStatus();
	status = getStatus(200);
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