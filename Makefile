CXX = g++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98
INCLUDES = -I./include

# Source files
SRCS = src/main.cpp src/Webserv.cpp src/Request.cpp
OBJS = $(SRCS:.cpp=.o)

# Target executable
TARGET = webserv

# Build target
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET)

# Compile source files into object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Clean up build files
clean:
	rm -f $(OBJS) $(TARGET) *.d

re: clean $(TARGET)

# Include dependency files
-include $(OBJS:.o=.d)