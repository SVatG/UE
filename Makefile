CXXFLAGS= \
	-I. \
	-Wall \
	-ffast-math \
	-O3 \
	-g \
	-std=c++11
	
CFLAGS= \
	-I. \
	-Wall \
	-ffast-math \
	-O3 \
	-g \
	-std=gnu11


LIBS=-lm
OBJECTS=\
	main.o\
	glad.o\
	glhelpers.o\
	effect_blobs.o \
	effect_trithing.o \
	objloader.o \
	rocket/device.o \
	rocket/track.o \
	bass_rocket.o
	
all: $(OBJECTS)
	g++ $(OBJECTS) -L. -lbass -lm -lGL -lglfw -ldl -o main
    
clean:
	rm -r *.o
