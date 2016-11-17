CXXFLAGS= \
	-I. \
	-Wall \
	-ffast-math \
	-O3 \
	-g \
	-std=c++11 \

LIBS=-lm
OBJECTS=\
	main.o\
	glad.o\
	glhelpers.o\
	effect_blobs.o

all: $(OBJECTS)
	g++ $(OBJECTS) -lm -lGL -lglfw -ldl -o main
    
clean:
	rm -r *.o
