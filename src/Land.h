#pragma once

#include <godot_cpp/core/binder_common.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/node_path.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/camera3d.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/transform3d.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include "CameraMode.hpp"
#include "Player.h"

using namespace godot;

class Land : public Node {
    GDCLASS(Land, Node);

private:
    NodePath land_path;
    NodePath second_land_path;
    NodePath camera_path;
    NodePath player_path;
    CameraMode camera_mode;

protected:
    static void _bind_methods();

public:
    Land();

    void    set_land_path(const NodePath &p);
    NodePath get_land_path() const;

    void    set_second_land_path(const NodePath &p);
    NodePath get_second_land_path() const;

    void    set_camera_path(const NodePath &p);
    NodePath get_camera_path() const;

    void    set_player_path(const NodePath &p);
    NodePath get_player_path() const;

    void _ready() override;
    void _physics_process(double delta) override;

private:
    void update_terrain(Node3D *terrain, double delta);
};
