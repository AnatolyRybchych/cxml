#ifndef STRCHUNK_H
#define STRCHUNK_H

#include <stdbool.h>
#include <wchar.h>

typedef struct StrChunk StrChunk;
typedef int StrChunkCondition;
enum StrChunkCondition{
    SC_MAYBE_SOME_IN,
    SC_MAYBE_SOME_NOT_IN,
    SC_SOME_IN,
    SC_SOME_NOT_IN,
    SC_ONE_IN,
    SC_ONE_NOT_IN,
    SC_SEQUANCE,
    SC_STOP
};

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
bool sc_start_matches_all(const StrChunk *source, StrChunkCondition condition1, const StrChunk *sample1, ...);
bool sc_skip_start_matches_condition(StrChunk *source, StrChunkCondition condition, const StrChunk *sample);

struct StrChunk{
    const wchar_t *beg;
    const wchar_t *end;
};

#endif //STRCHUNK_H
