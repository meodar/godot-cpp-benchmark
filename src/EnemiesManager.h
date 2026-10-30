#ifndef ENEMIES_MANAGER_HPP
#define ENEMIES_MANAGER_HPP

#include <godot_cpp/core/binder_common.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/classes/timer.hpp>
#include <godot_cpp/variant/node_path.hpp>

#include <cstddef>

using namespace godot;

class EnemiesManager : public Node {
    GDCLASS(EnemiesManager, Node);

private:
    // PackedScene для спавна врагов
    Ref<PackedScene> top_enemy_scene;
    Ref<PackedScene> side_enemy_scene;

    // Пути до таймеров в иерархии
    NodePath spawn_timer_path;
    NodePath benchmark_timer_path;

    // Значения для benchmark
    int benchmark_value;
    float benchmark_sum;

    // Позиции чтения в таблицах предгенерированных чисел
    size_t spawn_cursor;
    size_t side_cursor;
    size_t tick_cursor;
    size_t workload_cursor;

protected:
    static void _bind_methods();

public:
    EnemiesManager();

    // Спавн сцен
    void set_top_enemy_scene(const Ref<PackedScene> &s);
    Ref<PackedScene> get_top_enemy_scene() const;
    void set_side_enemy_scene(const Ref<PackedScene> &s);
    Ref<PackedScene> get_side_enemy_scene() const;

    // Таймеры
    void set_spawn_timer_path(const NodePath &p);
    NodePath get_spawn_timer_path() const;
    void set_benchmark_timer_path(const NodePath &p);
    NodePath get_benchmark_timer_path() const;

    // Benchmark value
    void set_benchmark_value(int v);
    int  get_benchmark_value() const;

    // Спавн врага
    void spawn_enemy(bool is_top);

    // Методы-коллбэки таймеров
    void _on_spawn_timeout();
    void _on_benchmark_timeout();

    // Жизненный цикл
    void _ready() override;
    void _physics_process(double delta) override;
};

#endif // ENEMIES_MANAGER_HPP