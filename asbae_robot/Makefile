# software that needs to be installed (sudo apt install <package>):
# libswscale-dev
# libgtk2.0-dev

TARGET = main

SOURCES = main.c\
          hw6.c \
          find_egg_blobs.c\
          video_interface.c \
          draw_bitmap_multiwindow.c \
          ../include/import_registers.c \
          ../include/enable_pwm_clock.c \
          ../include/wait_period.c \
		      ../include/keypress.c\
          scale_image_data.c\
          to_black_white.c\
          C_equiv_ctl.c\
          edge_detection.c


OBJECTS = $(patsubst %.cpp,%.o,$(patsubst %.c,%.o,$(SOURCES)))

CFLAGS = `pkg-config --cflags --libs gtk+-2.0` \
         -Werror \
         -Wno-deprecated-declarations \
         -g\
         -pg \
         -Og

all: $(TARGET)

clean:
	rm -f $(OBJECTS) $(TARGET)

$(TARGET): $(OBJECTS)
	gcc $^ $(CFLAGS) -o $@ -lm -lpthread -lswscale

%.o:%.c
	gcc $(CFLAGS) -c $< -o $@
