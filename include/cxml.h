#ifndef CXML_H
#define CXML_H

#include <str_chunk.h>

bool cxml_iter_tag(
    const StrChunk *source, 
    void (*on_tag)(const StrChunk *tag_name, const StrChunk *inner_text, const StrChunk *attributes, void *user_data), 
    void *user_data);

bool cxml_iter_attributes(const StrChunk *source, 
    void (*on_attribute)(const StrChunk *attribute_name, const StrChunk *attribute_value, void *user_data), 
    void *user_data);
    

#endif //CXML_H
