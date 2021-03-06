CC=g++
CFLAGS=-c -g -Wall -Wno-comment
LDFLAGS=-L/usr/local/lib -lglfw -lGLEW -lcurl -lstdc++ -lpng
SOURCES=src/main.cpp src/Dispatcher.cpp src/Atlas.cpp src/Fetcher.cpp src/PngDecoder.cpp src/Renderer.cpp src/Tile.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=3dmap

all: $(SOURCES) $(EXECUTABLE)
    
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@

clean:
	rm -rf src/*.o 3dmap
