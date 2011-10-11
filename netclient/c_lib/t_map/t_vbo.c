#include "./t_vbo.h"

//#include "./t_inline.c"

GLuint texture = 0;

SDL_Surface *surface;

int draw_mode_enabled = 0;

unsigned int _garbage_collection_counter = 0;
#define garbage_collection_frequency 10

int T_MAP_BLEND_MODE = 0;

void _toggle_terrain_map_blend_mode() {
    T_MAP_BLEND_MODE = (T_MAP_BLEND_MODE+1)%7;
    printf("toggle_terrain_map_blend_mode: blend mode= %i\n", T_MAP_BLEND_MODE);
}

int T_MAP_Z_BUFFER = 0;
void _toggle_z_buffer() {
    T_MAP_Z_BUFFER = (T_MAP_Z_BUFFER +1) %2;
    if(T_MAP_Z_BUFFER==0) printf("z_buffer off\n");
    if(T_MAP_Z_BUFFER==1) printf("z_buffer on\n");

}


//fulstrum culling globals
struct _Camera {
float x,y,z;
float vx,vy,vz;
float ux,uy,uz; //up direction
float ratio;
float viewangle;
};

int fulstrum_culling;
int view_distance = 40;
struct _Camera c;


/*
glDepthMask — enable or disable writing into the depth buffer

void glDepthMask(   GLboolean   flag);

Specifies whether the depth buffer is enabled for writing. If flag is GL_FALSE, depth buffer writing is disabled. Otherwise, it is enabled. Initially, depth buffer writing is enabled.
*/


///advice
/*
It turns out that if you disable depth testing (glDisable(GL_DEPTH_TEST)),
GL also disables writes to the depth buffer. The correct solution is to tell GL to ignore the depth test
results with glDepthFunc (glDepthFunc(GL_ALWAYS)). Be careful because in this state, if you render a far away
object last, the depth buffer will contain the values of that far object.
 ---
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
RenderScene();
SwapBuffers(hdc);  //For Windows
 *
 */

/*
 OpenGL has a function that can prevent pixels with a specified set of alpha values from writing the colour or Z buffers. For example:


     glAlphaFunc ( GL_GREATER, 0.1 ) ;
     glEnable ( GL_ALPHA_TEST ) ;
*/

//struct Vertex* quad_cache;
struct Vertex quad_cache[max_cubes*6*4];

struct Vertex* _get_quad_cache() {
    return (struct Vertex*) &quad_cache;
}

/*
#north/south is +/- x
#west/east is +/- y
l = [
        1,1,1 , 0,1,1 , 0,0,1 , 1,0,1 , #top
        0,1,0 , 1,1,0 , 1,0,0 , 0,0,0 , #bottom
        1,0,1 , 1,0,0 , 1,1,0 , 1,1,1 , #north
        0,1,1 , 0,1,0 , 0,0,0 , 0,0,1 , #south
        1,1,1 , 1,1,0 , 0,1,0,  0,1,1 , #west
        0,0,1 , 0,0,0 , 1,0,0 , 1,0,1 , #east
]
*/

const int _lighting = 0;


GLfloat mat_specular[] = {1.0, 1.0, 1.0, 1.0};
GLfloat mat_diffuse[] = {1.0, 0.0, 0.0};
GLfloat mat_shininess[] = { 50.0 };
GLfloat light_position[] = {256.0, 256.0, 128.0, 0.0};
GLfloat light_color[] = {1.0, 1.0, 1.0, 0.0};
GLfloat diffuse_color[] =  {1.0, 1.0, 1.0, 0.0};
//GLfloat lmodel_ambient[] = {0.8, 0.8, 0.8, 0.0};
GLfloat lmodel_ambient[] = {0.8, 0.8, 0.8, 0.8};

int _init_quad_cache_normals() {
    //int i,j;
    int cube_id, side, i;

    int n[3];
    for(cube_id=0;cube_id<max_cubes;cube_id++) {
        for(side=0;side<6;side++) {

            if(side == 0) {
                n[0]= 0;
                n[1]= 0;
                n[2]= 1;
            }
            if(side == 1) {
                n[0]= 0;
                n[1]= 0;
                n[2]= -1;
            }
            if(side == 2) {
                n[0]= 1;
                n[1]= 0;
                n[2]= 0;
            }

            if(side == 3) {
                n[0]= -1;
                n[1]= 0;
                n[2]= 0;
            }
            if(side == 4) {
                n[0]= 0;
                n[1]= 1;
                n[2]= 0;
            }
            if(side == 5) {
                n[0]= 0;
                n[1]= -1;
                n[2]= 0;
            }
        /*
            if(side == 1) n = {0,0,-1};
            if(side == 2) n = {1,0,0};
            if(side == 3) n = {-1,0,0};
            if(side == 4) n = {0,1,0};
            if(side == 5) n = {0,-1,0};
        */       
            for(i=0;i<4;i++) {
                quad_cache[cube_id*6*4+ 6*side+ i].normal[0] = n[0];
                quad_cache[cube_id*6*4+ 6*side+ i].normal[1] = n[1];
                quad_cache[cube_id*6*4+ 6*side+ i].normal[2] = n[2];
                quad_cache[cube_id*6*4+ 6*side+ i].normal[3] = 0;
            }
        }
    }

}

