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
	contentLenght = 0;
	bodySize = 0;
}

HttpRequest::ResponseException::ResponseException(std::string info) :
runtime_error(info){}