// EnemyMovement.hpp
#ifndef ENEMY_MOVEMENT_HPP
#define ENEMY_MOVEMENT_HPP

namespace godot {

/// Паттерны движения врагов
enum class EnemyMovement {
    LIN  = 0,
    COS  = 1,
    SIN  = 2,
    LOG  = 3,
    SQRT = 4,
};

} // namespace godot

#endif // ENEMY_MOVEMENT_HPP