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
static bool skip_close_tag(StrChunk *chunk, const StrChunk *close_tag_name);
static bool skip_open_tag_open(StrChunk *source);
static bool skip_simple_tag_end(StrChunk *source);
static bool skip_attribute_to_value(StrChunk *source);

static bool _write_to_file(CXML_StringWriter *self, const StrChunk *str);
static CXML_StringWriter _writer_to_file(FILE *file);
static bool _wcs_serialize(const CXML_Serializable *self, CXML_StringWriter *writer);
static CXML_Serializable _wcs_serializable(const wchar_t *wcs);

struct CXML_DefaultWrappers cxml_def = {
    .writer = {
        .to_file = _writer_to_file,
    },
    .serializable = {
        .wcs = _wcs_serializable,
    },
};

bool cxml_iter_tag(const StrChunk *source, StrChunk *actualChunk, CXML_OnTagHandler on_tag, void *user_data){
    StrChunk tag_name, inner_text, attributes;
    StrChunk cp = *source;
    sc_skip_all_whitespaces(&cp);
    StrChunk actual = {.beg = cp.beg, .end = cp.end};

    if(skip_open_tag_open(&cp) == false) return false;;

    sc_skip_copy_all_to(&cp, &tag_name, &tag_name_end);
    sc_skip_copy_all_to(&cp, &attributes, &tag_end);
    sc_skip_one(&cp, &tag_end, false);

    if(skip_simple_tag_end(&cp)){
       inner_text = (StrChunk){cp.beg, cp.beg};
       actual.end = cp.beg;
    }
    else{
        inner_text.beg = cp.beg;
        if(skip_to_close_tag(&cp, &tag_name)){
            inner_text.end = cp.beg;
            skip_close_tag(&cp, &tag_name);
            actual.end = cp.beg;
        }
        else inner_text.end = source->beg;
    }
    *actualChunk = actual;
    on_tag(&tag_name, &inner_text, &attributes, user_data);
    return true;
}

bool cxml_iter_attributes(const StrChunk *source, CXML_OnAttributeHandler on_attribute, void *user_data){
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

bool cxml_write(CXML_StringWriter *writer, const StrChunk *str){
    return writer->write(writer, str);
}

bool cxml_serialize(const CXML_Serializable *serializable, CXML_StringWriter *writer){
    return serializable->serialize(serializable, writer);
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
        SC_ONE_IN, &gt,
        SC_STOP
    );
}

static bool skip_close_tag(StrChunk *chunk, const StrChunk *close_tag_name){
    return sc_skip_start_matches_all(
        chunk,
        SC_ONE_IN, &lt,
        SC_MAYBE_SOME_IN, sc_get_whitespaces(),
        SC_ONE_IN, slash,
        SC_MAYBE_SOME_IN, sc_get_whitespaces(),
        SC_SEQUANCE, close_tag_name,
        SC_MAYBE_SOME_IN, sc_get_whitespaces(),
        SC_ONE_IN, &gt,
        SC_STOP
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

static bool skip_simple_tag_end(StrChunk *source){
    return sc_skip_start_matches_all(
        source,
        SC_MAYBE_SOME_IN, sc_get_whitespaces(),
        SC_ONE_IN, &slash,
        SC_MAYBE_SOME_IN, sc_get_whitespaces(),
        SC_ONE_IN, &gt,
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

static bool _write_to_file(CXML_StringWriter *self, const StrChunk *str){
    if(self == NULL || self->data == NULL) return false;
    int chars_cnt = str->end - str->beg;
    int bytes_cnt = chars_cnt / sizeof(wchar_t);
    if(chars_cnt > 0){
        return fprintf(
            (FILE*)self->data, "%.*ls", 
            chars_cnt, str->beg) == bytes_cnt;
    }
    else{
        return true;
    }
}

static CXML_StringWriter _writer_to_file(FILE *file){
    return (CXML_StringWriter){
        .data = file,
        .write = _write_to_file,
    };
}
static bool _wcs_serialize(const CXML_Serializable *self, CXML_StringWriter *writer){
    const wchar_t *wcs = (const wchar_t *)self->data;
    StrChunk chunk = {
        .beg = wcs,
        .end = wcs + wcslen(wcs),
    };
    return cxml_write(writer, &chunk);
}

static CXML_Serializable _wcs_serializable(const wchar_t *wcs){
    return (CXML_Serializable){
        .data = wcs,
        .serialize = _wcs_serialize,
    };
}
