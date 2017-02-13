// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef CUSTOM_MAP_H
#define CUSTOM_MAP_H

#include <IceUtil/Config.h>
#include <map>

namespace Test
{

template<typename K, typename V>
class CustomMap : public std::map<K, V>
{
};

}

#endif
