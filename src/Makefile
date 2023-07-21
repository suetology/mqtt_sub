CPPFLAGS:= -I$(CURDIR)
LDFLAGS:= -lmosquitto -lm -luci -lcurl
DEPS = $(wildcard *.h)
SOURCE = $(wildcard *.c)
OBJ = $(SOURCE:.c=.o)
BIN:= mqtt_sub

.PHONY: all clean

all: $(BIN)

%.o: %.c $(DEPS)
	$(CC) -c $< $(CPPFLAGS) -o $@

$(BIN): $(OBJ)
	$(CC) $^ $(LDFLAGS) -o $@

clean:
	$(RM) $(BIN) $(OBJ)

