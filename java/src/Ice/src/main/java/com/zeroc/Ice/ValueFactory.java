//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * A factory for values. Value factories are used in several
 * places, such as when Ice receives a class instance and
 * when Freeze restores a persistent value. Value factories
 * must be implemented by the application writer and registered
 * with the communicator.
 **/
@FunctionalInterface
public interface ValueFactory
{
    /**
     * Create a new value for a given value type. The type is the
     * absolute Slice type id, i.e., the id relative to the
     * unnamed top-level Slice module. For example, the absolute
     * Slice type id for an interface <code>Bar</code> in the module
     * <code>Foo</code> is <code>"::Foo::Bar"</code>.
     *
     * Note that the leading "<code>::</code>" is required.
     * @param type The value type.
     * @return The value created for the given type, or nil if the
     * factory is unable to create the value.
     **/
    Value create(String type);
}
