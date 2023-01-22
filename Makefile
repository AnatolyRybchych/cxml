
CC		:= gcc
CARGS	:= -c -fPIC -Iinclude -Wall -Wextra -Werror -pedantic
LNK_ARGS:= -shared -Iinclude -Wall -Wextra -Werror -pedantic
DLL_NAME:= ./bin/libcxml.dll
SLL_NAME:= ./lib/libcxml.a

objects	+= str_chunk.o
objects	+= cxml.o

build: build_dll build_static

build_dll: $(addprefix obj/, $(objects))
	@mkdir -p ./bin
	$(CC) $(LNK_ARGS) -o $(DLL_NAME) $^

build_static:$(addprefix obj/, $(objects))
	@mkdir -p ./lib
	ar rcs $(SLL_NAME) $^

obj/%.o:src/%.c
	@mkdir -p ./obj
	$(CC) $(CARGS) -o $@ $^


#################################################################
# TESTS

tests_bin += test1

tests: build_static build_tests
	./test_bin/test1

build_tests: $(addprefix test_bin/, $(tests_bin))

test_bin/%:test/%.c
	@mkdir -p $(dir $@)
	$(CC) -o $@ -Iinclude -Wall -Wextra -Werror -pedantic $^ -Llib -l:libcxml.a -lm
