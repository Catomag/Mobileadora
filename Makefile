# Terrific Makefile which does ALL the things
SOURCES = $(wildcard ./src/*.c)
DEMOS_SOURCES = $(wildcard ./demos/*.c)
SRC = $(addprefix src/, $(SOURCES))
DEMOS_SRC = $(addprefix src/, $(DEMOS_SOURCES))
OBJ = $(addsuffix .o, $(addprefix bin/, $(basename $(notdir $(SRC)))))
DEMOS_OBJ = $(addsuffix .out, $(addprefix demos/, $(basename $(notdir $(DEMOS_SRC)))));
INCLUDE = -I include
CFLAGS = -O2 # -g

all: objects demos

again: clean all

objects: $(OBJ)

demos: $(DEMOS_OBJ)

bin/%.o : src/%.c
	$(CC) $(INCLUDE) $(CFLAGS) -c $< -o $@

demos/%.out: demos/%.c
	$(CC) -W $(OBJ) $^ $(CFLAGS) -lpthread -lm -ldl -lcrypto -lssl -lraylib -o $@

clean:
	rm -f bin/*.o
	rm -f bin/*.out
	rm -f demos/*.out

install:
	echo "Can't install surry"
