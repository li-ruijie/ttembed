CFLAGS ?= -O2
PREFIX ?= /usr/local

ttembed: ttembed.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

clean:
	rm -f ttembed

.PHONY: clean
