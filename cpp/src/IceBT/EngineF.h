// Copyright (c) ZeroC, Inc.

#ifndef ICE_BT_ENGINE_F_H
#define ICE_BT_ENGINE_F_H

#include <memory>

namespace IceBT
{
    class Engine;
    using EnginePtr = std::shared_ptr<Engine>;

    class BluetoothService;
    using BluetoothServicePtr = std::shared_ptr<BluetoothService>;
}

#endif
