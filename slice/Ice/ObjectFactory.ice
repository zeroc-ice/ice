// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

[["cpp:header-ext:h", "objc:header-dir:objc"]]

["objc:prefix:ICE"]
module Ice
{

/**
 *
 * A factory for objects. Object factories are used in several
 * places, for example, when receiving "objects by value" and
 * when Freeze restores a persistent object. Object factories
 * must be implemented by the application writer, and registered
 * with the communicator.
 *
 **/
local interface ObjectFactory
{
    /**
     *
     * Create a new object for a given object type. The type is the
     * absolute Slice type id, i.e., the id relative to the
     * unnamed top-level Slice module. For example, the absolute
     * Slice type id for interfaces of type <tt>Bar</tt> in the module
     * <tt>Foo</tt> is <tt>::Foo::Bar</tt>.
     *
     * <p class="Note">The leading "<tt>::</tt>" is required.
     *
     * @param type The object type.
     *
     * @return The object created for the given type, or nil if the
     * factory is unable to create the object.
     *
     **/
    Object create(string type);

    /**
     *
     * Called when the factory is removed from the communicator, or if
     * the communicator is destroyed.
     *
     * @see Communicator#destroy
     *
     **/
    void destroy();
};

};

