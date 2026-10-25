CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra

TARGET = mini_juego
SRCS = main.cpp Juego.cpp

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)

clean:
	rm -f $(TARGET)
