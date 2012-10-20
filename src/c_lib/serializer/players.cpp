#include "players.hpp"

#include <serializer/items.hpp>
#include <serializer/parse.hpp>
#include <serializer/_state.hpp>

namespace serializer
{

static class PlayerLoadDataList* player_load_data_list = NULL;
static class PlayerContainerLoadDataList* player_container_load_data_list = NULL;

void init_players()
{
    GS_ASSERT(player_load_data_list == NULL);
    player_load_data_list = new class PlayerLoadDataList(PLAYER_LOAD_DATA_LIST_MAX);

    GS_ASSERT(player_container_load_data_list == NULL);
    player_container_load_data_list = new class PlayerContainerLoadDataList(PLAYER_CONTAINER_LOAD_DATA_LIST_MAX);
}

void teardown_players()
{
    if (player_load_data_list != NULL)
    {
        GS_ASSERT(player_load_data_list->ct == 0);
        delete player_load_data_list;
    }
    if (player_container_load_data_list != NULL)
    {
        GS_ASSERT(player_container_load_data_list->ct == 0);
        delete player_container_load_data_list;
    }
}

class ParsedPlayerData
{
    public:
        struct Color color;
        bool valid;

    void reset()
    {
        this->color = AGENT_DEFAULT_COLOR;
        this->valid = false;
    }

    ParsedPlayerData()
    {
        this->reset();
    }
};

class ParsedPlayerContainerData
{
    public:
        char name[LOCATION_NAME_MAX_LENGTH+1];
        UserID user_id;
        unsigned int container_count;
        bool valid;

    void reset()
    {
        this->user_id = NULL_USER_ID;
        this->container_count = -1;
        memset(this->name, 0, sizeof(this->name));
        this->valid = false;
    }

