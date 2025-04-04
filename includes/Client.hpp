#ifndef CLIENT_HPP
# define CLIENT_HPP
# include <stdio.h>
# include <iostream>
# include <string.h>
# include <fstream>
# include <unistd.h>
# include <sys/socket.h>
# include <vector>
# include <algorithm>
# include <sstream>
# include "../includes/Webserv.h"
# include "HttpResponse.hpp"

# define CHUNK_SIZE 1024

//class	EventHandler;

enum state
{
	WAITING_TO_RECIEVE,
	RECIEVING_REQUEST,
	BUILDING_RESPONSE_BODY,
	WAITING_TO_SEND,
	SENDING_HEADER,
	SENDING_BODY,
	CLEANING_INVALID_REQUEST
};

class Client
{
	private:
		// ATTRIBUTES
		// Client data
		int				_fd;
		EventHandler	&_events;

		// HTTP data
		std::vector<char>	_request;
		HttpResponse		_response;

		// Flags
		bool	_connected;
		bool	_recievingHeader;
		bool	_recievingBody;
		state	_state;

		// MEMBER FUNCTIONS
		// Appends a char* to _request(vector<char>)
		void	appendToRequest(char* str, int size);

		// Parses a request header
		void	parseRequestHeader(std::vector<char>::iterator header_end);

		// Parses a request body
		void	parseRequestBody(std::vector<char>::iterator body_end);

	public:
		// CONSTRUCTORS/DESTRUCTORS
		Client(int fd, EventHandler &events);
		Client(const Client &copy);
		~Client();

		// GETTERS
		int					getFD() const;
		state				getState() const;
		HttpResponse const	&getResponse() const;

		// SETTERS
		void	setState(state state);

		// MEMBER FUNCTIONS
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

		// Tells the Client to Send a max clients response
		void	maxClientsResponse();

		// Opens a file from the desired path
		void	openRequestedFile(std::string path);

		// Recieves from his _fd in a chunk
		int	recieveRequestChunk();

		// Sends the response header in a chunk
		void	sendHeaderChunk();

		// Sends the response body in a chunk 
		void	sendBodyChunk();


		// ERASE LATER
		std::string getPath();

	class	ClientException : public std::runtime_error
	{
		public :
			ClientException(std::string info, int fd);

		private:
			std::string createMessage(std::string info, int fd);
	};
};

#endif