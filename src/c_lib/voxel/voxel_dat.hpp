#pragma once

#include <common/color.hpp>
#include <physics/affine.hpp>

namespace Voxels
{

/* Dat storage */

class VoxDat; // forward decl;

class VoxColors
{
    public:
        unsigned char* rgba;
        int *index;
        int n;

        void init(int dx, int dy, int dz);
        void set(int i, int x, int y, int z, unsigned char r, unsigned char g, unsigned char b, unsigned char a);
        VoxColors();
        ~VoxColors();
};

class VoxPartDimension
{
    public:
        int x,y,z;

        void set(int x, int y, int z);
        int count();

        VoxPartDimension();
        VoxPartDimension(int x, int y, int z);
};

class VoxPart
{
    public:
        VoxPartDimension dimension;
        VoxColors colors;
        VoxDat* dat;    // parent
        
        int part_num;

        int skeleton_parent_matrix;
        float sx, sy, sz, srx, sry, srz;    // skeleton local matrix parameters
        
        float vox_size;
        bool biaxial; // true for horizontal+vertical (head). default=false

        char *filename;

		bool colorable;
		struct Color base_color;

        void set_local_matrix();   // uses cached x,y,z,rx,ry,rz values
        void set_dimension(int x, int y, int z);

        void set_filename(const char* filename);
        VoxPart(
            VoxDat* dat,
            int part_num,
            float vox_size,
            int dimension_x, int dimension_y, int dimension_z,
            const char* filename,
            bool biaxial=false
        );

        ~VoxPart();
};

class VoxDat {
    public:
        //voxel volume
        bool voxel_volume_inited;
        int n_parts;
        class VoxPart** vox_part;

        struct Affine* vox_volume_local_matrix;
        bool voxel_skeleton_inited;
        int* vox_skeleton_transveral_list;
        struct Affine* vox_skeleton_local_matrix;
        float** vox_skeleton_local_matrix_reference;
        int n_skeleton_nodes;
        
        void init_skeleton(int n_skeleton);
        
        void reset_skeleton_local_matrix(int node);
        void set_skeleton_local_matrix(int node, float x, float y, float z, float rx, float ry, float rz);
        void set_skeleton_node_parent(int node, int parent);
        void set_skeleton_parent_matrix(int part, int parent);

        void init_parts(int n_parts);

        void set_part_properties(
            int part_num,
            float vox_size,
            int dimension_x, int dimension_y, int dimension_z,
            const char* filename,
            bool biaxial=false);

        //anchor x,y,z then rotation x,y,z
        void set_part_local_matrix(int part_num, float x, float y, float z, float rx, float ry, float rz);

		void set_colorable(int part, bool colorable);
		void set_base_color(int part, unsigned char r, unsigned char g, unsigned char b);

        void set_color(int part, int x, int y, int z, unsigned char r, unsigned char g, unsigned char b, unsigned char a);

        void save(char *fn);

        VoxDat();
        ~VoxDat();
};

}   // Voxels
