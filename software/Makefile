#------------------------------------------------------------------------------
#
#
#
#
#------------------------------------------------------------------------------ 
APP_NAME = main 
HEADERS = intel_hex_parser.h
OBJECTS = main.o intel_hex_parser.o

default: $(APP_NAME)

%.o: %.c $(HEADERS)
	gcc -c $< -o $@ -Wall

$(APP_NAME): $(OBJECTS)
	gcc $(OBJECTS) -o $@ -Wall

clean:
	-rm -f $(OBJECTS)
	-rm -f $(APP_NAME)
