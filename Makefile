NAME		= webserv
CXX			= c++
CXXFLAGS	= -Wall -Wextra -Werror -g -std=c++98
RM			= rm -f

SRC_DIR		= srcs
INC_DIR		= inc
# TEMPLATES_DIR = templates
OBJ_DIR		= objs

SRC_FILES	= main Server CGIHandler HttpRequest Response ConfigParser ServerBlock HttpUtils Routing init


SRCS		= $(addsuffix .cpp, $(addprefix $(SRC_DIR)/, $(SRC_FILES)))

OBJS		= $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

INCS		= -I$(INC_DIR) #-I$(TEMPLATES_DIR)

all: $(NAME)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(INCS) -c $< -o $@

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)
	@echo "$(NAME) compiled successfully!"

serverup: all
		@./$(NAME)

valgrind: all
	valgrind -s --leak-check=full --track-origins=yes ./$(NAME)

clean:
	$(RM) -r $(OBJ_DIR)
	@echo "Object files removed!"

fclean: clean
	$(RM) $(NAME)
	@echo "$(NAME) removed!"

re: fclean all

.PHONY: all clean fclean re