int _init_draw_terrain() {
    //quad_cache = _get_quad_cache();
    _init_quad_cache_normals();

    //glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    //glEnable(GL_POLYGON_SMOOTH);
    printf("Terrain map: vertex size is %i bytes \n", sizeof(struct Vertex));

    if(texture == 0) { //load texture if texture is not set
    //surface=IMG_Load("media/texture/textures_03.png");  //should this be freed?
    surface=IMG_Load("media/texture/blocks_01.png");
    if(!surface) {printf("IMG_Load: %s \n", IMG_GetError());return 0;}

    //SDL_Surface *_surface = IMG_Load("media/texture/textures_03.png");
    //if(!_surface) {printf("IMG_Load: %s \n", IMG_GetError());return 0;}

    //SDL_FreeSurface(_surface);

    glEnable(GL_TEXTURE_2D);
    glGenTextures( 1, &texture );
    glBindTexture( GL_TEXTURE_2D, texture );

/*
 * Replace with tiles and use mipmapping for filter
*/
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);

    //Uint32 GL_PIXEL_TYPE = GL_BGR;
    int texture_format;
    if (surface->format->Rmask == 0x000000ff) texture_format = GL_RGBA;
    else texture_format = GL_BGRA;

    glTexImage2D(GL_TEXTURE_2D, 0, 4, surface->w, surface->h, 0, texture_format, GL_UNSIGNED_BYTE, surface->pixels );
    //glTexImage2D(GL_TEXTURE_2D, 0, 4, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels );
    //glTexImage2D(GL_TEXTURE_2D, 0, 4, surface->w, surface->h, 0, GL_BGRA, GL_UNSIGNED_BYTE, surface->pixels );
    glDisable(GL_TEXTURE_2D);
    }
    return 0;
}



/*
deprecated

int create_vbo(struct VBO* q_VBO, struct Vertex* v_list, int v_num) {
    GLuint VBO_id;
    VBO_id = 0;
    if (v_num == 0) { return 0; }
    glEnable(GL_TEXTURE_2D);
    q_VBO->v_list = (struct Vertex* )malloc(v_num*sizeof(struct Vertex)); ///dont forget to free this!!!
    q_VBO->v_num = v_num;
    memcpy(q_VBO->v_list, v_list, v_num*sizeof(struct Vertex));
    glGenBuffers(1, &VBO_id);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_id);
    glBufferData(GL_ARRAY_BUFFER, v_num*sizeof(struct Vertex), q_VBO->v_list, GL_STATIC_DRAW); // size, pointer to array, usecase
    q_VBO->VBO_id = VBO_id;
    glDisable(GL_TEXTURE_2D);
    //printf("inside: VBO_id= %i \n", VBO_id);
    return VBO_id;
}
*/

void delete_vbo(struct VBO* vbo) {

    if(vbo->VBO_id != 0) glDeleteBuffers(1, &vbo->VBO_id);
    ///free the system memory copy of the vertex buffer
    
    if(vbo->v_list != NULL) {
        free(vbo->v_list);
        vbo->v_list = NULL;
    }
    vbo->VBO_id = 0;
    vbo->v_num = 0;
    vbo->v_list_max_size = 0;
}

int print_vertex(struct Vertex* v) {
printf("x,y,z= %f, %f, %f\n", v->x,v->y,v->z);
printf("tx,ty= %f, %f\n", v->tx, v->ty);
printf("r,g,b,a= %i, %i, %i, %i\n", v->r,v->g,v->b,v->a);
printf("\n");
return 0;
}

/// start VBO

//buffers for VBO stuff
//struct Vertex cs[(128*8*8)*4*6]; //chunk scratch
//unsigned int cs_n; //number of vertices in chunk scratch

static const int CI[6*8*3] = {1, 1, 1, 0, 1, 1, -1, 1, 1, -1, 0, 1, -1, -1, 1, 0, -1, 1, 1, -1, 1, 1, 0, 1,
-1, 1, -1, 0, 1, -1, 1, 1, -1, 1, 0, -1, 1, -1, -1, 0, -1, -1, -1, -1, -1, -1, 0, -1,
1, -1, 1, 1, -1, 0, 1, -1, -1, 1, 0, -1, 1, 1, -1, 1, 1, 0, 1, 1, 1, 1, 0, 1,
-1, 1, 1, -1, 1, 0, -1, 1, -1, -1, 0, -1, -1, -1, -1, -1, -1, 0, -1, -1, 1, -1, 0, 1,
1, 1, 1, 1, 1, 0, 1, 1, -1, 0, 1, -1, -1, 1, -1, -1, 1, 0, -1, 1, 1, 0, 1, 1,
-1, -1, 1, -1, -1, 0, -1, -1, -1, 0, -1, -1, 1, -1, -1, 1, -1, 0, 1, -1, 1, 0, -1, 1 };

