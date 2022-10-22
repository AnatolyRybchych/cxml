
CC		:= gcc
CARGS	:= -Iinclude -c -Wall -Wextra -Werror -pedantic
LNK_ARGS:= -shared -Iinclude -Wall -Wextra -Werror -pedantic
DLL_NAME:= ./bin/libcxml.dll

objects	+= str_chunk.o
objects	+= cxml.o

build: build_dll build_static

build_dll: $(addprefix obj/, $(objects))
	@mkdir -p ./bin
	$(CC) $(LNK_ARGS) -o $(DLL_NAME) $^

build_static:
	@mkdir -p ./lib
	ar -crs ./lib/libcxml.a $^

obj/%.o:src/%.c
	@mkdir -p ./obj
	$(CC) $(CARGS) -o $@ $^
