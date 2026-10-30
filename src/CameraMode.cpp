#include "CameraMode.hpp"

CameraMode CameraMode::from_variant(const Variant &v)
{
    CameraMode cm;
    // Проверяем, что Variant — словарь
    if (v.get_type() != Variant::DICTIONARY)
    {
        UtilityFunctions::printerr("CameraMode::from_variant: expected Dictionary");
        return cm;
    }

    Dictionary dict = v;
    // Проверяем ключи
    if (!dict.has("mode"))
    {
        UtilityFunctions::printerr("CameraMode::from_variant: missing key 'mode'");
        return cm;
    }
    if (!dict.has("pos"))
    {
        UtilityFunctions::printerr("CameraMode::from_variant: missing key 'pos'");
        return cm;
    }

    // Читаем строку mode
    String mode_str = dict["mode"];
    // Читаем число pos
    double pos_d = dict["pos"];
    float pos = static_cast<float>(pos_d);

    if (mode_str == "top")
    {
        cm.mode = CameraMode::TOP;
        cm.last_side_pos = pos;
    }
    else if (mode_str == "side")
    {
        cm.mode = CameraMode::SIDE;
        cm.last_top_pos = pos;
    }
    else
    {
        UtilityFunctions::printerr("CameraMode::from_variant: unknown variant '" + mode_str + "'");
    }

    return cm;
}

Variant CameraMode::to_variant() const
{
    Dictionary dict;
    if (mode == CameraMode::TOP)
    {
        dict["mode"] = String("top");
        dict["pos"] = static_cast<double>(last_side_pos);
    }
    else
    {
        dict["mode"] = String("side");
        dict["pos"] = static_cast<double>(last_top_pos);
    }
    return Variant(dict);
}