#include <stdlib.h>
#include <stdio.h>
#include "/home/andres/Development/SO2/hpc/libs/netcdf/include/netcdf.h"


// /* Handle errors by printing an error message and exiting with a
//  * non-zero status. */
#define ERRCODE 2
#define ERR(e) {printf("Error: %s\n", nc_strerror(e)); exit(ERRCODE);}

// /* nombre del archivo a leer */
#define FILE_NAME "/home/andres/Facultad/SOII/Andres/Practico/SistemasOperativos2019/SO2_TP2/tierra.nc"

// /* Lectura de una matriz de 21696 x 21696 */
//#define NX 21696
//#define NY 21696


#define NX 5
#define NY 5

/*Matriz W*/
#define WX 3
#define WY 3

void conv(int i,int j,float data_in [NX][NY],float resultante [NX][NY], float mat_w [WX][WY]);

int main()
{
    
    int ncid, varid;
    // float data_prueba[NX][NY]={{10.0,2.0,2.0,2.0,2.0},{2.0,10.0,2.0,2.0,2.0},{2.0,2.0,10.0,2.0,2.0}
    //                         ,{2.0,2.0,2.0,10.0,2.0},{2.0,2.0,2.0,2.0,10.0}}; //matriz
    
    float mat_w[WX][WY]={{-1.0,-1.0,-1.0},{-1.0,8.0,-1.0},{-1.0,-1.0,-1.0}};
    float resultante[NX][NY]={0};
    float data_in[NX][NY]={0};
    int x, y, retval;
    size_t start[2], count[2];

    start[0]=0;
    start[1]=0;
    count[0]=5;
    count[1]=5;

  
    if ((retval = nc_open(FILE_NAME, NC_NOWRITE, &ncid)))
        printf("E\n");
        //ERR(retval);
    
    //printf("D\n");

    /* Obtenemos elvarID de la variable CMI. */
    if ((retval = nc_inq_varid(ncid, "CMI", &varid)))
        printf("R\n");
        //ERR(retval);

    //printf("A\n");

    /* Leemos la matriz. */
    // if ((retval = nc_get_var_float(ncid, varid, &data_in[0][0])))
    //     ERR(retval);
    
    if((retval=nc_get_vara_float(ncid,varid,start, count,&data_in[0][0])))
        printf("R\n");
        //ERR(retval);
    
    //printf("DATA\n");
    for(int i=0; i<NX; i=i+1)
    {
        for(int j=0; j<NY; j=j+1)
        {
           printf("%f ", data_in[i][j]);
        }
        printf("\n");
    } 
    printf("\n");
  

    // for(int i=1; i<NX-1; i=i+1)
    // {
    //     for(int j=1; j<NY-1; j=j+1)
    //     {
    //         conv(i,j,data_in,resultante,mat_w);
    //     }
    // } 

    // printf("RESULTANTE\n");
    // for(int i=0; i<NX; i=i+1)
    // {
    //     for(int j=0; j<NY; j=j+1)
    //     {
    //        printf("%f ", resultante[i][j]);
    //     }
    //     printf("\n");
    // } 
    // printf("\n");

    /* Se cierra el archivo y liberan los recursos*/
    if ((retval = nc_close(ncid)))
        ERR(retval);  

    return 0;
}

void conv(int x, int y, float data_in [NX][NY],float resultante [NX][NY], float mat_w [WX][WY])
{
    for(int i=-1; i<2; i=i+1)
    {
        for(int j=-1; j<2; j=j+1)
        {   
            resultante[x][y]=resultante[x][y] + data_in[x+i][y+j] * mat_w[i+1][j+1];
        }
    }   
}