#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <array>

// Simple universal (utf-8) trie implementation I written for early pass at 1st sem course "fundamentals of programming"
// I tested it on SJP wordlist: https://sjp.pl/sl/growy/sjp-20230402.zip (42MB, 3.2 milion polish words)
// On my machine:
// Load to tree takes 0.7s and uses ~88MB memory
// Quering tree with same (sorted) list takes 0.4s
// Quering tree with shuffled list (worst case I guess) takes 2.4s
// Yeah, random memory access cost a lot
//
// For the reference: 
// Load to std::unordered_set<std::string> takes 1.1s, sorted query - 1.1s, unsorted - 1.5s
// Memory usage is harder to estimate with that one, but it seems to use a lot more
// More finely tuned hashmap would probably be even better
//
// After all, trie (at least that simple) seems to be only worth to give a fuck if you 
// need prefix search (or maybe when you have a lot of similar queries or a lot of mismatches)

#define MAXLINE 256
// Max size of "word" buffer you can save in tree.

struct LetterTree {
    LetterTree* childs;
    uint8_t childCount;
    char letter; // in case of root this field is ignored

    // insert string into tree. If already in tree, do nothing
    void insertString(const char* str);
    
    // return true if string is in tree, false otherwise
    bool findString(const char* str);

    // print at most n words starting with suplied prefix
    void printPrefixed(const char* prefix, int n);

    // Print words stored in tree. If there is more words than limit, print "..."
    // Each printed word is prefixed with string stored in @path param
    // Return -1 if too much words, @limit if no words, and (@limit - word_count) otherwise
    int printWords(int limit, char path[MAXLINE], int pathLen);

    // Like printWords but letter in root is also printed
    int printWords2(int limit, char path[MAXLINE], int pathLen);

    // count all descendants
    int recursiveNodeCount();

    // Find child containing suplied char. NULL if not found.
    LetterTree* findChild(char ch);

    // Insert suplied character as node, and return its adress
    // If there is node with that ch, do nothing and just return its adress
    LetterTree* insertChild(char ch);

    // Free tree's memory recursivly.
    void recursiveFree();

#if defined(__GNUC__) && (defined(__i386__) || defined(__amd64__))
}__attribute__((packed));
// Removing struct padding saves us a lot of memory.
// On my machine it also slightly improves run time on big dicts.
// We only do it on x86 as it might be unsafe on other platforms
#else
};
#endif

void LetterTree::insertString(const char* str)
{
    LetterTree* node = insertChild(*str);
    if(*str) node->insertString(str+1);
}

LetterTree* LetterTree::findChild(char ch)
{
    for(uint8_t i = 0; i<childCount; ++i) {
        if(childs[i].letter == ch) return &childs[i];
    }
    return NULL;
}

LetterTree* LetterTree::insertChild(char ch)
{
    LetterTree* child = findChild(ch);
    if(child) return child;

    childs = (LetterTree*)realloc(childs, sizeof(LetterTree) * (childCount+1));
    // yeah realloc + 1 looks suspicious, but:
    // 1) it is simple and saves us a lot of memory
    // 2) memusage(1) shows that only 10-15% calls do copies so I guess it is not much of an issue.
    // NOTE: If you want to replicate results, insert `if(NULL) malloc else realloc`
    // as memusage does not distinguish realloc(NULL) from reallocations that require copying

    if(childs == NULL) { perror("dict error"); exit(1); }
    child = &childs[childCount++];
    child->letter = ch;
    child->childCount = 0;
    child->childs = NULL; // NULL makes calloc work like malloc 
    return child;
}

bool LetterTree::findString(const char* str)
{
    LetterTree* node = findChild(*str);
    if(!node) return false;
    if(*str == '\0') return true; // we reached end of string without any mismatch
    return node->findString(str+1);
}

void LetterTree::recursiveFree()
{
    for(uint8_t i = 0; i<childCount; ++i) {
        childs[i].recursiveFree();
    }
    free(childs);
}

int LetterTree::recursiveNodeCount()
{
    int count = childCount;
    for(uint8_t i = 0; i<childCount; ++i) {
        count += childs[i].recursiveNodeCount();
    }
    return count;
}

void LetterTree::printPrefixed(const char* prefix, int limit)
{
    char buf[MAXLINE] = {0};
    LetterTree* node = this;
    for(int i = 0; prefix[i] != '\0'; ++i) {
        node = node->findChild(prefix[i]);
        if(node == NULL) return; // there is no word starting with suplied prefix
        buf[i] = prefix[i];
    }
    if(node->printWords(limit, buf, strlen(buf)) == limit) { puts("No words found"); }
}

