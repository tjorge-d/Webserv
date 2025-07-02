#ifndef HTTPRESPONSE_HPP
# define HTTPRESPONSE_HPP
# include <map>
# include <vector>
# include <fstream>
# include <sstream>
# include <stdexcept>
# include <sys/stat.h>
# include <iostream>

class HttpResponse
{
	public:
		// ATTRIBUTES
		std::map<std::string, std::string>		supportedContentType;
		std::map<std::string, std::string>		failsafe_error_codes;
		
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
		void	simpleHTTP(std::string path);
		void	simpleHTTPerror(std::string path, std::string error_response);
		void	openRequestedFile();
		void	setStatus(std::string status);
		void	setContentType();
		void	setContentLength();
		void	setConnection();
		void	buildHeader();
		void	reset();

	// EXCEPTIONS
	class	ResponseException : public std::runtime_error
	{
		public :
			ResponseException(std::string info);
	};
};

#endif