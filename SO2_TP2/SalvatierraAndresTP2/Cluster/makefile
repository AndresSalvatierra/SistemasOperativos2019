#Makefile de Trabajo Practico N°2 de Sistemas Operativos
all: matrizicc matrizgcc

matrizicc : convolucion.c
        icc -xCORE_AVX2 -std=gnu99 -Wall -Werror -pedantic -L/home/Estudiante5/Development/SO2/hpc/libs/netcdf/lib/ -lnetcdf -O3 -o matrizicc convolucion.c -fopenmp

matrizgcc: convolucion.c
        gcc -Wall -Werror -pedantic -std=gnu99 -L/home/Estudiante5/Development/SO2/hpc/libs/netcdf/lib/ -lnetcdf -O3 -o matrizgcc convolucion.c -fopenmp

clean:
        rm matrizicc matrizgcc \


