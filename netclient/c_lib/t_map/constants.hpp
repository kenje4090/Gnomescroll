#pragma once

// deprecate in favor of MapDimension map_dim
const int XMAX = 512;
const int YMAX = 512;
const int ZMAX = 128;

namespace t_map
{

const int TERRAIN_MAP_HEIGHT = 128;
const int TERRAIN_CHUNK_WIDTH = 16;

const int MAP_WIDTH = 512;
const int MAP_HEIGHT = 512;

const int MAP_CHUNK_WIDTH = MAP_WIDTH/TERRAIN_CHUNK_WIDTH;
const int MAP_CHUNK_HEIGHT = MAP_HEIGHT/TERRAIN_CHUNK_WIDTH;


} // t_map
