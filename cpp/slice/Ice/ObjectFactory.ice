// **********************************************************************
//
// Copyright (c) 2003
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

#ifndef ICE_OBJECT_FACTORY_ICE
#define ICE_OBJECT_FACTORY_ICE

module Ice
{

/**
 *
 * A factory for objects. Object factories are used in several
 * places, for example, for receiving "objects by value" or for the
 * [Freeze] module. Object factories must be implemented by the
 * application writer, and installed with the communicator.
 *
 * @see Freeze
 * @see UserExceptionFactory
 *
 **/
local interface ObjectFactory
{
    /**
     *
     * Create a new object for a given object type. The type is the
     * absolute Slice type name, i.e., the name relative to the
     * unnamed top-level Slice module. For example, the absolute Slice
     * type name for objects for interfaces of type [Bar] in the
     * module [Foo] is [::Foo::Bar].
     *
     * <note><para>The leading "[::]" is required.</para></note>
     *
     * @param type The object type.
     *
     * @return The object created for the given type.
     *
     **/
    Object create(string type);

    /**
     *
     * Called when the factory is removed from the communicator, or if
     * the communicator is destroyed.
     *
     * @see Communicator::removeObjectFactory
     * @see Communicator::destroy
     *
     **/
    void destroy();
};

};

#endif
