// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_UTIL_UNICODE_H
#define ICE_UTIL_UNICODE_H

#include <IceUtil/Config.h>

namespace IceUtil
{

ICE_UTIL_API std::string wstringToString(const std::wstring&);
ICE_UTIL_API std::wstring stringToWstring(const std::string&);

}

#endif
