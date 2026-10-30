#ifndef CAMERA_MODE_HPP
#define CAMERA_MODE_HPP

#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/variant.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

struct CameraMode
{
    enum Mode
    {
        TOP,
        SIDE
    } mode;
    float last_side_pos;
    float last_top_pos;

    /// По-умолчанию — TOP { last_side_pos = 0 }
    CameraMode()
        : mode(TOP), last_side_pos(0.f), last_top_pos(0.f)
    {
    }

    /// Фабричные методы
    static CameraMode make_top(float last_side)
    {
        CameraMode cm;
        cm.mode = TOP;
        cm.last_side_pos = last_side;
        return cm;
    }
    static CameraMode make_side(float last_top)
    {
        CameraMode cm;
        cm.mode = SIDE;
        cm.last_top_pos = last_top;
        return cm;
    }

    /// Проверки текущего режима
    bool is_top() const { return mode == TOP; }
    bool is_side() const { return mode == SIDE; }

    /// Десериализация из Variant (Dictionary)
    static CameraMode from_variant(const Variant &v);

    /// Сериализация в Variant
    Variant to_variant() const;
};

#endif // CAMERA_MODE_HPP