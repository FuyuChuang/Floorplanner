CC=g++
LDFLAGS=-std=c++11 -O3 -lm
SOURCES=src/bStarTree.cpp src/floorplanner.cpp src/module.cpp src/main.cpp
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=Floorplanner
INCLUDES=src/module.h src/bStarTree.h src/floorplanner.h

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

%.o:  %.c  ${INCLUDES}
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf *.o $(EXECUTABLE)
