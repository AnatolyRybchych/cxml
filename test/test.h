#ifndef TEST_H_
#define TEST_H_

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define TEST(function) (Test){.test_name = __STRING(function), .test_func = function}
#define FAILED_WITH_MESSAGE(fmt, ...) do { printf(fmt"\n" ,##__VA_ARGS__ ); return false;} while(0)
#define PASSED() return true;

typedef struct Test Test;

struct Test{
    const char *test_name;
    bool (*test_func)(void);
};

void execute_test(Test test);

#ifdef TEST_MAIN

void execute_test(Test test){
    printf("\x1b[32mTEST %s:\x1b[0m\n", test.test_name);
    bool passed = test.test_func();
    if(passed){
        printf("\x1b[32mPASSED\x1b[0m\n");
    }
    else{
        printf("\x1b[31mFAILED[\x1b[0m\n");
    }
    puts("");
}

#endif

#endif //TEST_H_