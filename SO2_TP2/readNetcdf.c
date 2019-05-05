#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include "/home/andres/Development/SO2/hpc/libs/netcdf/include/netcdf.h"
//gcc -o matrix readNetcdf.c `nc-config --cflags --libs`

// /* Handle errors by printing an error message and exiting with a
//  * non-zero status. */
#define ERRCODE 2
#define ERR(e) {printf("Error: %s\n", nc_strerror(e)); exit(ERRCODE);}

// /* nombre del archivo a leer */
#define FOTO "/home/andres/Facultad/SOII/Andres/Practico/SistemasOperativos2019/SO2_TP2/tierra.nc"
#define FOTO_CONV "/home/andres/Facultad/SOII/Andres/Practico/SistemasOperativos2019/SO2_TP2/tierra_conv.nc"
// /* Lectura de una matriz de 21696 x 21696 */

#define NX_T 21696
#define NY_T 21696

#define NX 678
#define NY 678

#define LIMIT NX_T/NX
#define NDIMS 2

/*Matriz W*/
#define WX 3
#define WY 3

int escribir_nc(float data_out[][NY], int punterox, int punteroy);
void conv(int i,int j,float data_in[NX][NY],float **resultante, float mat_w [WX][WY]);

int main()
{
    
    
    // float data_prueba[NX][NY]={{10.0,2.0,2.0,2.0,2.0},{2.0,10.0,2.0,2.0,2.0},{2.0,2.0,10.0,2.0,2.0}
    //                         ,{2.0,2.0,2.0,10.0,2.0},{2.0,2.0,2.0,2.0,10.0}}; //matriz
    
    float mat_w[WX][WY]={{-1.0,-1.0,-1.0},{-1.0,8.0,-1.0},{-1.0,-1.0,-1.0}};
    float data_in[NX][NY];
    int x, y, retval, ncid,varid, nconv,nconvarid,x_dimid, y_dimid;
    size_t start[NDIMS], count[NDIMS];
    int dimids[NDIMS];

    float prueba[NX][NY]={0};

    start[0]=0;
    start[1]=0;
    count[0]=NX;
    count[1]=NY;

    float **resultante;

    resultante= malloc(NX*sizeof(float*));

    for(int i=0;i<NX;i=i+1)
    {
        resultante[i]=malloc(NX*sizeof(float));
    }

    for(int k=0; k<LIMIT;k=k+1)
    {   
        for(int t=0; t<LIMIT; t=t+1)
        {   
            if(k==0)
            {
                start[0]=NX*k;          
            }
            else
            {
                start[0]=(NX*k)-2;
            }

            if(t==0)
            {
                start[1]=NY*t;   
            }

            else
            {
                start[1]=(NY*t)-2;
            }

            // printf("Start0 %li\n",start[0]);
            // printf("Start1 %li\n",start[1]);
            if ((retval = nc_open(FOTO, NC_NOWRITE, &ncid)))
                ERR(retval);
                    
            /* Obtenemos elvarID de la variable CMI. */
            if ((retval = nc_inq_varid(ncid, "CMI", &varid)))
                ERR(retval);
            
            if((retval=nc_get_vara_float(ncid,varid,start, count,&data_in[0][0])))
                ERR(retval);

            if ((retval = nc_close(ncid)))
                ERR(retval);

            nc_close(ncid);

            // printf("Matriz inicial: \n ");
            // for (int i = 0; i < NX; i++)
            // {
            //     for (int j = 0; j < NY; j++)
            //     {
            //         printf("%lf ", data_in[i][j]);
            //     }
            //     printf("\n");
            // }
            
               //INICIALIZO RESULTANTE
            for(int o=0; o<NX; o=o+1)
            {
                for(int l=0; l<NY; l=l+1)
                {
                    resultante[o][l]=0;
                }
            } 
            for(int i=1; i<NX-1; i=i+1)
            {
                for(int j=1; j<NY-1; j=j+1)
                {
                    conv(i,j,data_in,resultante,mat_w);
                }
            }
            
            // printf("Matriz Resultante\n");
            for(int i=0; i<NX; i=i+1)
            {
                for(int j=0; j<NY; j=j+1)
                {
                    prueba[i][j]=resultante[i][j]*(float) 0.00031746001;
                }
            }   

            //escribir_nc(prueba,k,t);
            if((t==0) && (k==0))
            {   
                // printf("entre \n");
                if ((retval = nc_create(FOTO_CONV, NC_CLOBBER, &nconv)))
                    ERR(retval);
                if ((retval = nc_def_dim(nconv, "x", NX_T, &x_dimid)))
                    ERR(retval);
                if ((retval = nc_def_dim(nconv, "y", NY_T, &y_dimid)))
                    ERR(retval);
                
                dimids[0] = x_dimid;
                dimids[1] = y_dimid;

                if ((retval = nc_def_var(nconv, "CMI", NC_FLOAT, NDIMS, dimids, &nconvarid)))
                    ERR(retval);

                if ((retval = nc_enddef(nconv)))
                    ERR(retval);
            }

            else
            {
                if ((retval = nc_open(FOTO_CONV, NC_WRITE | NC_SHARE, &nconv)))
                    ERR(retval);
                if ((retval = nc_inq_varid(nconv, "CMI", &nconvarid)))
                    ERR(retval);
            }
            if((retval=nc_put_vara_float(nconv,nconvarid,start, count,&prueba[0][0])))
                ERR(retval);
           
            if ((retval = nc_close(nconv)))
                ERR(retval); 
        }
    }
    for(int i=0; i<NX; i=i+1)
    {
        free(resultante[i]);
    } 
    free(resultante);
    return 0;
}

