#include "EnemiesManager.h"
#include "Enemy.h"
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/core/memory.hpp>
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
        static const auto table = rust_table<PackedInt32Array>("workload_500");
        return table;
    }

    const std::vector<float> &spawn_x_table()
    {
        static const auto table = rust_table<PackedFloat32Array>("spawn_x");
        return table;
    }

    const std::vector<float> &spawn_y_table()
    {
        static const auto table = rust_table<PackedFloat32Array>("spawn_y");
        return table;
    }

    const std::vector<uint8_t> &spawn_top_table()
    {
        static const auto table = rust_table<PackedByteArray>("spawn_top");
        return table;
    }

    const std::vector<uint8_t> &heavy_tick_table()
    {
        static const auto table = rust_table<PackedByteArray>("heavy_tick");
        return table;
    }
}

void EnemiesManager::_bind_methods()
{
    // --- top_enemy_scene ---
    ClassDB::bind_method(D_METHOD("set_top_enemy_scene", "scene"),
                         &EnemiesManager::set_top_enemy_scene);
    ClassDB::bind_method(D_METHOD("get_top_enemy_scene"),
                         &EnemiesManager::get_top_enemy_scene);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "top_enemy_scene",
                              PROPERTY_HINT_RESOURCE_TYPE, "PackedScene"),
                 "set_top_enemy_scene", "get_top_enemy_scene");

    // --- side_enemy_scene ---
    ClassDB::bind_method(D_METHOD("set_side_enemy_scene", "scene"),
                         &EnemiesManager::set_side_enemy_scene);
    ClassDB::bind_method(D_METHOD("get_side_enemy_scene"),
                         &EnemiesManager::get_side_enemy_scene);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "side_enemy_scene",
                              PROPERTY_HINT_RESOURCE_TYPE, "PackedScene"),
                 "set_side_enemy_scene", "get_side_enemy_scene");

    // --- spawn_timer_path ---
    ClassDB::bind_method(D_METHOD("set_spawn_timer_path", "path"),
                         &EnemiesManager::set_spawn_timer_path);
    ClassDB::bind_method(D_METHOD("get_spawn_timer_path"),
                         &EnemiesManager::get_spawn_timer_path);
    ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "spawn_timer_path",
                              PROPERTY_HINT_NODE_PATH_VALID_TYPES, "Timer"),
                 "set_spawn_timer_path", "get_spawn_timer_path");

    // --- benchmark_timer_path ---
    ClassDB::bind_method(D_METHOD("set_benchmark_timer_path", "path"),
                         &EnemiesManager::set_benchmark_timer_path);
    ClassDB::bind_method(D_METHOD("get_benchmark_timer_path"),
                         &EnemiesManager::get_benchmark_timer_path);
    ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "benchmark_timer_path",
                              PROPERTY_HINT_NODE_PATH_VALID_TYPES, "Timer"),
                 "set_benchmark_timer_path", "get_benchmark_timer_path");

    // --- benchmark_value ---
    ClassDB::bind_method(D_METHOD("set_benchmark_value", "value"),
                         &EnemiesManager::set_benchmark_value);
    ClassDB::bind_method(D_METHOD("get_benchmark_value"),
                         &EnemiesManager::get_benchmark_value);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "benchmark_value"),
                 "set_benchmark_value", "get_benchmark_value");

    // --- методы спавна и коллбэки ---
    ClassDB::bind_method(D_METHOD("spawn_enemy", "is_top"),
                         &EnemiesManager::spawn_enemy);
    ClassDB::bind_method(D_METHOD("_on_spawn_timeout"),
                         &EnemiesManager::_on_spawn_timeout);
    ClassDB::bind_method(D_METHOD("_on_benchmark_timeout"),
                         &EnemiesManager::_on_benchmark_timeout);
}

EnemiesManager::EnemiesManager()
    : benchmark_value(10), // начальное значение из Rust: 10
      benchmark_sum(0.0f),
      spawn_cursor(0),
      side_cursor(0),
      tick_cursor(0),
      workload_cursor(0)
{
    // включаем физ-процесс (для обработки input)
    set_physics_process(true);
}

void EnemiesManager::set_top_enemy_scene(const Ref<PackedScene> &s) { top_enemy_scene = s; }
Ref<PackedScene> EnemiesManager::get_top_enemy_scene() const { return top_enemy_scene; }
void EnemiesManager::set_side_enemy_scene(const Ref<PackedScene> &s) { side_enemy_scene = s; }
Ref<PackedScene> EnemiesManager::get_side_enemy_scene() const { return side_enemy_scene; }