#define TOO_MUCH_CHILDS -1
int LetterTree::printWords(int limit, char path[MAXLINE], int pathLen)
{
    for(int i = 0; i < childCount && limit != TOO_MUCH_CHILDS; ++i) {
        limit = childs[i].printWords2(limit, path, pathLen);
    }
    return limit;
}
int LetterTree::printWords2(int limit, char path[MAXLINE], int pathLen)
{
    assert(pathLen+1 < MAXLINE);
    path[pathLen++] = letter;

    if(limit == 0) { puts("..."); return TOO_MUCH_CHILDS; }
    if(letter == '\0') { puts(path); return limit - 1; }

    limit = printWords(limit,path,pathLen);

    path[--pathLen] = '\0';
    return limit;
}

char* readword(FILE* file, char buf[MAXLINE])
{
    char* ptr = fgets(buf, MAXLINE, file);
    if(!ptr) return NULL;
    if(strlen(ptr) != 0 && buf[strlen(ptr) - 1] != '\n') {
        fprintf(stderr, "dict error: line too long\n"); exit(1);
    }
    buf[strcspn(buf, "\r\n")] = 0;
    return ptr;
}

LetterTree readWordlist(const char* filename)
{
    LetterTree tree = {NULL, 0, '\0'};
    FILE* file = fopen(filename, "r");
    if(file == NULL) { perror("dict error"); exit(1); }
    char buf[MAXLINE];
    while(readword(file, buf)) {
        tree.insertString(buf);
    }
    fclose(file);
    return tree;
}

void help(char* progname)
{
    fprintf(stderr, 
    "Usage:\n"
    " Print each matched line: %s worldlist.txt\n"
    " Print each mismatched line: %s wordlist.txt --invert\n"
    " Print 1 on match and 0 on mismatch: %s wordlist.txt --bool\n"
    " Print list of 'prefix matches': %s wordlist.txt --prefix\n"
    , progname, progname, progname, progname);
    exit(1);
}

int main(int argc, char** argv)
{
    bool invert = 0, boolmode = 0, prefixmode = 0;
    if(argc == 3 && !strcmp(argv[2], "--invert")) invert=true;
    else if(argc == 3 && !strcmp(argv[2], "--bool")) boolmode=true;
    else if(argc == 3 && !strcmp(argv[2], "--prefix")) prefixmode=true;
    else if(argc != 2 || !strcmp(argv[1], "-h")) help(argv[0]);

    LetterTree tree = readWordlist(argv[1]);
    if(invert + boolmode + prefixmode > 1) { 
        fprintf(stderr, "dict error: Options can't be combined\n");
        exit(1);
    }

    char buf[MAXLINE];
    while(readword(stdin, buf)) {
        if(prefixmode) {
            puts("---");
            tree.printPrefixed(buf, 10);
            puts("");
        } else {
            bool match = tree.findString(buf);
            if(boolmode) printf("%d\n", match);
            else if(!invert && match) printf("%s\n", buf);
            else if(invert && !match) printf("%s\n", buf);
        }
    }

    // tree.recursiveFree(); 
    // Tree lives through whole program, and freeing it is slow, so we leave it up to OS
}

// Optimization ideas (listed mostly out of academic curiosity)
// I don't know whether I will implement them, as:
// 1) They would likely make this "simple trie" complicated
// 2) I have no real use case for this and I probably have more important stuff to do
// 3) For sure, faster implementations already exist 
#if 0
// We could represent tree as LCRS (https://en.wikipedia.org/wiki/Left-child_right-sibling_binary_tree)
// It would allow us replace realloc with very simple (and fast) memory pool like this (cpp-like pseudocode):
std::vector<Node> nodePool;
...
nodepool.reserve(64MB);
...

// Return index of newly allocated node
// NOTE: We don't use pointers for size optimization
uint32_t nodeAlloc()
{
   nodePool.resize(nodePool.size+1);
   return nodePool.size-1;
}

struct Node {
   uint32_t childIdx;
   uint32_t siblingIdx;
   char letter;
   ...
}
// 
// I'm not sure, but I intuitively guess, that if we (re)build such LCRS in breadth-first order
// then (assuming that node has child) childIdx = indexof(node) + countOfSiblingsThatGoAfter
// If I am correct, then we could represent each node in just two bytes without worring about padding!
// struct Node {
//   uint8_t rightSiblingCount; // max is 254
//   char letter; // if not '\0' then it has child(s)
//   ...
// }
// It could be compressed even more by storing count of siblings once per each "level" (e.g. in separate vector)
#endif
