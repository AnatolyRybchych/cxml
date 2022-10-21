#include <stdio.h>
#include <cxml.h>

#define UNUSED(PARAM) ((PARAM) = (PARAM))

int main(){
    
    CXML_StringWriter stdout_writer = cxml_def.writer.to_file(stdout);
    CXML_Serializable wcs_serializable = cxml_def.serializable.wcs(L"some text here\n");

    cxml_serialize(&wcs_serializable, &stdout_writer);
    return 0;
}



