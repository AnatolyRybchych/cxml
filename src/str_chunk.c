#include <str_chunk.h>
#include <stdio.h>
#include <stdarg.h>

const StrChunk *sc_get_whitespaces(void){
    static wchar_t _white_spaces[] = L" \n\t\v\f\r";
    static StrChunk white_spaces = {
        .beg = _white_spaces,
        .end = _white_spaces + sizeof(_white_spaces) / sizeof(*_white_spaces) - 1 
    };

    return &white_spaces;
}

bool sc_contains(const StrChunk *source, wchar_t character){
    const wchar_t *curr = source->beg;
    while (curr < source->end){
        if(*curr++ == character) return true;
    }
    return false;
}

int sc_skip_all(StrChunk *source, const StrChunk *characters, bool reverse_characters){
    int result = 0;
    while (sc_skip_one(source, characters, reverse_characters)){
        result++;
    }
    return result;
}

bool sc_skip_one(StrChunk *source, const StrChunk *characters, bool reverse_characters){
    if(source->end - source->beg < 1) return false;
    if(sc_contains(characters, *source->beg) != reverse_characters){
        source->beg++;
        return true;
    }
    return false;
}

int sc_skip_all_whitespaces(StrChunk *source){
    return sc_skip_all(source, sc_get_whitespaces(), false);
}

int sc_skip_all_to_whitespace(StrChunk *source){
    return sc_skip_all(source, sc_get_whitespaces(), true);
}

bool sc_starts_with_sc(const StrChunk *source, const StrChunk *sample){
    StrChunk cp = *source;
    return sc_skip_starts_with_sc(&cp, sample);
}

bool sc_skip_starts_with_sc(StrChunk *source, const StrChunk *sample){
    const wchar_t *sample_curr = sample->beg;
    const wchar_t *source_curr = source->beg;

    if(source->end - source->beg < sample->end - sample->beg){
        return false;
    }

    while (sample_curr < sample->end){
        if(*sample_curr++ != *source_curr++){
            return false;
        }
    }
    source->beg = source_curr;
    return true;
}

#define SC_CONDITION_SIZE (sizeof(StrChunkCondition) + sizeof(StrChunk*))
bool sc_skip_start_matches_all(StrChunk *source, StrChunkCondition condition1, const StrChunk *sample1, ...){
    StrChunk cp = *source;

    StrChunkCondition *curr_condition = &condition1;
    const StrChunk **curr_sample = &sample1;

    while (*curr_condition < SC_STOP){
        if(sc_skip_start_matches_condition(&cp, *curr_condition, *curr_sample) == false){
            return false;
        }
        curr_condition = (StrChunkCondition*)((char*)curr_condition + SC_CONDITION_SIZE);
        curr_sample = (const StrChunk**)((char*)curr_condition + SC_CONDITION_SIZE);
    }

    *source = cp;
    return true;
}

bool sc_start_matches_all(const StrChunk *source, StrChunkCondition condition1, const StrChunk *sample1, ...){
    StrChunk cp = *source;
    if(condition1 == SC_STOP || !sc_skip_start_matches_condition(&cp, condition1, sample1)){
        return false;
    }

    va_list argp;   
    va_start(argp, sample1);

    while (true){
        StrChunkCondition condition = va_arg(argp, StrChunkCondition);
        if(condition >= SC_STOP) return true;
        const StrChunk *sample = va_arg(argp, const StrChunk *);

        //printf("%d\n", condition);
        if(sc_skip_start_matches_condition(&cp, condition, sample) == false){
            return false;
        }
    }
    return true;
}

bool sc_skip_start_matches_condition(StrChunk *source, StrChunkCondition condition, const StrChunk *sample){
    switch (condition){
    case SC_MAYBE_SOME_IN:
        return sc_skip_all(source, sample, false), true;
    case SC_MAYBE_SOME_NOT_IN:
        return sc_skip_all(source, sample, true), true;
    case SC_SOME_IN:
        return sc_skip_all(source, sample, false) > 0;
    case SC_SOME_NOT_IN:
        return sc_skip_all(source, sample, true) > 0;
    case SC_ONE_IN:
        return sc_skip_one(source, sample, false);
    case SC_ONE_NOT_IN:
        return sc_skip_one(source, sample, true);
    case SC_SEQUANCE:
        return sc_skip_starts_with_sc(source, sample);
    default:
        return false;
    }
}
