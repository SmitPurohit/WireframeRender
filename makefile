

SOURCES = ./src/main.c ./src/model.c
EXE = waverend.exe
CFLAGS = -Wall -Wextra -Wno-unused-variable -Wno-unused-parameter -g
LDFLAGS = -I include -L lib
LIBS = -lopengl32 -lgdi32 -lwinmm -lSDL3
CC = gcc

all: $(EXE)

$(EXE): $(SOURCES)
	$(CC) $(SOURCES) -o $(EXE) $(LDFLAGS) $(LIBS) $(CFLAGS) 

clean:
	rm -f $(EXE)

