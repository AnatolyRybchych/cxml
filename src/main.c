#include <stdio.h>
#include <str_chunk.h>

#define VAR_STR_CHUNK(MOD, NAME, WCS) MOD wchar_t _data_##NAME[] = WCS;MOD StrChunk NAME = {.beg = _data_##NAME, .end = _data_##NAME + sizeof(_data_##NAME) / sizeof(*_data_##NAME) - 1}

bool xml_iter_tag(
    const StrChunk *source, 
    void (*on_tag)(const StrChunk *tag_name, const StrChunk *innert_text, const StrChunk *attributes, void *user_data), 
    void *user_data);

void print_tag(const StrChunk *tag_name, const StrChunk *innert_text, const StrChunk *attributes, void *user_data);

int main(){
    VAR_STR_CHUNK(,chunk, L""
    "<tag attribute1>dsd</tag>"
    );

    if(xml_iter_tag(&chunk, print_tag, NULL)){
        printf("SUCCESS!\n");
    }
    else{
        printf("ERROR: parsing failed\n");
    }
    return 0;
}

bool xml_iter_tag(
    const StrChunk *source, 
    void (*on_tag)(const StrChunk *tag_name, const StrChunk *innert_text, const StrChunk *attributes, void *user_data), 
    void *user_data){
    
    StrChunk tag_name;
    StrChunk inner_text;
    StrChunk attributes;

    StrChunk cp = *source;

    VAR_STR_CHUNK(static, lt, L"<");
    VAR_STR_CHUNK(static, gt, L">");
    VAR_STR_CHUNK(static, tag_name_end, L" \n\t\v\f\r/>");
    VAR_STR_CHUNK(static, tag_end, L"/>");
    VAR_STR_CHUNK(static, slash, L"/");

    if(sc_skip_start_matches_all(
        &cp,
        SC_MAYBE_SOME_IN, sc_get_whitespaces(),
        SC_ONE_IN, &lt,
        SC_MAYBE_SOME_IN, sc_get_whitespaces(),
        SC_STOP
    )){
        sc_copy_all_to(&cp, &tag_name, &tag_name_end);
        cp.beg = tag_name.end;
        sc_copy_all_to(&cp, &attributes, &tag_end);
        cp.beg = attributes.end;

        if(*attributes.end == L'/'){
            inner_text = (StrChunk){
                .beg = attributes.end,
                .end = attributes.end,
            };
        }
        else{
            inner_text = (StrChunk){
                .beg = attributes.end + 1,
                .end = source->end
            };

            bool has_close_tag = sc_skip_to_start_matches_all(
                &cp,
                SC_ONE_IN, &lt,
                SC_MAYBE_SOME_IN, sc_get_whitespaces(),
                SC_ONE_IN, slash,
                SC_MAYBE_SOME_IN, sc_get_whitespaces(),
                SC_SEQUANCE, &tag_name,
                SC_MAYBE_SOME_IN, sc_get_whitespaces(),
                SC_ONE_IN, &gt
            );
            if(has_close_tag){
                inner_text.end = cp.beg;
            }
        }
        on_tag(&tag_name, &inner_text, &attributes, user_data);

        return true;
    }
    else{
        return false;
    }
}

void print_tag(const StrChunk *tag_name, const StrChunk *innert_text, const StrChunk *attributes, void *user_data){
    user_data = user_data; //unused

    printf("<%.*ls", (int)(tag_name->end - tag_name->beg), tag_name->beg);
    printf("{%.*ls}>", (int)(attributes->end - attributes->beg), attributes->beg);
    printf("%.*ls", (int)(innert_text->end - innert_text->beg), innert_text->beg);
    printf("</%.*ls>", (int)(tag_name->end - tag_name->beg), tag_name->beg);
}
