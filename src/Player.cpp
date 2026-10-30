#include <godot_cpp/variant/callable.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/input.hpp>
#include "Player.h"
#include "Bullet.h"

using namespace godot;

Player::Player()
{
    speed = 600.0f;
    health = 1000;
    camera_mode = CameraMode::make_top(0.0f);
    set_physics_process(true);
}

Player::~Player()
{
}

void Player::_bind_methods()
{

    ADD_SIGNAL(MethodInfo("take_damage", PropertyInfo(Variant::INT, "damage")));
    ClassDB::bind_method(D_METHOD("change_camera_mode", "mode", "pos"), &Player::change_camera_mode);
    ClassDB::bind_method(D_METHOD("on_take_damage", "damage"), &Player::on_take_damage);

    ClassDB::bind_method(D_METHOD("set_speed", "speed"), &Player::set_speed);
    ClassDB::bind_method(D_METHOD("get_speed"), &Player::get_speed);
    ClassDB::bind_method(D_METHOD("set_health", "health"), &Player::set_health);
    ClassDB::bind_method(D_METHOD("get_health"), &Player::get_health);
    ClassDB::bind_method(D_METHOD("set_bullet", "bullet"), &Player::set_bullet);
    ClassDB::bind_method(D_METHOD("get_bullet"), &Player::get_bullet);
    ClassDB::bind_method(D_METHOD("set_shooting_point", "shooting_point"), &Player::set_shooting_point);
    ClassDB::bind_method(D_METHOD("get_shooting_point"), &Player::get_shooting_point);

    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "speed"), "set_speed", "get_speed");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "health"), "set_health", "get_health");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "bullet",
                              PROPERTY_HINT_RESOURCE_TYPE, "PackedScene"),
                 "set_bullet", "get_bullet");
    ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "shooting_point",
                              PROPERTY_HINT_NODE_PATH_VALID_TYPES, "Marker3D"),
                 "set_shooting_point", "get_shooting_point");
}

void Player::change_camera_mode(int mode, float pos)
{
    camera_mode = (mode == 0)
                      ? CameraMode::make_top(pos)
                      : CameraMode::make_side(pos);
}

void Player::on_take_damage(int damage)
{
    int h = get_health();
    h = std::max(0, h - damage);
    set_health(h);
    if (h == 0)
    {
        queue_free();
    }
}

void Player::_ready()
{
    connect("take_damage", Callable(this, "on_take_damage"));
}

void Player::_physics_process(double delta)
{
    Input *inp = Input::get_singleton();
    Transform3D tf = get_transform();

    if (camera_mode.is_top())
    {
        if (inp->is_action_pressed("ui_up"))
            tf.origin.z -= speed * delta;
        if (inp->is_action_pressed("ui_down"))
            tf.origin.z += speed * delta;
        if (inp->is_action_pressed("ui_left"))
            tf.origin.x -= speed * delta;
        if (inp->is_action_pressed("ui_right"))
            tf.origin.x += speed * delta;
    }
    else
    {
        if (inp->is_action_pressed("ui_up"))
            tf.origin.y += speed * delta;
        if (inp->is_action_pressed("ui_down"))
            tf.origin.y -= speed * delta;
        if (inp->is_action_pressed("ui_left"))
            tf.origin.z += speed * delta;
        if (inp->is_action_pressed("ui_right"))
            tf.origin.z -= speed * delta;
    }

    set_transform(tf);

    if (inp->is_action_just_pressed("fire") && bullet_scene.is_valid())
    {
        if (Marker3D *mp = get_node<Marker3D>(shooting_point_path))
        {
            Transform3D gtf = mp->get_global_transform();
            if (Node *par = get_parent())
            {
                Node *bnode = bullet_scene->instantiate();
                if (Bullet *b = Object::cast_to<Bullet>(bnode))
                {
                    b->set_global_transform(gtf);
                    par->add_child(static_cast<Node *>(b));
                }
            }
        }
    }
}

void Player::set_speed(const float s) { speed = s; }
float Player::get_speed() const { return speed; }
void Player::set_health(const int h) { health = h; }
int Player::get_health() const { return health; }
void Player::set_bullet(const Ref<PackedScene> &b) { bullet_scene = b; }
Ref<PackedScene> Player::get_bullet() const { return bullet_scene; }
void Player::set_shooting_point(const NodePath &p) { shooting_point_path = p; }
NodePath Player::get_shooting_point() const { return shooting_point_path; }