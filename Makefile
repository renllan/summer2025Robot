# software that needs to be installed (sudo apt install <package>):
# libswscale-dev
# libgtk2.0-dev

TARGET = hw6

SOURCES = hw6.c \
          video_interface.c \
          draw_bitmap_multiwindow.c \
          ../include/import_registers.c \
        ../include/enable_pwm_clock.c \
        ../include/wait_period.c \
		../include/keypress.c\
          scale_image_data.c

OBJECTS = $(patsubst %.cpp,%.o,$(patsubst %.c,%.o,$(SOURCES)))

CFLAGS = `pkg-config --cflags --libs gtk+-2.0` \
         -Werror \
         -Wno-deprecated-declarations \
         -g

all: $(TARGET)

clean:
	rm -f $(OBJECTS) $(TARGET)

$(TARGET): $(OBJECTS)
	gcc $^ $(CFLAGS) -o $@ -lm -lpthread -lswscale

%.o:%.c
	gcc $(CFLAGS) -c $< -o $@
