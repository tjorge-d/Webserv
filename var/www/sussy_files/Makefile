NAME				=	minishell

CC					=	cc
CFLAGS				=	-Wall -Wextra -Werror -g
IFLAGS				=	-I $(INCLUDES)
EXTRA_LIBS			=	-lreadline
SANITIZE			=	-fsanitize=address
RANDOM_MALLOC		=	-Xlinker --wrap=malloc
AR					=	ar rcs
RM					=	rm -rf

INCLUDES			=	includes/
DEFAULT_INCLUDES	=	-I /usr/local/includes

LEXER_FILES			=	check_syntax_errors.c checkers.c lexer_memory_handle.c lexer.c token_list_creation_utils.c token_list_creation.c token_list_split.c
LEXER_PATH			=	srcs/Lexer/
LEXER				=	$(addprefix $(LEXER_PATH), $(LEXER_FILES))

PARSER_FILES		=	ast_creation_utils.c ast_creation.c command_table_creation_utils.c command_table_creation.c handling_here_docs.c new_ast_node_utils.c new_command_table_node_utils.c parser_memory_handle.c parser_utils.c parser.c
PARSER_PATH			=	srcs/Parser/
PARSER				=	$(addprefix $(PARSER_PATH), $(PARSER_FILES))

EXPANDER_FILES		=	ambiguous_redirect_utils.c create_final_cmd_table_utils.c create_final_cmd_table.c expander_memory_handle.c expander.c here_doc_expansions_utils.c here_doc_expansions.c parameter_expansion_utils.c parameter_expansion.c quote_removal_expansion_utils.c quote_removal_expansion.c wildcard_expansion_utils.c wildcard_expansion.c word_split_expansion.c
EXPANDER_PATH		=	srcs/Expander/
EXPANDER			=	$(addprefix $(EXPANDER_PATH), $(EXPANDER_FILES))

BUILTIN_FILES		=	builtins.c mini_cd.c mini_echo.c mini_env.c mini_exit.c mini_export.c mini_pwd.c mini_unset.c units_builtins.c
BUILTIN_PATH		=	Builtins/
BUILTINS			=	$(addprefix $(BUILTIN_PATH_PATH), $(BUILTIN_FILES))

EXECUTOR_FILES		=	$(BUILTIN_FILES) execution_in_fork_utils.c execution_in_fork.c executor_inits.c executor_memory_handle.c executor_utils.c executor.c fd_operations.c subshell_execution.c
EXECUTOR_PATH		=	srcs/Executor/
EXECUTOR			=	$(addprefix $(EXECUTOR_PATH), $(EXECUTOR_FILES))

SRCS				=	$(LEXER_FILES) $(PARSER_FILES) $(EXPANDER_FILES) $(EXECUTOR_FILES) env_utils.c inits.c memory_handle.c minishell.c signals.c utils.c #malloc.c
SRCS_PATH			=	srcs/

OBJ_DIR				=	objects/
OBJS				=	$(SRCS:%.c=$(OBJ_DIR)%.o)
ALL_OBJECTS			=	$(OBJ_DIR)*.o

LIBFT_PATH			=	libft
LIBFT				=	$(LIBFT_PATH)/libft.a
GNL_PATH			=	get_next_line
GNL					=	$(GNL_PATH)/get_next_line.a

TOTAL_SRCS			=	$(words $(SRCS))
TOTAL_OBJS			=	$(words $(wildcard $(OBJ_DIR)*))
FILES				=	0

vpath %.c $(SRCS_PATH) $(LEXER_PATH) $(PARSER_PATH) $(EXPANDER_PATH) $(EXECUTOR_PATH)$(BUILTIN_PATH) $(EXECUTOR_PATH)

$(OBJ_DIR)%.o: 		%.c
					@$(CC) $(CFLAGS) $(IFLAGS) $(DEFAULT_INCLUDES) -c $< -o $@ && \
					$(eval FILES=$(shell echo $$(($(FILES) + 1)))) \
					$(call PRINT_PROGRESS,$(TOTAL_SRCS),$(GRN),$(YELLOW)Compiling$(DEFAULT) $@)

