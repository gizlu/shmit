#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#ifndef RUNDECK_H
#define RUNDECK_H

// Legends of Runeterra deck encoding (https://github.com/RiotGames/LoRDeckCodes) 
// implemented in single header. Due to C not having base32 in stdlib, this lib 
// works with binary blobs and dealing with base32 is left to user.
// See examples/rundeck_b32.c for how to use it with shlag_b32.h lib (TODO)

#define RUNDECK_MAX_KNOWN_VER 5
#define RUNDECK_FORMAT 1

typedef struct RundeckEntry {
    uint8_t setId;     /* must be less than 100 */
    uint8_t factId;    /* use fact_to_str() to check if yours faction is supported. */
    uint16_t cardId;   /* must be less than 1000 */
    uint8_t count;     /* count of copies of this card */
} RundeckEntry;
// entries with count 0 are ignored by encoder

// convert faction str to factId. Returns true on success, false otherwise.
bool rundeck_fact_from_str(const char factionStr[3], uint8_t* factIdOut);
// convert factid into factionStr (3 bytes including terminator). On fail return "\0\0"
const char* rundeck_fact_to_str(uint8_t factId);

// test cases potential fuckups and issues:
// - normal decks
// - empty deck
// - uncompressible deck
// - deck with 
// - set 255 fact 255
// - decksorting
// - czy county i iteratory się nie przepełniają

// sort entries in contrived way to simplify encoding.
// "How exacly" sorting works is not part of public API and can change in future
void rundeck_contrived_decksort(RundeckEntry* deck, uint8_t deckSize);

// Encode deck (previously sorted with contrived_decksort() or outputed by decoder) to blob and return
// count of written bytes (or required buffer size if buf is too small)
// encode(deck,deckSize,NULL,0) may be used to get required buf size
uint16_t rundeck_encode(const RundeckEntry* deck, uint8_t deckSize, uint8_t* blob, uint16_t blobSize);

// TIP: I think in worst case, the size of blob is 7 + 6*deckSize
// You can use this fact to choose buffer size, but you should not mindlessly
// trust it i.e. you should still check encode() return code as I might have
// made mistake and it can change in future. NOTE: Usually size is much
// smaller, and this "uncompressible" deck would have to be created on purpose

// error codes/bitflags of rundeck_decode()
#define RUNDECK_EOK 0 // everything is ok
#define RUNDECK_EVER 1 // blob uses unsupported encoding version
#define RUNDECK_EFAIL 2 // misc error (e.g. size doesn't fit in uint8_t)
// other errors might be defined in future

// Decode blob to array of entries, return count of written entries and set @err to 0
// If deck size is insufficient, return required size (unless it would overflow u8)
// If errors are detected @err bitflags are set approprietly and return is unspecified
// NOTE: entries are not validated
//
// outputed deck is sorted like contrived_decksort() (so you can pass it directly to encoder without aditional sorting)
//
// decode(NULL,0,blob,blobsize,&err) may be used to get required deckSize 
uint8_t rundeck_decode(RundeckEntry* deck, uint8_t deckSize, const uint8_t* blob, uint16_t blobSize, uint32_t* err);

#endif // RUNDECK_H

#ifdef RUNDECK_IMPL

// lookup tables for mapping factId to strings, and minimal required lib version
static const char* rundeckFactStrLut[]  = {"DE","FR","IO","NX","PZ","SI","BW","SH","\0\0","MT","BC","RU"};
static const uint8_t rundeckFactVerLut[] = {1,1,1,1,1,1,2,3,255,2,4,5}; 

// shorthand
#define RUNDECK_LOOKUP(arr,idx,failreturn) \
    if(idx < sizeof(arr)/sizeof(arr[0])) return arr[idx]; \
    else return failreturn

const char* rundeck_fact_to_str(uint8_t factId)
{
    RUNDECK_LOOKUP(rundeckFactStrLut, factId, "\0\0");
}

// find minimal version required for suplied fact. On failure 255 is returned
uint8_t rundeck_fact_to_ver(uint8_t factId)
{
    RUNDECK_LOOKUP(rundeckFactVerLut, factId, 255);
}

bool rundeck_fact_from_str(const char factionStr[3], uint8_t* factIdOut) 
{
    uint8_t lutsize = sizeof(rundeckFactStrLut)/sizeof(rundeckFactStrLut[0]);
    for(uint8_t i = 0; i < lutsize; ++i) {
        if(memcmp(factionStr, rundeckFactStrLut[i], 2) == 0) {
            *factIdOut=i;
            return true;
        }
    }
    return false;
}


// helper func for rundeck_entry_sortcb()
// sort entries in asc order by set, then faction, then card id
static int rundeck_entry_sortcb2(const RundeckEntry* e1, const RundeckEntry* e2)
{
    if (e1->setId > e2->setId) return 1;
    if (e1->setId < e2->setId) return -1;
    if (e1->factId > e2->factId) return 1;
    if (e1->factId < e2->factId) return -1;
    if (e1->cardId > e2->cardId) return 1;
    if (e1->cardId < e2->cardId) return -1;
    return 0;
}

