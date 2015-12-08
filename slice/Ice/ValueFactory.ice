// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
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
 * A factory for values. Value factories are used in several
 * places, for example, when receiving "values by value" and
 * when Freeze restores a persistent value. Value factories
 * must be implemented by the application writer, and registered
 * with the communicator.
 *
 * @see Freeze
 *
 **/
["delegate"]
local interface ValueFactory
{
    /**
     *
     * Create a new value for a given value type. The type is the
     * absolute Slice type id, i.e., the id relative to the
     * unnamed top-level Slice module. For example, the absolute
     * Slice type id for interfaces of type <tt>Bar</tt> in the module
     * <tt>Foo</tt> is <tt>::Foo::Bar</tt>.
     *
     * <p class="Note">The leading "<tt>::</tt>" is required.
     *
     * @param type The value type.
     *
     * @return The value created for the given type, or nil if the
     * factory is unable to create the value.
     *
     **/
    Value create(string type);
};

};

