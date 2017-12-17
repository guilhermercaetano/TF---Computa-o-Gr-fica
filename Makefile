OPENGL_FLAGS=-lGL -lGLU -lglut
TINYXML_O_FILES=tinystr.o tinyxmlparser.o tinyxmlerror.o tinyxml.o
CUSTOM_OBJ_FILES=algebra.o imageloader.o tf.o glew.o objloader.o

all: trabalhocg

glew:
	gcc -c libs/glew.c

objloader:
	g++ -c libs/objloader/*.cpp -g

TinyXML:
	g++ -c libs/tinyxml/*.cpp -g

algebra:
	g++ -c codigo/algebra.cpp -lmath -std=c++11

tf:
	g++ -c codigo/tf.cpp -Ilibs/ -g -lmath -std=c++11

trabalhocg: TinyXML tf algebra objloader glew
	g++ -o trabalhocg $(TINYXML_O_FILES) $(CUSTOM_OBJ_FILES) $(OPENGL_FLAGS) -g -std=c++11

clean:
	rm -rf *.o trabalhocg