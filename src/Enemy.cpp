#include "Enemy.h"
#include <godot_cpp/classes/timer.hpp>
#include <godot_cpp/classes/marker3d.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include "Bullet.h"
#include "EnemyType.hpp"
#include "EnemyMovement.hpp"
#include <godot_cpp/classes/class_db_singleton.hpp>
#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/packed_float32_array.hpp>
#include <godot_cpp/variant/packed_int32_array.hpp>

#include <cmath>
#include <vector>

using namespace godot;

namespace
{
    template <typename Packed>
    auto rust_table(const char *name)
    {
        const Packed packed = ClassDBSingleton::get_singleton()->class_call_static("BenchmarkRandomTable", name);
        CRASH_COND_MSG(packed.is_empty(), String("BenchmarkRandomTable.") + name + " пуст: Rust-хелпер не загружен.");
        return std::vector(packed.ptr(), packed.ptr() + packed.size());
    }

    const std::vector<int32_t> &workload_table()
    {
        static const auto table = rust_table<PackedInt32Array>("workload_10k");
        return table;
    }

    const std::vector<float> &movement_mod_table()
    {
        static const auto table = rust_table<PackedFloat32Array>("movement_mod");
        return table;
    }

    const std::vector<uint8_t> &movement_kind_table()
    {
        static const auto table = rust_table<PackedByteArray>("movement_kind");
        return table;
    }
}

void Enemy::_bind_methods()
{
    ADD_SIGNAL(MethodInfo("bullet_hit"));
    ClassDB::bind_method(D_METHOD("on_bullet_hit"), &Enemy::on_bullet_hit);
    ClassDB::bind_method(D_METHOD("on_shoot_timeout"), &Enemy::on_shoot_timeout);

    // геттеры/сеттеры
    ClassDB::bind_method(D_METHOD("set_speed", "speed"), &Enemy::set_speed);
    ClassDB::bind_method(D_METHOD("get_speed"), &Enemy::get_speed);
    ClassDB::bind_method(D_METHOD("set_enemy_type", "enemy_type"), &Enemy::set_enemy_type);
    ClassDB::bind_method(D_METHOD("get_enemy_type"), &Enemy::get_enemy_type);
    ClassDB::bind_method(D_METHOD("set_enemy_movement", "enemy_movement"), &Enemy::set_enemy_movement);
    ClassDB::bind_method(D_METHOD("get_enemy_movement"), &Enemy::get_enemy_movement);
    ClassDB::bind_method(D_METHOD("set_movement_mod", "movement_mod"), &Enemy::set_movement_mod);
    ClassDB::bind_method(D_METHOD("get_movement_mod"), &Enemy::get_movement_mod);
    ClassDB::bind_method(D_METHOD("set_shooting_timer", "shooting_timer"), &Enemy::set_shooting_timer);
    ClassDB::bind_method(D_METHOD("get_shooting_timer"), &Enemy::get_shooting_timer);
    ClassDB::bind_method(D_METHOD("set_bullet", "bullet"), &Enemy::set_bullet);
    ClassDB::bind_method(D_METHOD("get_bullet"), &Enemy::get_bullet);
    ClassDB::bind_method(D_METHOD("set_shooting_point", "shooting_point"), &Enemy::set_shooting_point);
    ClassDB::bind_method(D_METHOD("get_shooting_point"), &Enemy::get_shooting_point);

    // свойства
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "speed"), "set_speed", "get_speed");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "enemy_type", PROPERTY_HINT_ENUM, "Top,Side"),
                 "set_enemy_type", "get_enemy_type");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "enemy_movement", PROPERTY_HINT_ENUM, "Lin,Cos,Sin,Log,Sqrt"),
                 "set_enemy_movement", "get_enemy_movement");
    ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "movement_mod"), "set_movement_mod", "get_movement_mod");
    ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "shooting_timer"), "set_shooting_timer", "get_shooting_timer");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "bullet",
                              PROPERTY_HINT_RESOURCE_TYPE, "PackedScene"),
                 "set_bullet", "get_bullet");
    ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "shooting_point"), "set_shooting_point", "get_shooting_point");
}

Enemy::Enemy()
    : speed(200.0f), enemy_type(EnemyType::TOP), enemy_movement(EnemyMovement::LIN), movement_mod(1, 1, 1), time_elapsed(0.0)
{
    set_physics_process(true);

    const size_t spawn_index = static_cast<size_t>(int64_t(
        ClassDBSingleton::get_singleton()->class_call_static("BenchmarkRandomTable", "next_spawn_index")));

    // тип движения (0..4)
    const std::vector<uint8_t> &kinds = movement_kind_table();
    enemy_movement = static_cast<EnemyMovement>(kinds[spawn_index & (kinds.size() - 1)]);

    const std::vector<float> &mods = movement_mod_table();
    const size_t mod_mask = mods.size() - 1;
    movement_mod.x = mods[(spawn_index * 3) & mod_mask];
    movement_mod.y = mods[(spawn_index * 3 + 1) & mod_mask];
    movement_mod.z = mods[(spawn_index * 3 + 2) & mod_mask];

    workload_cursor = spawn_index;
}