void EnemiesManager::set_spawn_timer_path(const NodePath &p) { spawn_timer_path = p; }
NodePath EnemiesManager::get_spawn_timer_path() const { return spawn_timer_path; }

void EnemiesManager::set_benchmark_timer_path(const NodePath &p) { benchmark_timer_path = p; }
NodePath EnemiesManager::get_benchmark_timer_path() const { return benchmark_timer_path; }

void EnemiesManager::set_benchmark_value(int v) { benchmark_value = v; }
int EnemiesManager::get_benchmark_value() const { return benchmark_value; }

void EnemiesManager::_ready()
{
    // подключаем спавн-таймер через get_node<Timer>
    if (Timer *t = get_node<Timer>(spawn_timer_path))
    {
        t->connect("timeout", Callable(this, "_on_spawn_timeout"));
    }
    else
    {
        UtilityFunctions::printerr("EnemiesManager: spawn_timer not found at ",
                                   spawn_timer_path);
    }

    // подключаем benchmark-таймер через get_node<Timer>
    if (Timer *bt = get_node<Timer>(benchmark_timer_path))
    {
        bt->connect("timeout", Callable(this, "_on_benchmark_timeout"));
    }
    else
    {
        UtilityFunctions::printerr("EnemiesManager: benchmark_timer not found at ",
                                   benchmark_timer_path);
    }
}

void EnemiesManager::_physics_process(double /*delta*/)
{
    if (auto *in = Input::get_singleton();
        in && in->is_action_just_pressed("benchmark_spawn"))
    {
        // спавним benchmark_value top и benchmark_value side
        for (int i = 0; i < benchmark_value; i++)
            spawn_enemy(true);
        for (int i = 0; i < benchmark_value; i++)
            spawn_enemy(false);
    }

    const std::vector<uint8_t> &ticks = heavy_tick_table();
    const bool heavy_tick = ticks[tick_cursor++ & (ticks.size() - 1)] != 0;

    if (heavy_tick)
    {
        float acc = 0.0f;
        std::vector<float> local_data;
        const std::vector<int32_t> &table = workload_table();
        const int32_t *workload = table.data();
        const size_t mask = table.size() - 1;
        size_t cursor = workload_cursor;
        for (int i = 0; i < 500; ++i)
        {
            int l = workload[cursor++ & mask];
            float x = std::sin(l * 0.0174533f) * std::sqrt(l + 1.0f);
            acc += std::pow(x, 1.5f);
            local_data.push_back(acc);
        }
        workload_cursor = cursor;
        benchmark_sum = acc;
    }
}

void EnemiesManager::_on_spawn_timeout()
{
    const std::vector<uint8_t> &sides = spawn_top_table();
    bool is_top = sides[side_cursor++ & (sides.size() - 1)] != 0;
    spawn_enemy(is_top);
}

void EnemiesManager::_on_benchmark_timeout()
{
    // тот же код, что и на нажатие
    for (int i = 0; i < benchmark_value; i++)
        spawn_enemy(true);
    for (int i = 0; i < benchmark_value; i++)
        spawn_enemy(false);
    // удваиваем для следующего раза
    benchmark_value += 10;
}

void EnemiesManager::spawn_enemy(bool is_top)
{
    Node *parent = get_parent();
    if (!parent)
    {
        UtilityFunctions::printerr("EnemiesManager: no parent");
        return;
    }

    Ref<PackedScene> scene = is_top ? top_enemy_scene : side_enemy_scene;
    if (scene.is_null())
    {
        UtilityFunctions::printerr("EnemiesManager: scene is null");
        return;
    }

    Node *node = scene->instantiate();
    auto *enemy = Object::cast_to<Enemy>(node);
    if (!enemy)
    {
        UtilityFunctions::printerr("EnemiesManager: wrong type");
        return;
    }

    Transform3D tf = enemy->get_transform();
    if (is_top)
    {
        tf.origin.x = spawn_x_table()[spawn_cursor & (spawn_x_table().size() - 1)];
        tf.origin.y = 0.0f;
    }
    else
    {
        tf.origin.x = 0.0f;
        tf.origin.y = spawn_y_table()[spawn_cursor & (spawn_y_table().size() - 1)];
    }
    ++spawn_cursor;
    tf.origin.z = -800.0f;
    enemy->set_transform(tf);

    parent->add_child(enemy);
}
