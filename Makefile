SOURCES = $(wildcard ./src/*.c)
SRC = $(addprefix src/, $(SOURCES))
OBJ = $(addsuffix .o, $(addprefix bin/, $(basename $(notdir $(SRC)))));
INCLUDE = -I include
CFLAGS = -pedantic -W -O3 -D_DEBUG_

all: server

again: clean server

server: $(OBJ)
	clang -W /usr/lib/Gaia/Hephaestus.so $^ -lpthread -lm -ldl -lcrypto -lssl -lraylib -o $@

bin/%.o : src/%.c
	clang $(INCLUDE) $(CFLAGS) -c $< -o $@

clean:
	rm -f bin/*.o
	rm server

install:
	echo "Can't install surry"

try: server
	./server

run:
	./server
