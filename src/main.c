#include <stdio.h>
#include <stdbool.h>
#include <wchar.h>

typedef struct StrChunk StrChunk;

struct StrChunk{
    const wchar_t *beg;
    const wchar_t *end;
};

bool xml_iter_tag(
    const StrChunk *source, 
    void (*on_tag)(const StrChunk *tag_name, void *user_data), 
    void (*on_attribute)(const StrChunk *attribute_name, const StrChunk *attribute_data, void *user_data), 
    void (*on_text)(const StrChunk *text, void *user_data), 
    void *user_data);

void print_tag(const StrChunk *tag_name, void *user_data);
void print_attribute(const StrChunk *attribute_name, const StrChunk *attribute_data, void *user_data);
void print_text(const StrChunk *text, void *user_data);

wchar_t _white_spaces[] = L" \n\t\v\f\r";
StrChunk white_spaces = {
    .beg = _white_spaces,
    .end = _white_spaces + sizeof(_white_spaces) / sizeof(*_white_spaces) - 1 
};

int main(){
    wchar_t data[] = L""
    "<tag></tag>";

    StrChunk chunk = {
        .beg = data,
        .end = data + wcslen(data)
    };

    if(!xml_iter_tag(&chunk, print_tag, print_attribute, print_text, NULL)){
        printf("ERROR: parsing failed\n");
    }
    else{
        printf("SUCCESS!\n");
    }


    return 0;
}

void print_tag(const StrChunk *tag_name, void *user_data){
    user_data = user_data;//unused
    printf("tag: <%*.ls>\n", (int)(tag_name->end - tag_name->beg), tag_name->beg);
}

void print_attribute(const StrChunk *attribute_name, const StrChunk *attribute_data, void *user_data){
    user_data = user_data;//unused
    printf("attribute: %*.ls=%*.ls\n", 
        (int)(attribute_name->end - attribute_name->beg), attribute_name->beg, 
        (int)(attribute_data->end - attribute_data->beg), attribute_data->beg);
}

void print_text(const StrChunk *text, void *user_data){
    user_data = user_data;//unused
    printf("text: %*.ls>\n",(int)(text->end - text->beg), text->beg);
}


bool xml_iter_tag(
    const StrChunk *source, 
    void (*on_tag)(const StrChunk *tag_name, void *user_data), 
    void (*on_attribute)(const StrChunk *attribute_name, const StrChunk *attribute_data, void *user_data), 
    void (*on_text)(const StrChunk *text, void *user_data), 
    void *user_data)
{
    source = source;//unused
    on_tag = on_tag;//unused
    on_attribute = on_attribute;//unused
    on_text = on_text;//unused
    user_data = user_data;//unused


    return true;
}
