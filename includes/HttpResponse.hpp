#ifndef HTTPRESPONSE_HPP
# define HTTPRESPONSE_HPP
# include <map>
# include <vector>
# include <fstream>
# include <sstream>
# include <stdexcept>
# include <sys/stat.h>
# include <iostream>
# include <ctime>

class HttpResponse
{
	public:
		// ATTRIBUTES
		std::map<std::string, std::string>		supportedContentType;
		
		int				statusCode;
		std::string 	status;
		std::string 	contentType;
		int				contentLenght;
		std::string 	connection;

		std::vector<char>	header;
		int					headerSize;
		int 				bytesSent;
		
		std::string		filePath;
		struct stat		fileStats;
		std::fstream	fileStream;

		// CONSTRUCTORS/DESTRUCTORS
		HttpResponse();
		~HttpResponse();

		// MEMBER FUNCTIONS
		void		createResponse();

		std::string	getHttpDateHeader();
		std::string getLastModifiedHeader();

		void		openRequestedFile();
		void		setContentType();
		void		setContentLength();
		void		reset();

	// EXCEPTIONS
	class	ResponseException : public std::runtime_error
	{
		public :
			ResponseException(std::string info);
	};
};

#endif