/*
will be 1 if is adjacent to any side
will be 2 only if both sides are occluded
 */

inline int calcAdj(int side_1, int side_2, int corner)
{
    int occ = (side_1 | side_2 | corner) + (side_1 & side_2);
    if( occ == 0) return 255;
    if(occ == 1) return 177;
    //if(occ == 2) return 177;  
    if(occ == 2) return 100;    
}

const static int s_array[18] = {
        0,0,1,  //top
        0,0,-1, //bottom
        1,0,0,  //north
        -1,0,0, //south
        0,1,0,  //west
        0,-1,0, //east
        };

int inline _is_occluded(int x,int y,int z, int side_num) {
    int i;
    i = 3*side_num;
    x += s_array[i+0];
    y += s_array[i+1];
    z += s_array[i+2];

    return isOccludes(_get(x,y,z));
}

int inline _is_occluded_transparent(int x,int y,int z, int side_num, int _tile_id) {
    int i;
    i = 3*side_num;
    x += s_array[i+0];
    y += s_array[i+1];
    z += s_array[i+2];

    int tile_id = _get(x,y,z);
    if(tile_id == _tile_id) return 1;
    return isActive(tile_id);
}

void __inline add_quad2(struct Vertex* v_list, int offset, int x, int y, int z, int side, int tile_id) {
    int i;
    //struct Vertex* v;
    //memcpy(&cs[cs_n], &quad_cache[tile_id*6*4+4*side], 4*sizeof(struct Vertex)); //id*6*4+4*side+vert_num

    memcpy(&v_list[offset], &quad_cache[tile_id*6*4+4*side], 4*sizeof(struct Vertex)); //id*6*4+4*side+vert_num

    int index;
    int CX[8];
    for(i=0; i<8; i++) {
        index = side*8*3+i*3;
        CX[i] = isActive(_get(x+CI[index+0],y+CI[index+1],z+CI[index+2]));
    }

    float _x = x;
    float _y = y;
    float _z = z;
    for(i=0; i<=3;i++) {
        v_list[offset+i].x += _x;
        v_list[offset+i].y += _y;
        v_list[offset+i].z += _z;
    }

    int occ = (x+y+z);
    if(occ > 255) occ = 255;

    occ = calcAdj(CX[7], CX[1], CX[0]);
    v_list[offset+0].r = occ;
    v_list[offset+0].g = occ;
    v_list[offset+0].b = occ;

    occ = calcAdj(CX[1], CX[3], CX[2]);
    v_list[offset+1].r = occ;
    v_list[offset+1].g = occ;
    v_list[offset+1].b = occ;

    occ = calcAdj(CX[3], CX[5], CX[4]);
    v_list[offset+2].r = occ;
    v_list[offset+2].g = occ;
    v_list[offset+2].b = occ;

    occ = calcAdj(CX[5], CX[7], CX[6]);
    v_list[offset+3].r = occ;
    v_list[offset+3].g = occ;
    v_list[offset+3].b = occ;
}

int inline _is_occluded_transparent(int x,int y,int z, int side_num, int _tile_id);
int inline _is_occluded(int x,int y,int z, int side_num);



//Non-normal blocks:vertex count for each
static int oddBlockVCount[7] = {0, 8, 8, 8, 8, 8, 8};

#define _0 0.00
#define _1 1.00

static float oddBlockV[2*12] = {
_0,_1,1, _1,_0,1, _1,_0,0, _0,_1,0,
_1,_1,1, _0,_0,1, _0,_0,0, _1,_1,0
};

//assume 8 vertices
inline void insert_oddBlock(struct Vertex* v_list, int offset, int x, int y, int z, int active, int tile_id) {
    //memcpy(&v_list[offset], &quad_cache[tile_id*6*4+4*side], 4*2*sizeof(struct Vertex)); //id*6*4+4*side+vert_num
    memcpy(&v_list[offset], &quad_cache[tile_id*6*4+0], 4*2*sizeof(struct Vertex));

    float _x = x;
    float _y = y;
    float _z = z;
    int j = active -2;
    int i;
    for(i=0; i<8;i++) {
        v_list[offset+i].x = _x + oddBlockV[12*j+3*i+0];
        v_list[offset+i].y = _y + oddBlockV[12*j+3*i+1];
        v_list[offset+i].z = _z + oddBlockV[12*j+3*i+2];
    }

}


static const int VERTEX_SLACK = 128;

static bool BUFFER_ORPHANING = true; //recycle buffer or create new

