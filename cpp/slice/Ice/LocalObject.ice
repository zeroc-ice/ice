// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

//
// This file is not used in the Ice core. It is only for documentation
// purposes, and for tool suchs as <literal>slice2pythonext</literal>.
//

#ifndef ICE_LOCAL_OBJECT_ICE
#define ICE_LOCAL_OBJECT_ICE

module Ice
{

/**
 *
 * The base interface for all local objects in Ice. In contrast to
 * [Object], no Proxies are generated for local objects, and neither
 * the supporting marshaling and unmarshaling code.
 *
 * @see Ice::Object
 *
 **/
local interface \LocalObject
{
};

};

#endif
