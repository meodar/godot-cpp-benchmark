#include <godot_cpp/variant/callable.hpp>
#include <godot_cpp/classes/marker3d.hpp>
#include "Player.h"
#include "Enemy.h"
#include "Bullet.h"

using namespace godot;

void Bullet::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("on_area_entered", "area"), &Bullet::on_area_entered);

    // геттеры/сеттеры
    ClassDB::bind_method(D_METHOD("set_speed", "speed"), &Bullet::set_speed);
    ClassDB::bind_method(D_METHOD("get_speed"), &Bullet::get_speed);
    ClassDB::bind_method(D_METHOD("set_damage", "damage"), &Bullet::set_damage);
    ClassDB::bind_method(D_METHOD("get_damage"), &Bullet::get_damage);


    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "speed"), "set_speed", "get_speed");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "damage"), "set_damage", "get_damage");
}

Bullet::Bullet()
{
    speed = 1000.0f;
    damage = 300;
    set_physics_process(true);
}

Bullet::~Bullet()
{
}

void Bullet::on_area_entered(Area3D *area)
{
    if (Player *p = Object::cast_to<Player>(area))
    {
        p->emit_signal("take_damage", damage);
        queue_free();
    }
    else if (Enemy *e = Object::cast_to<Enemy>(area))
    {
        e->emit_signal("bullet_hit");
        queue_free();
    }
}

void Bullet::_ready()
{
    connect("area_entered", Callable(this, "on_area_entered"));
}

void Bullet::_physics_process(double delta)
{
    Transform3D tf = get_transform();
    Vector3 forward = tf.basis.xform_inv(Vector3(0, 0, 1));
    tf.origin += forward.normalized() * speed * static_cast<float>(delta);
    set_transform(tf);
    if (std::abs(tf.origin.z) > 1000.0f)
        queue_free();
}

void Bullet::set_speed(const float s) { speed = s; }
float Bullet::get_speed() const { return speed; }
void Bullet::set_damage(const int d) { damage = d; }
int Bullet::get_damage() const { return damage; }