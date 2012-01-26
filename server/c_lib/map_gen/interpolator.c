#include "interpolator.h"

// For lowering sample resolution and interpolating in between


/*
 * Interpolation methods borrowed from
 *
 * http://www.paulinternet.nl/?page=bicubic

 * see also:
 * http://paulbourke.net/miscellaneous/interpolation/index.html
 *
 */
inline float cubicInterpolate (float p[4], float x) {
    return p[1] + 0.5 * x*(p[2] - p[0] + x*(2.0*p[0] - 5.0*p[1] + 4.0*p[2] - p[3] + x*(3.0*(p[1] - p[2]) + p[3] - p[0])));  /* Catmull-Rom splines */
}

inline float cubicInterpolate_standard (float p[4], float x) {
    return p[1] + x*(p[2] - p[0] + x*(2.0*(p[0] - p[1]) + p[2] - p[3] + x*(p[1] - p[2] + p[3] - p[0]))); /* cubic (standard) */
}

inline float bicubicInterpolate (float p[4][4], float x, float y) {
    float arr[4];
    arr[0] = cubicInterpolate(p[0], y);
    arr[1] = cubicInterpolate(p[1], y);
    arr[2] = cubicInterpolate(p[2], y);
    arr[3] = cubicInterpolate(p[3], y);
    return cubicInterpolate(arr, x);
}

float tricubicInterpolate (float p[4][4][4], float x, float y, float z) {
    float arr[4];
    arr[0] = bicubicInterpolate(p[0], y, z);
    arr[1] = bicubicInterpolate(p[1], y, z);
    arr[2] = bicubicInterpolate(p[2], y, z);
    arr[3] = bicubicInterpolate(p[3], y, z);
    return cubicInterpolate(arr, x);
}


inline float linearInterpolate (float p[2], float x) {
    return x * (p[1] - p[0]) + p[0];
}

inline float bilinearInterpolate (float p[2][2], float x, float y) {
    float arr[2];
    arr[0] = linearInterpolate(p[0], y);
    arr[1] = linearInterpolate(p[1], y);
    return linearInterpolate(arr, x);
}

float trilinearInterpolate (float p[2][2][2], float x, float y, float z) {
    float arr[2];
    arr[0] = bilinearInterpolate(p[0], y, z);
    arr[1] = bilinearInterpolate(p[1], y, z);
    return linearInterpolate(arr, x);
}

inline float trilinearInterpolate_condensed(float p[2][2][2], float x, float y, float z) {
    return p[0][0][0] * (1 - x) * (1 - y) * (1 - z) +
            p[1][0][0] * (1 - y) * (1 - z) * x + 
            p[0][1][0] * (1 - x) * (1 - z) * y + 
            p[0][0][1] * (1 - x) * (1 - y) * z +
            p[1][0][1] * (1 - y) * x * z + 
            p[0][1][1] * (1 - x) * y * z + 
            p[1][1][0] * (1 - z) * x * y + 
            p[1][1][1] * x * y * z;
}

const int interp_margin = 2;

/* 1 dimensional */

// core interpolator
void _interp1(float final[], float points[], int x, int x_interval) {

    int nx = (x / x_interval) + (interp_margin * 2);

    //fractional interval btwn anchor pts
    float fx_interval = (float)x_interval;

    //interpolate
    int mx, px, sx;
    int i,ii;
    float samples[4];
    float pt;

    for (i=0; i<x; i++) {
        mx = i % x_interval;
        px = mx / fx_interval;
        sx = (i / x_interval) + interp_margin;

        // collect local samples
        for (ii=-1; ii<3; ii++) {
            samples[ii+1] = points[ii+sx];
        }

        pt = cubicInterpolate(samples, px);
        final[i] = pt;
    }

    // See _interp3 for why the following is disabled

    ////// recalculate the anchor points from the interpolated points
    //// use linear interpolation for this
    //float resamples[2];
    //int ix, iix;
    //int index;

    //for (i=interp_margin; i < nx-interp_margin; i++) {
        //ix = (i-interp_margin) * x_interval;
        //if (!ix) continue;
        //for (ii=0; ii < 2; ii++) {
            //iix = (ii == 0) ? -1 : ii;
            //index = ix + iix;
            //resamples[ii] = final[index];
        //}

        //pt = linearInterpolate(resamples, 0.5f); // midpoints
        //points[i] = pt;
    //}

    // merge
    for (i=interp_margin; i < nx-interp_margin; i++) {
        ii = (i-interp_margin) * x_interval;
        final[ii] = points[i];
    }
}

