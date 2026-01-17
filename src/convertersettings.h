#pragma once

#include <QMetaType>
#include <QObject>

namespace ConverterSettings {
Q_NAMESPACE

enum Setting : uint32_t
{
    // String settings
    DefaultCodec   = 5 << 28 | 1,  // Settings::String

    // Int settings
    WindowWidth    = 2 << 28 | 2,  // Settings::Int
    WindowHeight   = 2 << 28 | 3,  // Settings::Int
};

Q_ENUM_NS(Setting)
}

Q_DECLARE_METATYPE(ConverterSettings::Setting)
