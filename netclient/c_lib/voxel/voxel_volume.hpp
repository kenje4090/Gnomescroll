#pragma once 

#include <voxel/common.h>
#include <physics/vector.hpp>

//z vector rotated in x,y plane by theta degrees
//theta from 0 to 1
static inline struct Vector unit_z_rot(float theta) __attribute((always_inline));
struct Vector unit_z_rot(float theta)
{   
    Vector u;
    u.x = 0.0;
    u.y = sin(theta * 2*PI);
    u.z = cos(theta * 2*PI);
    return u;
}

/*
optimization: compute matix and return matrix
*/
struct Vector euler_rotation(Vector v, float theta) __attribute((always_inline));
struct Vector euler_rotation(Vector v, float x, float y, float z)
{   
    x *= 2*PI;
    y *= 2*PI;
    z *= 2*PI;

    double cx = cos(x);
    double sx = sin(x);
    double cy = cos(y);
    double sy = sin(y);
    double cz = cos(z);
    double sz = sin(z);
    
    Vector m[3];
    Vector u;

    m[0].x = (cy*cz); 
    m[0].y = (cy*sz);
    m[0].z = (-sy);

    double sxsy = sx*sy;
    double cxsy = cx*sy;
    
    m[1].x = (sxsy*cz-cx*sz);
    m[1].y = (sxsy*sz+cx*cz);
    m[1].z = (sx*cy);

    m[2].x = (cxsy*cz+sx*sz);
    m[2].y = (cxsy*sz-sx*cz);
    m[2].z = (cx*cy);


    u.x = v.x*m[0].x + v.y*m[1].x + v.z*m[2].x, 
    u.y = v.x*m[0].y + v.y*m[1].y + v.z*m[2].y, 
    u.z = v.x*m[0].z + v.y*m[1].z + v.z*m[2].z;

    return u;
}

#ifdef DC_CLIENT
    class Voxel_render_list; //forward declaration
#endif

#ifdef DC_CLIENT

    struct Voxel_vertex
    {
        unsigned char x,y,z,t;
        unsigned char rgba[4]; //8
        //char normal[4]; //16
        //can compute normals from t
    };

    class Voxel_vertex_list
    {
        public:
        Voxel_vertex* vertex_list;   //number of vertices
        int size;   //offset of vertices

        unsigned short vnum;   //number of vertices
        unsigned short voff;   //offset of vertices

        Voxel_vertex_list()
        {
            vertex_list = NULL;
            size = 0;
            vnum = 0;
            voff = 0;
        }
    };
#endif

class Voxel_volume
{
    public:

#ifdef DC_CLIENT
    Voxel_vertex_list vvl;
#endif

    Vector v[4]; // forward, up, right (x,y,z), offset
    float scale;    //size of voxels
    //bounding sphere
    Vector center;
    float radius2;

    int xdim,ydim,zdim;
    int _xdim,_ydim,_zdim;
    Voxel* voxel;
    int index1, index12;

    float hdx,hdy,hdz;  //half of width, height, depth as floats

    bool needs_vbo_update;

#ifdef DC_CLIENT
    Voxel_render_list* voxel_render_list;
    void register_with_renderer(Voxel_render_list* vrl);
    void update_vertex_list();
#endif

    void set_unit_axis();

    //forward and up vector
    void set_axis(Vector* f, Vector* u);

    void set_rotated_unit_axis(float x_angle, float y_angle, float z_angle);

    void update_center();
    void set_center(float x, float y, float z);

    void set(int x, int y, int z, unsigned char r, unsigned char g, unsigned char b, unsigned char a);
    void set(int x, int y, int z, Voxel* v);

    inline Voxel* get(int x, int y, int z) __attribute((always_inline)) 
    {
        return &voxel[x+(y << index1)+(z << index1)];
    }


    inline int get_as_int(int x, int y, int z) __attribute((always_inline)) 
    {
        return *((int*)(&voxel[x+(y << index1)+(z << index1)].r));
    }

    //internal methods
    inline void _set(int x, int y, int z, Voxel* v) __attribute((always_inline))
    {
        voxel[x+(y << index1)+(z << index1)] = *v;
    }

    inline void _set(int x, int y, int z, unsigned char r, unsigned char g, unsigned char b, unsigned char a) __attribute((always_inline))
    {
        Voxel* v = &voxel[x+(y << index1)+(z << index1)];
        v->r = r;v->g = g;v->b = b;v->a = a;
    }

    Voxel_volume(int __xdim, int __ydim, int __zdim, float _scale);

    ~Voxel_volume();

    void draw_bounding_box();

};

