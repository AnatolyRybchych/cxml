#include <cxml.h>

#define SERIALIZE_DECL(NAME) static bool NAME##_serialize(const CXML_Serializable *self, CXML_StringWriter *writer)
#define SERIALIZE_FORMAT_IMPL(NAME, BUF_LEN, FMT, ...) SERIALIZE_DECL(NAME){\
    wchar_t buffer[BUF_LEN];\
    int chars_cnt = swprintf(buffer, BUF_LEN, FMT, __VA_ARGS__);\
    StrChunk chunk = {.beg = buffer, .end = buffer + chars_cnt};\
    return cxml_write(writer, &chunk);\
}

#define SERIALIZABLE_DECL(NAME, TYPE) static CXML_Serializable NAME##_serializable(const TYPE *NAME)
#define SERIALIZABLE_IMPL(NAME, TYPE) SERIALIZABLE_DECL(NAME, TYPE) {return cxml_serializable(NAME, NAME##_serialize);}
#define CXML_DEF_DECL(NAME, TYPE) SERIALIZE_DECL(NAME); SERIALIZABLE_DECL(NAME, TYPE)

#define SERIALIZE_BUF_LEN 64

STATIC_VAR_STR_CHUNK(lt, L"<");
STATIC_VAR_STR_CHUNK(gt, L">");
STATIC_VAR_STR_CHUNK(space, L" ");
STATIC_VAR_STR_CHUNK(tag_name_end, SC_WHITE_SPACES"/>");
STATIC_VAR_STR_CHUNK(tag_end, L"/>");
STATIC_VAR_STR_CHUNK(slash, L"/");
STATIC_VAR_STR_CHUNK(attrib_name_end, SC_WHITE_SPACES"=");
STATIC_VAR_STR_CHUNK(eq, L"=");
STATIC_VAR_STR_CHUNK(quotes, L"\"");
VAR_STR_CHUNK(decl_open, L"<?xml ");
VAR_STR_CHUNK(decl_close, L"?>");

static bool skip_to_close_tag(StrChunk *chunk, const StrChunk *close_tag_name);
static bool skip_close_tag(StrChunk *chunk, const StrChunk *close_tag_name);
static bool skip_open_tag_open(StrChunk *source);
static bool skip_simple_tag_end(StrChunk *source);
static bool skip_attribute_to_value(StrChunk *source);
static void cp_str_to_wcs(wchar_t *wcs, const char *str, unsigned int cnt);

static bool _write_to_file(CXML_StringWriter *self, const StrChunk *str);
static CXML_StringWriter _writer_to_file(FILE *file);

CXML_DEF_DECL(wcs, wchar_t);
CXML_DEF_DECL(cstr, char);
CXML_DEF_DECL(tag, CXML_Tag);
CXML_DEF_DECL(attribute, CXML_Attribute);
CXML_DEF_DECL(concat, CXML_Concat);
CXML_DEF_DECL(decl, CXML_Declaration);
CXML_DEF_DECL(_float, float);
CXML_DEF_DECL(_double, double);
CXML_DEF_DECL(_int, int);
CXML_DEF_DECL(_uint, unsigned int);
CXML_DEF_DECL(_short, short int);
CXML_DEF_DECL(_ushort, unsigned short int);
CXML_DEF_DECL(_long, long int);
CXML_DEF_DECL(_ulong, unsigned long int);
CXML_DEF_DECL(_sbyte, char);
CXML_DEF_DECL(_byte, unsigned char);
CXML_DEF_DECL(_char, char);
CXML_DEF_DECL(_wchar, wchar_t);
CXML_DEF_DECL(_bool, bool);