// calllback for qsort for grouping/sorting entries in this (ridiculus) way:
// - first go entries with <= 3 copies.
//    Internally they are sorted *desc* by count, then *asc* by set, faction and id
// - then go entries with  < 3 copies.
//    Internally they are sorted *asc* by set, faction and card id
static int rundeck_entry_sortcb(const void* v1, const void* v2) {
    const RundeckEntry* card1 = v1;
    const RundeckEntry* card2 = v2;
    if (card1->count <= 3 && card2->count <= 3) {
        if (card1->count < card2->count) return 1;
        if (card1->count > card2->count) return -1;
        return rundeck_entry_sortcb2(card1, card2);
    }
    if (card1->count > 3 && card2->count > 3) return rundeck_entry_sortcb2(card1, card2);
    if (card1->count <= 3) return -1; /* only first card have <= 3 copies */
    else return 1; /* only second card have <= 3 copies */
}

void rundeck_contrived_decksort(RundeckEntry* deck, uint8_t deckSize)
{
    qsort(deck, deckSize, sizeof(RundeckEntry), rundeck_entry_sortcb);
}

typedef struct RundeckGroup {
    uint8_t begin;
    uint8_t len;
} RundeckGroup;

typedef struct RundeckGroups {
    RundeckGroup arr[256];
    uint8_t count; // count of groups
    uint8_t lenSum; // sum of cards in groups
} RundeckGroups;

// callback for sorting groups by len in asc order
int rundeck_group_sort_cb(const void* group1v, const void* group2v)
{
    const RundeckGroup* g1 = group1v;
    const RundeckGroup* g2 = group2v;
    if(g1->len > g2->len) return 1;
    if(g1->len < g2->len) return -1;
    return 0;
}

typedef struct RundeckState {
    RundeckEntry* deck;
    uint8_t* blob; // blob with encoded deck
    uint32_t* err;
    uint16_t blobSize;
    uint16_t blobIdx; // index of next byte to encode/decode in blob
    uint8_t deckSize;
    uint8_t deckIdx; // index of next record to encode/decode in deck
} RundeckState;

// From group of entries with @n copies, extract groups of set and fact combinations
RundeckGroups rundeck_extract_set_fact_groups(const RundeckState* s, uint8_t n)
{
    RundeckGroups groups = {0};
    for(uint8_t i = s->deckIdx; (i < s->deckSize && s->deck[i].count == n); ++i) {
        if(i == s->deckIdx || s->deck[i].factId != s->deck[i-1].factId || s->deck[i].setId != s->deck[i-1].setId) { // new group
            RundeckGroup newgrp = {i, 1};
            groups.arr[groups.count++] = newgrp;
        } else {
            ++groups.arr[groups.count - 1].len;
        }
        ++groups.lenSum;
    }
    return groups;
}

// write @num into blob as varint (little endian, https://developers.google.com/protocol-buffers/docs/encoding#varints)
// blobIdx is updated even if buffer is insufficient (but nothing is written)
void rundeck_encode_varint(RundeckState* s, uint16_t num)
{
    do
    {
        uint8_t byte = num & 127; // get 7 bytes
        num >>= 7;
        if (num) byte |= 128; // if not last byte, set MSB to 1
        if (s->blobIdx < s->blobSize) s->blob[s->blobIdx] = byte;
        s->blobIdx++;
        assert(s->blobIdx > 0); // we do overflow check as assert only, as it should be impossible
    } while (num);
}

// "pop" varint from front of blob. If there are no bytes to pop,
// then output is unspecified (but there should not be out of buffer reads)
// If there are no bytes to pop set s->malformed=1 and return 0
// We don't check for number overflows (I think there is no need)
uint16_t rundeck_decode_varint(RundeckState* s)
{
    uint16_t result = 0;
    uint8_t shift = 0;
    for(; s->blobIdx < s->blobSize; ++s->blobIdx) {
        result |= (s->blob[s->blobIdx] & 127) << shift;
        if(!(s->blob[s->blobIdx] & 128)) { // end of num
            ++s->blobIdx; return result;
        }
        shift += 7;
    }
    return 0; // TODO FIXME
    /* s->err |= RUNDECK_EOBSCURE; */
}

void rundeck_push_entry(RundeckState* s, RundeckEntry e)
{
    // FIXME: potencjalny problem z overflowem
    if(s->deckIdx + 1 < s->deckSize) s->deck[s->deckIdx++] = e;
    /* else s->err |= RUNDECK_ENOBUFS; */
}

