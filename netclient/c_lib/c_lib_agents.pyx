
'''
cdef extern from "./agent/agent_draw.h":
    void agent_draw() #draw all agents
'''

cdef extern from "./agent/agent_draw.hpp" namespace "AgentDraw":
    void draw_agent_aiming_direction(float x, float y, float z, float xangle, float yangle)
    void draw_agent_bounding_box(float x, float y, float z, float radius, float head_height, float height)
    void draw_agent_cube_selection(int x, int y, int z, int r, int g, int b)
    void draw_agent_cube_side_selection(int x, int y, int z, int cx, int cy, int cz, int r, int g, int b)

'''
cdef extern from "./agent/agent_state.cpp":
    void send_control_state()

def _send_control_state():
    send_control_state()
'''

'''
cdef extern from "./agent/control_state.h":
    void set_agent_control_state(int* cs, float xangle, float yangle)
'''

'''
cdef extern from "./agent/agent_client.h":
    void set_agent_control_state(int x[32], float theta, float phi)

def _set_agent_control_state(int f, int b, int l, int r, int jump, float theta, float phi):
    cdef int x[32]
    x[0] = f
    x[1] = b
    x[2] = l
    x[3] = r
    x[4] = jump
    set_agent_control_state(x, theta, phi)
'''


'''
cdef extern from "./agent/agent.h":
    int create_agent(float x, float y, float z)
    void set_agent_state(int id, float xangle, float yangle)
'''

'''
cdef extern from "./agent/agent_vox.h":
    void init_agent_vox_volume(int id, int part, int xdim, int ydim, int zdim, float vosize)
    void set_agent_vox_volume(int id, int part, int x, int y, int z, int r, int g, int b, int a)

    void set_agent_limb_direction(int id, int part, float fx,float fy,float fz, float nx,float ny, float nz)
    void set_agent_limb_anchor_point(int id, int part, float length, float ax,float ay,float az)
'''


def _draw_agent_aiming_direction(float x, float y, float z, float xangle, float yangle):
    draw_agent_aiming_direction(x, y, z, xangle, yangle)

def _draw_agent_bounding_box(float x, float y, float z, float radius, float head_height, float height):
    draw_agent_bounding_box(x, y, z, radius, head_height, height)

def _draw_agent_cube_selection(int x, int y, int z, int r, int g, int b):
    draw_agent_cube_selection(x,y,z, r,g,b)

def _draw_agent_cube_side_selection(int x, int y, int z, int cx, int cy, int cz, int r, int g, int b):
    draw_agent_cube_side_selection( x,  y,  z,  cx,  cy,  cz,  r,  g,  b)



#agent

'''
from dat.agent_dim import lu1, lu2, lu3, vosize, skel_tick
agent_list = []


def _set_agent_control_state(l, xangle,yangle):
    cdef int x[6]
    for i in range(0,6):
        x[i] = int(l[i])
    set_agent_control_state(x, xangle, yangle)

def agent_skeleton_update():
    global agent_list,lu2,lu3
    skel_tick()
    for id in agent_list:
        for part in range(0,6):
            length, ax,ay,az= lu2[part]
            set_agent_limb_anchor_point(id, part, length,ax,ay,az)
        for part in range(0,6):
            fx,fy,fz,nx,ny,nz = lu3[part]
            set_agent_limb_direction(id, part, fx, fy, fz, nx,ny,nz)
'''



'''
def default_vox_model_init(int id, int part, int xdim, int ydim, int zdim, float vosize):
    init_agent_vox_volume(id, part, xdim,ydim,zdim, vosize)
    for x in range(0,xdim):
        for y in range(0,ydim):
            for z in range(0,zdim):
                a = 255
                r = 32*x
                g = 32*y
                b = 32*z
                set_agent_vox_volume(id, part, x,y,z, r,g,b,a)
'''

'''
def _set_agent_model(int id):
    #cdef float vosize = .0625
    cdef int part
    cdef int xdim, ydim, zdim

    global lu1, lu2, lu3, vosize

    for part in range(0,6):
        xdim,ydim,zdim = lu1[part]
        default_vox_model_init(id, part, xdim,ydim,zdim, vosize)

    for part in range(0,6):
        length, ax,ay,az= lu2[part]
        set_agent_limb_anchor_point(id, part, length,ax,ay,az)
    for part in range(0,6):
        fx,fy,fz, nx,ny,nz = lu3[part]
        set_agent_limb_direction(id, part, fx, fy, fz, nx,ny,nz)

def _create_agent(float x, float y, float z):
    cdef int id
    id = create_agent(x,y,z)
    _set_agent_model(id)
    ##
    global agent_list
    agent_list.append(id)
    return id
'''

'''
def _set_agent_state(int id, float xangle, float yangle):
    set_agent_state(id, xangle, yangle)
'''

