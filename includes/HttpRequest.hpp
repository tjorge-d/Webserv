#ifndef HTTPREQUEST_HPP
# define HTTPREQUEST_HPP
# include <map>
# include <vector>
# include <fstream>
# include <sstream>
# include <stdexcept>
# include <sys/stat.h>
# include <algorithm>
# include <climits>
# include <cctype>
# include "Webserv.h"

class HttpRequest
{
	public:
		// ATTRIBUTES
		std::vector<char>					buffer;
		std::vector<MultiFormData>			formParts;
		std::string		 					chunkBuffer;
		std::string							cookie; //use istringstream instead?
		std::string 						method;
		std::string 						path;
		std::string							version;
		std::string							body;
		std::string							postFileName;
		std::string							bodyContentType;
		std::map<std::string, std::string>	headerInfo;
		int									contentLenght;
		int									bodySize;
		bool 								isChunked;
		bool 								chunkedComplete;

		//will need to add sections for cookies, at least a private attribute to store sessionId

		
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

		// Parses a request header
		int	parseRequestHeader(std::vector<char>::iterator header_end);

		// Chooses the correct way to parse the body depending on the content type specified on the request
		void	parseRequestBody(void);

		// Parses body of type multipart/form-data
		void	parseMultiPartFormData(void);

		// Parses body of any text type
		void	parseTextPlain(void);

		// Safe integer parsing with bounds checking
		bool	safeParseInt(const std::string& str, int& result, int min_val = 0, int max_val = INT_MAX);

		// Convert string to lowercase for case-insensitive header comparison
		std::string	toLowerCase(const std::string& str) const;

		// Get header value in case-insensitive manner
		std::string	getHeader(const std::string& key) const;

	// EXCEPTIONS
	class	ResponseException : public std::runtime_error
	{
		public :
			ResponseException(std::string info);
	};
};

#endif