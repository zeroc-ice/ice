// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
     * absolute Slice type name, i.e., the the name relative to the
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
