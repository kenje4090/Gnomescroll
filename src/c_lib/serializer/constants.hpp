#pragma once

/*
 * RULES:
 * DONT CHANGE STRING NAMES UNLESS YOU KNOW WHAT THE FUCK YOU ARE DOING AND HAVE A GOOD REASON
 * EVERY TIME YOU CHANGE A STRING NAME, YOU RISK BREAKING THINGS
 * YOU WILL BREAK THINGS IF YOU DON'T UPDATE THE RENAME SCHEME
 * IF THERE IS NO RENAME SCHEME, DON'T CHANGE A NAME
 * 
 * DONT USE ENUMS FOR DATA THAT LIVES OUTSIDE OF THE GAME SERVER INSTANCE
 */


#include <t_map/common/constants.hpp>
#include <t_mech/common/common.hpp>

namespace serializer
{

const int REDIS_TIMEOUT   = 300;  // seconds
const int KEEP_ALIVE_RATE = (REDIS_TIMEOUT*30)/2; // send keep-alive halfway

// Cached data queues
const unsigned int PLAYER_LOAD_DATA_LIST_MAX = HARD_MAX_CONNECTIONS * 2;
const unsigned int PLAYER_CONTAINER_LOAD_DATA_LIST_MAX = PLAYER_LOAD_DATA_LIST_MAX * N_PLAYER_CONTAINERS;

const unsigned int PARSED_ITEM_DATA_LIST_MAX = MAX_CONTAINER_SIZE * N_PLAYER_CONTAINERS;

// DONT CHANGE ANYTHING BELOW THIS LINE

// Paths for server-specific flat-file data

#define WORLD_DATA_PATH "./world/"

#define DATA_TMP_EXT     ".tmp"
#define DATA_BACKUP_EXT  ".bak"
#define DATA_PALETTE_EXT ".pal"
//#define DATA_ERROR_EXT  ".err"

#define MAP_DATA_PATH       WORLD_DATA_PATH "map/"
#define MECH_DATA_PATH      WORLD_DATA_PATH "mechs/"
#define PLAYER_DATA_PATH    WORLD_DATA_PATH "players/"
#define CONTAINER_DATA_PATH WORLD_DATA_PATH "containers/"

#define MAP_DATA_EXT       ".map"
#define MECH_DATA_EXT      ".mch"
#define PLAYER_DATA_EXT    ".plr"
#define CONTAINER_DATA_EXT ".ctr"

// data files
const char map_filename[]                = MAP_DATA_PATH "map-" GS_STR(GS_VERSION) MAP_DATA_EXT;
const char map_filename_tmp[]            = MAP_DATA_PATH "map-" GS_STR(GS_VERSION) MAP_DATA_EXT DATA_TMP_EXT;
const char map_filename_backup[]         = MAP_DATA_PATH "map-" GS_STR(GS_VERSION) MAP_DATA_EXT DATA_BACKUP_EXT;
const char map_palette_filename[]        = MAP_DATA_PATH "map-" GS_STR(GS_VERSION) MAP_DATA_EXT DATA_PALETTE_EXT;
const char map_palette_filename_tmp[]    = MAP_DATA_PATH "map-" GS_STR(GS_VERSION) MAP_DATA_EXT DATA_PALETTE_EXT DATA_TMP_EXT;
const char map_palette_filename_backup[] = MAP_DATA_PATH "map-" GS_STR(GS_VERSION) MAP_DATA_EXT DATA_PALETTE_EXT DATA_BACKUP_EXT;

const char mech_filename[]                = MECH_DATA_PATH "mechs-" GS_STR(GS_VERSION) MECH_DATA_EXT;
const char mech_filename_tmp[]            = MECH_DATA_PATH "mechs-" GS_STR(GS_VERSION) MECH_DATA_EXT DATA_TMP_EXT;
const char mech_filename_backup[]         = MECH_DATA_PATH "mechs-" GS_STR(GS_VERSION) MECH_DATA_EXT DATA_BACKUP_EXT;
const char mech_palette_filename[]        = MECH_DATA_PATH "mechs-" GS_STR(GS_VERSION) MECH_DATA_EXT DATA_PALETTE_EXT;
const char mech_palette_filename_tmp[]    = MECH_DATA_PATH "mechs-" GS_STR(GS_VERSION) MECH_DATA_EXT DATA_PALETTE_EXT DATA_TMP_EXT;
const char mech_palette_filename_backup[] = MECH_DATA_PATH "mechs-" GS_STR(GS_VERSION) MECH_DATA_EXT DATA_PALETTE_EXT DATA_BACKUP_EXT;

const char player_filename[]        = PLAYER_DATA_PATH "players-" GS_STR(GS_VERSION) PLAYER_DATA_EXT;
const char player_filename_tmp[]    = PLAYER_DATA_PATH "players-" GS_STR(GS_VERSION) PLAYER_DATA_EXT DATA_TMP_EXT;
const char player_filename_backup[] = PLAYER_DATA_PATH "players-" GS_STR(GS_VERSION) PLAYER_DATA_EXT DATA_BACKUP_EXT;

const char container_filename[]        = CONTAINER_DATA_PATH "containers-" GS_STR(GS_VERSION) CONTAINER_DATA_EXT;
const char container_filename_tmp[]    = CONTAINER_DATA_PATH "containers-" GS_STR(GS_VERSION) CONTAINER_DATA_EXT DATA_TMP_EXT;
const char container_filename_backup[] = CONTAINER_DATA_PATH "containers-" GS_STR(GS_VERSION) CONTAINER_DATA_EXT DATA_BACKUP_EXT;

// error files
//const char map_error_filename[]       = MAP_DATA_PATH       "map-"        GS_STR(GS_VERSION) MAP_DATA_EXT       DATA_ERROR_EXT;
//const char mech_error_filename[]      = MECH_DATA_PATH      "mechs-"      GS_STR(GS_VERSION) MECH_DATA_EXT      DATA_ERROR_EXT;
//const char player_error_filename[]    = PLAYER_DATA_PATH    "players-"    GS_STR(GS_VERSION) PLAYER_DATA_EXT    DATA_ERROR_EXT;
//const char container_error_filename[] = CONTAINER_DATA_PATH "containers-" GS_STR(GS_VERSION) CONTAINER_DATA_EXT DATA_ERROR_EXT;

// Format data for items, containers, redis keys

#define LOCATION_NAME_MAX_LENGTH 24

#define UUID_TAG                 "GID"
#define NAME_TAG                 "NAM"
#define COLOR_TAG                "RGB"
#define CUBE_ID_TAG              "CUB"
#define USER_ID_TAG              "UID"
#define VERSION_TAG              "VER"
#define MECH_TYPE_TAG            "MCH"
#define DURABILITY_TAG           "DUR"
#define STACK_SIZE_TAG           "STA"
#define MAP_POSITION_TAG         "MAP"
#define CONTAINER_ID_TAG         "CID"
#define CONTAINER_SLOT_TAG       "CSL"
#define CONTAINER_COUNT_TAG      "CNT"
#define CONTAINER_ITEM_COUNT_TAG "CNT"

#define TAG_DELIMITER                      "="
#define PROPERTY_DELIMITER                 ";"
#define CONTAINER_SEPARATOR                "+"
#define COLOR_COMPONENT_DELIMITER          ","
#define MAP_POSITION_COMPONENT_DELIMITER   ","

const size_t TAG_LENGTH                                = 3;
const size_t TAG_DELIMITER_LENGTH                      = sizeof(TAG_DELIMITER)                    - 1;
const size_t PROPERTY_DELIMITER_LENGTH                 = sizeof(PROPERTY_DELIMITER)               - 1;
const size_t CONTAINER_SEPARATOR_LENGTH                = sizeof(CONTAINER_SEPARATOR)              - 1;
const size_t COLOR_COMPONENT_DELIMITER_LENGTH          = sizeof(COLOR_COMPONENT_DELIMITER)        - 1;
const size_t MAP_POSITION_COMPONENT_DELIMITER_LENGTH   = sizeof(MAP_POSITION_COMPONENT_DELIMITER) - 1;

const size_t UUID_STRING_LENGTH = 36;

#define VERSION_LENGTH                4
#define USER_ID_LENGTH                10
#define CUBE_ID_LENGTH                4
#define MECH_TYPE_LENGTH              4
#define ITEM_UUID_LENGTH              36
#define CONTAINER_ID_LENGTH           5
#define CONTAINER_COUNT_LENGTH        5
#define ITEM_DURABILITY_LENGTH        5
#define ITEM_STACK_SIZE_LENGTH        5
#define ITEM_CONTAINER_SLOT_LENGTH    3
#define CONTAINER_ITEM_COUNT_LENGTH   3

#define MAP_POSITION_COMPONENT_LENGTH 4
#define MAP_POSITION_LENGTH ((MAP_POSITION_COMPONENT_LENGTH*3) + ((MAP_POSITION_COMPONENT_LENGTH-1)*MAP_POSITION_COMPONENT_DELIMITER_LENGTH))

#define COLOR_COMPONENT_LENGTH        3  // color component (0-255)
#define COLOR_LENGTH        ((COLOR_COMPONENT_LENGTH*3) + ((COLOR_COMPONENT_LENGTH-1)*COLOR_COMPONENT_DELIMITER_LENGTH))

const size_t ITEM_FIELD_COUNT             = 5;
const size_t PLAYER_FIELD_COUNT           = 1;
const size_t CONTAINER_FIELD_COUNT        = 3;
const size_t MAP_PALETTE_FIELD_COUNT      = 2;
const size_t MECH_PALETTE_FIELD_COUNT     = 2;
const size_t CONTAINER_FILE_FIELD_COUNT   = 2;
const size_t PLAYER_CONTAINER_FIELD_COUNT = 3;

// These values are just for reference and should not be relied upon
// A line's property count is subject to change, invalidating these lengths for anything saved before the change
// However, never make them SMALLER
const size_t PLAYER_LINE_LENGTH =
       PLAYER_FIELD_COUNT * (TAG_LENGTH + TAG_DELIMITER_LENGTH)
    + (PLAYER_FIELD_COUNT - 1) * PROPERTY_DELIMITER_LENGTH
    + COLOR_LENGTH;
    
const size_t PLAYER_CONTAINER_LINE_LENGTH =
       PLAYER_CONTAINER_FIELD_COUNT * (TAG_LENGTH + TAG_DELIMITER_LENGTH)
    + (PLAYER_CONTAINER_FIELD_COUNT - 1) * PROPERTY_DELIMITER_LENGTH
    + DAT_NAME_MAX_LENGTH
    + USER_ID_LENGTH
    + CONTAINER_ITEM_COUNT_LENGTH;

const size_t CONTAINER_LINE_LENGTH =
       CONTAINER_FIELD_COUNT * (TAG_LENGTH + TAG_DELIMITER_LENGTH)
    + (CONTAINER_FIELD_COUNT - 1) * PROPERTY_DELIMITER_LENGTH
    + CONTAINER_ID_LENGTH
    + DAT_NAME_MAX_LENGTH
    + CONTAINER_ITEM_COUNT_LENGTH
    + MAP_POSITION_LENGTH;

const size_t ITEM_LINE_LENGTH =
       ITEM_FIELD_COUNT * (TAG_LENGTH + TAG_DELIMITER_LENGTH)
    + (ITEM_FIELD_COUNT - 1) * PROPERTY_DELIMITER_LENGTH
    + ITEM_UUID_LENGTH
    + DAT_NAME_MAX_LENGTH
    + ITEM_DURABILITY_LENGTH
    + ITEM_STACK_SIZE_LENGTH
    + ITEM_CONTAINER_SLOT_LENGTH;

const size_t CONTAINER_FILE_LINE_LENGTH =
       CONTAINER_FILE_FIELD_COUNT * (TAG_LENGTH + TAG_DELIMITER_LENGTH)
    + (CONTAINER_FILE_FIELD_COUNT - 1) * PROPERTY_DELIMITER_LENGTH
    + VERSION_LENGTH
    + CONTAINER_COUNT_LENGTH;

const size_t MAP_PALETTE_LINE_LENGTH =
       MAP_PALETTE_FIELD_COUNT * (TAG_LENGTH + TAG_DELIMITER_LENGTH)
    + (MAP_PALETTE_FIELD_COUNT - 1) * PROPERTY_DELIMITER_LENGTH
    + CUBE_ID_LENGTH
    + DAT_NAME_MAX_LENGTH;
    
const size_t MECH_PALETTE_LINE_LENGTH =
       MECH_PALETTE_FIELD_COUNT * (TAG_LENGTH + TAG_DELIMITER_LENGTH)
    + (MECH_PALETTE_FIELD_COUNT - 1) * PROPERTY_DELIMITER_LENGTH
    + MECH_TYPE_LENGTH
    + DAT_NAME_MAX_LENGTH;

const char CONTAINER_FILE_HEADER_FMT[] =
    VERSION_TAG         TAG_DELIMITER
        "%0" GS_STR(VERSION_LENGTH)         "d"
        PROPERTY_DELIMITER
    CONTAINER_COUNT_TAG TAG_DELIMITER
        "%0" GS_STR(CONTAINER_COUNT_LENGTH) "d";
        
const char PLAYER_DATA_FMT[] =
    COLOR_TAG       TAG_DELIMITER
        "%0" GS_STR(COLOR_COMPONENT_LENGTH) "d"
            COLOR_COMPONENT_DELIMITER
        "%0" GS_STR(COLOR_COMPONENT_LENGTH) "d"
            COLOR_COMPONENT_DELIMITER
        "%0" GS_STR(COLOR_COMPONENT_LENGTH) "d";

const char PLAYER_CONTAINER_HEADER_FMT[] =
    NAME_TAG            TAG_DELIMITER
        "%-" GS_STR(DAT_NAME_MAX_LENGTH)      "s"
        PROPERTY_DELIMITER
    USER_ID_TAG         TAG_DELIMITER
        "%0" GS_STR(USER_ID_LENGTH)                 "d"
        PROPERTY_DELIMITER
    CONTAINER_ITEM_COUNT_TAG TAG_DELIMITER
        "%0" GS_STR(CONTAINER_ITEM_COUNT_LENGTH)    "d";

const char CONTAINER_HEADER_FMT[] =
    NAME_TAG            TAG_DELIMITER
        "%-" GS_STR(DAT_NAME_MAX_LENGTH)     "s"
        PROPERTY_DELIMITER
    CONTAINER_ITEM_COUNT_TAG TAG_DELIMITER
        "%0" GS_STR(CONTAINER_ITEM_COUNT_LENGTH)   "d"
        PROPERTY_DELIMITER
    MAP_POSITION_TAG    TAG_DELIMITER
        "%0" GS_STR(MAP_POSITION_COMPONENT_LENGTH) "d"
            MAP_POSITION_COMPONENT_DELIMITER
        "%0" GS_STR(MAP_POSITION_COMPONENT_LENGTH) "d"
            MAP_POSITION_COMPONENT_DELIMITER
        "%0" GS_STR(MAP_POSITION_COMPONENT_LENGTH) "d"
        PROPERTY_DELIMITER
    CONTAINER_ID_TAG    TAG_DELIMITER
        "%0" GS_STR(CONTAINER_ID_LENGTH)           "d";
    
const char ITEM_FMT[] =
    NAME_TAG           TAG_DELIMITER
        "%-" GS_STR(DAT_NAME_MAX_LENGTH)       "s"
        PROPERTY_DELIMITER
    DURABILITY_TAG     TAG_DELIMITER
        "%0" GS_STR(ITEM_DURABILITY_LENGTH)     "d"
        PROPERTY_DELIMITER
    STACK_SIZE_TAG     TAG_DELIMITER
        "%0" GS_STR(ITEM_STACK_SIZE_LENGTH)     "d"
        PROPERTY_DELIMITER
    CONTAINER_SLOT_TAG TAG_DELIMITER
        "%0" GS_STR(ITEM_CONTAINER_SLOT_LENGTH) "d"
        PROPERTY_DELIMITER        
    UUID_TAG           TAG_DELIMITER
        "%-" GS_STR(ITEM_UUID_LENGTH)           "s";

const char MAP_PALETTE_FMT[] =
    NAME_TAG    TAG_DELIMITER
        "%-" GS_STR(DAT_NAME_MAX_LENGTH) "s"
        PROPERTY_DELIMITER
    CUBE_ID_TAG TAG_DELIMITER
        "%0" GS_STR(CUBE_ID_LENGTH)       "d";
        
const char MECH_PALETTE_FMT[] =
    NAME_TAG    TAG_DELIMITER
        "%-" GS_STR(DAT_NAME_MAX_LENGTH) "s"
        PROPERTY_DELIMITER
    MECH_TYPE_TAG TAG_DELIMITER
        "%0" GS_STR(MECH_TYPE_LENGTH)       "d";

#define PLAYER_REDIS_KEY_PREFIX "player:"

#define PLAYER_CONTAINER_LOCATION_PREFIX PLAYER_REDIS_KEY_PREFIX

#define PLAYER_HAND_LOCATION_SUBNAME         "hand"
#define PLAYER_TOOLBELT_LOCATION_SUBNAME     "toolbelt"
#define PLAYER_INVENTORY_LOCATION_SUBNAME    "inventory"
#define PLAYER_SYNTHESIZER_LOCATION_SUBNAME  "synthesizer"
#define PLAYER_ENERGY_TANKS_LOCATION_SUBNAME "energy_tanks"

#define PLAYER_HAND_LOCATION_NAME         PLAYER_CONTAINER_LOCATION_PREFIX PLAYER_HAND_LOCATION_SUBNAME
#define PLAYER_TOOLBELT_LOCATION_NAME     PLAYER_CONTAINER_LOCATION_PREFIX PLAYER_TOOLBELT_LOCATION_SUBNAME
#define PLAYER_INVENTORY_LOCATION_NAME    PLAYER_CONTAINER_LOCATION_PREFIX PLAYER_INVENTORY_LOCATION_SUBNAME
#define PLAYER_SYNTHESIZER_LOCATION_NAME  PLAYER_CONTAINER_LOCATION_PREFIX PLAYER_SYNTHESIZER_LOCATION_SUBNAME
#define PLAYER_ENERGY_TANKS_LOCATION_NAME PLAYER_CONTAINER_LOCATION_PREFIX PLAYER_ENERGY_TANKS_LOCATION_SUBNAME

// The following are not safe to store -- they are only valid per compilation. Only strings are safe

const char* get_player_container_location_name(ItemContainerType container_type)
{
    switch (container_type)
    {        
        case AGENT_HAND:
            return PLAYER_HAND_LOCATION_NAME;
        case AGENT_TOOLBELT:
            return PLAYER_TOOLBELT_LOCATION_NAME;
        case AGENT_INVENTORY:
            return PLAYER_INVENTORY_LOCATION_NAME;
        case AGENT_SYNTHESIZER:
            return PLAYER_SYNTHESIZER_LOCATION_NAME;
        case AGENT_ENERGY_TANKS:
            return PLAYER_ENERGY_TANKS_LOCATION_NAME;
            
        default:
            GS_ASSERT(false);
            return NULL;
    }
    GS_ASSERT(false);
    return NULL;
}

inline bool is_valid_location_name_char(char c)
{
    return (isalnum(c) || c == ':' || c == '_' || c == '-');
}

bool is_valid_location_name(const char* name)
{
    size_t len = strlen(name);
    if (len <= 0 || len > LOCATION_NAME_MAX_LENGTH) return false;
    for (size_t i=0; i<len; i++)
        if (!is_valid_location_name_char(name[i]))
            return false;
    return true;
}

void verify_config();

}   // serializer
