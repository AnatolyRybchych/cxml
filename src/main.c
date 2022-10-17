#include <stdio.h>
#include <cxml.h>

#define UNUSED(PARAM) ((PARAM) = (PARAM))
 
void print_tag(const StrChunk *tag_name, const StrChunk *innert_text, const StrChunk *attributes, void *user_data);
void print_attributes(const StrChunk *attribute_name, const StrChunk *attribute_value, void *user_data);

int main(){
    VAR_STR_CHUNK(chunk, L""
    "<tag attib=\"value\"></tag>"
    );

    StrChunk actual;
    if(cxml_iter_tag(&chunk, &actual, print_tag, NULL)){
        printf("SUCCESS!\n");
        printf("actual tag: \"%.*ls\"", (int)(actual.end - actual.beg), actual.beg);
    }
    else printf("ERROR: parsing failed\n");
    
    return 0;
}

void print_tag(const StrChunk *tag_name, const StrChunk *innert_text, const StrChunk *attributes, void *user_data){
    UNUSED(user_data);
    printf("<%.*ls", (int)(tag_name->end - tag_name->beg), tag_name->beg);
    cxml_iter_attributes(attributes, print_attributes, NULL);
    printf(">");
    printf("%.*ls", (int)(innert_text->end - innert_text->beg), innert_text->beg);
    printf("</%.*ls>", (int)(tag_name->end - tag_name->beg), tag_name->beg);
}

void print_attributes(const StrChunk *attribute_name, const StrChunk *attribute_value, void *user_data){
    UNUSED(user_data);
    printf(" %.*ls", (int)(attribute_name->end - attribute_name->beg), attribute_name->beg);
    printf("=\"%.*ls\"", (int)(attribute_value->end - attribute_value->beg), attribute_value->beg);
}


