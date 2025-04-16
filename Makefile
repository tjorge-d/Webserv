NAME= webserv

CXX= c++
CXXFLAGS= -Wall -Werror -Wextra -g

SRC_A= $(addprefix src/, $(SOURCES_A))
SOURCES_A= main.cpp	\
EventHandler.cpp	\
BindingSocket.cpp	\
ListeningSocket.cpp	\
ConnectingSocket.cpp	\
Socket.cpp	\
Client.cpp	\
HttpResponse.cpp	\
HttpRequest.cpp	\
Parser.cpp

OBJ_DIR= objects
OBJ= $(addprefix $(OBJ_DIR)/, $(SRC_A:src/%.cpp=%.o))

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $@

$(OBJ_DIR)/%.o: src/%.cpp
	mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

run: re
	clean && ./webserv

.PHONY: all fclean clean re run