int update_column_VBO(struct vm_column* column) {
    int tile_id, side_num;
    int _x, _y, _z;

    struct VBO* vbo = &column->vbo;
    struct vm_chunk* chunk;
    int i;

    //cs_n = 0; //clear chunk scratch

    int cube_vertex_count[4] = {0, 0, 0, 0};
    int vertex_count = 0; //clear counter
/*    
    if(vbo->VBO_id != 0) {
        delete_vbo(&column->vbo);
    }
    Dont delete , reuse
*/
    //first pass, count quads
    int active;
    int transparency;
    for(i = 0; i < vm_column_max; i++) {
        if(column->chunk[i] == NULL) { continue; }
        //printf("chunk= %i\n", i);
        chunk = column->chunk[i];
        //chunk->vbo_needs_update = 0;
        //printf("1,2,3 = %i, %i, %i \n", 8*chunk->x_off, 8*chunk->y_off, 8*chunk->z_off);
        for(_x = 8*chunk->x_off; _x < 8*chunk->x_off +8 ; _x++) {
        for(_y = 8*chunk->y_off; _y < 8*chunk->y_off +8 ; _y++) {
        for(_z = 8*chunk->z_off; _z < 8*chunk->z_off +8 ; _z++) {
            tile_id = _get(_x,_y,_z);
            //printf("test %i, %i, %i tile= %i\n", _x,_y,_z,tile_id );
            active = isActive(tile_id);
            if(active == 0) continue;

            transparency = isTransparent(tile_id);
            if(active == 1)  
            {

                //if(isOccludes(tile_id) == 1) {
                if(transparency == 0)
                { 
                    for(side_num=0; side_num<6; side_num++) {
                        if(! _is_occluded(_x,_y,_z,side_num)) {
                            //add_quad(_x,_y,_z,side_num,tile_id);
                            vertex_count += 4;
                            cube_vertex_count[0] += 4;
                        }
                    }
                } 
                else
                {
                    //active block that does not occlude
                    for(side_num=0; side_num<6; side_num++) 
                    {
                        if(! _is_occluded_transparent(_x,_y,_z,side_num, tile_id)) 
                        {
                            //add_quad(_x,_y,_z,side_num,tile_id);
                            vertex_count += 4;
                            cube_vertex_count[transparency] += 4;
                        }
                    }
                }
                continue;    
            }
        if(active > 1)  {
            vertex_count += oddBlockVCount[active];
            cube_vertex_count[transparency] += oddBlockVCount[active];
            continue;
        }

        }}}
    }
    //set offsets
    
    //vbo->_v_num[4];       //parameters for draw pass
    //vbo->_v_offset[4];

    vbo->_v_num[0] = cube_vertex_count[0];
    vbo->_v_num[1] = cube_vertex_count[1];
    vbo->_v_num[2] = cube_vertex_count[2];
    vbo->_v_num[3] = cube_vertex_count[3];

    vbo->_v_offset[0] = 0;
    vbo->_v_offset[1] = cube_vertex_count[0];
    vbo->_v_offset[2] = cube_vertex_count[0]+cube_vertex_count[1];
    vbo->_v_offset[3] = cube_vertex_count[0]+cube_vertex_count[1]+cube_vertex_count[2];

    /*
        Now that quads have been counted, malloc memory
        -malloc is slow, so over allocated and reuse if possible 
    */

    //if(cs_n == 0 ) {

    if(vertex_count == 0) {
        //vbo->VBO_id = 0;
        vbo->v_num = 0;

        set_flag(column, VBO_loaded, 0);
        set_flag(column, VBO_needs_update, 0);
        set_flag(column, VBO_has_blocks, 0);
        return 0;
    } else {
        set_flag(column, VBO_loaded, 1);
        set_flag(column, VBO_needs_update, 0);
        set_flag(column, VBO_has_blocks, 1);

        //create_vbo(&column->vbo, cs, cs_n);

        //printf("Malloc: %i vertex \n", vertex_count);
        vbo->v_num = vertex_count; //total vertices, size of VBO

        if(vertex_count > vbo->v_list_max_size) {
            if(vbo->v_list != NULL) {
                free(vbo->v_list);
                vbo->v_list = NULL;
            }
            vbo->v_list_max_size = VERTEX_SLACK+vertex_count;
            vbo->v_list = (struct Vertex*)malloc(vbo->v_list_max_size*sizeof(struct Vertex)); 
        }
    }
    //printf("vbo_id= %i v_num= %i \n", vbo->VBO_id,vbo->v_num);


    int _cube_vertex_count[4];
    _cube_vertex_count[0] = vbo->_v_offset[0];
    _cube_vertex_count[1] = vbo->_v_offset[1];
    _cube_vertex_count[2] = vbo->_v_offset[2];
    _cube_vertex_count[3] = vbo->_v_offset[3];

    struct Vertex* v_list2 = vbo->v_list;

    for(i = 0; i < vm_column_max; i++) {
        if(column->chunk[i] == NULL) { continue; }
        //printf("chunk= %i\n", i);
        chunk = column->chunk[i];
        //chunk->vbo_needs_update = 0;
        //printf("1,2,3 = %i, %i, %i \n", 8*chunk->x_off, 8*chunk->y_off, 8*chunk->z_off);
        for(_x = 8*chunk->x_off; _x < 8*chunk->x_off +8 ; _x++) {
        for(_y = 8*chunk->y_off; _y < 8*chunk->y_off +8 ; _y++) {
        for(_z = 8*chunk->z_off; _z < 8*chunk->z_off +8 ; _z++) {
            tile_id = _get(_x,_y,_z);
            //printf("test %i, %i, %i tile= %i\n", _x,_y,_z,tile_id );
            active = isActive(tile_id);
            if(active == 0) continue;
            transparency = isTransparent(tile_id);
            if(active == 1)
            {
                if(transparency == 0)
                { 
                    for(side_num=0; side_num<6; side_num++) {
                        if(! _is_occluded(_x,_y,_z,side_num)) {
                            //printf("count= %i\n",_cube_vertex_count[0]);
                            add_quad2(v_list2, _cube_vertex_count[0], _x,_y,_z,side_num,tile_id);
                            _cube_vertex_count[0] += 4;
                        }
                    }
                } 
                else
                {
                    //active block that does not occlude
                    for(side_num=0; side_num<6; side_num++) 
                    {
                        if(! _is_occluded_transparent(_x,_y,_z,side_num, tile_id)) 
                        {
                            add_quad2(v_list2, _cube_vertex_count[transparency],_x,_y,_z,side_num,tile_id);
                            _cube_vertex_count[transparency] += 4;
                        }
                    }
                }
                continue;
            }

            if(active > 1)  {
                insert_oddBlock(v_list2, _cube_vertex_count[transparency],_x,_y,_z, active, tile_id);
                _cube_vertex_count[transparency] += oddBlockVCount[active];
                continue;
            }
        }}}
    }
/*
    if(vbo->_v_num[3] > 0) {
        printf("vnum= %i\n", vbo->v_num);
        printf("counts= %i, %i, %i, %i \n", _cube_vertex_count[0],_cube_vertex_count[1],_cube_vertex_count[2],_cube_vertex_count[3]);
        printf("offsets= %i, %i, %i, %i \n", vbo->_v_offset[0], vbo->_v_offset[1], vbo->_v_offset[2], vbo->_v_offset[3]);
        printf("vnum= %i, %i, %i, %i \n", vbo->_v_num[0], vbo->_v_num[1], vbo->_v_num[2], vbo->_v_num[3]);
    }
*/
    /*
        Now that memory is full of quads, push to graphics card
    */

//  GLuint VBO_id;
    glEnable(GL_TEXTURE_2D);

    if(vbo->VBO_id == 0)  glGenBuffers(1, &vbo->VBO_id);
    glBindBuffer(GL_ARRAY_BUFFER, vbo->VBO_id);
    glBufferData(GL_ARRAY_BUFFER, vbo->v_list_max_size*sizeof(struct Vertex), NULL, GL_STATIC_DRAW);
    glBufferData(GL_ARRAY_BUFFER, vbo->v_list_max_size*sizeof(struct Vertex), vbo->v_list, GL_STATIC_DRAW);

/*
    if( BUFFER_ORPHANING ) {
        if(vbo->VBO_id == 0)  glGenBuffers(1, &vbo->VBO_id);
        glBindBuffer(GL_ARRAY_BUFFER, vbo->VBO_id);
        glBufferData(GL_ARRAY_BUFFER, vbo->v_list_max_size*sizeof(struct Vertex), NULL, GL_STATIC_DRAW);
        glBufferData(GL_ARRAY_BUFFER, vbo->v_list_max_size*sizeof(struct Vertex), vbo->v_list, GL_STATIC_DRAW);
    } else {
        if(vbo->VBO_id == 0)
        {
            glGenBuffers(1, &vbo->VBO_id);
        } 
        else 
        {
            glDeleteBuffers(1, &vbo->VBO_id);
            glGenBuffers(1, &vbo->VBO_id);
        }
        glBindBuffer(GL_ARRAY_BUFFER, vbo->VBO_id);
        glBufferData(GL_ARRAY_BUFFER, vbo->v_list_max_size*sizeof(struct Vertex), vbo->v_list, GL_STATIC_DRAW);
    }
*/
    glDisable(GL_TEXTURE_2D);

    return 0;
}


