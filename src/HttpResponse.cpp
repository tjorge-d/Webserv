#include "../includes/HttpResponse.hpp"
#include "../includes/Webserv.h"

// CONSTRUCTORS & DESTRUCTORS

HttpResponse::HttpResponse(): cgi(false)
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

// MEMBER FUNCTIONS

void	HttpResponse::createResponse()
{
	std::string	headerStr;

	std::stringstream lenght, codeStr;
	codeStr << statusCode;

	headerStr += std::string(HTTP_ACCEPTED_VERSION) + " " + codeStr.str() + " " + getStatus(statusCode) + std::string(RESPONSE_LINE_END);
	headerStr += std::string(SERVER_TYPE_RESPONSE_HEADER) + " " + std::string(SERVER_VERSION) + std::string(RESPONSE_LINE_END);
	headerStr += std::string(DATE_TYPE_RESPONSE_HEADER) + " " + getHttpDateHeader() + std::string(RESPONSE_LINE_END);
	setContentType();
	headerStr += std::string(CONTENT_TYPE_RESPONSE_HEADER) + " " + contentType + std::string(RESPONSE_LINE_END);
	if (!filePath.empty() && !cgi)
		openRequestedFile();
	setContentLength();
	lenght << contentLenght;
	headerStr += std::string(CONTENT_LENGTH_RESPONSE_HEADER) + " " + lenght.str() + " " + std::string(RESPONSE_LINE_END);
	headerStr += std::string(LAST_MODIFIED_RESPONSE_HEADER) + " " + getLastModifiedHeader() + std::string(RESPONSE_LINE_END);
	headerStr += std::string(CONNECTION_RESPONSE_HEADER);
	headerStr += statusCode == OK ? " " + connection : " " + std::string(CLOSE_CONNECTION);
	headerStr += std::string(RESPONSE_LINE_END);

	//COOKIES BEGIN HERE

	if (this->currentCookie.find("sessionId=") == std::string::npos)
		headerStr += "Set-Cookie: sessionId=" + sessionId + "; Max-Age=600; Path=/; HttpOnly" + std::string(RESPONSE_LINE_END);
	//put condition to check if theme switch button was pressed or not. default theme is white (alternate, check filepath to see if alt is there before .html)
	if (this->currentPath.find("_alt.html") != std::string::npos)
		headerStr += "Set-Cookie: theme=dark; Max-Age=600; Path=/" + std::string(RESPONSE_LINE_END);
	else if (this->currentPath.find(".html") != std::string::npos)
		headerStr += "Set-Cookie: theme=light; Max-Age=600; Path=/" + std::string(RESPONSE_LINE_END);

	//COOKIES END HERE
	headerStr += "Cache-Control: no-cache, no-store, must-revalidate\r\n";
	headerStr += "Expires: 0\r\n";
	headerStr += std::string(RESPONSE_LINE_END);
	header = std::vector<char>(headerStr.begin(), headerStr.end());
	headerSize = headerStr.size();

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
	if (filePath.empty() || stat(filePath.c_str(), &fileStats) == -1)
		return "";

	std::tm gmt;
	gmtime_r(&fileStats.st_mtime, &gmt);

	char buffer[100];
	std::strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", &gmt);
	return std::string(buffer);
}

void	HttpResponse::setSessionId(std::string clientSessionId){

	this->sessionId = clientSessionId;
}

void HttpResponse::setPath(std::string path){

	this->currentPath = path;
}

void	HttpResponse::openRequestedFile()
{
	size_t queryPos = filePath.find('?');
	std::string pathWithoutQuery = (queryPos != std::string::npos) ? filePath.substr(0, queryPos) : filePath;
	// Protects the function to execute it safely
	if (fileStream.is_open())
		throw ResponseException("A file is already opened");
	
	// Opens the file and retrieves the necessary information
	fileStream.open(pathWithoutQuery.c_str(), std::ios::in);
	if (!fileStream.is_open())
		throw ResponseException("Failed to open the file \"" + pathWithoutQuery + "\"");
	if (!fileStream.good())
		{
			fileStream.close();
			throw ResponseException("Error while opening File");
		}
	if (stat(pathWithoutQuery.c_str(), &fileStats) == -1)
		throw ResponseException("Failed to retrieve the stats of the file \"" + pathWithoutQuery + "\"");
}

void	HttpResponse::setContentType()
{
	// Finds the extension of the file
	if (cgi)
		return ;

	if (statusCode != OK) {
		contentType = PLAIN_TEXT;
		return ;
	}

	if (filePath.empty()) {
		contentType = "application/octet-stream";
		return;
	}
	std::string extension = filePath.substr(filePath.find_last_of('.'));
	if (supportedContentType.count(extension))
		contentType = supportedContentType[extension];
	else
		contentType = "application/octet-stream";
}

void	HttpResponse::setContentLength()
{
	if (statusCode != OK)
		contentLenght = getStatus(statusCode).size();
	else if (!fileStream.is_open())
		contentLenght = body.size();
	else
		contentLenght = fileStats.st_size;
}

void	HttpResponse::reset()
{
	status.clear();
	contentType.clear();
	contentLenght = 0;
	connection.clear();

	header.clear();
	body.clear();
	cgi = false;
	headerSize = 0;
	bytesSent = 0;

	filePath.clear();
	if (fileStream.is_open())
		fileStream.close();
}

HttpResponse::ResponseException::ResponseException(std::string info) :
runtime_error(info){}