INCLUDE_DIRS = "-I/usr/local/include -I/opt/X11/include  -I/usr/local/include/assimp"
LIB_DIRS = "-L/usr/local/lib -L/opt/X11/lib"

build:
	g++ -w main.cpp -o main -framework OpenGL -framework GLUT -lGLEW -lassimp