/* 1D wrappers */

void _perlin_interp1(float final[], int x, int x_interval,

                                      int rep, int base) {

    int nx = (x / x_interval) + (interp_margin*2);

    float fnx = (float)(nx + interp_margin);
    
    float *points = (float*)malloc(sizeof(float)*nx);
    
    // generate anchor points
    int i;
    for (i=0; i<nx; i++) {
        points[i] = perlin1(((float)(i+1)/fnx)*xnoise_scale, rep, base);   
    }

    _interp1(final, points, x, x_interval);
    free(points);
}
                                      
void _rmf_perlin_interp1(float final[], int x, int x_interval,

                                      int rep, int base) {

    int nx = (x / x_interval) + (interp_margin*2);

    float fnx = (float)(nx + interp_margin);
    
    float *points = (float*)malloc(sizeof(float)*nx);
    
    // generate anchor points
    int i;
    for (i=0; i<nx; i++) {
        points[i] = rmf_perlin1(((float)(i+1)/fnx)*xnoise_scale, rep, base);   
    }

    _interp1(final, points, x, x_interval);
    free(points);
}


/* 2 dimensional */

// base interpolator
void _interp2(float final[], float points[], int x, int y, int x_interval, int y_interval) {

    int nx = (x / x_interval) + (interp_margin*2),
        ny = (y / y_interval) + (interp_margin*2);

    // fractional interval between anchor points
    float fx_interval = (float)x_interval,
           fy_interval = (float)y_interval;

    // interpolate
    int mx,my;
    int px,py;
    int sx,sy;
    int i,j;
    int ii,jj;
    float samples[4][4];
    float pt;
    
    for (i=0; i<x; i++) {
        mx = i % x_interval;
        px = mx / fx_interval;
        sx = (i / x_interval) + interp_margin;

        for (j=0; j<y; j++) {
            my = j % y_interval;
            py = my / fy_interval;
            sy = (j / y_interval) + interp_margin;

            // collect local samples
            for (ii=-1; ii < 3; ii++) {
                for (jj=-1; jj < 3; jj++) {
                    samples[ii+1][jj+1] = points[(ii+sx) + nx*(jj+sy)];
                }
            }

            pt = bicubicInterpolate(samples, px, py);
            final[i + x*j] = pt;
        }
    }

    // See _interp3 for why the following is disabled

    //// recalculate the anchor points from the interpolated points
    // use linear interpolation for this
    //float resamples[2][2];
    //int ix,iy;
    //int index;
    //int iix,jjy;
    //for (i=interp_margin; i < nx-interp_margin; i++) {
        //ix = (i-interp_margin) * x_interval;
        //if (!ix) continue;          // ix=0 is the same as final's x=0. It cannot interpolate because interpolated values are not calculated for n<0. This applies to x and y.
                                      //// one solution is to generate the interpolate box one level extra backward in each dimension. or, calculate one level forward, but shift the anchor point ix=0 back 1.

        //for (j=interp_margin; j < ny-interp_margin; j++) {
            //iy = (j-interp_margin) * y_interval;
            //if (!iy) continue;

            //// collect samples from final array's interpolated values.
            //for (ii=0; ii < 2; ii++) {
                //iix = (ii == 0) ? -1 : ii;
                //for (jj=0; jj < 2; jj++) {
                    //jjy = (jj == 0) ? -1 : jj;
                    //index = ix + iix + x*(iy + jjy);
                    //resamples[ii][jj] = final[index];
                //}
            //}

            //pt = bilinearInterpolate(resamples, 0.5f, 0.5f);
            //points[i + nx*j] = pt;
        //}
    //}
    
    // merge (recalculated, linearly interpolated) anchor points with (cubic) interpolated
    for (i=interp_margin; i < nx-interp_margin; i++) {
        ii = (i-interp_margin) * x_interval;
        for (j=interp_margin; j < ny-interp_margin; j++) {
            jj = (j-interp_margin) * y_interval;

            final[ii + x*jj] = points[i + nx*j];
        }
    }

}

