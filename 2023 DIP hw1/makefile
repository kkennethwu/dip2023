CXX = g++
CXXFLAGS = -std=c++11  # Adjust this to your desired C++ version

all: hw1

hw1: hw1.cpp
	$(CXX) $(CXXFLAGS) hw1.cpp -o hw1

run: hw1
	./hw1 $(VAR)

clean:
	rm -f hw1