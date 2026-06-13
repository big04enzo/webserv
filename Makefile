NAME = irc

CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

SRC =	src/main.cpp \
		src/server.cpp \
		src/client.cpp \
		src/cmd.cpp \
		src/auth.cpp \
		src/clienttools.cpp 

OBJ = $(SRC:.cpp=.o)

INCLUDES = -Iincludes

RM = rm -f

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	$(RM) $(OBJ)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re