/* 2D Wrappers */

void _perlin_interp2(float final[], int x, int y,
                                      int x_interval, int y_interval,

                                      int rep_x, int rep_y,
                                      int base) {

    int nx = (x / x_interval) + (interp_margin*2),
        ny = (y / y_interval) + (interp_margin*2);

    float fnx = (float)(nx + interp_margin),
           fny = (float)(ny + interp_margin);

    float *points = (float*)malloc(sizeof(float)*nx*ny);
        
    // generate anchor points
    int i,j;
    for (i=0; i<nx; i++) {
        for (j=0; j<ny; j++) {
            points[i + nx*j] = perlin2(((float)(i+1)/fnx)*xnoise_scale, ((float)(j+1)/fny)*ynoise_scale, rep_x, rep_y, base);   
        }
    }

    _interp2(final, points, x, y, x_interval, y_interval);
    free(points);
}

void _rmf_perlin_interp2(float final[], int x, int y,
                                      int x_interval, int y_interval,
                                      int rep_x, int rep_y,
                                      int base) {

    int nx = (x / x_interval) + (interp_margin*2),
        ny = (y / y_interval) + (interp_margin*2);

    float fnx = (float)(nx + interp_margin),
           fny = (float)(ny + interp_margin);

    float *points = (float*)malloc(sizeof(float)*nx*ny);

    // generate anchor points
    int i,j;
    for (i=0; i<nx; i++) {
        for (j=0; j<ny; j++) {
            points[i + nx*j] = rmf_perlin2(((float)(i+1)/fnx)*xnoise_scale, ((float)(j+1)/fny)*ynoise_scale, rep_x, rep_y, base);   
        }
    }
    _interp2(final, points, x, y, x_interval, y_interval);
    free(points);
}

void _simplex_interp2(float final[], int x, int y,
                                      int x_interval, int y_interval) {

    int nx = (x / x_interval) + (interp_margin*2),
        ny = (y / y_interval) + (interp_margin*2);

    float fnx = (float)(nx + interp_margin),
           fny = (float)(ny + interp_margin);

    float *points = (float*)malloc(sizeof(float)*nx*ny);

    // generate anchor points
    int i,j;
    for (i=0; i<nx; i++) {
        for (j=0; j<ny; j++) {
            points[i + nx*j] = simplex2(((float)(i+1)/fnx)*xnoise_scale, ((float)(j+1)/fny)*ynoise_scale);   
        }
    }
    _interp2(final, points, x, y, x_interval, y_interval);
    free(points);
}

void _rmf_simplex_interp2(float final[], int x, int y,
                                      int x_interval, int y_interval) {

    int nx = (x / x_interval) + (interp_margin*2),
        ny = (y / y_interval) + (interp_margin*2);

    float fnx = (float)(nx + interp_margin),
           fny = (float)(ny + interp_margin);

    float *points = (float*)malloc(sizeof(float)*nx*ny);

    // generate anchor points
    int i,j;
    for (i=0; i<nx; i++) {
        for (j=0; j<ny; j++) {
            points[i + nx*j] = rmf_simplex2(((float)(i+1)/fnx)*xnoise_scale, ((float)(j+1)/fny)*ynoise_scale);   
        }
    }
    _interp2(final, points, x, y, x_interval, y_interval);
    free(points);
}



/* 3 dimensional */

