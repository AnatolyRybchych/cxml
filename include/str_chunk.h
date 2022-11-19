#ifndef STRCHUNK_H
#define STRCHUNK_H

#include <stdbool.h>
#include <wchar.h>
#include <math.h>
#include <stdarg.h>

#define SC_WHITE_SPACES L" \n\t\v\f\r"

#define _VAR_STR_CHUNK(MOD, NAME, WCS) MOD wchar_t _data_##NAME[] = WCS;MOD StrChunk NAME = {.beg = _data_##NAME, .end = _data_##NAME + sizeof(_data_##NAME) / sizeof(*_data_##NAME) - 1}
#define VAR_STR_CHUNK(NAME, WCS) _VAR_STR_CHUNK(,NAME,WCS)
#define STATIC_VAR_STR_CHUNK(NAME, WCS) _VAR_STR_CHUNK(static,NAME,WCS)

typedef struct StrChunk StrChunk;
typedef unsigned int StrChunkCondition;

bool sc_contains(const StrChunk *source, wchar_t character);
const StrChunk *sc_get_whitespaces(void); 
// returns count characters skipped
int sc_skip_all(StrChunk *source, const StrChunk *characters, bool reverse_characters);
bool sc_skip_one(StrChunk *source, const StrChunk *characters, bool reverse_characters);
int sc_skip_all_whitespaces(StrChunk *source);
int sc_skip_all_to_whitespace(StrChunk *source);
bool sc_starts_with_sc(const StrChunk *source, const StrChunk *sample);
bool sc_skip_starts_with_sc(StrChunk *source, const StrChunk *sample);
bool sc_skip_start_matches_all(StrChunk *source, StrChunkCondition condition1, const StrChunk *sample1, ...);
bool sc_skip_start_matches_all_v(StrChunk *source, StrChunkCondition condition1, const StrChunk *sample1, va_list argp);
bool sc_start_matches_all(const StrChunk *source, StrChunkCondition condition1, const StrChunk *sample1, ...);
bool sc_skip_start_matches_condition(StrChunk *source, StrChunkCondition condition, const StrChunk *sample);
void sc_copy_all_to(const StrChunk *source, StrChunk *destination, const StrChunk *characters);
void sc_skip_copy_all_to(StrChunk *source, StrChunk *destination, const StrChunk *characters);
void sc_iter_split(const StrChunk *source, void (*on_chunk)(const StrChunk *chunk, void *user_data), const StrChunk *delimiters, void *user_data);
bool sc_skip_to_start_matches_all(StrChunk *source, StrChunkCondition condition1, const StrChunk *sample1, ...);
bool sc_skip_to_start_matches_all_including(StrChunk *source, StrChunkCondition condition1, const StrChunk *sample1, ...);
bool sc_is_empty(const StrChunk *source);
bool sc_equals(const StrChunk *source, const wchar_t *wcs);
bool sc_int(const StrChunk *source, int *result);
bool sc_double(const StrChunk *source, double *result);
bool sc_float(const StrChunk *source, float *result);
bool sc_bool(const StrChunk *source, bool *result);
bool sc_is_c_name_compatible(const StrChunk *source);
bool sc_char_is_decimal_digit(wchar_t ch);
bool sc_char_is_hexidecimal_digit(wchar_t ch);
bool sc_char_is_period(wchar_t ch);

struct StrChunk{
    const wchar_t *beg;
    const wchar_t *end;
};

enum StrChunkCondition{
    SC_MAYBE_SOME_IN,
    SC_MAYBE_SOME_NOT_IN,
    SC_SOME_IN,
    SC_SOME_NOT_IN,
    SC_ONE_IN,
    SC_ONE_NOT_IN,
    SC_ONE_MORE_NOT_IN,
    SC_SEQUANCE,
    SC_STOP,
};

#endif //STRCHUNK_H
