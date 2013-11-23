/*-----------------------------------------------------------------------------
/
/
/
/
/
/
/----------------------------------------------------------------------------*/
#include "intel_hex_parser.h"
/*---------------------------------------------------------------------------*/
int parseIntelHex(char *hexfile, uint8_t* buffer, int *startAddr, int *endAddr) 
{
  int address, base, d, segment, i, lineLen, sum;
  FILE *input;
  
  input = strcmp(hexfile, "-") == 0 ? stdin : fopen(hexfile, "r");
  if (input == NULL) {
    printf("> Error opening %s: %s\n", hexfile, strerror(errno));
    return 1;
  }
  
  while (parseUntilColon(input) == ':') {
    sum = 0;
    sum += lineLen = parseHex(input, 2);
    base = address = parseHex(input, 4);
    sum += address >> 8;
    sum += address;
    sum += segment = parseHex(input, 2);  /* segment value? */
    if (segment != 0) {   /* ignore lines where this byte is not 0 */
      continue;
    }
    
    for (i = 0; i < lineLen; i++) {
      d = parseHex(input, 2);
      buffer[address++] = d;
      sum += d;
    }
    
    sum += parseHex(input, 2);
    if ((sum & 0xff) != 0) {
      printf("> Warning: Checksum error between address 0x%x and 0x%x\n", base, address);
    }
    
    if(*startAddr > base) {
      *startAddr = base;
    }
    if(*endAddr < address) {
      *endAddr = address;
    }
  }
  
  fclose(input);
  return 0;
}
/*---------------------------------------------------------------------------*/
int parseUntilColon(FILE *file_pointer) 
{
  int character;
  
  do {
    character = getc(file_pointer);
  } while(character != ':' && character != EOF);
  
  return character;
}
/*---------------------------------------------------------------------------*/
int parseHex(FILE *file_pointer, int num_digits) 
{
  int iter;
  char temp[9];

  for(iter = 0; iter < num_digits; iter++) {
    temp[iter] = getc(file_pointer);
  }
  temp[iter] = 0;
  
  return strtol(temp, NULL, 16);
}
/*---------------------------------------------------------------------------*/