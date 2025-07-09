#include "../includes/HttpRequest.hpp"

// CONSTRUCTORS & DESTRUCTORS

HttpRequest::HttpRequest()
{
	reset();
}

HttpRequest::~HttpRequest()
{}

// MEMBER FUNCTIONS

std::vector<char>::iterator	HttpRequest::findHeader(int	tail_size)
{
	int offset = tail_size + 4 * (static_cast<int>(buffer.size()) >= tail_size + 4);
	std::vector<char>::iterator it;
	it = std::search(buffer.end() - offset, buffer.end(), "\r\n\r\n", &"\r\n\r\n"[4]);
	return (it);
}

void	HttpRequest::appendToBuffer(char* ar, int size)
{
	buffer.insert(buffer.end(), ar, ar + size);
}

void	HttpRequest::eraseBufferRange(std::vector<char>::iterator begin, std::vector<char>::iterator end)
{
	buffer.erase(begin, end);
}

void	HttpRequest::reset()
{
	buffer.clear();
	method.clear();
	path.clear();
	version.clear();
	body.clear();
	headerInfo.clear();
	chunkBuffer.clear();
	isChunked = false;
	chunkedComplete = false;
	contentLenght = 0;
	bodySize = 0;
}

int	HttpRequest::parseRequestHeader(std::vector<char>::iterator header_end)
{
	// Convert the request vector to a string
    std::string requestString(buffer.begin(), buffer.end());

    // Find the first line (request line)
    std::istringstream requestStream(requestString);
    std::string requestLine;
    if (!std::getline(requestStream, requestLine) || requestLine.empty()){
		//basicClientResponse("Bad request.", getStatus(400));
		//setConnection(false);
		return (BAD_REQUEST);
	}

    // Split the request line into components
    std::istringstream lineStream(requestLine);
    lineStream >> method >> path >> version;

	if (version != HTTP_ACCEPTED_VERSION){
		//basicClientResponse("Unsupported HTTP version.", getStatus(505));
		//setConnection(false);
		return (HTTP_VERSION_NOT_SUPPORTED);
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

		headerInfo[key] = value;

		if (key == "Content-Length")
			contentLenght = std::atoi(value.c_str());
		else if (key == "Transfer-Encoding" && value == "chunked"){
			isChunked = true;
		}
	}

	// Clean request buffer
	eraseBufferRange(buffer.begin(), header_end);

	return (OK);
}

void	HttpRequest::parseRequestBody(){
	if (headerInfo["Content-Type"].substr(0, headerInfo["Content-Type"].find(";")) == "multipart/form-data")
		parseMultiPartFormData();
	else if (headerInfo["Content-Type"].substr(0, headerInfo["Content-Type"].find(";")).substr(0, headerInfo["Content-Type"].find("/")) == "text")
		parseTextPlain();
}

void	HttpRequest::parseMultiPartFormData(){
	std::string	boundaryKey = "boundary=";
	size_t		pos = headerInfo["Content-Type"].find(boundaryKey), nextPart;
	std::string	boundary = headerInfo["Content-Type"].substr(pos + boundaryKey.size());
	std::string delimiter = "--" + boundary, endDelimiter = delimiter + "--";
	std::string	requestBody(buffer.begin(), buffer.end());

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

		formParts.push_back(tmpStruct);
        pos = partEnd;
	}

	for (std::vector<MultiFormData>::iterator it = formParts.begin(); it != formParts.end(); ++it){
		MultiFormData &part = *it;

		if (part.headers.count("Content-Disposition")){
			std::string	fileNameKey = "filename=";
			pos = part.headers["Content-Disposition"].find(fileNameKey);
	 		postFileName = part.headers["Content-Disposition"].substr(pos + fileNameKey.size());
			postFileName = "./var/www/sussy_files/" + postFileName.substr(1,  postFileName.size() - 2);
		}
		if (part.headers.count("Content-Type"))
	 		bodyContentType = part.headers["Content-Type"];

		body.append(part.content.c_str(), part.content.size());
	}
}

void	HttpRequest::parseTextPlain(){
	std::string	requestBody(buffer.begin(), buffer.end());

	postFileName = "./var/www/sussy_files/" + path.substr(1);
	body = requestBody;
}

HttpRequest::ResponseException::ResponseException(std::string info) :
runtime_error(info){}