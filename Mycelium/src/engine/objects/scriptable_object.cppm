// scriptable_object.cppm
export module scriptable_object;

import object; // Import the base class module

export class ScriptableObject : public Object
{
    public:
        // Inherit constructors from Object (optional, but can be convenient)
        using Object::Object;

        // Ensure virtual destructor
        ~ScriptableObject() override = default;

        // Lifecycle methods - pure virtual
        virtual void on_create() = 0;
        virtual void on_destroy() = 0;
        virtual void on_update(float deltaTime) = 0;
        virtual void on_render() = 0;
};