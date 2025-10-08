# Компилятор и флаги
CXX = g++
CXXFLAGS = -std=c++11 -g -Wall

# Цели
all: test

# Тесты с нашим Google Test
test: device_with_gtest.cpp
	$(CXX) $(CXXFLAGS) device_with_gtest.cpp -o test_runner
	./test_runner

clean:
	rm -f test_runner a.out

.PHONY: all test clean