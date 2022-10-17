#ifndef CXML_H
#define CXML_H

#include <str_chunk.h>

typedef void(*CXML_OnTagHandler)(const StrChunk *tag_name, const StrChunk *inner_text, const StrChunk *attributes, void *user_data);
typedef void(*CXML_OnAttributeHandler)(const StrChunk *tag_name, const StrChunk *inner_text, const StrChunk *attributes, void *user_data);

bool cxml_iter_tag(const StrChunk *source, CXML_OnTagHandler on_tag, void *user_data);
bool cxml_iter_attributes(const StrChunk *source, CXML_OnAttributeHandler on_attribute, void *user_data);
    

#endif //CXML_H