// base interpolator
void _interp3(float final[], float points[],
                              int x, int y, int z,
                              int x_interval, int y_interval, int z_interval) {

    int nx = (x / x_interval) + (interp_margin*2),
        ny = (y / y_interval) + (interp_margin*2),
        nz = (z / z_interval) + (interp_margin*2);

    // fractional interval between sample points
    float fx_interval = (float)x_interval,
           fy_interval = (float)y_interval,
           fz_interval = (float)z_interval;

    // interpolate
    int mx,my,mz;
    float px,py,pz;

    int sx,sy,sz;
    int i,j,k;
    int ii,jj,kk;
    float samples[4][4][4];
    float pt;
    
    for (i=0; i<x; i++) {
        mx = i % x_interval;
        px = mx / fx_interval;
        sx = (i / x_interval) + interp_margin; // sample point index in anchor pts array
        
        for (j=0; j<y; j++) {
            my = j % y_interval;
            py = my / fy_interval;
            sy = (j / y_interval) + interp_margin;

            for (k=0; k<z; k++) {
                mz = k % z_interval;
                pz = mz / fz_interval;
                sz = (k / z_interval) + interp_margin;
    
                // collect local samples
                for (ii=-1; ii < 3; ii++) {
                    for (jj=-1; jj < 3; jj++) {
                        for (kk=-1; kk < 3; kk++) {
                            samples[ii+1][jj+1][kk+1] = points[(ii+sx) + nx*(jj+sy) + nx*ny*(kk+sz)];  // correct this. increased interpolation interp_margins from 2 to 4, so adjust proper code elsewhere
                        }
                    }
                }

                // inefficiency here: calculating interpolate values for anchor points (just returns the anchor point)
                // also recalculating the spline for each interpolate value, which is redundant for interval>2
                pt = tricubicInterpolate(samples, px, py, pz);
                final[i + x*j + x*y*k] = pt;
            }
        }
    }

    // I was doing this because of artifacts on the underside of overhangs
    // Artifacts still were possible, but only  on 3 sides of the cube
    // Those artifacts became a problem, and in the process of debugging I disable the following code
    // All artifacts disappeared.

    //// recalculate the anchor points from the interpolated points
    // use linear interpolation for this
    //float resamples[2][2][2];
    //int ix,iy,iz;
    //int index;
    //int iix,jjy,kkz;
    //for (i=interp_margin; i < nx-interp_margin; i++) {
        //ix = (i-interp_margin) * x_interval;
        //if (!ix) continue;          // ix=0 is the same as final's x=0. It cannot interpolate because interpolated values are not calculated for n<0. This applies to x and y.
                                      //// one solution is to generate the interpolate box one level extra backward in each dimension. or, calculate one level forward, but shift the anchor point ix=0 back 1.

        //for (j=interp_margin; j < ny-interp_margin; j++) {
            //iy = (j-interp_margin) * y_interval;
            //if (!iy) continue;

            //for (k=interp_margin; k < nz-interp_margin; k++) {
                //iz = (k-interp_margin) * z_interval;
                //if (!iz) continue;

                //// collect samples from final array's interpolated values.
                //for (ii=0; ii < 2; ii++) {
                    //iix = (ii == 0) ? -1 : ii;
                    //for (jj=0; jj < 2; jj++) {
                        //jjy = (jj == 0) ? -1 : jj;
                        //for (kk=0; kk < 2; kk++) {
                            //kkz = (kk == 0) ? -1 : kk;
                            //index = ix + iix + x*(iy + jjy) + x*y*(iz + kkz);
                            //resamples[ii][jj][kk] = final[index];
                        //}
                    //}
                //}

                //pt = trilinearInterpolate(resamples, 0.5f, 0.5f, 0.5f);
                //points[i + nx*j + nx*ny*k] = pt;
            //}
        //}
    //}
    
    // merge (recalculated, linearly interpolated) anchor points with (cubic) interpolated
    for (i=interp_margin; i < nx-interp_margin; i++) {
        ii = (i-interp_margin) * x_interval;

        for (j=interp_margin; j < ny-interp_margin; j++) {
            jj = (j-interp_margin) * y_interval;

            for (k=interp_margin; k < nz-interp_margin; k++) {
                kk = (k-interp_margin) * z_interval;

                final[ii + x*jj + x*y*kk] = points[i + nx*j + nx*ny*k];
            }
        }
    }

}

/* 3d Wrappers */

