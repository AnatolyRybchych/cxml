#include <stdio.h>
#include <cxml.h>

#define UNUSED(PARAM) ((PARAM) = (PARAM))

int main(){
    CXML_StringWriter stdout_writer = cxml_def.writer.to_file(stdout);
    CXML_Serializable root_name = cxml_def.serializable.wcs(L"root");

    CXML_Serializable attr_name = cxml_def.serializable.wcs(L"attr");
    float attr_fval = true;
    CXML_Serializable attr_val = cxml_def.serializable._float(&attr_fval);
    CXML_Serializable attr_sufix = cxml_def.serializable.wcs(L"f");

    CXML_Concat attr_val_full = cxml_concat(&attr_val, &attr_sufix);
    CXML_Serializable attr_full = cxml_def.serializable.concat(&attr_val_full);

    CXML_Attribute attr = cxml_attribute(&attr_name, &attr_full);
    
    CXML_Tag root = cxml_tag(&root_name, &attr, 1, NULL);
    CXML_Serializable root_serializable = cxml_def.serializable.tag(&root);

    cxml_serialize(&root_serializable, &stdout_writer);
    return 0;
}



