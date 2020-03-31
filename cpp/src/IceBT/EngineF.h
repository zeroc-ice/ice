//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_BT_ENGINE_F_H
#define ICE_BT_ENGINE_F_H

#include <IceUtil/Shared.h>
#include <Ice/Handle.h>

namespace IceBT
{

class Engine;
IceUtil::Shared* upCast(Engine*);
typedef IceInternal::Handle<Engine> EnginePtr;

class BluetoothService;
using BluetoothServicePtr = ::std::shared_ptr<BluetoothService>;

}

#endif
