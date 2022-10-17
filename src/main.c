#include <stdio.h>
#include <str_chunk.h>


bool xml_iter_tag(
    StrChunk source, 
    void (*on_tag)(StrChunk tag_name, void *user_data), 
    void (*on_attribute)(StrChunk attribute_name, StrChunk attribute_data, void *user_data), 
    void (*on_text)(StrChunk text, void *user_data), 
    void *user_data);

void print_tag(StrChunk tag_name, void *user_data);
void print_attribute(StrChunk attribute_name, StrChunk attribute_data, void *user_data);
void print_text(StrChunk text, void *user_data);

int main(){
    wchar_t data[] = L""
    "<tag></tag>";

    StrChunk chunk = {
        .beg = data,
        .end = data + wcslen(data)
    };

    #define CAR_STR_CHUNK(NAME, WCS) wchar_t _##NAME[] = WCS;StrChunk NAME = {.beg = _##NAME, .end = _##NAME + wcslen(_##NAME)}

    CAR_STR_CHUNK(tag, L"tag");
    CAR_STR_CHUNK(open_tag, L"<");
    CAR_STR_CHUNK(close_tag, L">");

    if(sc_start_matches_all(&chunk, 
        SC_MAYBE_SOME_IN, sc_get_whitespaces(),
        SC_ONE_IN, &open_tag,
        SC_SEQUANCE, &tag,
        SC_MAYBE_SOME_IN, sc_get_whitespaces(),
        SC_ONE_IN, &close_tag,
        SC_STOP
    )){
        printf("MATCHES ALL!\n");
    }
    else{
        printf("DOESNT MATCHES ALL!\n");
    }

    // if(!xml_iter_tag(chunk, print_tag, print_attribute, print_text, NULL)){
    //     printf("ERROR: parsing failed\n");
    // }
    // else{
    //     printf("SUCCESS!\n");
    // }


    return 0;
}

void print_tag(StrChunk tag_name, void *user_data){
    user_data = user_data;//unused
    printf("tag: <%*.ls>\n", (int)(tag_name.end - tag_name.beg), tag_name.beg);
}

void print_attribute(StrChunk attribute_name, StrChunk attribute_data, void *user_data){
    user_data = user_data;//unused
    printf("attribute: %*.ls=%*.ls\n", 
        (int)(attribute_name.end - attribute_name.beg), attribute_name.beg, 
        (int)(attribute_data.end - attribute_data.beg), attribute_data.beg);
}

void print_text(StrChunk text, void *user_data){
    user_data = user_data;//unused
    printf("text: %*.ls>\n",(int)(text.end - text.beg), text.beg);
}


bool xml_iter_tag(
    StrChunk source, 
    void (*on_tag)(StrChunk tag_name, void *user_data), 
    void (*on_attribute)(StrChunk attribute_name, StrChunk attribute_data, void *user_data), 
    void (*on_text)(StrChunk text, void *user_data), 
    void *user_data)
{
    source = source;//unused
    on_tag = on_tag;//unused
    on_attribute = on_attribute;//unused
    on_text = on_text;//unused
    user_data = user_data;//unused


    return true;
}
