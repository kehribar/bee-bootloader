/*-----------------------------------------------------------------------------
/
/
/
/
/
/
/----------------------------------------------------------------------------*/
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

int parseIntelHex(char *hexfile, uint8_t* buffer, int *startAddr, int *endAddr);

int parseHex(FILE *file_pointer, int num_digits);

int parseUntilColon(FILE *file_pointer);