// Encode "normal" entries with @n copies. n shall be <= 3
void rundeck_encode_normals(RundeckState* s, uint8_t n)
{
    assert(n <= 3);
    RundeckGroups groups = rundeck_extract_set_fact_groups(s, n);
    qsort(groups.arr,  groups.count, sizeof(RundeckGroup), rundeck_group_sort_cb);

    rundeck_encode_varint(s, groups.count); // how many set/faction combinations
    for(uint8_t i = 0; i < groups.count; ++i) {
        RundeckGroup group = groups.arr[i];
        rundeck_encode_varint(s, group.len); // count of entries in set/faction combination
        rundeck_encode_varint(s, s->deck[group.begin].setId);
        rundeck_encode_varint(s, s->deck[group.begin].factId);
        for(uint8_t j = 0; j < group.len; ++j) {
            rundeck_encode_varint(s, s->deck[group.begin + j].cardId);
        }
    }
    s->deckIdx += groups.lenSum;
}
// Decode "normal" entries with @n copies. n shall be <= 3
void rundeck_decode_normals(RundeckState* s, uint8_t n)
{
    assert(n <= 3);
    RundeckEntry entry;
    entry.count = n;
    uint8_t countOfN = rundeck_decode_varint(s); // how many set/faction combinations
    for(uint8_t i = 0; i < countOfN; ++i) {
        uint8_t groupLen = rundeck_decode_varint(s); // count of entries in set/faction combination
        entry.setId = rundeck_decode_varint(s);
        entry.factId = rundeck_decode_varint(s);
        for(uint8_t j = 0; j < groupLen; ++j) {
            entry.cardId = rundeck_decode_varint(s);
            rundeck_push_entry(s, entry);
        }
    }
}

// Encode all "leftover" entries (with count > 3)
void rundeck_encode_leftovers(RundeckState* s)
{
    for(; s->deckIdx < s->deckSize; ++s->deckIdx) {
        rundeck_encode_varint(s, s->deck[s->deckIdx].count);
        rundeck_encode_varint(s, s->deck[s->deckIdx].setId);
        rundeck_encode_varint(s, s->deck[s->deckIdx].factId);
        rundeck_encode_varint(s, s->deck[s->deckIdx].cardId);
    }
}

// Decode all "leftover" entries (with count > 3)
void rundeck_decode_leftovers(RundeckState* s)
{
    while(s->blobIdx < s->blobSize) {
        RundeckEntry e = {rundeck_decode_varint(s), rundeck_decode_varint(s), rundeck_decode_varint(s), rundeck_decode_varint(s)};
        rundeck_push_entry(s, e);
    }
}

// skip empty records from encoding
void rundeck_skip_empty(RundeckState* s)
{
    for(; s->deckIdx < s->deckSize && s->deck[s->deckIdx].count == 0; ++s->deckIdx) {}
}

// write minimal required version of encoding required for blob decoding
void rundeck_write_format_and_ver(RundeckState* s)
{
    uint8_t maxVer = 0;
    for(uint8_t i = 0; i < s->deckSize; ++i) {
        uint8_t ver = rundeck_fact_to_ver(s->deck[i].factId);
        if(ver > maxVer) maxVer = ver;
    }
    assert(maxVer <= RUNDECK_MAX_KNOWN_VER);
    if(s->blobSize > 0) s->blob[0] = (RUNDECK_FORMAT << 4 | maxVer);
    ++s->blobIdx;
}
// check if blob uses suported encoding version
void rundeck_read_format_and_ver(RundeckState* s)
{
    if(s->blobSize > 0) {
        uint8_t ver = s->blob[0] & 15, fmt = s->blob[0] >> 4;
        if(ver > RUNDECK_MAX_KNOWN_VER || fmt != RUNDECK_FORMAT) *s->err |= RUNDECK_EVER;
        ++s->blobIdx;
    } else *s->err |= RUNDECK_EFAIL;
}

uint16_t rundeck_encode(const RundeckEntry* deck, uint8_t deckSize, uint8_t* blob, uint16_t blobSize)
{
    RundeckState state = {0};
    state.deck = (RundeckEntry*)deck; state.blob = blob; state.blobSize = blobSize; state.deckSize = deckSize;
    rundeck_write_format_and_ver(&state);
    rundeck_encode_normals(&state, 3);
    rundeck_encode_normals(&state, 2);
    rundeck_encode_normals(&state, 1);
    rundeck_skip_empty(&state);
    rundeck_encode_leftovers(&state);
    return state.blobIdx;
}

uint8_t rundeck_decode(RundeckEntry* deck, uint8_t deckSize, const uint8_t* blob, uint16_t blobSize, uint32_t* err)
{
    RundeckState state = {0}; *err = 0;
    state.deck = deck; state.blob = (uint8_t*)blob; state.blobSize = blobSize; state.deckSize = deckSize; state.err = err;
    rundeck_read_format_and_ver(&state);
    rundeck_decode_normals(&state, 3);
    rundeck_decode_normals(&state, 2);
    rundeck_decode_normals(&state, 1);
    rundeck_decode_leftovers(&state);
    return state.deckIdx;
}

#endif // RUNDECK_IMPL

// playground: https://godbolt.org/z/43rMh7nK5
