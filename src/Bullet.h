#pragma once

#include <godot_cpp/classes/area3d.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

class Bullet : public Area3D
{
    GDCLASS(Bullet, Area3D);

    float speed;
    int damage;

protected:
    static void _bind_methods();

public:
    Bullet();
    ~Bullet();
    void on_area_entered(Area3D *area);
    void _ready() override;
    void _physics_process(double delta) override;

    void set_speed(float s);
    float get_speed() const;
    void set_damage(int d);
    int get_damage() const;
};