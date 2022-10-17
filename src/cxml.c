#include <cxml.h>

STATIC_VAR_STR_CHUNK(lt, L"<");
STATIC_VAR_STR_CHUNK(gt, L">");
STATIC_VAR_STR_CHUNK(tag_name_end, SC_WHITE_SPACES"/>");
STATIC_VAR_STR_CHUNK(tag_end, L"/>");
STATIC_VAR_STR_CHUNK(slash, L"/");
STATIC_VAR_STR_CHUNK(attrib_name_end, SC_WHITE_SPACES"=");
STATIC_VAR_STR_CHUNK(eq, L"=");
STATIC_VAR_STR_CHUNK(quotes, L"\"");

static bool skip_to_close_tag(StrChunk *chunk, const StrChunk *close_tag_name);
static bool skip_open_tag_open(StrChunk *source);
static bool is_simple_tag_end(const StrChunk *source);
static bool skip_attribute_to_value(StrChunk *source);

bool cxml_iter_tag(
    const StrChunk *source, 
    void (*on_tag)(const StrChunk *tag_name, const StrChunk *inner_text, const StrChunk *attributes, void *user_data), 
    void *user_data){
    
    StrChunk tag_name, inner_text, attributes;
    StrChunk cp = *source;

    if(skip_open_tag_open(&cp) == false) return false;;

    sc_skip_copy_all_to(&cp, &tag_name, &tag_name_end);
    sc_skip_copy_all_to(&cp, &attributes, &tag_end);

    if(is_simple_tag_end(&cp)){
       inner_text = (StrChunk){cp.beg, cp.beg};
    }
    else{
        inner_text.beg = cp.beg;
        if(skip_to_close_tag(&cp, &tag_name)) inner_text.end = cp.beg;
        else inner_text.end = source->end;
    }
    on_tag(&tag_name, &inner_text, &attributes, user_data);
    return true;
}

bool cxml_iter_attributes(const StrChunk *source, 
    void (*on_attribute)(const StrChunk *attribute_name, const StrChunk *attribute_value, void *user_data), 
    void *user_data){
    StrChunk cp = *source;

    while (cp.beg < cp.end){
        StrChunk attribute_name, attribute_value;
        sc_skip_all_whitespaces(&cp);
        sc_skip_copy_all_to(&cp, &attribute_name, &attrib_name_end);
        sc_skip_all_whitespaces(&cp);

        if(sc_is_empty(&cp)) return true;
        
        if(*cp.beg == L'='){
            if(skip_attribute_to_value(&cp) == false) return false;
            sc_skip_copy_all_to(&cp, &attribute_value, &quotes);
            on_attribute(&attribute_name, &attribute_value, user_data);
            sc_skip_one(&cp, &quotes, false);
        }
        else{
            attribute_value = (StrChunk){
                .beg = cp.beg,
                .end = cp.end
            };
            on_attribute(&attribute_name, &attribute_value, user_data);
        }
    }
    return true;    
}

static bool skip_to_close_tag(StrChunk *chunk, const StrChunk *close_tag_name){
    return sc_skip_to_start_matches_all(
        chunk,
        SC_ONE_IN, &lt,
        SC_MAYBE_SOME_IN, sc_get_whitespaces(),
        SC_ONE_IN, slash,
        SC_MAYBE_SOME_IN, sc_get_whitespaces(),
        SC_SEQUANCE, close_tag_name,
        SC_MAYBE_SOME_IN, sc_get_whitespaces(),
        SC_ONE_IN, &gt
    );
}

static bool skip_open_tag_open(StrChunk *source){
    return sc_skip_start_matches_all(
        source,
        SC_MAYBE_SOME_IN, sc_get_whitespaces(),
        SC_ONE_IN, &lt,
        SC_MAYBE_SOME_IN, sc_get_whitespaces(),
        SC_STOP
    );
}

static bool is_simple_tag_end(const StrChunk *source){
    return sc_start_matches_all(
        source,
        SC_MAYBE_SOME_IN, sc_get_whitespaces(),
        SC_ONE_IN, &lt,
        SC_MAYBE_SOME_IN, sc_get_whitespaces(),
        SC_ONE_IN, &slash,
        SC_STOP
    );
}

static bool skip_attribute_to_value(StrChunk *source){
    return sc_skip_start_matches_all(
        source,
        SC_ONE_IN, &eq,
        SC_MAYBE_SOME_IN, sc_get_whitespaces(),
        SC_ONE_IN, &quotes,
        SC_STOP
    );
}