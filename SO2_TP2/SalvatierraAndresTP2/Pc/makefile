#Makefile de Trabajo Practico N°2 de Sistemas Operativos
all: matriz
matriz : convolucion.c
	gcc -Wall -Werror -pedantic -O3 -o matriz convolucion.c -fopenmp `nc-config --cflags --libs`
clean:
	rm matriz \
