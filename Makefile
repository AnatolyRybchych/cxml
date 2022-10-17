
objects	+= main.o
objects	+= str_chunk.o

build: $(addprefix obj/, $(objects))
	gcc -Iinclude -Wall -Wextra -Werror -pedantic -o run.exe $^

obj/%.o:src/%.c
	gcc -Iinclude -c -Wall -Wextra -Werror -pedantic -o $@ $^

run: build
	./run.exe