void _perlin_interp3(float final[], int x, int y, int z,
                                      int x_interval, int y_interval, int z_interval,
                                      int rep_x, int rep_y, int rep_z,
                                      int base) {

    int nx = (x / x_interval) + (interp_margin*2),
        ny = (y / y_interval) + (interp_margin*2),
        nz = (z / z_interval) + (interp_margin*2);

    float fnx = (float)(nx + interp_margin),
           fny = (float)(ny + interp_margin),
           fnz = (float)(nz + interp_margin);

    float *points = (float*)malloc(sizeof(float)*nx*ny*nz);
        
    // generate anchor points
    int i,j,k;
    for (i=0; i<nx; i++) {
        for (j=0; j<ny; j++) {
            for (k=0; k<nz; k++) {
                points[i + nx*j + nx*ny*k] = perlin3(((float)(i+1)/fnx)*xnoise_scale, ((float)(j+1)/fny)*ynoise_scale, ((float)(k+1)/fnz)*znoise_scale, rep_x, rep_y, rep_z, base);   
            }
        }
    }
    _interp3(final, points, x, y, z, x_interval, y_interval, z_interval);
    free(points);
}

void _rmf_perlin_interp3(float final[], int x, int y, int z,
                                      int x_interval, int y_interval, int z_interval,
                                      int rep_x, int rep_y, int rep_z,
                                      int base) {

    int nx = (x / x_interval) + (interp_margin*2),
        ny = (y / y_interval) + (interp_margin*2),
        nz = (z / z_interval) + (interp_margin*2);

    float fnx = (float)(nx + interp_margin),
           fny = (float)(ny + interp_margin),
           fnz = (float)(nz + interp_margin);

    float *points = (float*)malloc(sizeof(float)*nx*ny*nz);
        
    // generate anchor points
    int i,j,k;
    for (i=0; i<nx; i++) {
        for (j=0; j<ny; j++) {
            for (k=0; k<nz; k++) {
                points[i + nx*j + nx*ny*k] = rmf_perlin3(((float)(i+1)/fnx)*xnoise_scale, ((float)(j+1)/fny)*ynoise_scale, ((float)(k+1)/fnz)*znoise_scale, rep_x, rep_y, rep_z, base);
            }
        }
    }
    _interp3(final, points, x, y, z, x_interval, y_interval, z_interval);
    free(points);
}

void _simplex_interp3(float final[], int x, int y, int z,
                                      int x_interval, int y_interval, int z_interval) {

    int nx = (x / x_interval) + (interp_margin*2),
        ny = (y / y_interval) + (interp_margin*2),
        nz = (z / z_interval) + (interp_margin*2);

    float fnx = (float)(nx + interp_margin),
           fny = (float)(ny + interp_margin),
           fnz = (float)(nz + interp_margin);

    float *points = (float*)malloc(sizeof(float)*nx*ny*nz);
        
    // generate anchor points
    int i,j,k;
    for (i=0; i<nx; i++) {
        for (j=0; j<ny; j++) {
            for (k=0; k<nz; k++) {
                points[i + nx*j + nx*ny*k] = simplex3(((float)(i+1)/fnx)*xnoise_scale, ((float)(j+1)/fny)*ynoise_scale, ((float)(k+1)/fnz)*znoise_scale);   
            }
        }
    }
    _interp3(final, points, x, y, z, x_interval, y_interval, z_interval);
    free(points);
}

void _rmf_simplex_interp3(float final[], int x, int y, int z,
                            int x_interval, int y_interval, int z_interval) {

    int nx = (x / x_interval) + (interp_margin*2),
        ny = (y / y_interval) + (interp_margin*2),
        nz = (z / z_interval) + (interp_margin*2);

    float fnx = (float)(nx + interp_margin),
           fny = (float)(ny + interp_margin),
           fnz = (float)(nz + interp_margin);

    float *points = (float*)malloc(sizeof(float)*nx*ny*nz);
        
    // generate anchor points
    int i,j,k;
    for (i=0; i<nx; i++) {
        for (j=0; j<ny; j++) {
            for (k=0; k<nz; k++) {
                points[i + nx*j + nx*ny*k] = rmf_simplex3(((float)(i+1)/fnx)*xnoise_scale, ((float)(j+1)/fny)*ynoise_scale, ((float)(k+1)/fnz)*znoise_scale);   
            }
        }
    }
    _interp3(final, points, x, y, z, x_interval, y_interval, z_interval);
    free(points);
}


