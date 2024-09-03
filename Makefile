NAME	:=	webserv

CPP		:=	c++
CFLAGS	:=	-std=c++98 -g3 -MMD

INC_DIR	:=	./include
SRC_DIR	:=	./src
BIN_DIR	:=	./bin

# SRC		:=	$(wildcard $(SRC_DIR)/*.cpp)
SRC		:=	$(SRC_DIR)/main.cpp $(SRC_DIR)/Configuration.cpp

OBJ		:=	$(SRC:$(SRC_DIR)/%.cpp=$(BIN_DIR)/%.o)

-include $(wildcard *.d)

all: $(NAME)
# ./$(NAME) file.conf

$(NAME): $(OBJ)
	$(CPP) $(CFLAGS) $(OBJ) -I $(INC_DIR) -o $(NAME)

$(BIN_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CPP) $(CFLAGS) -I $(INC_DIR) -c $< -o $@

# Clean up build files
clean:
	rm -rf $(BIN_DIR)

fclean: clean $(TARGET)

re: fclean all

.PHONY: all clean fclean re
