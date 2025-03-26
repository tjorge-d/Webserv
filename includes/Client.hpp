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

enum state
{
	WAITING,
	RECIEVING_REQUEST,
	CLEANING_INVALID_REQUEST,
	SENDING_RESPONSE
};

class Client
{
	private:
		// ATTRIBUTES
		// Client Data
		int					_fd;
		
		// Request data
		int					_pendingRequests;
		int					_chunksNumber;
		std::vector<char>	_request;

		// Response data
		std::vector<char>	_response;
		int					_responseSize;
		int 				_bytesSent;
		
		// Flags
		bool				_waitingHeader;
		bool				_waitingBody;
		state				_state;

		// MEMBER FUNCTIONS
		// Appends a char* to _request(vector<char>)
		void	appendToRequest(char* str, int size);

		// Parses a request header;
		void	parseRequestHeader(std::vector<char>::iterator header_end);

		// Parses a request body;
		void	parseRequestBody(std::vector<char>::iterator body_end);

	public:
		// CONSTRUCTORS/DESTRUCTORS
		Client(int fd);
		Client(const Client &copy);
		~Client();

		// GETTERS
		int					getFD();
		state				getState();
		std::vector<char>	getResponse();

		// MEMBER FUNCTIONS
		// Safely closes the Client
		void	closeClient();

		// Tells the Client he wants to wait for a new request
		void	newRequest();

		// Recieves from his _fd in a chunk
		int	recieveRequestChunk(int chunk_size);

		// Sends to his _fd in a chunk
		int	sendResponseChunk(int chunk_size);
	
	class	ClientCloseFailure : public std::runtime_error
	{
		public :
			ClientCloseFailure();
	};

	class	RecieveFailure : public std::runtime_error
	{
		public :
			RecieveFailure();
	};

	class	SendFailure : public std::runtime_error
	{
		public :
			SendFailure();
	};
};

#endif