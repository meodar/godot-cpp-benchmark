#include "Land.h"
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/core/math.hpp>
#include <godot_cpp/classes/class_db_singleton.hpp>

using namespace godot;

Land::Land() : camera_mode()
{
    // включаем физ. процесс
    set_physics_process(true);
}

void Land::_bind_methods()
{
    // экспортируем NodePath’ы
    ClassDB::bind_method(D_METHOD("set_land_path", "path"), &Land::set_land_path);
    ClassDB::bind_method(D_METHOD("get_land_path"), &Land::get_land_path);
    ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "land",
                              PROPERTY_HINT_NODE_PATH_VALID_TYPES, "Node3D"),
                 "set_land_path", "get_land_path");

    ClassDB::bind_method(D_METHOD("set_second_land_path", "path"), &Land::set_second_land_path);
    ClassDB::bind_method(D_METHOD("get_second_land_path"), &Land::get_second_land_path);
    ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "second_land",
                              PROPERTY_HINT_NODE_PATH_VALID_TYPES, "Node3D"),
                 "set_second_land_path", "get_second_land_path");

    ClassDB::bind_method(D_METHOD("set_camera_path", "path"), &Land::set_camera_path);
    ClassDB::bind_method(D_METHOD("get_camera_path"), &Land::get_camera_path);
    ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "camera",
                              PROPERTY_HINT_NODE_PATH_VALID_TYPES, "Camera3D"),
                 "set_camera_path", "get_camera_path");

    ClassDB::bind_method(D_METHOD("set_player_path", "path"), &Land::set_player_path);
    ClassDB::bind_method(D_METHOD("get_player_path"), &Land::get_player_path);
    ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "player",
                              PROPERTY_HINT_NODE_PATH_VALID_TYPES, "Player"),
                 "set_player_path", "get_player_path");

    // сигнал camera_view_changed(mode: Dictionary)
    ADD_SIGNAL(MethodInfo(
        "camera_view_changed",
        PropertyInfo(Variant::INT, "mode"),
        PropertyInfo(Variant::FLOAT, "pos")));
}

// Сеттеры / геттеры
void Land::set_land_path(const NodePath &p) { land_path = p; }
NodePath Land::get_land_path() const { return land_path; }
void Land::set_second_land_path(const NodePath &p) { second_land_path = p; }
NodePath Land::get_second_land_path() const { return second_land_path; }
void Land::set_camera_path(const NodePath &p) { camera_path = p; }
NodePath Land::get_camera_path() const { return camera_path; }
void Land::set_player_path(const NodePath &p) { player_path = p; }
NodePath Land::get_player_path() const { return player_path; }

void Land::_ready()
{
    ClassDBSingleton::get_singleton()->class_call_static("BenchmarkRandomTable", "warm_up");

    if (Player *p = get_node<Player>(player_path))
    {
        connect("camera_view_changed",
                Callable(p, "change_camera_mode"));
    }
}

void Land::_physics_process(double delta)
{
    if (Node3D *t = get_node<Node3D>(land_path))
        update_terrain(t, delta);
    if (Node3D *t2 = get_node<Node3D>(second_land_path))
        update_terrain(t2, delta);

    Input *in = Input::get_singleton();
    if (in && in->is_action_just_pressed("camera_change"))
    {
        Camera3D *cam = get_node<Camera3D>(camera_path);
        Player *p = get_node<Player>(player_path);
        if (!cam || !p)
            return;

        Transform3D tf = cam->get_transform();
        float last_pos = 0.0f;

        const real_t a90 = Math::PI / 2.0;

        if (camera_mode.mode == CameraMode::TOP)
        {
            tf.basis = tf.basis
                           .rotated(Vector3(1, 0, 0), a90)
                           .rotated(Vector3(0, 1, 0), a90);
            tf.origin += Vector3(1000.0f, -1000.0f, 0.0f);
            cam->set_transform(tf);

            Transform3D ptf = p->get_transform();
            last_pos = ptf.origin.x;
            ptf.origin = Vector3(0.0f, camera_mode.last_side_pos, ptf.origin.z);
            p->set_transform(ptf);

            camera_mode.mode = CameraMode::SIDE;
            camera_mode.last_top_pos = last_pos;
        }
        else
        {
            tf.basis = tf.basis
                           .rotated(Vector3(0, 1, 0), -a90)
                           .rotated(Vector3(1, 0, 0), -a90);
            tf.origin += Vector3(-1000.0f, 1000.0f, 0.0f);
            cam->set_transform(tf);

            Transform3D ptf = p->get_transform();
            last_pos = ptf.origin.y;
            ptf.origin = Vector3(camera_mode.last_top_pos, 0.0f, ptf.origin.z);
            p->set_transform(ptf);

            camera_mode.mode = CameraMode::TOP;
            camera_mode.last_side_pos = last_pos;
        }
        int mode = (camera_mode.mode == CameraMode::TOP) ? 0 : 1;
        float pos = (camera_mode.mode == CameraMode::TOP)
                        ? camera_mode.last_side_pos
                        : camera_mode.last_top_pos;

        emit_signal("camera_view_changed", mode, pos);
    }
}

void Land::update_terrain(Node3D *terrain, double delta)
{
    Transform3D tf = terrain->get_transform();
    tf.origin.z += 80.0f * float(delta);
    if (tf.origin.z >= 32000.0f)
    {
        tf.origin.z -= 64000.0f;
    }
    terrain->set_transform(tf);
}