void Enemy::on_bullet_hit()
{
    queue_free();
}

void Enemy::on_shoot_timeout()
{
    if (Marker3D *mp = get_node<Marker3D>(shooting_point_path))
    {
        Transform3D tf = mp->get_global_transform();
        if (Node *parent = get_parent())
        {
            Node *bnode = bullet_scene->instantiate();
            if (Bullet *b = Object::cast_to<Bullet>(bnode))
            {
                b->set_global_transform(tf);
                parent->add_child(static_cast<Node *>(b));
            }
        }
    }
}

void Enemy::_ready()
{
    if (Timer *t = get_node<Timer>(shooting_timer_path))
    {
        t->connect("timeout", Callable(this, "on_shoot_timeout"));
    }
    connect("bullet_hit", Callable(this, "on_bullet_hit"));
}

void Enemy::_physics_process(double delta)
{
    time_elapsed += delta;
    float t = static_cast<float>(time_elapsed);
    float dz = speed * static_cast<float>(delta);

    double sum = 0;
    const std::vector<int32_t> &table = workload_table();
    const int32_t *workload = table.data();
    const size_t mask = table.size() - 1;
    size_t cursor = workload_cursor;
    for (int i = 0; i < 7000; ++i)
    {
        int l = workload[cursor++ & mask];
        sum += std::sqrt(l * 3.1415) * std::cos(l * 0.01);
    }
    workload_cursor = cursor;

    if (sum < 0)
        UtilityFunctions::printerr("weird");

    Transform3D tf = get_transform();
    float a = movement_mod.x, b = movement_mod.y, c = movement_mod.z;

    switch (enemy_movement)
    {
    case EnemyMovement::LIN:
        tf.origin.z += dz;
        break;
    case EnemyMovement::COS:
    {
        float phase = a * std::cos(b * t + c) * 500.0f;
        if (enemy_type == EnemyType::TOP)
            tf.origin.x = phase;
        else
            tf.origin.y = phase;
        tf.origin.z += dz;
    }
    break;
    case EnemyMovement::SIN:
    {
        float phase = a * std::sin(b * t + c) * 500.0f;
        if (enemy_type == EnemyType::TOP)
            tf.origin.x = phase;
        else
            tf.origin.y = phase;
        tf.origin.z += dz;
    }
    break;
    case EnemyMovement::LOG:
    {
        float new_z = tf.origin.z + dz;
        float v = a * std::logf(std::max(1.0f, std::abs(new_z)) + c) * 200.0f;
        if (enemy_type == EnemyType::TOP)
            tf.origin.x = v;
        else
            tf.origin.y = v;
        tf.origin.z = new_z;
    }
    break;
    case EnemyMovement::SQRT:
    {
        float new_z = tf.origin.z + dz;
        float v = a * (std::sqrtf(std::abs(new_z)) + c) * 100.0f;
        if (enemy_type == EnemyType::TOP)
            tf.origin.x = v;
        else
            tf.origin.y = v;
        tf.origin.z = new_z;
    }
    break;
    }

    set_transform(tf);
    if (tf.origin.z >= 800.0f)
        queue_free();
}

// Геттеры/сеттеры…
void Enemy::set_speed(const float s) { speed = s; }
float Enemy::get_speed() const { return speed; }
void Enemy::set_enemy_type(const int et) { enemy_type = static_cast<EnemyType>(et); }
int Enemy::get_enemy_type() const { return int(enemy_type); }
void Enemy::set_enemy_movement(const int em) { enemy_movement = static_cast<EnemyMovement>(em); }
int Enemy::get_enemy_movement() const { return int(enemy_movement); }
void Enemy::set_movement_mod(const Vector3 m) { movement_mod = m; }
Vector3 Enemy::get_movement_mod() const { return movement_mod; }
void Enemy::set_shooting_timer(const NodePath &p) { shooting_timer_path = p; }
NodePath Enemy::get_shooting_timer() const { return shooting_timer_path; }
void Enemy::set_bullet(const Ref<PackedScene> &b) { bullet_scene = b; }
Ref<PackedScene> Enemy::get_bullet() const { return bullet_scene; }
void Enemy::set_shooting_point(const NodePath &p) { shooting_point_path = p; }
NodePath Enemy::get_shooting_point() const { return shooting_point_path; }