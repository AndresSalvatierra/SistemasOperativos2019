#!/bin/python
import numpy as np
import netCDF4
from netCDF4 import Dataset
import xarray
from scipy import signal
from scipy.ndimage.filters import convolve
from scipy.misc import imread, imshow
import matplotlib.pyplot as plt
import matplotlib.image as mpimg
from matplotlib.colors import LinearSegmentedColormap

'''
Parametros
'''
w = np.array([[-1,-1,-1],[-1,8,-1],[-1,-1,-1]]) #MATRIZ CON LA QUE SE REALIZA LA CONVOLUCION
#print w

dataDIR = "/home/andres/Facultad/SOII/Andres/Practico/SistemasOperativos2019/SO2_TP2/tierra_conv.nc"

'''
Abrir el dataset como una matriz XARRAY y guardar la matriz CMI
'''
DS = Dataset(dataDIR)
f = DS.variables['CMI']
# print f.shape
#print f

h = f[0:21696,0:21696]
# maximo=0
# for i in range(0, 21696)
#     for j in range (0,21696)
#         if(h[i][j]>maximo):
#             maximo=h[i][j]
#g = convolve(h,w)

plt.imshow(h[0:10000,0:10000])#, cmap='RdBu')
#plt.imshow(h[0:10000,0:10000],cmap=plt.cm.get_cmap('hot',2))
#plt.imshow(h, cmap='gray')


# fig = plt.figure(1)
# fig.clf()
# ax = fig.add_subplot(1,1,1)
# img = ax.imshow(h, interpolation='nearest')
# img.set_cmap('hot')'
# fig.colorbar(img)
#print h.shape
#print g[10000:10005,10000:10005]
#img=mpimg.imread("./imagen.png")
#lum_img = img[:,:,0]
#imgplot = plt.imshow(lum_img)

#h= plt.imshow(h)
#plt.hist(h,ravel(), bins=256)
#view_colormap('jet')
#plt.colorbar()
#plt.imshow(lum_img, cmap="hot")
#plt.imshow(lum_img)
plt.savefig('pip7.png',format="png",dpi=1200)
#plt.show()
'''
matrix = np.array([[10,2,2,2,2],[2,10,2,2,2],[2,2,10,2,2],[2,2,2,10,2],[2,2,2,2,10]])
print (matrix)

Convolucion

g = signal.convolve2d(matrix,w,boundary='fill',mode='same')
h = convolve(matrix,w)
y = signal.convolve2d(matrix,w,boundary='symm',mode='valid')
print(g)
print g.shape
print(h)
print h.shape
print(y)
print y.shape

y = g[10000:10200,10000:10200]
print y.shape
plt.imshow(y)
plt.show()'''
