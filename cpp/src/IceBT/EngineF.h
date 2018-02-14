// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

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
#ifdef ICE_CPP11_MAPPING
using BluetoothServicePtr = ::std::shared_ptr<BluetoothService>;
#else
IceUtil::Shared* upCast(BluetoothService*);
typedef IceInternal::Handle<BluetoothService> BluetoothServicePtr;
#endif

}

#endif
