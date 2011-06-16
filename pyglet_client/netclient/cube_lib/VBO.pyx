from libc.stdlib cimport malloc, free

#core functionality


#constants
cdef enum:
    max_cubes = 1024

cdef enum:
    x_chunk_size = 8
    y_chunk_size = 8
    z_chunk_size = 8
    chunk_size = 512

#structs
cdef struct Vertex:
    float x,y,z
    float tx,ty
    unsigned char r,g,b,a #for packing

cdef struct Quad:
    Vertex vertex[4]

#cdef struct Cube:
#    Quad quad[6]

cdef struct Chunk_scratch:
    Quad quad[chunk_size*6] #6 quad per cube
    int v_num

#cdef struct Quad_cache:
#   Quad quad[max_cubes*6]  #6 quad per cube

#globals


cdef Chunk_scratch* chunk_scratch
chunk_scratch = <Chunk_scratch *>malloc(sizeof(Chunk_scratch))

cdef Quad* quad_cache
quad_cache = <Quad *>malloc(max_cubes*6 * sizeof(Quad))
#quad_cache = <Quad *>malloc(1024*6 * sizeof(Quad))

#cdef Quad_cache quad_cache2

#print "size = " + str(sizeof(max_cubes*6))
#quad_cache = <Quad *>malloc(max_cubes*6*sizeof(Quad))

cdef float * v_index
v_index = <float*> malloc(72*sizeof(float))

l = [
         0,1,1 , 0,0,1 , 1,0,1 , 1,1,1 , #top
         1,0,0 , 0,0,0 , 0,1,0 , 1,1,0 , #bottom
         0,1,1 , 1,1,1 , 1,1,0 , 0,1,0 , #north
         0,0,1 , 0,0,0 , 1,0,0 , 1,0,1 , #south
         0,1,1 , 0,1,0 , 0,0,0 , 0,0,1 , #west
         1,0,1 , 1,0,0 , 1,1,0 , 1,1,1 , #east
    ]

for i in range(0, 72):
    v_index[i] = float(l[i])

cdef inline set_tex(int vert_num, Vertex* vertex, float x, float y):
    vertex.tx = x * (1/8)
    vertex.ty = x * (1/8)
    if vert_num == 0:
        vertex.tx += 0
        vertex.ty += 0
    if vert_num == 1:
        vertex.tx += 1/8
        vertex.ty += 0
    if vert_num == 2:
        vertex.tx += 1/8
        vertex.ty += 1/8
    if vert_num == 3:
        vertex.tx += 0
        vertex.ty += 1/8

def convert_index(index, height, width):
    index = int(index)
    height = int(height)
    width = int(width)
    x_ = index % width
    y_ = int((index - x_) / width)
    y = height - y_ -1
    rvalue =  x_ + y*width
#init

cpdef test():
    global v_index, quad_cache
    cdef int i
    print str(max_cubes)
    for i in range(0, max_cubes*6):
        print str(i)
        #print str(quad_cache[i].vertex.r)
        quad_cache[i].vertex[0].y = 5

def init_quad_cache():
    global v_index, quad_cache
    cdef Quad* quad
    cdef Vertex* vertex
    cdef int i,j,k,index
    for k in range(0, max_cubes):
        for i in range(0,6):
            quad = &quad_cache[6*k+i]
            for j in range(0,4):
                index = 12*i + 3*j
                vi = 6*k+i
                vertex = &quad.vertex[j]
                #vertices
                vertex.x = v_index[index + 0]
                vertex.y = v_index[index + 1]
                vertex.z = v_index[index + 2]
                #colors
                vertex.r = 255
                vertex.g = 255
                vertex.b = 255
                vertex.a = 255
                #tex
                set_tex(j, vertex, 0, 3)
                #print "(%i,%i,%i)" % (k,i,j)
    print "done"

cdef inline set_side(float x, float y, float z, int tile_id, int side_num, Quad* quad):
    global quad_cache
    cdef int i
    cdef Vertex* vertex
    quad = &quad_cache[6*tile_id + side_num]
    for i in range(0,4):
        vertex = &quad.vertex[i]
        vertex.x +=x
        vertex.y +=y
        vertex.z +=z

#(tv_list, tc_list, ttex_list) = self.cubeRenderCache.get_side(rx, ry, rz, tile_id, side_num)

#    int x_off,y_off,z_off

#from libc.stdlib cimport malloc, free

#cdef Chunk_scratch * chunk_scratch = <Chunk_scratch *>malloc(sizeof(Chunk_scratch))

## control state

def init():
    init_quad_cache()
    clear_buffer()

def clear_buffer():
    chunk_scratch.v_num = 0

def add_quad(float x,float y,float z,int side,int tile):
    cdef Quad* quad = &chunk_scratch.quad[chunk_scratch.v_num]
    chunk_scratch.v_num += 1
    set_side(x,y,z, tile, side, quad)

#testing
def test_chunk():
    cdef int i
    for i in range(0,150):
        add_quad(5,5,i,3,0)


cimport SDL.gl
#import SDL.gl
from SDL.gl cimport bind_VBO

#cdef exern _bind_VBO(Quad* quad_list, int v_num)

#extern from 'draw_functions.h':
#    cdef exern _bind_VBO(Quad* quad_list, int v_num)

def draw_test_chunk():
    cdef Quad* quad_list = chunk_scratch.quad
    cdef int v_num = chunk_scratch.v_num
    #SGL.gl._bind_VBO(quad_list, v_num)
    bind_VBO(quad_list, v_num)
