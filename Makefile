OPENGL_FLAGS=-lGL -lGLU -lglut
TINYXML_O_FILES=tinystr.o tinyxmlparser.o tinyxmlerror.o tinyxml.o

all: trabalhocg

imageloader:
	g++ -c libs/imageloader/imageloader.cpp -g

TinyXML:
	g++ -c libs/tinyxml/*.cpp -g

algebra:
	g++ -c codigo/algebra.cpp -lmath -std=c++11

tf:
	g++ -c codigo/tf.cpp -Ilibs/ -g -lmath -std=c++11

trabalhocg: TinyXML tf algebra imageloader
	g++ -o trabalhocg $(TINYXML_O_FILES) algebra.o imageloader.o tf.o  $(OPENGL_FLAGS) -g -std=c++11

clean:
	rm -rf *.o trabalhocg