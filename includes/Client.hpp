#ifndef CLIENT_HPP
# define CLIENT_HPP
# include <vector>
# include <fstream>
# include <sstream>
# include <iostream>
# include <string.h>
# include <unistd.h>
# include <algorithm>
# include <sys/socket.h>
# include <cstdlib>
# include <ctime>
# include <string>
# include "EventHandler.hpp"
# include "ServerBlock.hpp"
# include "HttpRequest.hpp"
# include "HttpResponse.hpp"


# define CHUNK_SIZE 4096
# define MAX_BODY 1048576

class EventHandler;

enum client_state
{
	WAITING_TO_RECIEVE,
	RECIEVING_REQUEST,
	WAITING_TO_SEND,
	SENDING_HEADER,
	SENDING_BODY,
	DONE
};

class Client
{
	private:
		// ATTRIBUTES
		// Client data
		int				fd;
		EventHandler	&events;
		ServerBlock		&serverBlock;
		std::string		extracted_path; //location block path
		std::string		sessionId; //used for cookies, initialized at constructor even if cookies are not used. please see function generateSessionId();
		
		// HTTP data
		HttpRequest			request;
        HttpResponse        response;
		std::fstream		postFile;
		
		// Flags
		bool			connected;
		bool			recievingHeader;
		bool			recievingBody;
		client_state	state;
		
		// MEMBER FUNCTIONS
		// Appends a char* to _request(vector<char>)
		void	appendToRequest(char* str, int size);
		
		public:
		// CONSTRUCTORS/DESTRUCTORS
		Client(int fd, EventHandler &events, ServerBlock &serverBlockInfo);
		~Client();
		
		// GETTERS
		
		int					getFD() const;
		client_state		getState() const;
		HttpResponse const	&getResponse() const;

		// SETTERS
		void	setState(client_state state);
		void	setConnection(bool connection);
		
		// MEMBER FUNCTIONS
		void	setRequestStatus(int code);

		// Tells if the client has a regular connection
		bool	isConnected() const;
		
		// Safely closes the Client
		void	closeClient();

		// Activates send mode
		void	sendMode();
		
		// Activates send mode
		void	recieveMode();

		// Activates send mode
		void	waitingMode();

		// Sets the content type of the file to send
		void	setContentType();

		// Opens a file from the desired path
		void	openRequestedFile(std::string path);

		// Recieves from his _fd in a chunk
		int		recieveRequestChunk();

		// Executes the necessary actions in accordance to the request method
		void 	handleMethod();

		// Called when a request body is sent in chunks
		void	resolveChunkedBody();

		// Sends the response header in a chunk
		void	sendHeaderChunk();

		// Sends the response body in a chunk 
		void	sendBodyChunk();

		// Generates a random sessionId to be used for cookie identification
		void	generateSessionId();

	class	ClientException : public std::runtime_error
	{
		public :
			ClientException(std::string info, int fd);

		private:
			std::string createMessage(std::string info, int fd);
	};

	class	ClientErrorException : public std::runtime_error
	{
		public :
			ClientErrorException(std::string info, int fd);

		private:
			std::string createMessage(std::string info, int fd);
	};
};

#endif