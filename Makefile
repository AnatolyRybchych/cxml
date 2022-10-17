
objects	:= main.o

build: $(addprefix obj/, $(objects))
	gcc -Wall -Wextra -Werror -pedantic -o run.exe $^

obj/%.o:src/%.c
	gcc -c -Wall -Wextra -Werror -pedantic -o $@ $^

run: build
	./run.exe