void conv(int x, int y, float data_in[NX][NY],float **resultante, float mat_w [WX][WY])
{   

    for(int i=-1; i<2; i=i+1)
    {
        for(int j=-1; j<2; j=j+1)
        {   
            if(data_in[x+i][y+j]==-1.0)
            {
                resultante[x][y]=resultante[x][y] + (float)(0.0/0.0)* mat_w[i+1][j+1];
            }      
            
            else
            {
                resultante[x][y]=resultante[x][y] + data_in[x+i][y+j] * mat_w[i+1][j+1];
            }
        }
    }
  
}

// int escribir_nc(float data_out[][NY], int punterox, int punteroy)
// {

//     /* When we create netCDF variables and dimensions, we get back an
//     * ID for each one. */
//     int ncid, x_dimid, y_dimid, varid;
//     int dimids[NDIMS];
//     size_t start[2] = {0};
//     start[0] = punterox * NX;
//     start[1] = punteroy * NY;
//     size_t conteo[2] = {0};
//     conteo[0] = NX;
//     conteo[1] = NX;

//     /* Loop indexes, and error handling. */
//     int x, y, retval;

//     /* Create the file. The NC_CLOBBER parameter tells netCDF to
//     * overwrite this file, if it already exists.*/
//     if ((punteroy == 0) && (punterox == 0))
//     {
//         if ((retval = nc_create("./MEODIO.nc", NC_CLOBBER, &ncid)))
//             ERR(retval);
//         /* Define the dimensions. NetCDF will hand back an ID for each. */
//         if ((retval = nc_def_dim(ncid, "x", NX_T, &x_dimid)))
//             ERR(retval);
//         if ((retval = nc_def_dim(ncid, "y", NY_T, &y_dimid)))
//             ERR(retval);

//         /* The dimids array is used to pass the IDs of the dimensions of
//         * the variable. */
//         dimids[0] = x_dimid;
//         dimids[1] = y_dimid;

//         /* Define the variable. The type of the variable in this case is
//         * NC_FLOAT (4-byte integer). */
//         if ((retval = nc_def_var(ncid, "CMI", NC_FLOAT, NDIMS,
//                                  dimids, &varid)))
//             ERR(retval);

//         /* End define mode. This tells netCDF we are done defining
//         * metadata. */
//         if ((retval = nc_enddef(ncid)))
//             ERR(retval);
//     }
//     else
//     {
//         if ((retval = nc_open("./MEODIO.nc", NC_WRITE | NC_SHARE, &ncid)))
//             ERR(retval);
//         if ((retval = nc_inq_varid(ncid, "CMI", &varid)))
//             ERR(retval);
//     }

//     /* Write the pretend data to the file. Although netCDF supports
//     * reading and writing subsets of data, in this case we write all
//     * the data in one operation. */
//     if ((retval = nc_put_vara_float(ncid, varid, start, conteo, &data_out[0][0])))
//         ERR(retval);

//     /* Close the file. This frees up any internal netCDF resources
//     * associated with the file, and flushes any buffers. */
//     if ((retval = nc_close(ncid)))
//         ERR(retval);

//     return 0;
// }