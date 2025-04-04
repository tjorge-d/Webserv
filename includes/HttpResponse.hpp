#ifndef HTTPRESPONSE_HPP
# define HTTPRESPONSE_HPP
# include <stdio.h>
# include <iostream>
# include <string.h>
# include <fstream>
# include <sys/stat.h>
# include <vector>
# include <string.h>

class HttpResponse
{
	public:
		// ATTRIBUTES
		std::map<std::string, std::string>    supportedContentType;
		
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
		void	setStatus();
		void	openRequestedFile();
		void	setContentType();
		void	setContentLength();
		void	setConnection();
		void	buildHeader();
		void	resetResponse();

	// EXCEPTIONS
	class	ResponseException : public std::runtime_error
	{
		public :
			ResponseException(std::string info);
	};
};

#endif