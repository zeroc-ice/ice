// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#include <IceUtil/UUID.h>
#include <IceUtil/Unicode.h>

#ifdef _WIN32
#   include <rpc.h>
#else
extern "C" // uuid/uuid.h seems to miss extern "C" declarations.
{
#   include <uuid/uuid.h>
}
#endif

using namespace std;

string
IceUtil::generateUUID()
{
#ifdef _WIN32

    UUID uuid;
    UuidCreate(&uuid);

#if _MSC_VER > 1200
    wchar_t* str;
#else
    unsigned char* str;
#endif

    UuidToString(&uuid, &str);

    string result;

#if _MSC_VER > 1200
    result = wstringToString(wstring(str));
#else
    result = reinterpret_cast<char*>(str);
#endif

    RpcStringFree(&str);
    return result;
    
#else

    uuid_t uuid;
    uuid_generate(uuid);

    char str[37];
    uuid_unparse(uuid, str);

    return str;

#endif
}
