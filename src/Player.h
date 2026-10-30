#pragma once

#include <godot_cpp/core/binder_common.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/area3d.hpp>
#include <godot_cpp/variant/node_path.hpp>
#include <godot_cpp/variant/transform3d.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/classes/marker3d.hpp>

#include "CameraMode.hpp"

using namespace godot;

class Player : public Area3D
{
    GDCLASS(Player, Area3D);

private:
    float speed;
    int health;
    Ref<PackedScene> bullet_scene;
    NodePath shooting_point_path;
    CameraMode camera_mode;

protected:
    static void _bind_methods();

public:
    Player();
    ~Player();

    void change_camera_mode(int mode, float pos);
    void on_take_damage(int damage);
    void _ready() override;
    void _physics_process(double delta) override;

    void set_speed(float s);
    float get_speed() const;
    void set_health(int h);
    int get_health() const;
    void set_bullet(const Ref<PackedScene> &b);
    Ref<PackedScene> get_bullet() const;
    void set_shooting_point(const NodePath &p);
    NodePath get_shooting_point() const;
};