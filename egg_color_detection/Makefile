TARGET = egg_sort

SOURCES = egg_sort.c \
          ../include/video_interface.c \
          ../include/draw_bitmap_multiwindow.c \
          ../asbae/find_egg_blobs.c \
          ../asbae/to_black_white.c \
          ../asbae/draw_bbox.c

OBJECTS = $(SOURCES:.c=.o)

CFLAGS = -g -Og -Werror -Wno-deprecated-declarations `pkg-config --cflags gtk+-2.0`
LDFLAGS = `pkg-config --libs gtk+-2.0` -lm -lpthread -lswscale

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) $(LDFLAGS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

../include/%.o: ../include/%.c
	$(CC) $(CFLAGS) -c $< -o $@

../asbae/%.o: ../asbae/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)
