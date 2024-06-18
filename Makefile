CC = gcc

BINDIR = bin
INCLUDEDIR = include
SRCDIR = src

CFLAGS = -Wall -I$(INCLUDEDIR)
LDLIBS = -lm

# wildcard関数で SRCDIR以下のcファイルをリスト
SRCS = $(wildcard $(SRCDIR)/*.c)
SRCS_THREAD = $(filter-out src/phone_app.c, $(wildcard src/*.c))
SRCS_PHONE = $(filter-out src/thread_app.c, $(wildcard src/*.c))
# SRCSの拡張子を置き換える
OBJS_THREAD = $(SRCS_THREAD:%.c=%.o)
OBJS_PHONE = $(SRCS_PHONE:%.c=%.o)

TARGET = $(BINDIR)/thread
PHONE = $(BINDIR)/phone

all: $(TARGET) $(PHONE)

$(TARGET): $(OBJS_THREAD)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)
	
$(PHONE): $(OBJS_PHONE)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

.PHONY: tmpclean clean

tmpclean:
	rm -f $(SRCDIR)/*~ *~
clean: tmpclean
	rm -f $(TARGET) $(PHONE) $(OBJS_THREAD) $(OBJS_PHONE)