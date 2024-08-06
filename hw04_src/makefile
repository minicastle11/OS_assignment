CC = gcc
CFLAGS = -Wall -Wextra -g

TARGET = vmsim
OBJS = vmsim_main.o vmsim_op.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

vmsim_main.o: vmsim_main.c vmsim_main.h vmsim_op.h
	$(CC) $(CFLAGS) -c vmsim_main.c

vmsim_op.o: vmsim_op.c vmsim_op.h
	$(CC) $(CFLAGS) -c vmsim_op.c

clean:
	rm -f $(TARGET) $(OBJS)
