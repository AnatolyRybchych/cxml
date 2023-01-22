#include <cxml.h>

#define TEST_MAIN
#include "test.h"

#define XML(...) L## #__VA_ARGS__

bool parse_declaration_pretty_format_without_values(void){
    const wchar_t *xml = XML(<?xml?>);
    StrChunk xml_chunk = {xml, xml + wcslen(xml)};

    StrChunk xml_decl;
    CXML_Declaration decl;

    if(cxml_read_decl(&xml_chunk, &xml_decl, &decl)){
        if(strcmp(decl.encoding, "UTF-8")) FAILED_WITH_MESSAGE("encoding mismatch");
        if(decl.version_major != 1) FAILED_WITH_MESSAGE("version major mismatch");
        if(decl.version_minor != 0) FAILED_WITH_MESSAGE("version minor mismatch");
        if(decl.standalone) FAILED_WITH_MESSAGE("standalone mismatch");
        PASSED();
    }
    else{
        printf("cxml_read_decl() -> false\n");
        return false;
    }
}

bool parse_declaration_pretty_format_default_values(void){
    const wchar_t *xml = XML(<?xml version="1.0", encoding="UTF-8", standalone="no"?>);
    StrChunk xml_chunk = {xml, xml + wcslen(xml)};

    StrChunk xml_decl;
    CXML_Declaration decl;

    if(cxml_read_decl(&xml_chunk, &xml_decl, &decl)){
        if(strcmp(decl.encoding, "UTF-8")) FAILED_WITH_MESSAGE("encoding mismatch");
        if(decl.version_major != 1) FAILED_WITH_MESSAGE("version major mismatch");
        if(decl.version_minor != 0) FAILED_WITH_MESSAGE("version minor mismatch");
        if(decl.standalone) FAILED_WITH_MESSAGE("standalone mismatch");
        PASSED();
    }
    else{
        printf("cxml_read_decl() -> false\n");
        return false;
    }
}

bool parse_declaration_ugly_format_version1_1_encoding_unicode_standalone(void){
    const wchar_t *xml = XML(
        <     \t  ?  xml \n
              version       \t\r ="1.1"   ,\t encoding          =       "UNICODE" ,\n
        standalone=      "yes" ?      >);
    StrChunk xml_chunk = {xml, xml + wcslen(xml)};

    StrChunk xml_decl;
    CXML_Declaration decl;

    if(cxml_read_decl(&xml_chunk, &xml_decl, &decl)){
        if(strcmp(decl.encoding, "UNICODE")) FAILED_WITH_MESSAGE("encoding mismatch");
        if(decl.version_major != 1) FAILED_WITH_MESSAGE("version major mismatch");
        if(decl.version_minor != 1) FAILED_WITH_MESSAGE("version minor mismatch");
        if(!decl.standalone) FAILED_WITH_MESSAGE("standalone mismatch");
        PASSED();
    }
    else{
        printf("cxml_read_decl() -> false\n");
        return false;
    }
}

int main(void){
    Test tests[] = {
        TEST(parse_declaration_pretty_format_default_values),
        TEST(parse_declaration_pretty_format_without_values),
        TEST(parse_declaration_ugly_format_version1_1_encoding_unicode_standalone),
    };
    #define TESTS_CNT (sizeof(tests) / sizeof(tests[0]))

    for(size_t i = 0; i < TESTS_CNT; i++){
        execute_test(tests[i]);
    }
    return 0;
}
