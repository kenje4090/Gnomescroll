#pragma once

// TODO:

/*  Keep adding various interfaces here (t_mech, t_map, agent etc)
 *  When time is right, create client/ folder and split interfaces
 *
 */

#if DC_CLIENT

#include <c_lib/items/inventory/base.hpp>
#include <c_lib/items/inventory/contents.hpp>
#include <c_lib/objects/common/interface/policy.hpp>
#include <c_lib/items/constants.hpp>

int get_icon_spritesheet_id(Object_types type, int subtype)
{
    const int ERROR_SPRITE = 0;
    switch (type)
    {
        case OBJ_TYPE_REFILL:
        case OBJ_TYPE_BLOCK_DROP:
        case OBJ_TYPE_GEMSTONE:
            return ItemDrops::get_object_type_sprite_index(type, subtype);

        default: return ERROR_SPRITE;
    }
}

class InventoryProperties;

class InventoryContents: public BaseInventoryContents<InventoryProperties>
{};

typedef BaseInventory<InventoryContents> BaseInventoryClient;

class Inventory;
class InventoryProperties: public BaseInventoryProperties, public SpriteProperties
{
    public:
        Inventory* inventory;
        float spacing; // render icon spacing

    void load(int id, Object_types type, int subtype);
    void get_sprite_data(struct Draw::SpriteData* data);
        
    InventoryProperties()
    :
    BaseInventoryProperties(), SpriteProperties(),
    inventory(NULL), spacing(32.0f)
    {
        this->scale = 2.0f;
    }
};

class Inventory: public BaseInventoryClient
{
    public:

    int selected_slot;

    InventoryProperties* selected_item()
    {
        if (this->selected_slot < 0) return NULL;
        return get_slot_item(this->selected_slot);
    }
    
    InventoryProperties* get_slot_item(int slot)
    {
        if (!this->contents.is_valid_slot(slot)) return NULL;
        return &this->contents.objects[slot];
    }

    void select_slot(int slot)
    {
        if (slot == this->selected_slot)
        {   // unselect
            this->unselect_slot();
            return;
        }
        if (this->selected_slot > 0)
        {   // attempt swap
            this->swap_action(this->selected_slot, slot);
            this->unselect_slot();
            return;
        }

        // only select if not empty
        InventoryProperties* item = this->get_slot_item(slot);
        if (item != NULL && !item->empty())
            this->selected_slot = slot;
        
    }

    void select_slot(int row, int col)
    {
        int slot = row * this->contents.x + col;
        this->select_slot(slot);
    }

    void unselect_slot()
    {
        this->selected_slot = -1;
    }
    
    bool selected()
    {
        //static int t = 0;
        //t++;
        //if (t % 5 == 0) this->selected_slot++;
        //this->selected_slot %= this->contents.max;
        //printf("slot %d\n", this->selected_slot);
        return (this->selected_slot >= 0);
    }

    void get_selected_icon_render_data(Draw::SpriteData* data);

    void remove_selected_action()
    {
        this->remove_action(this->selected_slot);
    }

    void remove_action(int slot)
    {
        bool can_remove = this->contents.can_remove(slot);
        if (!can_remove) return;
        remove_item_from_inventory_CtoS msg;
        msg.inventory_id = this->_state.id;
        msg.slot = slot;
        msg.send();
    }

    void add_action(int id, Object_types type, int subtype)
    {
        int slot = this->contents.get_empty_slot();
        this->add_action(id, type, subtype, slot);
    }
    
    void add_action(int id, Object_types type, int subtype, int slot)
    {
        bool can_add = this->can_add(type, slot);
        if (!can_add) return;
        add_item_to_inventory_CtoS msg;
        msg.inventory_id = this->_state.id;
        msg.id = id;
        msg.type = type;
        msg.subtype = subtype;
        msg.slot = slot;
        msg.send();
    }

    bool swap_action(int slota, int slotb)
    {
        bool swapped = this->can_swap(slota, slotb);
        if (!swapped) return false;
        swap_item_in_inventory_CtoS msg;
        msg.inventory_id = this->_state.id;
        msg.slota = slota;
        msg.slotb = slotb;
        msg.send();
        return swapped;
    }

    void select_and_remove_action(int row, int col)
    {   // removes item at input-selected row,col
        this->unselect_slot();
        int slot = row * this->contents.x + col;
        this->remove_action(slot);
    }

    /* Expose API here */
    bool dimension_mismatch(int x, int y)
    {
        return BaseInventoryClient::dimension_mismatch(x,y);
    }
    bool can_add(Object_types type)
    {
        return BaseInventoryClient::can_add(type);
    }
    bool can_add(Object_types type, int slot)
    {
        return BaseInventoryClient::can_add(type, slot);
    }
    bool add(int id, Object_types type, int subtype, int slot)
    {
        return BaseInventoryClient::add(id, type, subtype, slot);
    }
    bool remove(int slot)
    {
        return BaseInventoryClient::remove(slot);
    }
    bool swap(int slota, int slotb)
    {
        return BaseInventoryClient::swap(slota, slotb);
    }
    void init(int x, int y)
    {
        BaseInventoryClient::init(x,y);
        for (int i=0; i<this->contents.max; i++)
            this->contents.objects[i].inventory = this;
    }

    DUMMY_NETWORK_INTERFACE // to conform to object api

    explicit Inventory(int id);
};

#endif
