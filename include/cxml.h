#ifndef CXML_H
#define CXML_H

#include <str_chunk.h>
#include <stdio.h>

typedef struct CXML_StringWriter CXML_StringWriter;
typedef struct CXML_Serializable CXML_Serializable;

typedef void(*CXML_OnTagHandler)(const StrChunk *tag_name, const StrChunk *inner_text, const StrChunk *attributes, void *user_data);
typedef void(*CXML_OnAttributeHandler)(const StrChunk *attribute_name, const StrChunk *attribute_value, void *user_data);

extern struct CXML_DefaultWrappers cxml_def;

bool cxml_iter_tag(const StrChunk *source, StrChunk *actualChunk, CXML_OnTagHandler on_tag, void *user_data);
bool cxml_iter_attributes(const StrChunk *source, CXML_OnAttributeHandler on_attribute, void *user_data);
bool cxml_write(CXML_StringWriter *writer, const StrChunk *str);
bool cxml_serialize(const CXML_Serializable *serializable, CXML_StringWriter *writer);

struct CXML_StringWriter{
    void *data;
    bool (*write)(CXML_StringWriter *self, const StrChunk *str);
};

struct CXML_Serializable{
    const void *data;
    bool (*serialize)(const CXML_Serializable *self, CXML_StringWriter *writer);
};

struct CXML_DefaultStringWriters{
    CXML_StringWriter (*to_file)(FILE *file);
};

struct CXML_DefaultSerializables{
    CXML_Serializable (*wcs)(const wchar_t *wcs);
};


struct CXML_DefaultWrappers{
    struct CXML_DefaultStringWriters writer;
    struct CXML_DefaultSerializables serializable;
};


#endif //CXML_H
