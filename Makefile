CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O3 -Iinclude

# Core source and object files
SRC = src/socket.cpp src/sender.cpp src/receiver.cpp src/loss_emulator.cpp
OBJ = $(SRC:.cpp=.o)

# Default target builds both binaries
all: sender_app receiver_app

# Build sender application
sender_app: $(OBJ) apps/sender_main.cpp
	$(CXX) $(CXXFLAGS) $(OBJ) apps/sender_main.cpp -o sender_app

# Build receiver application
receiver_app: $(OBJ) apps/receiver_main.cpp
	$(CXX) $(CXXFLAGS) $(OBJ) apps/receiver_main.cpp -o receiver_app

# Pattern rule for compiling source files
src/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build artifacts
clean:
	rm -f src/*.o sender_app receiver_app