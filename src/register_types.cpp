#include "register_types.h"

#include "player.h"
#include "Bullet.h"
#include "Enemy.h"
#include "EnemiesManager.h"
#include "Land.h"

#include <gdextension_interface.h>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>

using namespace godot;

void initialize_benchmark_module(ModuleInitializationLevel p_level)
{
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE)
    {
        return;
    }

    GDREGISTER_RUNTIME_CLASS(Player);
    GDREGISTER_RUNTIME_CLASS(Bullet);
    GDREGISTER_RUNTIME_CLASS(Enemy);
    GDREGISTER_RUNTIME_CLASS(EnemiesManager);
    GDREGISTER_RUNTIME_CLASS(Land);
}

void uninitialize_benchmark_module(ModuleInitializationLevel p_level)
{
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE)
    {
        return;
    }
}

extern "C"
{
    GDExtensionBool GDE_EXPORT gdext_cpp_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, const GDExtensionClassLibraryPtr p_library, GDExtensionInitialization *r_initialization)
    {
        godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

        init_obj.register_initializer(initialize_benchmark_module);
        init_obj.register_terminator(uninitialize_benchmark_module);
        init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

        return init_obj.init();
    }
}