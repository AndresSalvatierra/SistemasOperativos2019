#Makefile de Trabajo Practico N°2 de Sistemas Operativos
all: matriz
matriz : convolucion.c
	gcc -O3 -o matriz convolucion.c -fopenmp `nc-config --cflags --libs`
clean:
	rm matriz \
