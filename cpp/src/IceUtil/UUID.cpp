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

#ifdef WIN32
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
#ifdef WIN32
#else
    uuid_t uuid;
    uuid_generate(uuid);

    char str[37];
    uuid_unparse(uuid, str);

    return str;
#endif
}
