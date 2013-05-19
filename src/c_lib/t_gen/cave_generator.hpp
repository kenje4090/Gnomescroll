#pragma once

#if DC_CLIENT
# error Do not include this file in the client
#endif

#include <t_gen/noise_map2.hpp>
#include <t_gen/twister.hpp>
#include <t_map/t_map.hpp>
#include <t_mech/_interface.hpp>
#include <t_mech/properties.hpp>


namespace t_gen
{

// v is starting point of line
// vz is unit vector for direction of line
// px is the point
// returns square of distance between line and point
OPTIMIZED
inline float point_line_distance2(float vx, float vy, float vz, float wx, float wy, float wz, float px, float py, float pz)
{
    px -= vx;
    py -= vy;
    pz -= vz;
    float t = px*wx + py*wy + pz*wz;
    float x = t*wx - px;
    float y = t*wy - py;
    float z = t*wz - pz;
    return x*x + y*y + z*z;
}

OPTIMIZED
void generate_node(float xs, float ys, float zs, float theta, float phi, float cave_size)
{
    const int baseline = 0;

    while (mrandf() < 0.999f)
    {
            const static float length = 3.0f;
            const float _theta = theta * 2 * PI;
            const float _phi = phi * 2 * PI;

            float dx = sinf(_phi)*cosf(_theta);
            float dy = sinf(_phi)*sinf(_theta);
            float dz = cosf(0.75f*_phi);
            dz *= dz;

            float size = (cave_size * mrandf() * 1.25f) + 0.5f;  // variable diameter

            float xm = abs(dx) + size;
            float ym = abs(dy) + size;
            float zm = abs(dz) + size;

            int xmin = xs - xm;
            int xmax = xs + xm;

            int ymin = ys - ym;
            int ymax = ys + ym;

            int zmin = zs - zm;
            int zmax = zs + zm;
            bool hits_bottom = false;
            //can speed up by 8
            for (int i=xmin; i<=xmax; i++)
            for (int j=ymin; j<=ymax; j++)
            for (int k=zmin; k<=zmax; k++)
            {
                if (k < baseline || k >= 128)
                {
                    hits_bottom = true;
                    continue;
                }

                float x = i + 0.5f;
                float y = j + 0.5f;
                float z = k + 0.5f;

                int ii = i % 512;
                int jj = j % 512;

                float d = point_line_distance2(xs,ys,zs, dx,dy,dz, x,y,z);
                if (d < size*size)
                    t_map::set_fast(ii, jj, k, EMPTY_CUBE);
            }

            if (hits_bottom) phi *= -1;

            xs += length * dx;
            ys += length * dy;
            zs += length * dz;

            static const float theta_adj = 0.15f;
            static const float phi_adj = 0.10f;

            theta += theta_adj * (2.0f*mrandf() - 1.0f);
            phi += phi_adj * (2.0f*mrandf() - 1.0f);

            if (phi < 0) phi += 1;
            if (phi > 1) phi -= 1;

            //static const float phi_target = 0.0f;
            //static const float phi_damp = 0.03f;

            //phi -= phi_damp*(phi - phi_target);
        /*
            if (phi < 0) phi += 1;
            if (phi > 1) phi -= 1;

            if (zs < 32 && mrandf() < 0.20)
            {
                if (phi > 0.25) phi -= 0.10;
            }
        */
    }

}


void start_cave_generator()
{
    const int nodes = 300; //actually, 4 times more nodes will generate but many of the will be clustered together because of the entanglement script below
    const float cave_size = 2.0f; //this also controls the size of stalactites and holes between caves

    const float baseline = 0.0f;
    const int try_limit = 10000;

    seed_twister(rand());

    for (int i=0; i<nodes; i++)
    {
        float x,y,z;
        int tries = 0;
        int cavecount = 0; //used for better generation of ellipses and stalactites
        do
        {
            x = mrandf()*map_dim.x;
            y = mrandf()*map_dim.y;
            z = mrandf()*(map_dim.z - baseline) + baseline;
            x = x+(cavecount % 100);
            y = y+(99 - (cavecount % 100));
            z = z+(cavecount % 3);
            if (z >= 5) //just for nicer and more round entrances
            {
                z = z + cavecount;
                cavecount=cavecount / 2;
            }
            if (z >= 15) //makes cave entrances rabidly drop and twist in 1 direction to stop them from getting tangled together
            {
                x = x*x*y;
                y = y*y*x;
                z = z / 2;
            }
            if (cavecount >= 100) //makes caves go further x and y instead of continuing downwards
            {
                x = x + 30 + x * y;
                y = y + 30 + x * y;
            }
            cavecount++;
        } while (t_map::get(x,y,z) == 0 && tries++ < try_limit);

        if (tries >= try_limit) return;

        float phi = mrandf() * 2.0f * PI;
        float theta = mrandf() * 2.0f * PI;

        generate_node(x,y,z, theta, phi, cave_size); //the entanglement script, helps to generate stalactites
        generate_node(y,x,z, theta, phi, cave_size);
        generate_node(y,y,z, theta, phi, cave_size);
        generate_node(x,x,z, theta, phi, cave_size);
    }
}


}   // t_gen
