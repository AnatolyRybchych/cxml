#ifndef CXML_H
#define CXML_H

#include <str_chunk.h>
#include <stdio.h>

typedef struct CXML_StringWriter CXML_StringWriter;
typedef struct CXML_Serializable CXML_Serializable;
typedef struct CXML_Tag CXML_Tag;
typedef struct CXML_Attribute CXML_Attribute;

typedef void(*CXML_OnTagHandler)(const StrChunk *tag_name, const StrChunk *inner_text, const StrChunk *attributes, void *user_data);
typedef void(*CXML_OnAttributeHandler)(const StrChunk *attribute_name, const StrChunk *attribute_value, void *user_data);

extern struct CXML_DefaultWrappers cxml_def;

bool cxml_iter_tag(const StrChunk *source, StrChunk *actualChunk, CXML_OnTagHandler on_tag, void *user_data);
bool cxml_iter_attributes(const StrChunk *source, CXML_OnAttributeHandler on_attribute, void *user_data);
bool cxml_write(CXML_StringWriter *writer, const StrChunk *str);
bool cxml_serialize(const CXML_Serializable *serializable, CXML_StringWriter *writer);
CXML_Attribute cxml_attribute(const CXML_Serializable *name, const CXML_Serializable *value);
CXML_Tag cxml_tag(const CXML_Serializable *name, const CXML_Attribute *attribs, unsigned int attribs_cnt, const CXML_Serializable *value);

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
    CXML_Serializable (*attribute)(const CXML_Attribute *attrib);
    CXML_Serializable (*tag)(const CXML_Tag *tag);
};

struct CXML_DefaultWrappers{
    struct CXML_DefaultStringWriters writer;
    struct CXML_DefaultSerializables serializable;
};

struct CXML_Attribute{
    const CXML_Serializable *name;//should be valid attribute name (unchecked)
    const CXML_Serializable *value;//should be valid value (unchecked)
};

struct CXML_Tag{
    const CXML_Serializable *name; //should be valid tag name (unchecked)
    const CXML_Serializable *value; //can be NULL
    const unsigned int attribs_cnt; // can be 0
    const CXML_Attribute *attribs; // can be NULL if attribs_cnt == 0
};




#endif //CXML_H
