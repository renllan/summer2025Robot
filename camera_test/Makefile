TARGET = camera_test
SOURCES = camera_test.c ../include/video_interface.c ../include/draw_bitmap_multiwindow.c
OBJECTS = camera_test.o video_interface.o draw_bitmap_multiwindow.o

CFLAGS = -g -Og -Werror -Wno-deprecated-declarations `pkg-config --cflags gtk+-2.0`
LDFLAGS = `pkg-config --libs gtk+-2.0` -lm -lpthread -lswscale

all: $(TARGET)

clean:
	rm -f $(OBJECTS) $(TARGET)

$(TARGET): $(OBJECTS)
	gcc $(OBJECTS) $(LDFLAGS) -o $@

%.o: %.c
	gcc $(CFLAGS) -c $< -o $@
%.o: ../include/%.c
	gcc $(CFLAGS) -c $< -o $@