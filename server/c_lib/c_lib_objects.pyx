cdef extern from "./physics/vector.h":
    struct Vector:
        float x
        float y
        float z

cdef extern from "./objects/grenade.h":
    void grenade_tick()
    int create_grenade(int type, float x, float y, float z, float vx, float vy, float vz, int ttl, int ttl_max)
    void destroy_grenade(int gid)
    Vector* _get_grenade_position(int gid)

cdef extern from "./objects/neutron.h":
    void neutron_tick()
    void create_neutron(int type, int energy, float x, float y, float z, float vx, float vy, float vz)

cdef extern from "./objects/cspray.h":
    void cspray_tick()
    void create_cspray(int type, float x, float y, float z, float vx, float vy, float vz)

cdef extern from "./agent/agent.h":
    void agent_tick()
    int create_agent(float x, float y, float z)
    void set_agent_state(int id, float xangle, float yangle)

cdef extern from "./agent/agent_vox.h":
    void init_agent_vox_volume(int id, int part, int xdim, int ydim, int zdim, float vosize)
    void set_agent_vox_volume(int id, int part, int x, int y, int z, int r, int g, int b, int a)

    void set_agent_limb_direction(int id, int part, float fx,float fy,float fz, float nx,float ny, float nz)
    void set_agent_limb_anchor_point(int id, int part, float length, float ax,float ay,float az)

def tick():
    grenade_tick()
    neutron_tick()
    cspray_tick()
    agent_tick()

def _create_grenade(float x, float y, float z, float vx, float vy, float vz, int ttl, int ttl_max):
    print "CYTHON CREATE GRENADE"
    return create_grenade(1, x,y,z, vx,vy,vz, ttl, ttl_max)

def _destroy_grenade(int gid):
    print "CYTHON DESTROY GRENADE"
    destroy_grenade(gid)

def _create_neutron(int type, int energy, float x, float y, float z, float vx, float vy, float vz):
    create_neutron(type,energy, x,y,z, vx,vy,vz)

def _create_cspray(int type, float x, float y, float z, float vx, float vy, float vz):
    create_cspray(type, x,y,z, vx,vy,vz)

def get_grenade_position(int gid):
    cdef Vector *pos
    pos = _get_grenade_position(gid)
    p = [pos.x, pos.y, pos.z]
    return p
    
#agent

def _create_agent(float x, float y, float z):
    cdef int id
    id = create_agent(x,y,z)

def _set_agent_state(int id, float xangle, float yangle):
    set_agent_state(id, xangle, yangle)

### map gen
'''
cdef extern from "./map_gen/density.h":
    void init_map_gen_density()
    void compute_density_map()
    void map_density_visualize(int density, float min, float max)


def _compute_density_map():
    compute_density_map()

def _map_density_visualize(int density, float min, float max):
    map_density_visualize(density, min, max)
'''
