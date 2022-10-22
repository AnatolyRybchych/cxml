


objects	+= str_chunk.o
objects	+= cxml.o

build: build_dll build_static

build_dll: $(addprefix obj/, $(objects))
	gcc -shared -Iinclude -Wall -Wextra -Werror -pedantic -o ./bin/libcxml.dll $^

build_static:
	ar -crs ./lib/libcxml.a $^

obj/%.o:src/%.c
	gcc -Iinclude -c -Wall -Wextra -Werror -pedantic -o $@ $^
