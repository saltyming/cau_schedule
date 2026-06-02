CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -O2
TARGET  = schedule
SRCS    = main.c course.c schedule.c optimizer.c display.c export.c
OBJS    = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
