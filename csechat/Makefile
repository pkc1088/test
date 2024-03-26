CC = gcc

CFLAGS = -Wall -Wextra -std=c99

LIBS = -lncurses

OBJ_READ = csechatread.o
OBJ_WRITE = csechatwrite.o
OBJ_REMOVE = csechatremove.o

TARGET_READ = csechatread
TARGET_WRITE = csechatwrite
TARGET_REMOVE = csechatremove

DEPS = csechatshm.h

all: $(TARGET_READ) $(TARGET_WRITE) $(TARGET_REMOVE)

$(TARGET_READ): $(OBJ_READ)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

$(TARGET_WRITE): $(OBJ_WRITE)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

$(TARGET_REMOVE): $(OBJ_REMOVE)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJ_READ) $(OBJ_WRITE) $(OBJ_REMOVE) $(TARGET_READ) $(TARGET_WRITE) $(TARGET_REMOVE)

