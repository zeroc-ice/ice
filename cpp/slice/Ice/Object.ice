// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_OBJECT_ICE
#define ICE_OBJECT_ICE

module Ice
{

/**
 *
 * The base Ice Object. All Ice object derive from this class, however it cannot be 
 * instantiated (pure virtual). This Slice definition is for use, primarily, by other
 * language bindings that need to wrap the C++ implementation. Renamed _guard_Object since
 * Object is a keyword.
 *
 **/
local interface _guard_Object
{
    /**
     *
     * Is this class of type className? Returns true if it is, 
     * this includes if it is a base class of classname.
     *
     * @param className The scoped name of the class we want to compare against.
     * 
     **/
    bool _isA(string className);

    /**
     *
     * Just a simple call to see if this object is still alive. 
     * 
     **/
    void _ping();
};

};

#endif
