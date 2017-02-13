// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_STRING_UTIL_H
#define ICE_STRING_UTIL_H

#include <Ice/Initialize.h>
#include <IceUtil/StringUtil.h>

namespace IceInternal
{

//
// Adapter for ToStringMode
//
inline std::string 
escapeString(const std::string& s, const std::string& special, Ice::ToStringMode mode)
{
    return IceUtilInternal::escapeString(s, special, static_cast<IceUtilInternal::ToStringMode>(mode));
}

//
// Provided for consistency with escapeString
//
using IceUtilInternal::unescapeString;

}

#endif
