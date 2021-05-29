CXX = g++
LDFLAGS = -ljpeg
INC = -I./include

all: header clean simpeg 

header: include/simpeg.h

simpeg: src/main.cpp src/simpeg.cpp
	$(CXX) -o $@ $^ $(INC) $(LDFLAGS)

clean:
	rm -f simpeg