int _update_chunks() {
    //printf("_update_chunks \n");
    struct vm_map* m;
    struct vm_column* c;
    int i,j;
    m = _get_map();
    //garbage collection

    _garbage_collection_counter++;
    if(_garbage_collection_counter >= garbage_collection_frequency) {
        _garbage_collection_counter = 0;
        for(i=0; i<vm_map_dim; i++) {
        for(j=0; j<vm_map_dim;j++) {
            c = &m->column[j*vm_map_dim+i];
            //if VBO is farther than 10 viewing units, delete it
            if (flag_is_true(c, VBO_loaded) && !chunk_render_check(c->x_off, c->y_off, 15)) {
                //printf("unloaded VBO: %i, %i \n", c->x_off, c->y_off);
                delete_vbo(&c->vbo);
                set_flag(c, VBO_loaded, 0);
            }
        }}
    }

    //VBO construction loop

    for(i=0; i<vm_map_dim; i++) {
    for(j=0; j<vm_map_dim;j++) {
        c = &m->column[j*vm_map_dim+i];
        //update or create VBO for chunks within 10 units of viewing distance
        if(chunk_render_check(c->x_off, c->y_off, 10)) {
            //if(c->vbo_needs_update == 1 || (c->vbo_loaded==0 && c->vbo_needs_update)) {

        if(flag_is_true(c, VBO_has_blocks) && ( flag_is_true(c, VBO_needs_update) || flag_is_false(c, VBO_loaded))) {
            /*
                if(c->vbo.VBO_id == 0) {
                    //printf("create VBO: %i, %i \n", c->x_off, c->y_off);
                } else {
                    //printf("update VBO: %i, %i \n", c->x_off, c->y_off);
                }
            */
                update_column_VBO(c);
                return 0;
            }
        }


    }}
        return 0;

}