/* Public API */

/* 1D */
void apply_interp1_perlin(int x, int x_interval,
                            int repeat, int base) {
    if (x > XMAX || x < 0) {
        printf("interpolation error: dimensions out of map range\n");
        return;
    }
    _perlin_interp1(noisemap, x, x_interval, repeat, base);
}

void apply_interp1_rmf_perlin(int x, int x_interval,
                            int repeat, int base) {
    if (x > XMAX || x < 0) {
        printf("interpolation error: dimensions out of map range\n");
        return;
    }
    _rmf_perlin_interp1(noisemap, x, x_interval, repeat, base);
}

/* 2d */
void apply_interp2_perlin(int x, int y,
                           int x_interval, int y_interval,
                           int rep_x, int rep_y,
                           int base) {
    if (x > XMAX || y > YMAX || x < 0 || y < 0) {
        printf("interpolation error: dimensions out of map range\n");
        return;
    }
    _perlin_interp2(noisemap, x,y, x_interval, y_interval, rep_x, rep_y, base);
}

void apply_interp2_rmf_perlin(int x, int y, int x_interval, int y_interval,
                            int rep_x, int rep_y,
                            int base) {
    if (x > XMAX || y > YMAX || x < 0 || y < 0) {
        printf("interpolation error: dimensions out of map range\n");
        return;
    }
    _rmf_perlin_interp2(noisemap, x,y, x_interval, y_interval, rep_x, rep_y, base);
}

void apply_interp2_simplex(int x, int y, int x_interval, int y_interval) {
    if (x > XMAX || y > YMAX || x < 0 || y < 0) {
        printf("interpolation error: dimensions out of map range\n");
        return;
    }
    _simplex_interp2(noisemap, x,y, x_interval, y_interval);
}

void apply_interp2_rmf_simplex(int x, int y, int x_interval, int y_interval) {
    if (x > XMAX || y > YMAX || x < 0 || y < 0) {
        printf("interpolation error: dimensions out of map range\n");
        return;
    }
    _rmf_simplex_interp2(noisemap, x,y, x_interval, y_interval);
}

/* 3d */

void apply_interp3_perlin(int x, int y, int z,
                            int x_interval, int y_interval, int z_interval,
                            int rep_x, int rep_y, int rep_z,
                            int base) {
    if (x > XMAX || y > YMAX || z > ZMAX || x < 0 || y < 0 || z < 0) {
        printf("interpolation error: dimensions out of map range\n");
        return;
    }

    _perlin_interp3(noisemap, x,y,z, x_interval, y_interval, z_interval, rep_x, rep_y, rep_z, base);
}

void apply_interp3_rmf_perlin(int x, int y, int z,
                            int x_interval, int y_interval, int z_interval,
                            int rep_x, int rep_y, int rep_z,
                            int base) {
    if (x > XMAX || y > YMAX || z > ZMAX || x < 0 || y < 0 || z < 0) {
        printf("interpolation error: dimensions out of map range\n");
        return;
    }

    _rmf_perlin_interp3(noisemap, x,y,z, x_interval, y_interval, z_interval, rep_x, rep_y, rep_z, base);
}

void apply_interp3_simplex(int x, int y, int z,
                            int x_interval, int y_interval, int z_interval) {
    if (x > XMAX || y > YMAX || z > ZMAX || x < 0 || y < 0 || z < 0) {
        printf("interpolation error: dimensions out of map range\n");
        return;
    }

    _simplex_interp3(noisemap, x,y,z, x_interval, y_interval, z_interval);
}

void apply_interp3_rmf_simplex(int x, int y, int z,
                            int x_interval, int y_interval, int z_interval) {
                                
    if (x > XMAX || y > YMAX || z > ZMAX || x < 0 || y < 0 || z < 0) {
        printf("interpolation error: dimensions out of map range\n");
        return;
    }

    _rmf_simplex_interp3(noisemap, x,y,z, x_interval, y_interval, z_interval);
}
