OBJS = bss.o
SOURCE = bss.c
HEADER = sg_rs232.h sg_tcp.h
OUT = bss
LFLAGS =

all: bss

bss: $(OBJS)
	$(CC) -o $@ $^ $(LFLAGS)

%.o: %.c $(HEADER)
	$(CC) -c -o $@ $< $(LFLAGS)

clean:
	rm -f $(OBJS) $(OUT)

run: $(OUT)
	./$(OUT)