GLuint filter = 0; ;                                // Which Filter To Use
GLuint fogMode[]= { GL_EXP, GL_EXP2, GL_LINEAR };   // Storage For Three Types Of Fog
GLuint fogfilter= 2;                    // Which Fog To Use
GLfloat fogColor[4]= {0.5f, 0.5f, 0.5f, 1.0f};      // Fog Color

int _draw_fog() {

glClearColor(0.5f,0.5f,0.5f,1.0f);          // We'll Clear To The Color Of The Fog ( Modified )

glFogi(GL_FOG_MODE, fogMode[fogfilter]);        // Fog Mode
glFogfv(GL_FOG_COLOR, fogColor);            // Set Fog Color
glFogf(GL_FOG_DENSITY, 0.35f);              // How Dense Will The Fog Be
glHint(GL_FOG_HINT, GL_DONT_CARE);          // Fog Hint Value
glFogf(GL_FOG_START, 1.0f);             // Fog Start Depth
glFogf(GL_FOG_END, 5.0f);               // Fog End Depth
glEnable(GL_FOG);                   // Enables GL_FOG

    return 0;

}
/*
int _t_ = 0;
int _c_ = 0;
*/

static int draw_vbo_n;
static struct VBO* draw_vbo_array[512];  //this should not be hardcoded; will piss someone off

int _draw_terrain() {
/*
    _c_++;
    if(_c_ % 120 == 0) {
        _t_ ++;
        if(_t_ %2 == 0) {
            printf("multisample enabled\n");
            glEnable(GL_MULTISAMPLE);
        } else {
            printf("multisample disabled\n");
            glDisable(GL_MULTISAMPLE);
        }
    }
*/
    //int s,f;
    //s= SDL_GetTicks();
    struct vm_map* m;
    struct vm_column* col;
    int i,j;
    int c_drawn, c_pruned;
    c_drawn=0; c_pruned=0;
    //start_vbo_draw();
    draw_vbo_n = 0;
    m = _get_map();
    //printf("Start Map Draw\n");
    for(i=0; i<vm_map_dim; i++) {
    for(j=0; j<vm_map_dim; j++) {
        col = &m->column[j*vm_map_dim+i];
        if(flag_is_true(col, VBO_loaded) && chunk_render_check(col->x_off, col->y_off, 0)) {
            c_drawn++;
            set_flag(col,VBO_drawn,1);
            //draw_quad_vbo(&col->vbo);
            /*
                Que up map VBOs to be drawn
                !!! May want to sort VBOs in front to back order
            */
            draw_vbo_array[draw_vbo_n] = &col->vbo;
            draw_vbo_n++;
        } else {
            set_flag(col,VBO_drawn,0);
            c_pruned++;
        }
    }}

    DRAW_VBOS1();
    //DRAW_VBOS1a();

    //DRAW_VBOS2();    
    //end_vbo_draw();
    
    //printf("drawn chunks= %i, pruned chunks= %i \n", c_drawn, c_pruned);
    //_draw_fog();
    //f = SDL_GetTicks();
    //printf("Terrain rendering time= %i \n", f-s);
    return 0;

}

int __inline chunk_render_check(int x_off, int y_off, int tolerance) {
float x,y;
x = 8*x_off + 4;
y = 8*y_off + 4;

//printf("c.x= %f, c.y= %f, x_off= %i, y_off= %i \n", c.x,c.y,x_off, y_off);
if((c.x-x)*(c.x-x) + (c.y-y)*(c.y-y) < (tolerance+view_distance)*(tolerance+view_distance))
    {
        return 1;
    }
//else
    //{
        return 0;
    //}
}

int _set_camera(float x, float y, float z, float vx, float vy, float vz, float ux, float uy, float uz, float ratio, float viewangle) {
c.x = x; c.y = y; c.z = z;
c.ux=ux; c.uy=uy; c.uz=uz;
c.vx=vx; c.vy=vy; c.vz=vz;
c.ratio = ratio;
c.viewangle = viewangle;
return 0;
}

int _set_view_distance(int vd) {
    view_distance = vd;
    return 0;
}

int _set_fulstrum_culling(int value) {
    fulstrum_culling = value;
    return 0;
}


int crb[3]; //the request buffer

