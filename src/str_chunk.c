#include <str_chunk.h>
#include <stdarg.h>

static bool parse_hexidecimal(const StrChunk *source, int *result);
static bool parse_decimal(const StrChunk *source, int *result);
static bool parse_decimal_digit(wchar_t ch, int *result);
static bool parse_hexidecimal_digit(wchar_t ch, int *result);
static int skip_int_sign(StrChunk *source);
static bool is_ufloat_valid_format(const StrChunk *source);

STATIC_VAR_STR_CHUNK(hexidecimal_mark, L"0x");
STATIC_VAR_STR_CHUNK(hexidecimal_mark2, L"0X");

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

bool sc_equals(const StrChunk *source, const wchar_t *wcs){
    const wchar_t *curr = source->beg;

    while (*wcs){
        if(curr >= source->end) return false;
        else if(*curr++ != *wcs++) return false;
    }
    if(curr == source->end) return true;
    else return false;
}

bool sc_int(const StrChunk *source, int *result){
    StrChunk cp = *source;
    bool hexidecimal = sc_starts_with_sc(&cp, &hexidecimal_mark) || sc_starts_with_sc(&cp, &hexidecimal_mark2);
    if(hexidecimal){
        cp.beg += 2;
        return parse_hexidecimal(&cp, result);
    }
    else{
        return parse_decimal(&cp, result);
    }
}

bool sc_double(const StrChunk *source, double *result){
    StrChunk cp = *source;
    int sign = skip_int_sign(&cp);
    double value = 0;

    int fractials = 0;
    
    if(cp.beg >= cp.end) return false;
    if(is_ufloat_valid_format(&cp) == false) return false;

    while (cp.beg < cp.end){
        int digit = 0;
        if(parse_decimal_digit(*cp.beg, &digit)){
            if(fractials) fractials++;
            value = value * 10 + digit;
        }
        else{
            if(sc_char_is_period(*cp.beg)){//if fractional, parse integral part
                if(fractials) return false;
                else fractials++;
                value = value * 10 + digit;
            }
            else{
                return false;
            }
        }
        cp.beg++;
    }
    if(fractials) fractials--;
    *result = value * sign * pow(0.1, fractials);
    return true;
}

bool sc_float(const StrChunk *source, float *result){
    double d_result = 0;
    bool status = sc_double(source, &d_result);
    if(status){
        *result = (float)d_result;
        return true;
    }
    else{
        return false;
    }
}

bool sc_bool(const StrChunk *source, bool *result){
    int i_result;
    if(sc_equals(source, L"true")) return *result = true, true;
    else if(sc_equals(source, L"True")) return *result = true, true;
    else if(sc_equals(source, L"TRUE")) return *result = true, true;
    else if(sc_equals(source, L"false")) return *result = false, true;
    else if(sc_equals(source, L"False")) return *result = false, true;
    else if(sc_equals(source, L"FALSE")) return *result = false, true;
    else if(sc_int(source, &i_result)) return *result = i_result != 0, true;
    else return false;
}

bool sc_is_c_name_compatible(const StrChunk *source){
    if(sc_is_empty(source)) return false;
    const wchar_t *curr = source->beg;
    
    if(*curr == L'_' 
    || (*curr >= L'a' && *curr <= L'z')
    || (*curr >= L'A' && *curr <= L'Z')){
        curr++;
        while (curr < source->end){
            if((*curr == L'_' 
            || (*curr >= L'a' && *curr <= L'z')
            || (*curr >= L'A' && *curr <= L'Z')
            || (*curr >= L'0' && *curr <= L'9')) == false){
                return false;
            }
        }
        return true;
    }
    else{
        return false;
    }
}

bool sc_char_is_decimal_digit(wchar_t ch){
    return ch >= L'0' && ch <= L'9';
}

bool sc_char_is_hexidecimal_digit(wchar_t ch){
    return sc_char_is_decimal_digit(ch) || (ch >= L'a' && ch <= L'f') || (ch >= L'A' && ch <= L'F');
}

bool sc_char_is_period(wchar_t ch){
    return ch == L'.' || ch == L',';
}

static bool parse_hexidecimal(const StrChunk *source, int *result){
    StrChunk cp = *source;
    if(cp.end - cp.beg < 1 || cp.end - cp.beg > 8) return false;
    unsigned int value;
    while (cp.beg < cp.end){
        int digit;
        if(parse_hexidecimal_digit(*cp.beg, &digit)){
            value = value << 4 | digit;
        }
        else{
            return false;
        }
        cp.beg++;
    }
    *result = (int)(value & ~(1 << 31) ) * (value >> 31 ? -1 : 1);
    return true;
}

static bool parse_decimal(const StrChunk *source, int *result){
    StrChunk cp = *source;
    int sign = skip_int_sign(&cp);
    int minus = sign < 0? -1 : 0;
    unsigned int value = 0;
    
    if(cp.beg >= cp.end || cp.end - cp.beg > 10) return false;
    if(is_ufloat_valid_format(&cp) == false) return false;

    while (cp.beg < cp.end){
        int digit = 0;
        if(parse_decimal_digit(*cp.beg, &digit)){
            value = value * 10 + digit;
            if((value + minus) & (1 << 31)) return false;//check overflow
        }
        else{
            if(sc_char_is_period(*cp.beg)){//if fractional, parse integral part
                *result = value * sign;
                return true;
            }
            else{
                return false;
            }
        }
        cp.beg++;
    }
    *result = value * sign;
    return true;
}

static bool parse_decimal_digit(wchar_t ch, int *result){
    if(sc_char_is_decimal_digit(ch)){
        *result = ch - L'0';
        return true;
    }
    else{
        return false;
    }
}

static bool parse_hexidecimal_digit(wchar_t ch, int *result){
    if(parse_decimal_digit(ch, result)){
        return true;
    }
    else{
        if(ch >= L'a' && ch <= L'f' ){
            *result = 10 + ch - L'a';
            return true;
        }
        else if(ch >= L'A' && ch <= L'F' ){
            *result = 10 + ch - L'A';
            return true;
        }
        else{
            return false;
        }
    }
}

static int skip_int_sign(StrChunk *source){
    if(source->beg >= source->end) return 1;
    else if(*source->beg == L'-') return source->beg++, -1;
    else if(*source->beg == L'+') return source->beg++, 1;
    else return 1;
}

static bool is_ufloat_valid_format(const StrChunk *source){
    StrChunk cp = *source;
    if(sc_char_is_decimal_digit(*cp.beg++) == false) return false;
    while(cp.beg < cp.end){
        if(sc_char_is_decimal_digit(*cp.beg) || sc_char_is_period(*cp.beg)){
            cp.beg++;
        }
        else{
            return false;
        }
    }
    return true;
}
