#include "../includes/HttpResponse.hpp"

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

// MEMBER FUNCTIONS
void	HttpResponse::simpleHTTP(std::string path)
{
	filePath = path;
	openRequestedFile();
	setStatus();
	setContentType();
	setContentLength();
	setConnection();
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

void	HttpResponse::setStatus()
{
	status = "200 OK";
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