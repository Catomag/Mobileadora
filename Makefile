SOURCES = $(wildcard ./src/*.c)
SRC = $(addprefix src/, $(SOURCES))
OBJ = $(addsuffix .o, $(addprefix bin/, $(basename $(notdir $(SRC)))));
INCLUDE = -I include
CFLAGS = -std=c99 -pedantic -W -O2 -D_DEBUG_

all: server

again: clean server

server: $(OBJ)
	# windows
	# x86_64-w64-mingw32-gcc -L deps/bin $^ -lOpenAL32 -lopengl32 --static -lglfw3 -lglad -lm -lpthread -o $@
	# linux (assuming all libraries are installed in the system
	gcc -W $^ -lpthread -lm -ldl -o $@

bin/%.o : src/%.c
	# x86_64-w64-mingw32-gcc $(INCLUDE) $(CFLAGS) -c $< -o $@
	# linux
	gcc $(INCLUDE) $(CFLAGS) -c $< -o $@

clean:
	rm -f bin/*.o
	rm server

install:
	echo "Can't install surry"

try: server
	./server

run:
	./server
