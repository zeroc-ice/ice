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
 * A factory for Objects. Object Factories are used in several
 * places, for example, for receiving "objects by value" or for the
 * [Freeze] module. Object Factories must be implemented by the
 * application writer, and installed with the Communicator.
 *
 * @see Freeze
 * @see UserExceptionFactory
 *
 **/
local interface ObjectFactory
{
    /**
     *
     * Create a new Object for a given Object type. The type is the
     * absolute Slice type name, i.e., the the name relative to the
     * unnamed top-level Slice module. For example, the absolute Slice
     * type name for Objects for interfaces of type [Bar] in the
     * module [Foo] is [::Foo::Bar].
     *
     * <note><para>The leading "[::]" is required.</para></note>
     *
     * @param type The Object type.
     *
     * @return The Object created for the given type.
     *
     **/
    Object create(string type);

    /**
     *
     * Called when the factory is removed from the Communicator, or if
     * the Communicator is destroyed.
     *
     * @see Communicator::removeObjectFactory
     * @see Communicator::destroy
     *
     **/
    void destroy();
};

};

#endif
