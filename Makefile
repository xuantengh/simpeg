CXX = g++
CXXFLAGS = -Ofast -march=native
LDFLAGS = -ljpeg
INC = -I./include

all: header clean simpeg 

header: include/simpeg.h

simpeg: src/main.cpp src/simpeg.cpp
	$(CXX) -o $@ $^ $(INC) $(CXXFLAGS) $(LDFLAGS)

clean:
	rm -f simpeg
