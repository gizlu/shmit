#define RUNDECK_IMPL
#include <stdio.h>
#include <unistd.h>
#include "rundeck.h"


void print_entry(RundeckEntry e)
{
    printf("%u:%02u%s%03u\n", e.count, e.setId, rundeck_fact_to_str(e.factId), e.cardId);
}

int main()
{
    unsigned char encoded[] = {
      0x11, 0x02, 0x04, 0x01, 0x05, 0x0f, 0x2c, 0x30, 0x36, 0x08, 0x01, 0x01,
      0x03, 0x0c, 0x14, 0x18, 0x21, 0x24, 0x27, 0x34, 0x02, 0x01, 0x01, 0x01,
      0x04, 0x01, 0x01, 0x05, 0x05, 0x00
    };
    unsigned int enc_len = 30;
    RundeckEntry deck[255];
    uint32_t err;
    
    uint8_t deckLen = rundeck_decode(deck, 255, encoded, enc_len, &err);
    if(err) { 
        printf("err: %u\n", err); exit(1);
    }

    
    /* printf("deckLen: %u\n", deckLen); */
    /* for(int i = 0; i < deckLen; ++i) { */
    /*     print_entry(deck[i]); */
    /* } */
    /* rundeck_contrived_decksort(deck, deckLen); */
    /* printf("deckLen: %u\n", deckLen); */
    /* for(int i = 0; i < deckLen; ++i) { */
    /*     print_entry(deck[i]); */
    /* } */

    uint8_t blob[2137];
    uint8_t blobLen = rundeck_encode(deck, deckLen, blob, sizeof(blob));
    /* printf("blobLen: %u\n", blobLen); */
    write(STDOUT_FILENO, blob, blobLen);    
}
