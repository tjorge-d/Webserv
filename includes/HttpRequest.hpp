#ifndef HTTPREQUEST_HPP
# define HTTPREQUEST_HPP
# include <map>
# include <vector>
# include <fstream>
# include <sstream>
# include <stdexcept>
# include <sys/stat.h>


class HttpRequest
{
	public:
		// ATTRIBUTES
		std::vector<char>	buffer;
		std::string 		method;
		std::string 		path;
		int					contentLenght;
		int					bodySize;

		// CONSTRUCTORS/DESTRUCTORS
		HttpRequest();
		~HttpRequest();
		
		// MEMBER FUNCTIONS
		// Returns a pointer to the end of a HTTP Header if found
		std::vector<char>::iterator findHeader(int tail_size);
		
		// Appends a portion of an array to the buffer
		void	appendToBuffer(char* ar, int size);

		// Erases a portion of the buffer
		void	eraseBufferRange(std::vector<char>::iterator begin, std::vector<char>::iterator end);
		
		// Resets the request info
		void	reset();

	// EXCEPTIONS
	class	ResponseException : public std::runtime_error
	{
		public :
			ResponseException(std::string info);
	};
};

#endif