int* _chunk_request() {
    struct vm_map* m;
    struct vm_chunk* ch;
    struct vm_column* col;
    int x; int y;
    struct vm_chunk* ch_lowest = NULL;
    int score, score_lowest;
    int i,j,k;

    score_lowest = -1;

    m = _get_map();
    for(i=0; i<vm_map_dim; i++) {
    for(j=0; j<vm_map_dim;j++) {
        col = &m->column[j*vm_map_dim+i];
        for(k = 0; k < vm_column_max; k++) {
            ch = col->chunk[k];
            if(ch == NULL) { continue; }
            if(ch->server_version == ch->local_version || ch->requested == 1) { continue; }
            x = 8*col->x_off + 4;
            y = 8*col->y_off + 4;
            score = (c.x-x)*(c.x-x) + (c.y-y)*(c.y-y);
            //printf("score= %i, x,y,z= %i, %i, %i \n", score, ch->x_off, ch->y_off, ch->z_off);
            if(score_lowest == -1){
                score_lowest = score;
                ch_lowest = ch;
            }
            if(score < score_lowest) {
                score_lowest = score;
                ch_lowest = ch;
            }
        }
    }}

    if(score_lowest == -1) {
        return NULL;
    } else {
        ch = ch_lowest;
        //printf("score= %i, x,y,z= %i, %i, %i \n", score_lowest, ch->x_off, ch->y_off, ch->z_off);
        ch_lowest->requested = 1;
        crb[0] = ch_lowest->x_off;
        crb[1] = ch_lowest->y_off;
        crb[2] = ch_lowest->z_off;
        return crb;
    }
    return NULL;
}

void _refresh_map_vbo() {
    printf("Refreshing map VBOs\n");
    struct vm_map* m;
    struct vm_column* c;
    int i,j;
    m = _get_map();

    //VBO construction loop

    for(i=0; i<vm_map_dim; i++) {
    for(j=0; j<vm_map_dim;j++) {
        c = &m->column[j*vm_map_dim+i];
        if(flag_is_true(c, VBO_has_blocks)) {
                //c->vbo.VBO_id == 0
                set_flag(c, VBO_needs_update, 1);
                set_flag(c, VBO_loaded, 0);
                //flag_is_false(c, VBO_loaded)
            }

    }}
}

//BIG FUNCTION


//int draw_vbo_n;
//struct VBO* draw_vbo_array[512];  

void DRAW_VBOS1() {
        
    glColor3b(255,255,255);

    glEnable(GL_TEXTURE_2D);
    glEnable (GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
    glEnable (GL_DEPTH_TEST);
    //glEnable(GL_CULL_FACE);

    glAlphaFunc ( GL_GREATER, 0.1 ) ;

    glBindTexture( GL_TEXTURE_2D, texture );

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    int i;
    struct VBO* vbo;
    for(i=0;i<draw_vbo_n;i++) {
        vbo = draw_vbo_array[i];

        if(vbo->v_num == 0) continue;

        glBindBuffer(GL_ARRAY_BUFFER, vbo->VBO_id);
        //start_vbo_draw();

        glVertexPointer(3, GL_FLOAT, sizeof(struct Vertex), (GLvoid*)0);
        glTexCoordPointer(2, GL_FLOAT, sizeof(struct Vertex), (GLvoid*)12);
        glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(struct Vertex), (GLvoid*)20);
        //glNormalPointer(GL_BYTE, sizeof(struct Vertex), (GLvoid*)24);

        glEnable(GL_CULL_FACE);
        glDrawArrays(GL_QUADS,0, vbo->_v_num[0]);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable (GL_ALPHA_TEST);
        if(vbo->_v_num[1] != 0) 
        {
            glDrawArrays(GL_QUADS, vbo->_v_offset[1], vbo->_v_num[1]);
        }
    
        glDisable(GL_CULL_FACE);
        if(vbo->_v_num[2] != 0) 
        {
            glDrawArrays(GL_QUADS, vbo->_v_offset[2], vbo->_v_num[2]);
        }
        glDisable(GL_ALPHA_TEST);      
    /*
        if(vbo->_v_num[2] != 0) 
        {
            glDepthMask(false);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            glDrawArrays(GL_QUADS, vbo->_v_offset[3], vbo->_v_num[3]);
            glDepthMask(true);
        }
    */
        glDisable(GL_BLEND);
    }

/*
    glEnable(GL_BLEND);
    glDepthMask(false);
    for(i=0;i<draw_vbo_n;i++) {
        vbo = draw_vbo_array[i];
        if(vbo->_v_num[3] == 0) continue; 
        glBindBuffer(GL_ARRAY_BUFFER, vbo->VBO_id);
        glDrawArrays(GL_QUADS, vbo->_v_offset[3], vbo->_v_num[3]);
    }
    glDepthMask(true); 
    glDisable(GL_BLEND);
*/
    glEnable(GL_BLEND);
    glDepthMask(false);
    for(i=0;i<draw_vbo_n;i++) {
        vbo = draw_vbo_array[i];
        if(vbo->_v_num[3] == 0) continue; 
        glBindBuffer(GL_ARRAY_BUFFER, vbo->VBO_id);
        glVertexPointer(3, GL_FLOAT, sizeof(struct Vertex), (GLvoid*)0);
        glTexCoordPointer(2, GL_FLOAT, sizeof(struct Vertex), (GLvoid*)12);
        glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(struct Vertex), (GLvoid*)20);
        //glNormalPointer(GL_BYTE, sizeof(struct Vertex), (GLvoid*)24);
        glDrawArrays(GL_QUADS, vbo->_v_offset[3], vbo->_v_num[3]);
    }
    glDepthMask(true); 
    glDisable(GL_BLEND);
