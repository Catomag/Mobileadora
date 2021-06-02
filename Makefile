SOURCES = $(wildcard ./src/*.c)
DEMOS_SOURCES = $(wildcard ./demos/*.c)
SRC = $(addprefix src/, $(SOURCES))
DEMOS_SRC = $(addprefix src/, $(DEMOS_SOURCES))
OBJ = $(addsuffix .o, $(addprefix bin/, $(basename $(notdir $(SRC)))))
DEMOS_OBJ = $(addsuffix .out, $(addprefix bin/, $(basename $(notdir $(DEMOS_SRC)))));
INCLUDE = -I include
CFLAGS = -pedantic -W -O3 -D_DEBUG_

all: objects demos

again: clean all

objects: $(OBJ)

demos: $(DEMOS_OBJ)


bin/%.o : src/%.c
	clang $(INCLUDE) $(CFLAGS) -c $< -o $@

bin/%.out: demos/%.c
	clang -W /usr/lib/Gaia/Hephaestus.so $(OBJ) $^ -lpthread -lm -ldl -lcrypto -lssl -lraylib -o $@

clean:
	rm -f bin/*

install:
	echo "Can't install surry"

try: demos
	./bin/asteroids.out

run:
	./server
