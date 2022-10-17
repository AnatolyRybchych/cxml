
objects	+= main.o
objects	+= str_chunk.o
objects	+= cxml.o

build: $(addprefix obj/, $(objects))
	gcc -Iinclude -ggdb -Wall -Wextra -Werror -pedantic -o run.exe $^

obj/%.o:src/%.c
	gcc -Iinclude -ggdb -c -Wall -Wextra -Werror -pedantic -o $@ $^

run: build
	./run.exe

gdb: build
	gdb --tui ./run.exe
