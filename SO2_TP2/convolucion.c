#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include "/home/andres/Development/SO2/hpc/libs/netcdf/include/netcdf.h"
#include <omp.h>

#define ERRCODE 2
#define ERR(e) {printf("Error: %s\n", nc_strerror(e)); exit(ERRCODE);}

#define FOTO "./tierra.nc"
#define FOTO_CONV "./tierra_conv.nc"

#define NX_T 21696
#define NY_T 21696

#define NDIMS 2

/*Matriz W*/
#define WX 3
#define WY 3

void conv(int i,int j,float *data_in,float *resultante, float mat_w [WX][WY]);

int main()
{        
    float mat_w[WX][WY]={{-1.0,-1.0,-1.0},{-1.0,8.0,-1.0},{-1.0,-1.0,-1.0}};
    int /*x, y,*/ retval, ncid,varid, nconv,nconvarid;//,x_dimid, y_dimid;
    size_t start[NDIMS], count[NDIMS];
    //int dimids[NDIMS];

    float *resultante=malloc(NX_T*NY_T*sizeof(float));
    float *data_in=malloc(NX_T*NY_T*sizeof(float));

    start[0]=0;
    start[1]=0;
    count[0]=NX_T;
    count[1]=NY_T;

    if ((retval = nc_open(FOTO, NC_NOWRITE, &ncid)))
            ERR(retval);
                
    /* Obtenemos elvarID de la variable CMI. */
    if ((retval = nc_inq_varid(ncid, "CMI", &varid)))
        ERR(retval);
    
    if((retval=nc_get_vara_float(ncid,varid,start, count,data_in)))
        ERR(retval);

    if ((retval = nc_close(ncid)))
        ERR(retval);

    struct timespec inicio, fin;

    if( clock_gettime( CLOCK_MONOTONIC_RAW, &inicio) == -1 ) {
      perror( "clock gettime" );
      exit( EXIT_FAILURE );
    }
   
    #pragma omp parallel for num_threads(4)
    for(int i=0; i<NX_T; i=i+1)
    {
        for(int j=0; j<NY_T; j=j+1)
        {
            if(data_in[i*NX_T + j]==-1)
            {
                data_in[i*NX_T + j]=(float)(0.0/0.0);
            }                       
        }
    }

    if( clock_gettime( CLOCK_MONOTONIC_RAW, &fin) == -1 ) {
      perror( "clock gettime" );
      exit( EXIT_FAILURE );
    }

    u_int64_t delta_us = (fin.tv_sec - inicio.tv_sec) * 1000000 + (fin.tv_nsec - inicio.tv_nsec) / 1000;
    u_int64_t total_time_s = delta_us/1000000;
    u_int64_t total_time_ms = (delta_us%1000000)/1000;
    printf("TERMINO DESCARGA EN %lds %ldms %ldus\n",total_time_s, total_time_ms, delta_us%1000);
    
    conv(1,1,data_in,resultante,mat_w);

    free(data_in);

    if ((retval = nc_open(FOTO_CONV, NC_WRITE , &nconv)))
            ERR(retval);
    
    if ((retval = nc_inq_varid(nconv, "CMI", &nconvarid)))
        ERR(retval);
    if((retval=nc_put_vara_float(nconv,nconvarid,start, count,resultante)))
        ERR(retval);
    if ((retval = nc_close(nconv)))
        ERR(retval);
    // if ((retval = nc_create(FOTO_CONV, NC_CLOBBER, &nconv)))
    //     ERR(retval);
    // if ((retval = nc_def_dim(nconv, "x", NX_T, &x_dimid)))
    //     ERR(retval);
    // if ((retval = nc_def_dim(nconv, "y", NY_T, &y_dimid)))
    //     ERR(retval);
    
    // dimids[0] = x_dimid;
    // dimids[1] = y_dimid;

    // if ((retval = nc_def_var(nconv, "CMI", NC_FLOAT, NDIMS, dimids, &nconvarid)))
    //     ERR(retval);

    // if ((retval = nc_enddef(nconv)))
    //     ERR(retval);

    // if((retval=nc_put_vara_float(nconv,nconvarid,start, count,resultante)))
    //     ERR(retval);

    // if ((retval = nc_close(nconv)))
    //     ERR(retval); 
    
    free(resultante);
    
    return 0;
}

void conv(int x, int y,float *data_in,float *resultante, float mat_w [WX][WY])
{   
    struct timespec start, end;
    double start_time = omp_get_wtime();
    if( clock_gettime( CLOCK_MONOTONIC_RAW, &start) == -1 ) {
      perror( "clock gettime" );
      exit( EXIT_FAILURE );
    }

    #pragma omp parallel for num_threads(4)
    for(int i=x; i<NX_T-1; i=i+1)
    {   

        for(int j=y; j<NY_T-1; j=j+1)
        {
            resultante[i*NX_T+j]= (data_in[(i-1)*NX_T + (j-1)]*mat_w[0][0]+ data_in[(i-1)*NX_T + j]*mat_w[0][1] +data_in[(i-1)*NX_T + (j+1)]*mat_w[0][2]+
                                data_in[i*NX_T + (j-1)]*mat_w[1][0]+ data_in[i*NX_T + j]*mat_w[1][1] +data_in[i*NX_T + (j+1)]*mat_w[1][2]+
                                data_in[(i+1)*NX_T + (j-1)]*mat_w[2][0]+ data_in[(i+1)*NX_T + j]*mat_w[2][1] +data_in[(i+1)*NX_T + (j+1)]*mat_w[2][2])*0.00031746;
        }
    }
    
    if( clock_gettime( CLOCK_MONOTONIC_RAW, &end) == -1 ) {
      perror( "clock gettime" );
      exit( EXIT_FAILURE );
    }
   double time_omp=omp_get_wtime()-start_time;
   u_int64_t delta_us = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
   u_int64_t total_time_s = delta_us/1000000;
   u_int64_t total_time_ms = (delta_us%1000000)/1000;
   printf("TERMINO DESCARGA EN %lds %ldms %ldus\n",total_time_s, total_time_ms, delta_us%1000);
   printf("%f OMP\n",time_omp);
}

//Acceso ssh Estudiante5@200.16.30.253
//       pass BER5!a4!
//Instalar el script de NETCDF en el cluster

//cpu info-> 16 cores, te genera dos thread por cada core fisico. Hay cambio de contexto todo el tiempo. Aca no hay cambio de contexto, no hace falta, lo retrasa. Hace que sea menos eficiente.
//Set de instrucciones AVX-512 registros de 512 bit para registros flotantes
//Source a donde estan los opt/intel/compilers_and_libraries_2018.5.../linux/bin/compilervars.sh -arch intel64 -platform linux
//                                                                                y compilervars_global.sh
//ark intel


//Compilar con icc
//chunk
//icc -qopenmp -xCORE_AVX2(o el 512 probar) archivo.c -o ejecutable