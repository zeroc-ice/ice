// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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

    unsigned char* str;
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
