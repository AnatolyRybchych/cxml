
CC		:= gcc
CARGS	:= -Iinclude -c -Wall -Wextra -Werror -pedantic
LNK_ARGS:= -shared -Iinclude -Wall -Wextra -Werror -pedantic
DLL_NAME:= ./bin/libcxml.dll

objects	+= str_chunk.o
objects	+= cxml.o

build: build_dll build_static

build_dll: $(addprefix obj/, $(objects))
	$(CC) $(LNK_ARGS) -o $(DLL_NAME) $^

build_static:
	ar -crs ./lib/libcxml.a $^

obj/%.o:src/%.c
	$(CC) $(CARGS) -o $@ $^
