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

#if _MSC_VER == 1200
    unsigned char* str;
#else
    unsigned short* str;    // Type has changed for some reason in VC++ 2002 (but doc still
#endif			    // says it's unsigned char *...)

    UuidToString(&uuid, &str);

    string result(reinterpret_cast<char*>(str));
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
