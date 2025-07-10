#include "../includes/HttpResponse.hpp"
#include "../includes/Webserv.h"

// CONSTRUCTORS & DESTRUCTORS

HttpResponse::HttpResponse()
{
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
	int					size = getStatus(code).size();
	int					seconds = 10;
	std::stringstream sizeStr, codeStr, sec;
	sizeStr << size;
	codeStr << code;
	sec << seconds;

	std::string response_str;
	response_str += "HTTP/1.1 " + codeStr.str() + " " + getStatus(code) + "\r\n"
	"Content-Type: text/plain\r\n"
	"Content-Length: " + sizeStr.str() + "\r\n"
	"Retry-After: " + sec.str() + "\r\n"
	"Connection: close\r\n"
	"\r\n" + getStatus(code);

    // Convert the string to a vector<char>
    header = std::vector<char>(response_str.begin(), response_str.end());
	headerSize = header.size();
	contentLenght = 0;
}

void	HttpResponse::simpleHTTP(std::string path)
{
	filePath = path;
	// std::cout << "Path -> " << filePath << std::endl;
	// if (filePath == "./var/www/dev/")
	// 	filePath += "index.html";
	std::cout << "Path -> " << filePath << std::endl;
	openRequestedFile();
	//setStatus();
	//status = getStatus(OK);
	setContentType();
	setContentLength();
	//setConnection();
	buildHeader();
}

void	HttpResponse::buildHeader()
{
	// Builds the header
	std::stringstream lenght, codeStr;
	lenght << contentLenght;
	codeStr << statusCode;

	std::string header_str;
	header_str += "HTTP/1.1 " + codeStr.str() + " " + status + "\r\n";
	header_str += "Content-Type: " + contentType + "\r\n";
	header_str += "Content-Length: " + lenght.str() + "\r\n";
	header_str += "Connection: " + connection + "\r\n\r\n";

	header = std::vector<char>(header_str.begin(), header_str.end());
	headerSize = header_str.size();
	std::cout << "Response:" << std::endl << header_str << std::endl;
}

void	HttpResponse::createResponse()
{
	std::string	headerStr;

	std::stringstream lenght, codeStr;
	codeStr << statusCode;

	headerStr += std::string(HTTP_ACCEPTED_VERSION) + " " + codeStr.str() + " " + getStatus(statusCode) + std::string(RESPONSE_LINE_END);
	headerStr += std::string(SERVER_TYPE_RESPONSE_HEADER) + " " + std::string(SERVER_VERSION) + std::string(RESPONSE_LINE_END);
	headerStr += std::string(DATE_TYPE_RESPONSE_HEADER) + " " + getHttpDateHeader() + std::string(RESPONSE_LINE_END);
	if (statusCode == OK)
		setContentType();
	else
		contentType = PLAIN_TEXT;
	headerStr += std::string(CONTENT_TYPE_RESPONSE_HEADER) + " " + contentType + std::string(RESPONSE_LINE_END);
	if (!filePath.empty())
		openRequestedFile();
	setContentLength();
	lenght << contentLenght;
	headerStr += std::string(CONTENT_LENGTH_RESPONSE_HEADER) + " " + lenght.str() + " " + std::string(RESPONSE_LINE_END);
	headerStr += std::string(DATE_TYPE_RESPONSE_HEADER) + " " + getLastModifiedHeader() + std::string(RESPONSE_LINE_END);
	headerStr += std::string(CONNECTION_RESPONSE_HEADER);
	headerStr += statusCode == OK ? " " + connection : " " + std::string(CLOSE_CONNECTION);
	headerStr += std::string(RESPONSE_LINE_END);
	//headerStr += statusCode == OK ? "" : getStatus(statusCode);
	headerStr += std::string(RESPONSE_LINE_END);

	header = std::vector<char>(headerStr.begin(), headerStr.end());
	headerSize = headerStr.size();
	std::cout << "Response:" << std::endl << headerStr << std::endl;
}

std::string	HttpResponse::getHttpDateHeader()
{
	std::time_t date = std::time(NULL);
    std::tm gmt;
    gmtime_r(&date, &gmt);

	char	buffer[100];
    std::strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", &gmt);
    return std::string(buffer);
}

std::string HttpResponse::getLastModifiedHeader()
{
    if (stat(filePath.c_str(), &fileStats) == -1)
		return "";

    std::tm gmt;
    gmtime_r(&fileStats.st_mtime, &gmt);

	char	buffer[100];
    std::strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", &gmt);
    return std::string(buffer);    
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
	else if (statusCode != OK)
		contentLenght = getStatus(statusCode).size();
	else
		contentLenght = fileStats.st_size;
}

void	HttpResponse::setConnection()
{
	connection = "keep-alive";
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