/*
    Do Stuff
*/

/*
    vbo->_v_num[0]
    vbo->_v_num[1]
    vbo->_v_num[2]
    vbo->_v_num[3]

    vbo->_v_offset[0]
    vbo->_v_offset[1]
    vbo->_v_offset[2]
    vbo->_v_offset[3]
*/

//end draw
glDisableClientState(GL_VERTEX_ARRAY);
glDisableClientState(GL_COLOR_ARRAY);
glDisableClientState(GL_TEXTURE_COORD_ARRAY);

glShadeModel(GL_FLAT);

/*
glDisable (GL_DEPTH_TEST);
glDisable(GL_CULL_FACE);
*/

glDisable(GL_TEXTURE_2D);

}

void DRAW_VBOS1a() {
        
    glColor3b(255,255,255);

    glEnable(GL_TEXTURE_2D);
    glEnable (GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
    glEnable (GL_DEPTH_TEST);
    //glEnable(GL_CULL_FACE);

    glAlphaFunc ( GL_GREATER, 0.1 ) ;

    glBindTexture( GL_TEXTURE_2D, texture );

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    int i;
    struct VBO* vbo;
    for(i=0;i<draw_vbo_n;i++) {
        vbo = draw_vbo_array[i];

        if(vbo->v_num == 0) continue;

        glBindBuffer(GL_ARRAY_BUFFER, vbo->VBO_id);
        //start_vbo_draw();

        glVertexPointer(3, GL_FLOAT, sizeof(struct Vertex), (GLvoid*)0);
        glTexCoordPointer(2, GL_FLOAT, sizeof(struct Vertex), (GLvoid*)12);
        glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(struct Vertex), (GLvoid*)20);
        //glNormalPointer(GL_BYTE, sizeof(struct Vertex), (GLvoid*)24);

        glEnable(GL_CULL_FACE);
        glDrawArrays(GL_QUADS,0, vbo->_v_num[0]);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable (GL_ALPHA_TEST);
    
        glDrawArrays(GL_QUADS, vbo->_v_offset[1], vbo->_v_num[1]);
    
        glDisable(GL_CULL_FACE);

        glDrawArrays(GL_QUADS, vbo->_v_offset[2], vbo->_v_num[2]);

        glDisable(GL_ALPHA_TEST);      
    /*
        if(vbo->_v_num[2] != 0) 
        {
            glDepthMask(false);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            glDrawArrays(GL_QUADS, vbo->_v_offset[3], vbo->_v_num[3]);
            glDepthMask(true);
        }
    */
        glDisable(GL_BLEND);
    }

    glEnable(GL_BLEND);
    glDepthMask(false);
    for(i=0;i<draw_vbo_n;i++) {
        vbo = draw_vbo_array[i];
        if(vbo->v_num == 0) continue; 
        glBindBuffer(GL_ARRAY_BUFFER, vbo->VBO_id);
        glVertexPointer(3, GL_FLOAT, sizeof(struct Vertex), (GLvoid*)0);
        glTexCoordPointer(2, GL_FLOAT, sizeof(struct Vertex), (GLvoid*)12);
        glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(struct Vertex), (GLvoid*)20);

        glDrawArrays(GL_QUADS, vbo->_v_offset[3], vbo->_v_num[3]);
    }
    glDepthMask(true); 
    glDisable(GL_BLEND);

/*
    Do Stuff
*/

/*
    vbo->_v_num[0]
    vbo->_v_num[1]
    vbo->_v_num[2]
    vbo->_v_num[3]

    vbo->_v_offset[0]
    vbo->_v_offset[1]
    vbo->_v_offset[2]
    vbo->_v_offset[3]
*/

//end draw
glDisableClientState(GL_VERTEX_ARRAY);
glDisableClientState(GL_COLOR_ARRAY);
glDisableClientState(GL_TEXTURE_COORD_ARRAY);

glShadeModel(GL_FLAT);

/*
glDisable (GL_DEPTH_TEST);
glDisable(GL_CULL_FACE);
*/

glDisable(GL_TEXTURE_2D);

}

void DRAW_VBO2() {
    

}

void DRAW_VBOS2b() {
    
}

/*

if(T_MAP_BLEND_MODE != 0) {
    glEnable(GL_BLEND);
    if(T_MAP_BLEND_MODE ==1 ) glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    if(T_MAP_BLEND_MODE ==2 ) glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if(T_MAP_BLEND_MODE ==3 ) glBlendFunc(GL_ONE, GL_ONE);
    if(T_MAP_BLEND_MODE ==4 ) glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    if(T_MAP_BLEND_MODE ==5 ) glBlendFunc(GL_ONE_MINUS_SRC_COLOR, GL_ONE_MINUS_DST_COLOR);
    if(T_MAP_BLEND_MODE ==6 ) glBlendFunc(GL_ONE, GL_ALPHA);
}
*/