#!/bin/bash

g++ -I/usr/local/include/ -I/public/ig/glm/ -c main5.cpp  -omain5.o
g++ -I/usr/local main5.o -lglfw  -lGLEW  -lGL  -omain5
./main5