#ifdef DC_CLIENT

/*
    struct Voxel_vertex
    {
        unsigned char x,y,z,t;
        unsigned char rgba[4]; //8
    };
*/

/*
inline uint32_t PACK(uint8_t c0, uint8_t c1, uint8_t c2, uint8_t c3) {
    return (c0 << 24) | (c1 << 16) | (c2 << 8) | c3;
}
*/

#include <stdio.h>

void Voxel_volume::update_vertex_list()
{   
    static Voxel_vertex* scratch = new Voxel_vertex[65536]; //64 k of memory
    int index = 0;

    for(int x=0; x < xdim; x++){
    for(int y=0; y < ydim; y++){
    for(int z=0; z < zdim; z++){
        if( get_as_int(x,y,z) == 0) continue;

        if(x == 0 || get_as_int(x-1,y,z) != 0)
        {
            scratch[index].x = x;
            scratch[index].y = y;
            scratch[index].z = z;
            scratch[index].t = 0;   //size zero

            //*(int*)&scratch[index].x = (x << 24) | (y << 16) | (z << 8) | t;
            *(int*)scratch[index].rgba = get_as_int(x,y,z);
            index++;
        }
 
        if(x+1 == xdim || get_as_int(x+1,y,z) != 0)
        {
            scratch[index].x = x;
            scratch[index].y = y;
            scratch[index].z = z;
            scratch[index].t = 1;   //size zero

            //*(int*)&scratch[index].x = (x << 24) | (y << 16) | (z << 8) | t;
            *(int*)scratch[index].rgba = get_as_int(x,y,z);
            index++;
        }

        if(y == 0 || get_as_int(x,y-1,z) != 0)
        {
            scratch[index].x = x;
            scratch[index].y = y;
            scratch[index].z = z;
            scratch[index].t = 0;   //size zero

            //*(int*)&scratch[index].x = (x << 24) | (y << 16) | (z << 8) | t;
            *(int*)scratch[index].rgba = get_as_int(x,y,z);
            index++;
        }
 
        if(y+1 ==ydim || get_as_int(x,y+1,z) != 0)
        {
            scratch[index].x = x;
            scratch[index].y = y;
            scratch[index].z = z;
            scratch[index].t = 1;   //size zero

            //*(int*)&scratch[index].x = (x << 24) | (y << 16) | (z << 8) | t;
            *(int*)scratch[index].rgba = get_as_int(x,y,z);
            index++;
        }


        if(z == 0 || get_as_int(x,y,z-1) != 0)
        {
            scratch[index].x = x;
            scratch[index].y = y;
            scratch[index].z = z;
            scratch[index].t = 0;   //size zero

            //*(int*)&scratch[index].x = (x << 24) | (y << 16) | (z << 8) | t;
            *(int*)scratch[index].rgba = get_as_int(x,y,z);
            index++;
        }
 
        if(z+1 == zdim || get_as_int(x,y,z+1) != 0)
        {
            scratch[index].x = x;
            scratch[index].y = y;
            scratch[index].z = z;
            scratch[index].t = 1;   //size zero

            //*(int*)&scratch[index].x = (x << 24) | (y << 16) | (z << 8) | t;
            *(int*)scratch[index].rgba = get_as_int(x,y,z);
            index++;
        }

    }}}

    if(index == 0)
    {
        printf("Warning: generate vertex voxel list, 0 voxels\n");

    }
    if(vvl.vertex_list != NULL) delete vvl.vertex_list;
    vvl.vertex_list = new Voxel_vertex[index];
    //void * memcpy ( void * destination, const void * source, size_t num );
    memcpy(vvl.vertex_list, scratch, index*sizeof(Voxel_vertex));
    vvl.size = index;
}

#endif
void voxel_test()
{
    //static Voxel_volume vv(1,1,1, 1.0);
    static Voxel_volume vv(4,4,4, 1.0);
    //Voxel* v = vv.get(4,5,6);

    //static float c = 0.0;
    //c -= 0.02;
    
    //vv.set_center( -5.0, -5.0, 10.0);


    static float c0 = 0.0;
    static float c1 = 0.0;
    static float c2 = 0.0;
    c0 += 0.0050 / (2*PI);
    c1 += 0.0025 / (2*PI);
    c2 += 0.0100 / (2*PI);

    //printf("start\n");
    //printf("1 v[3] x,y,z= %f, %f, %f \n", vv.v[3].x, vv.v[3].y, vv.v[3].z);
    
    vv.set_rotated_unit_axis(c0, c1, c2);

    vv.set_center( 2.0, 2.0, 2.0);

    vv.draw_bounding_box();
}