all:				$(NAME)

$(NAME):			$(OBJ_DIR) $(LIBFT) $(OBJS) 
					@$(CC) $(CFLAGS) $(IFLAGS) $(ALL_OBJECTS) $(EXTRA_LIBS) -o $@
					@echo "\033[2F\033[0K$(CYAN)$@$(DEFAULT) successfully created\033[E"
					@if norminette | grep -q -v "OK!"; then \
						norminette | grep -v OK!; echo "Norminette has$(RED) errors!$(DEFAULT)"; \
					else \
						echo "Norminette$(GRN) OK!!$(DEFAULT)"; \
					fi

sanitize:			$(OBJ_DIR) $(LIBFT) $(OBJS)
					@$(CC) $(CFLAGS) $(IFLAGS) $(SANITIZE) $(ALL_OBJECTS) $(EXTRA_LIBS) -o $(NAME)
					@echo "\033[2F\033[0K$(CYAN)$(NAME)$(DEFAULT) successfully created\033[E"

random_m:			$(OBJ_DIR) $(LIBFT) $(OBJS)
					@$(CC) $(CFLAGS) $(IFLAGS) $(SANITIZE) $(RANDOM_MALLOC) $(ALL_OBJECTS) $(EXTRA_LIBS) -o $(NAME)
					@echo "\033[2F\033[0K$(CYAN)$(NAME)$(DEFAULT) successfully created\033[E"

bonus:				$(NAME)

v:					$(NAME)
					clear && valgrind --show-leak-kinds=all --leak-check=full --track-fds=all --track-origins=yes --suppressions=readline.supp ./minishell

$(LIBFT):
					@make -s -C $(LIBFT_PATH) all

$(OBJ_DIR):
					@mkdir -p $@

clean:				
					@$(foreach file,$(wildcard $(OBJ_DIR)*), \
						$(eval FILES=$(shell echo $$(($(FILES) + 1)))) \
						$(call PRINT_PROGRESS,$(TOTAL_OBJS),$(RED),$(YELLOW)Deleting$(DEFAULT) $(file)); \
						$(RM) $(file); \
					)
					@if [ -d "$(OBJ_DIR)" ]; then \
						$(RM) $(OBJ_DIR); \
						echo "\033[2F\033[0K$(PURPLE)$(OBJ_DIR)$(DEFAULT) deleted\033[E"; \
					fi
					$(eval FILES=0)

fclean:				clean
					@if [ -e "$(NAME)" ]; then \
						$(RM) $(NAME); \
						echo "$(PURPLE)$(NAME)$(DEFAULT) deleted"; \
					fi
					@if [ -e "$(LIBFT)" ]; then \
						$(RM) $(LIBFT); \
						echo "$(PURPLE)$(LIBFT)$(DEFAULT) deleted"; \
					fi
					@if [ -e "$(GNL)" ]; then \
						$(RM) $(GNL); \
						echo "$(PURPLE)$(GNL)$(DEFAULT) deleted"; \
					fi

re:					fclean all

.PHONY:				all sanitize random_m clean fclean re bonus sanitize_b random_m_b

define PRINT_PROGRESS
    if [ "$(FILES)" -eq "1" ]; then \
        printf "\033[0K$(3)\n["; \
    else \
        printf "\033[0K\033[1F\033[0K$(3)\n["; \
    fi
    @for i in `seq 1 $(shell expr $(FILES) \* 70 / $(1))`; do \
        printf "$(2)=\033[0m"; \
    done
    @for i in `seq 1 $(shell expr 70 - $(FILES) \* 70 / $(1))`; do \
        printf " "; \
    done
    @printf "] $(shell echo $$(($(FILES) * 100 / $(1))))%%"
	if [ "$(FILES)" -eq "$(1)" ]; then \
        printf "\n"; \
	fi
endef

CYAN				=	\033[36m
PURPLE				=	\033[35m
YELLOW				=	\033[33m
GRN					=	\033[32m
RED					=	\033[31m
DEFAULT				=	\033[0m

