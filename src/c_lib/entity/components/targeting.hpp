#pragma once

#include <entity/component/component.hpp>

// forward decl
class Agent_state;

namespace Components
{

class TargetingComponent: public Component
{
    public:
        // data
        float target_acquisition_failure_rate;
        float sight_range;

        // state
        int target_id;
        ObjectType target_type;
        Vec3 target_direction;
        bool locked_on_target;

        virtual void lock_target(Vec3 camera_position) = 0;

        virtual void broadcast_target_choice() = 0;
    
    virtual ~TargetingComponent() {}

    explicit TargetingComponent(ComponentType type)
    : Component(type, COMPONENT_INTERFACE_TARGETING),
    target_acquisition_failure_rate(0.0f), sight_range(10.0f),
    target_id(-1), target_type(OBJECT_NONE), locked_on_target(false)
    {}
};

} // Components
