#include <str_chunk.h>
#include <stdarg.h>


const StrChunk *sc_get_whitespaces(void){
    static wchar_t _white_spaces[] = SC_WHITE_SPACES;
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
    va_list argp;   
    va_start(argp, sample1);

    return sc_skip_start_matches_all_v(source, condition1, sample1, argp);
}

bool sc_skip_start_matches_all_v(StrChunk *source, StrChunkCondition condition1, const StrChunk *sample1, va_list argp){
    StrChunk cp = *source;
    if(condition1 == SC_STOP || !sc_skip_start_matches_condition(&cp, condition1, sample1)){
        return false;
    }

    while (true){
        StrChunkCondition condition = va_arg(argp, StrChunkCondition);
        if(condition >= SC_STOP) {
            *source = cp;
            return true;
        }
        const StrChunk *sample = va_arg(argp, const StrChunk *);
        if(sc_skip_start_matches_condition(&cp, condition, sample) == false){
            return false;
        }
    }
}

bool sc_start_matches_all(const StrChunk *source, StrChunkCondition condition1, const StrChunk *sample1, ...){
    StrChunk cp = *source;
    va_list argp;   
    va_start(argp, sample1);

    return sc_skip_start_matches_all_v(&cp, condition1, sample1, argp);
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
    case SC_ONE_MORE_NOT_IN:
        return sc_skip_all(source, sample, true) > 0;
    case SC_SEQUANCE:
        return sc_skip_starts_with_sc(source, sample);
    default:
        return false;
    }
}

void sc_copy_all_to(const StrChunk *source, StrChunk *destination, const StrChunk *characters){
    StrChunk cp = *source; 
    sc_skip_all(&cp, characters, true);
    destination->beg = source->beg;
    destination->end = cp.beg;
}

void sc_skip_copy_all_to(StrChunk *source, StrChunk *destination, const StrChunk *characters){
    sc_copy_all_to(source, destination, characters);
    source->beg = destination->end;
}

void sc_iter_split(const StrChunk *source, void (*on_chunk)(const StrChunk *chunk, void *user_data), const StrChunk *delimiters, void *user_data){
    StrChunk chunk = {
        .beg = source->beg,
        .end = source->end
    };

    while (chunk.end < source->end){
        if(sc_contains(delimiters, *chunk.end)){
            on_chunk(&chunk, user_data);
            chunk.beg = chunk.end + 1; 
        }
        chunk.end++;
    }
    if(chunk.beg != chunk.end){
        on_chunk(&chunk, user_data);
    }
}


bool sc_skip_to_start_matches_all(StrChunk *source, StrChunkCondition condition1, const StrChunk *sample1, ...){
    StrChunk cp = *source;
    while (cp.beg < source->end){
        va_list argp;
        va_start(argp, sample1);
        StrChunk cp_cp = cp;
        if(sc_skip_start_matches_all_v(&cp_cp, condition1, sample1, argp)){
            *source = cp;
            return true;
        }
        cp.beg++;
    }
    return false;
}

bool sc_skip_to_start_matches_all_including(StrChunk *source, StrChunkCondition condition1, const StrChunk *sample1, ...){
    StrChunk cp = *source;
    while (source->beg < source->end){
        va_list argp;
        va_start(argp, sample1);
        if(sc_skip_start_matches_all_v(&cp, condition1, sample1, argp)){
            *source = cp;
            return true;
        }
    }
    return false;
}

bool sc_is_empty(const StrChunk *source){
    return source->beg >= source->end;
}