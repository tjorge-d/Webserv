NAME= webserv

CC= cc
CFLAGS= -Wall -Werror -Wextra -g

SRC_A= $(addprefix src/a/, $(SOURCES_A))
SOURCES_A=					

SRC_B= $(addprefix src/b/, $(SOURCES_B))
SOURCES_B=

SRC_C= $(addprefix src/c/, $(SOURCES_C))
SOURCES_C=
		
OBJ_DIR= objects
OBJ=	$(addprefix $(OBJ_DIR)/, $(SRC_BI:src/a/%.c=%.o)) \
		$(addprefix $(OBJ_DIR)/, $(SRC_EX:src/b/%.c=%.o)) \
		$(addprefix $(OBJ_DIR)/, $(SRC_PA:src/c/%.c=%.o))

LIB_DIR= ./lib
LIB= $(LIB_DIR)/lib.a

all: $(NAME)

$(NAME): $(OBJ) $(LIB)
	$(CC) $(CFLAGS) $(OBJ) -lreadline -o $@ $(LIB)

$(OBJ_DIR)/%.o: src/a/%.c
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: src/b/%.c
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: src/c/%.c
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(LIB):
	$(MAKE) -C $(LIB_DIR)

clean:
	rm -rf $(OBJ_DIR)
	$(MAKE) -C $(LIB_DIR) clean

fclean: clean
	rm -f $(NAME)
	$(MAKE) -C $(LIB_DIR) fclean

re: fclean all

run: re
	clear && ./webserv

.PHONY: all fclean clean re run