struct CXML_DefaultWrappers cxml_def = {
    .writer = {
        .to_file = _writer_to_file,
    },
    .serializable = {
        .wcs = wcs_serializable,
        .cstr = cstr_serializable,
        .attribute = attribute_serializable,
        .tag = tag_serializable,
        .concat = concat_serializable,
        .decl = decl_serializable,
        ._float = _float_serializable, 
        ._double = _double_serializable,
        ._bool = _bool_serializable,
        ._int = _int_serializable,
        ._uint = _uint_serializable,
        ._short = _short_serializable,
        ._ushort = _ushort_serializable,
        ._long = _long_serializable,
        ._ulong = _ulong_serializable,
        ._byte = _byte_serializable,
        ._sbyte = _sbyte_serializable,
        ._char = _char_serializable,
        ._wchar = _wchar_serializable,
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

CXML_Attribute cxml_attribute(const CXML_Serializable *name, const CXML_Serializable *value){
    return (CXML_Attribute){
        .name = name,
        .value = value,
    };
}

CXML_Tag cxml_tag(const CXML_Serializable *name, const CXML_Attribute *attribs, unsigned int attribs_cnt, const CXML_Serializable *value){
    return (CXML_Tag){
        .name = name,
        .value = value,
        .attribs = attribs,
        .attribs_cnt = attribs_cnt,
    };
}

CXML_Serializable cxml_serializable(const void *data, bool (*serialize)(const CXML_Serializable *self, CXML_StringWriter *writer)){
    return (CXML_Serializable){
        .data = data,
        .serialize = serialize
    };
}

CXML_Concat cxml_concat(const CXML_Serializable *first, CXML_Serializable *second){
    return (CXML_Concat){
        .first = first,
        .second = second
    };
}

CXML_Declaration cxml_default_declaration(void){
    return (CXML_Declaration){
        .version_major = 1,
        .version_minor = 0,
        .encoding = {'U', 'T', 'F', '-', '8', 0},
        .standalone = true,
    };
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

static void cp_str_to_wcs(wchar_t *wcs, const char *str, unsigned int cnt){
    while (cnt){
        wcs[cnt] = str[cnt];
        cnt++;
    }
}

static bool _write_to_file(CXML_StringWriter *self, const StrChunk *str){
    if(self == NULL || self->data == NULL) return false;
    int chars_cnt = str->end - str->beg;
    if(chars_cnt > 0){
        return fprintf(
            (FILE*)self->data, "%.*ls", 
            chars_cnt, str->beg) == chars_cnt;
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
static bool wcs_serialize(const CXML_Serializable *self, CXML_StringWriter *writer){
    const wchar_t *wcs = (const wchar_t *)self->data;
    StrChunk chunk = {
        .beg = wcs,
        .end = wcs + wcslen(wcs),
    };
    return cxml_write(writer, &chunk);
}



static bool cstr_serialize(const CXML_Serializable *self, CXML_StringWriter *writer){
    const char *cstr = (const char *)self->data;
    int len = strlen(cstr);
    
    #define CP_BUF_LEN 256
    wchar_t wcs[CP_BUF_LEN];
    while (len > CP_BUF_LEN){
        cp_str_to_wcs(wcs, cstr, CP_BUF_LEN);

        StrChunk chunk = {.beg = wcs, .end = wcs + CP_BUF_LEN + 1};
        if(!cxml_write(writer, &chunk)) return false; 

        len -= CP_BUF_LEN;
        cstr += CP_BUF_LEN;
    }

    cp_str_to_wcs(wcs, cstr, len);

    StrChunk chunk = {.beg = wcs, .end = wcs + len + 1};
    if(!cxml_write(writer, &chunk)) return false; 
    else return true;
}

static bool tag_serialize(const CXML_Serializable *self, CXML_StringWriter *writer){
    if(self == NULL || self->data == NULL) return false;
    const CXML_Tag *tag = (const CXML_Tag *)self->data; 

    cxml_write(writer, &lt);
    cxml_serialize(tag->name, writer);
    if(tag->attribs_cnt){
        if(tag->attribs == NULL) return false;

        const CXML_Attribute *curr = tag->attribs;
        const CXML_Attribute *end = curr + tag->attribs_cnt;
        while (curr != end){
            cxml_write(writer, &space);
            CXML_Serializable attr_serializable = cxml_def.serializable.attribute(curr);
            if(!cxml_serialize(&attr_serializable, writer)) return false;
            curr++;
        }
    }

    if(tag->value == NULL){
        if(!cxml_write(writer, &slash)) return false;
        if(!cxml_write(writer, &gt)) return false;
        return true;
    }
    else{
        if(!cxml_write(writer, &gt)) return false;
        if(!cxml_serialize(tag->value, writer)) return false;
        if(!cxml_write(writer, &lt)) return false;
        if(!cxml_write(writer, &slash)) return false;
        if(!cxml_serialize(tag->name, writer)) return false;
        if(!cxml_write(writer, &gt)) return false;
        return true;
    }
}

static bool attribute_serialize(const CXML_Serializable *self, CXML_StringWriter *writer){
    if(self == NULL || self->data == NULL) return false;
    const CXML_Attribute *attribute = (const CXML_Attribute *)self->data;
    if(!cxml_serialize(attribute->name, writer)) return false;
    if(!cxml_write(writer, &eq)) return false;
    if(!cxml_write(writer, &quotes)) return false;
    if(!cxml_serialize(attribute->value, writer)) return false;
    if(!cxml_write(writer, &quotes)) return false;
    return true;
}

static bool concat_serialize(const CXML_Serializable *self, CXML_StringWriter *writer){
    const CXML_Concat *concat = (const CXML_Concat*)self->data;
    return cxml_serialize(concat->first, writer) && cxml_serialize(concat->second, writer);
}

static bool decl_serialize(const CXML_Serializable *self, CXML_StringWriter *writer){
    const CXML_Declaration *decl = (const CXML_Declaration*)self->data;

    if(!cxml_write(writer, &decl_open)) return false;
    CXML_Serializable version_name = cxml_def.serializable.wcs(L"version=\"");
    CXML_Serializable version_maj = cxml_def.serializable._uint(&decl->version_major);
    CXML_Serializable version_delim = cxml_def.serializable.wcs(L".");
    CXML_Serializable version_min = cxml_def.serializable._uint(&decl->version_minor);
    CXML_Serializable version_end = cxml_def.serializable.wcs(L"\" ");
    if(!(cxml_serialize(&version_name, writer)
        && cxml_serialize(&version_maj, writer)
        && cxml_serialize(&version_delim, writer)
        && cxml_serialize(&version_min, writer)
        && cxml_serialize(&version_end, writer))) return false;
    
    CXML_Serializable encoding_name = cxml_def.serializable.wcs(L"encoding");
    CXML_Serializable encoding_val = cxml_def.serializable.cstr(decl->encoding);
    CXML_Attribute encoding_attrib = cxml_attribute(&encoding_name, &encoding_val);
    CXML_Serializable encoding_attrib_ser = cxml_def.serializable.attribute(&encoding_attrib);
    if(!cxml_serialize(&encoding_attrib_ser, writer)) return false;


    if(!cxml_write(writer, &decl_close)) return false;

    return true;
}


SERIALIZE_FORMAT_IMPL(_float, SERIALIZE_BUF_LEN, L"%f", *(float*)self->data)
SERIALIZE_FORMAT_IMPL(_double, SERIALIZE_BUF_LEN, L"%lf", *(double*)self->data)
SERIALIZE_FORMAT_IMPL(_int, SERIALIZE_BUF_LEN, L"%i", *(int*)self->data)
SERIALIZE_FORMAT_IMPL(_uint, SERIALIZE_BUF_LEN, L"%u", *(unsigned int*)self->data)
SERIALIZE_FORMAT_IMPL(_short, SERIALIZE_BUF_LEN, L"%hi", *(short int*)self->data)
SERIALIZE_FORMAT_IMPL(_ushort, SERIALIZE_BUF_LEN, L"%hu", *(unsigned short int*)self->data)
SERIALIZE_FORMAT_IMPL(_long, SERIALIZE_BUF_LEN, L"%li", *(long int*)self->data)
SERIALIZE_FORMAT_IMPL(_ulong, SERIALIZE_BUF_LEN, L"%lu", *(unsigned long int*)self->data)
SERIALIZE_FORMAT_IMPL(_sbyte, SERIALIZE_BUF_LEN, L"%hhi", *(char*)self->data)
SERIALIZE_FORMAT_IMPL(_byte, SERIALIZE_BUF_LEN, L"%hhu", *(unsigned char*)self->data)
SERIALIZE_FORMAT_IMPL(_char, SERIALIZE_BUF_LEN, L"%c", *(char*)self->data)
SERIALIZE_FORMAT_IMPL(_wchar, SERIALIZE_BUF_LEN, L"%lc", *(wchar_t*)self->data)
SERIALIZE_FORMAT_IMPL(_bool, SERIALIZE_BUF_LEN, L"%s", *(bool*)self->data ? "true" : "false")

SERIALIZABLE_IMPL(wcs, wchar_t)
SERIALIZABLE_IMPL(cstr, char)
SERIALIZABLE_IMPL(attribute, CXML_Attribute)
SERIALIZABLE_IMPL(tag, CXML_Tag)
SERIALIZABLE_IMPL(concat, CXML_Concat)
SERIALIZABLE_IMPL(decl, CXML_Declaration)
SERIALIZABLE_IMPL(_float, float)
SERIALIZABLE_IMPL(_double, double)
SERIALIZABLE_IMPL(_int, int)
SERIALIZABLE_IMPL(_uint, unsigned int)
SERIALIZABLE_IMPL(_short, short int)
SERIALIZABLE_IMPL(_ushort, unsigned short int)
SERIALIZABLE_IMPL(_long, long int)
SERIALIZABLE_IMPL(_ulong, unsigned long int)
SERIALIZABLE_IMPL(_byte, unsigned char)
SERIALIZABLE_IMPL(_sbyte, char)
SERIALIZABLE_IMPL(_char, char)
SERIALIZABLE_IMPL(_wchar, wchar_t)
SERIALIZABLE_IMPL(_bool, bool)
