CFLAGS +=  -g -fsanitize=address
LDFLAGS += -lavformat -lavcodec -lavutil -lswresample -lavfilter -lm

SRCS := $(wildcard *.c)
SRCS := $(filter-out main.c, $(SRCS))

OBJS = $(SRCS:.c=.o)
LIBNAME = libcore.a

%.o: %.c
	$(CC)  -o $@ -c $< $(CFLAGS) $(LDFLAGS)

$(LIBNAME): $(OBJS)
	ar rcs $@ $^


main: main.c
	$(CC) -o $@ $< $(CFLAGS) $(LDFLAGS) -lcore -L./ -I./

clean: 
	rm $(OBJS) $(LIBNAME)
	rm main


