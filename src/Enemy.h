#pragma once

#include <godot_cpp/classes/area3d.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/transform3d.hpp>
#include <godot_cpp/variant/node_path.hpp>
#include <godot_cpp/core/binder_common.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <cstddef>

#include "EnemyType.hpp"
#include "EnemyMovement.hpp"

using namespace godot;

class Enemy : public Area3D
{
    GDCLASS(Enemy, Area3D);

    float speed;
    EnemyType enemy_type;
    EnemyMovement enemy_movement;
    Vector3 movement_mod;
    double time_elapsed;

    NodePath shooting_timer_path;
    Ref<PackedScene> bullet_scene;
    NodePath shooting_point_path;

    /// Позиция чтения в таблице чисел для горячего цикла.
    size_t workload_cursor;

protected:
    static void _bind_methods();

public:
    Enemy();

    // только объявляем, реализация в Enemy.cpp
    void on_bullet_hit();
    void on_shoot_timeout();

    void _ready() override;
    void _physics_process(double delta) override;
    
    // геттеры/сеттеры …
    void set_speed(float s);
    float get_speed() const;

    void set_enemy_type(int et);
    int get_enemy_type() const;

    void set_enemy_movement(int em);
    int get_enemy_movement() const;

    void set_movement_mod(Vector3 m);
    Vector3 get_movement_mod() const;

    void set_shooting_timer(const NodePath &p);
    NodePath get_shooting_timer() const;

    void set_bullet(const Ref<PackedScene> &b);
    Ref<PackedScene> get_bullet() const;

    void set_shooting_point(const NodePath &p);
    NodePath get_shooting_point() const;
};