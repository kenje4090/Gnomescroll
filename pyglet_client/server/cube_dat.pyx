
cube_list = {
    0 : {
        'id' : 0,
        'occludes' : False,
        'active' : False,
        },
    1 : {
        'id' : 1,
        'occludes' : True,
        'active' : True,

        'texture' : [ #t, b, w, e, n, s
        (0, []),  #top
        (0, []),  #bottom
        (0, []), #west
        (0, []), #east
        (0, []), #north
        (0, []), #south
        ],
        },
    2 : {
        'id' : 2,
        'occludes' : True,
        'active' : True,

        'texture' : [ #t, b, w, e, n, s
        (0, [0,1,2,3]),  #top
        (6, [0,1,2,3]),  #bottom
        (7, [0,1,2,3]), #west
        (3, [0,1,2,3]), #east
        (4, [0,1,2,3]), #north
        (5, [0,1,2,3]), #south
        ],
    },
    3 : {
        'id' : 3,
        'occludes' : True,
        'active' : True,

        'texture' : [ #t, b, w, e, n, s
        (1, [0,1,2,3]),  #top
        (1, [0,1,2,3]),  #bottom
        (1, [0,1,2,3]), #west
        (1, [0,1,2,3]), #east
        (1, [0,1,2,3]), #north
        (1, [0,1,2,3]), #south
        ],
    },
    4 : {
        'id' : 4,
        'occludes' : True,
        'active' : True,

        'texture' : [ #t, b, w, e, n, s
        (16, [0,1,2,3]),  #top
        (16, [0,1,2,3]),  #bottom
        (16, [0,1,2,3]), #west
        (16, [0,1,2,3]), #east
        (16, [0,1,2,3]), #north
        (16, [0,1,2,3]), #south
        ],
    },
 }

cdef enum:
    max_cubes = 4096

cdef class CubePhysicalProperties:
    cdef CubePhysical cube_array[4096]

    def __init__(self):
        global cube_list
        for cube in cube_list.values():
            self.add_cube(d)

    def add_cube(d):
        id = int(d['id'])
        if id >= max_cubes: #max number of cubes
            print "Error: cube id is too high"
            return
        active = int(d['active'])
        occludes = int(d['occludes'])
        self.cube_array[id] = CubePhysical(id, active, occludes)

    cdef inline int isActive(CubePhysicalProperties self, unsigned int id):
        if id >= max_cubes: #max number of cubes
            return 0
        return self.cube_array[id].active

    cdef inline int isOcclude(CubePhysicalProperties self, unsigned int id):
        if id >= max_cubes: #max number of cubes
            return 0
        return self.cube_array[id].occludes

cdef class CubePhysical:
    cdef int id = 0
    cdef int active = 0
    cdef int occludes = 0

    def __init__(CubePhysical self, int id,int active,int occludes):
        self.id = id
        self.active = active
        self.occludes = occludes

class CubeVisualProperties:
    def __init__(self):
        pass

class CubeProperties(object):

    def __init__(self):
        pass

    def getTexture(self, tile_id, side_num):
        global cube_list
        if cube_list.has_key(tile_id):
            tex_a = cube_list[tile_id]['texture']
            return tex_a[side_num]
        else:
            return 0

    #def isActive(self, tile_id):
        #if self.cubes.has_key(tile_id):
            #return self.cubes[tile_id]['active']
        #else:
            #print "Error, cube type does not exist"
            #return False

    #def isOcclude(self, tile_id):
        #if self.cubes.has_key(tile_id):
            #return self.cubes[tile_id]['occludes']
        #else:
            #print "Error, cube type does not exist"
            #return False


#import ctypes

cdef enum:
    x_chunk_size = 8
    y_chunk_size = 8
    z_chunk_size = 8

class MapChunk(object):

    terrainMap = None
    cubePhysicalProperties = CubePhysicalProperties()
#    cubeProperties = None
#    cubeRenderCache = None

    def __init__(self, x_offset, y_offset, z_offset):

        self.vertexList = None #an in describing batch number
        self.x_offset = x_offset
        self.y_offset = y_offset
        self.z_offset = z_offset
        self.update = True
        self.empty = True

    def update_vertex_buffer(self, batch = None):
        cdef int tile_id, x, y, z
        cdef int active_cube_numer, culled_quads
        cdef int side_num


        draw_list = []
        active_cube_number = 0
        culled_quads = 0
        for x in range(self.x_offset, self.x_offset+x_chunk_size):
            for y in range(self.x_offset, self.x_offset +y_chunk_size):
                for z in range(self.x_offset, self.x_offset+z_chunk_size):
                    tile_id = self.terrainMap.get(x,y,z)
                    ###
                    if self.cubePhysicalProperties.isActive(tile_id): #non-active tiles are not draw
                        active_cube_number += 1
                        for side_num in [0,1,2,3,4,5]:
                            if not self._is_occluded(x,y,z,side_num):
                                draw_list.append((x,y,z,tile_id, side_num))
                            else:
                                culled_quads += 1

        if False:
            print "quads in chunk=" + str(active_cube_number*6)
            print "culled quads=" + str(culled_quads)
            print "draw_list= "
            for xa in draw_list:
                print str(xa)

        cdef int v_num
        cdef float rx, ry, rz

        v_list = []
        c_list = []
        tex_list = []
        v_num = 0
        for (x,y,z,tile_id, side_num) in draw_list:
            rx = self.x_offset + x
            ry = self.y_offset + y
            rz = z

            (tv_list, tc_list, ttex_list) = self.cubeRenderCache.get_side(rx, ry, rz, tile_id, side_num)
            v_list += tv_list
            c_list += tc_list
            tex_list += ttex_list
            v_num += 4


        #print str(v_list)
        #print str(c_list)
        #print str(tex_list)
        #print str((len(v_list), len(c_list), len(tex_list)))

        if v_num == 0:
            self.empty = True
            self.update = False
            return

        if batch == None:
            self.vertexList = pyglet.graphics.vertex_list(v_num,
            ('v3f\static', v_list),
            ('c4B\static', c_list),
            ("t3f\static", tex_list)
        )
        else:
            self.vertexList = batch.add(v_num, pyglet.gl.GL_QUADS, None,
            ('v3f\static', v_list),
            ('c4B\static', c_list),
            ("t3f\static", tex_list)
            )

        self.update = False

    cdef inline int _is_occluded(self,int x,int y,int z,int side_num):
        cdef int _x, _y, _z, tile_id

        s_array = [(0,0,1), (0,0,-1), (0,1,0), (0,-1,0), (-1,0,0),(1,0,0)] #replace with if/then statement
        temp = s_array[side_num]
        _x = temp[0] + x
        _y = temp[1] + y
        _z = temp[2] + z

        tile_id = self.terrainMap.get(_x,_y,_z)
        return self.cubePhysicalProperties.isOcclude(tile_id)