    ParsedPlayerContainerData()
    {
        this->reset();
    }
};

static bool parse_player_container_token(const char* key, const char* val, class ParsedPlayerContainerData* data)
{
    bool err = false;
    if (strcmp(NAME_TAG, key) == 0)
    {
        GS_ASSERT(ItemContainer::is_valid_container_name(val));
        if (!ItemContainer::is_valid_container_name(val)) return false;
        strncpy(data->name, val, CONTAINER_NAME_MAX_LENGTH);
        data->name[CONTAINER_NAME_MAX_LENGTH] = '\0';
    }
    else
    if (strcmp(USER_ID_TAG, key) == 0)
    {
        long long user_id = parse_int(val, err);
        GS_ASSERT(!err && user_id > 0 && user_id < INT32_MAX);
        if (err || user_id <= 0 || user_id > INT32_MAX) return false;
        data->user_id = (UserID)user_id;
    }
    else
    if (strcmp(CONTAINER_ITEM_COUNT_TAG, key) == 0)
    {
        long long container_count = parse_int(val, err);
        GS_ASSERT(!err && container_count >= 0 && container_count < MAX_CONTAINER_SIZE);
        if (err || container_count < 0 || container_count > MAX_CONTAINER_SIZE) return false;
        data->container_count = (int)container_count;
    }
    else
    {   // unrecognized field
        GS_ASSERT(false);
        return false;
    }
    return true;
}

static bool parse_player_token(const char* key, const char* val, class ParsedPlayerData* data)
{
    bool err = false;
    if (strcmp(COLOR_TAG, key) == 0)
    {
        static char buf[COLOR_LENGTH+1] = {'\0'};
        strncpy(buf, val, COLOR_LENGTH);
        buf[COLOR_LENGTH] = '\0';
        
        int pts = 1;
        char d;
        int j = 0;
        int cmp_len = 0;
        while ((d = buf[j]) != '\0')
        {
            if (d == COLOR_COMPONENT_DELIMITER[0])
            {
                GS_ASSERT(cmp_len == COLOR_COMPONENT_LENGTH);
                if (cmp_len != COLOR_COMPONENT_LENGTH) return false;
                cmp_len = 0;
                pts++;
                buf[j] = '\0';
            }
            else
                cmp_len++;
            j++;
        }
        GS_ASSERT(cmp_len == COLOR_COMPONENT_LENGTH);
        if (cmp_len != COLOR_COMPONENT_LENGTH) return false;
        GS_ASSERT(pts == 3);
        if (pts != 3) return false;

        int base_offset = COLOR_COMPONENT_LENGTH + COLOR_COMPONENT_DELIMITER_LENGTH;
        long long r = parse_int(&buf[0 * base_offset], err);
        GS_ASSERT(!err && r >= 0 && r <= 255);
        if (err || r < 0 || r > 255) return false;
        long long g = parse_int(&buf[1 * base_offset], err);
        GS_ASSERT(!err && g >= 0 && g <= 255);
        if (err || g < 0 || g > 255) return false;
        long long b = parse_int(&buf[2 * base_offset], err);
        GS_ASSERT(!err && b >= 0 && b <= 255);
        if (err || b < 0 || b > 255) return false;

        data->color.r = (unsigned char)r;
        data->color.g = (unsigned char)g;
        data->color.b = (unsigned char)b;
    }
    else
    {   // unrecognized field
        GS_ASSERT(false);
        return false;
    }
    return true;
}

void process_player_container_blob(const char* str, class PlayerLoadData* player_load_data, class PlayerContainerLoadData* container_load_data)
{    
    // allocate scratch buffer long enough to hold the largest line
    static const size_t LONGEST_LINE = GS_MAX(ITEM_LINE_LENGTH, PLAYER_CONTAINER_LINE_LENGTH);
    char buf[LONGEST_LINE+1] = {'\0'};

    ItemLocationType location = IL_NOWHERE;
    int location_id = NULL_LOCATION;
    if (container_load_data->container_type == AGENT_HAND)
    {
        location = IL_HAND;
        location_id = player_load_data->agent_id;
    }
    else
    {
        location = IL_CONTAINER;
        location_id = container_load_data->container_id;
    }

    // copy container header line
    size_t i = 0;
    char c = '\0';
    while ((c = str[i]) != '\0' && c != '\n' && i < LONGEST_LINE)
        buf[i++] = c;
    buf[i] = '\0';
    GS_ASSERT(c == '\n' || c == '\0');
    if (c != '\0' && c != '\n') return;  // TODO -- log error
    
    // read header
    class ParsedPlayerContainerData container_data;
    parse_line<class ParsedPlayerContainerData>(&parse_player_container_token, buf, i, &container_data);
    GS_ASSERT(container_data.valid);
    if (!container_data.valid) return;  // TODO -- log error
    i++;

    // Check properties on the container data
    // The parser only makes sure the values it found were in range
    // It doesn't handle missing values (so that the format can remain flexible)
    
    GS_ASSERT(container_data.user_id == player_load_data->user_id);
    if (container_data.user_id != player_load_data->user_id) return;  // TODO -- log error

    // make sure location name matches container type
    const char* expected_container_name = ItemContainer::get_container_name(container_load_data->container_type);
    GS_ASSERT(expected_container_name != NULL && strcmp(expected_container_name, container_data.name) == 0);
    if (expected_container_name == NULL || strcmp(expected_container_name, container_data.name) != 0) return;   // TODO -- log error

    // make sure container count is less than the container's size
    unsigned int max_slots = ItemContainer::get_container_max_slots(container_load_data->container_type);
    GS_ASSERT(container_data.container_count <= max_slots);
    if (container_data.container_count > max_slots) return; // TODO -- log error

    // parse items
    class ParsedItemData item_data;
    for (unsigned int j=0; j<container_data.container_count; j++)
    {
        GS_ASSERT(c != '\0');
        if (c == '\0') break;

        size_t k = 0;
        while ((c = str[i++]) != '\0' && c != '\n' && k < LONGEST_LINE)
            buf[k++] = c;
        buf[k] = '\0';
        GS_ASSERT(c == '\n');
        if (c != '\n') break;    // TODO -- log error

        item_data.reset();
        parse_line<class ParsedItemData>(&parse_item_token, buf, k, &item_data);
        GS_ASSERT(item_data.valid);
        if (!item_data.valid) continue; // TODO -- log error
        
        GS_ASSERT(item_data.container_slot < max_slots);
        if (item_data.container_slot >= max_slots) continue;    // TODO -- log error

        class Item::Item* item = create_item_from_data(&item_data, location, location_id);
        GS_ASSERT(item != NULL);
        if (item == NULL) continue; // TODO -- log error

        if (location == IL_HAND)
            ItemContainer::load_item_into_hand(item->id, player_load_data->agent_id);
        else
        if (location == IL_CONTAINER)
            ItemContainer::load_item_into_container(item->id, container_load_data->container_id, item->container_slot);
    }
}

void process_player_blob(const char* str, class PlayerLoadData* player_load_data)
{
    static const size_t LONGEST_LINE = PLAYER_LINE_LENGTH;
    char buf[LONGEST_LINE+1] = {'\0'};

    // copy main data line
    size_t i = 0;
    char c = '\0';
    while ((c = str[i]) != '\0' && c != '\n' && i < LONGEST_LINE)
        buf[i++] = c;
    buf[i] = '\0';
    GS_ASSERT(c == '\0');
    if (c != '\0') return;  // TODO -- log error

    class ParsedPlayerData player_data;
    parse_line<class ParsedPlayerData>(&parse_player_token, buf, i, &player_data);
    GS_ASSERT(player_data.valid);
    if (!player_data.valid) return; // TODO -- log error
    i++;

    class Agent* agent = Agents::get_any_agent(player_load_data->agent_id);
    GS_ASSERT(agent != NULL);
    if (agent == NULL) return;  // TODO -- log error

    agent->status.set_color_silent(player_data.color);
}

void player_load_cb(redisAsyncContext* ctx, void* _reply, void* _data)
{
    class PlayerLoadData* data = (class PlayerLoadData*)_data;
    redisReply* reply = (redisReply*)_reply;

    if (reply->type == REDIS_REPLY_STRING)
    {
        process_player_blob(reply->str, data);
        // TODO -- catch errors and abort -- yes, players inventory will get corrupted otherwise
        data->player_data_was_loaded();
    }
    else
    if (reply->type == REDIS_REPLY_NIL)
        data->player_data_was_loaded();
    else
    if (reply->type == REDIS_REPLY_ERROR)
    {
        GS_ASSERT(reply->type != REDIS_REPLY_ERROR);
        printf("Loading player %d failed with redis error: %s\n", data->user_id, reply->str); 
    }
    
    player_load_data_list->destroy(data->id);
}

void player_container_load_cb(redisAsyncContext* ctx, void* _reply, void* _data)
{
    redisReply* reply = (redisReply*)_reply;
    class PlayerContainerLoadData* data = (class PlayerContainerLoadData*)_data;
    class PlayerLoadData* player_data = player_load_data_list->get(data->player_data_id);
    GS_ASSERT(player_data != NULL);
    if (player_data == NULL) return;    // TODO -- better handling
    
    if (reply->type == REDIS_REPLY_STRING)
    {
        process_player_container_blob(reply->str, player_data, data);
        // TODO -- catch errors and abort -- yes, players inventory will get corrupted otherwise
        player_data->container_was_loaded(data->container_id);
    }
    else
    if (reply->type == REDIS_REPLY_NIL)
        player_data->container_was_loaded(data->container_id);
    else
    if (reply->type == REDIS_REPLY_ERROR)
    {
        GS_ASSERT(reply->type != REDIS_REPLY_ERROR);
        printf("Loading player %d container %d failed with redis error: %s\n", player_data->user_id, data->container_id, reply->str); 
    }

    player_container_load_data_list->destroy(data->id);
}

const char* write_player_container_string(int container_id, UserID user_id)
{
    class ItemContainer::ItemContainerInterface* container = ItemContainer::get_container(container_id);
    GS_ASSERT(container != NULL);
    if (container == NULL) return NULL;

    const char* container_name = ItemContainer::get_container_name(container->type);
    GS_ASSERT(container_name != NULL);
    if (container_name == NULL) return NULL;

    size_t ibuf = 0;
    
    // write header
    int could_write = snprintf(&_buffer[ibuf], BUF_SIZE - ibuf, PLAYER_CONTAINER_HEADER_FMT,
        container_name, user_id, container->slot_count);
    GS_ASSERT(could_write > 0 && (size_t)could_write < BUF_SIZE - ibuf);
    if (could_write <= 0 || (size_t)could_write >= BUF_SIZE - ibuf) return NULL;
    ibuf += (size_t)could_write;
    
    _buffer[ibuf++] = '\n';
    if (ibuf >= BUF_SIZE) return NULL;

    if (container->slot_count > 0)
    {   // we must check the slot count, because write_container_contents_string returns 0 on error or if the container is empty
        size_t wrote = write_container_contents_string(&_buffer[ibuf], BUF_SIZE - ibuf, container);
        if (wrote <= 0) return NULL;  // error
        ibuf += wrote;
    }
    
    _buffer[ibuf] = '\0';

    return _buffer;
}

const char* write_player_string(AgentID agent_id)
{
    class Agent* agent = Agents::get_agent(agent_id);
    GS_ASSERT(agent != NULL);
    if (agent == NULL) return NULL;

    size_t ibuf = 0;

    int could_write = snprintf(&_buffer[ibuf], BUF_SIZE - ibuf, PLAYER_DATA_FMT,
        agent->status.color.r, agent->status.color.g, agent->status.color.b);
        
    GS_ASSERT(could_write > 0 && (size_t)could_write < BUF_SIZE - ibuf);
    if (could_write <= 0 || (size_t)could_write >= BUF_SIZE - ibuf) return NULL;
    ibuf += could_write;

    return _buffer;
}

bool save_player_container(ClientID client_id, int container_id)
{
    if (!Options::serializer) return true;

    ASSERT_VALID_CLIENT_ID(client_id);
    IF_INVALID_CLIENT_ID(client_id) return false;
    
    NetPeerManager* client = NetServer::get_client(client_id);
    GS_ASSERT(client != NULL);
    if (client == NULL) return false;
    GS_ASSERT(client->user_id != NULL_USER_ID);
    if (client->user_id == NULL_USER_ID) return false;

    ItemContainerType container_type = ItemContainer::get_container_type(container_id);
    const char* location_name = get_player_container_location_name(container_type);
    GS_ASSERT(location_name != NULL);
    if (location_name == NULL) return false;

    const char* container_string = write_player_container_string(container_id, client->user_id);
    GS_ASSERT(container_string != NULL);
    if (container_string == NULL) return false;

    int ret = redisAsyncCommand(ctx, NULL, NULL,
        "SET %s:%d %s", location_name, client->user_id, container_string);

    GS_ASSERT(ret == REDIS_OK);
    return (ret == REDIS_OK);
}

bool save_player(UserID user_id, AgentID agent_id)
{
    if (!Options::serializer) return true;
    
    ASSERT_VALID_USER_ID(user_id);
    IF_INVALID_USER_ID(user_id) return false;
    ASSERT_VALID_AGENT_ID(agent_id);
    IF_INVALID_AGENT_ID(agent_id) return false;
    
    const char* player_string = write_player_string(agent_id);
    GS_ASSERT(player_string != NULL);
    if (player_string == NULL) return false;
    
    int ret = redisAsyncCommand(ctx, NULL, NULL,
        "SET " PLAYER_REDIS_KEY_PREFIX "%d %s", user_id, player_string);

    GS_ASSERT(ret == REDIS_OK);
    return (ret == REDIS_OK);
}

int begin_player_load(UserID user_id, ClientID client_id, AgentID agent_id)
{
    if (!Options::serializer) return true;

    ASSERT_VALID_USER_ID(user_id);
    IF_INVALID_USER_ID(user_id) return -1;
    ASSERT_VALID_AGENT_ID(agent_id);
    IF_INVALID_AGENT_ID(agent_id) return -1;
    ASSERT_VALID_CLIENT_ID(client_id);
    IF_INVALID_CLIENT_ID(client_id) return -1;

    class PlayerLoadData* data = player_load_data_list->create();
    GS_ASSERT(data != NULL);
    if (data == NULL) return -1;
    
    data->setup_begin();
    data->user_id = user_id;
    data->agent_id = agent_id;
    data->client_id = client_id;
        
    return data->id;
}

bool load_player_container(int player_load_id, int container_id)
{
    if (!Options::serializer) return true;

    class PlayerLoadData* player_data = player_load_data_list->get(player_load_id);
    GS_ASSERT(player_data != NULL);
    if (player_data == NULL) return false;
    if (!player_data->expect_container(container_id)) return false;

    class PlayerContainerLoadData* data = player_container_load_data_list->create();
    GS_ASSERT(data != NULL);
    if (data == NULL) return false;

    ItemContainerType container_type = ItemContainer::get_container_type(container_id);
    const char* container_name = get_player_container_location_name(container_type);
    GS_ASSERT(container_name != NULL);
    if (container_name == NULL) return false;

    data->container_id = container_id;
    data->container_type = container_type;
    data->player_data_id = player_data->id;

    int ret = redisAsyncCommand(ctx, &player_container_load_cb, data,
        "GET %s:%d", container_name, player_data->user_id);
        
    GS_ASSERT(ret == REDIS_OK);
    return (ret == REDIS_OK);
}

bool end_player_load(int player_load_id)
{
    if (!Options::serializer) return true;

    class PlayerLoadData* data = player_load_data_list->get(player_load_id);
    GS_ASSERT(data != NULL);
    if (data == NULL) return false;

    GS_ASSERT(data->user_id != NULL_USER_ID);
    if (data->user_id == NULL_USER_ID) return false;

    data->setup_complete();

    // Call the player data load last, so we can destroy the player load data struct safely in its callback
    // callbacks return in-order
    int ret = redisAsyncCommand(ctx, &player_load_cb, data,
        "GET " PLAYER_REDIS_KEY_PREFIX "%d", data->user_id);
    
    GS_ASSERT(ret == REDIS_OK);
    return (ret != REDIS_OK);
}

}   // serializer