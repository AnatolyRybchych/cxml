#include <stdio.h>
#include <cxml.h>
#include <stdlib.h>

#define XML(...) L## #__VA_ARGS__

int main(void){
    const wchar_t *xml = XML(
        <?xml version="1.0",  encoding="UTF-8", standalone="no" ?> 
    );

    CXML_StringWriter stdout_writer = cxml_def.writer.to_file(stdout);
    
    StrChunk xml_chunk = {xml, xml + wcslen(xml)};
    StrChunk xml_decl;
    CXML_Declaration decl;

    if(cxml_read_decl(&xml_chunk, &xml_decl, &decl)){
        CXML_Serializable decl_serializable = cxml_def.serializable.decl(&decl);
        cxml_serialize(&decl_serializable, &stdout_writer);

        puts("");
    }
    else{
        fprintf(stderr, "ERROR: cannot read xml decalration\n");
        exit(-1);
    }

    return 0;
}
