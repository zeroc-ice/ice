// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** A factory for class instances. */
@FunctionalInterface
public interface ValueFactory {
    /**
     * Create a new instance for a given Slice class. The type is the absolute Slice type id, i.e.,
     * the id relative to the unnamed top-level Slice module. For example, the absolute Slice type
     * id for an interface <code>Bar</code> in the module <code>Foo</code> is <code>"::Foo::Bar"
     * </code> . Note that the leading "<code>::</code>" is required.
     *
     * @param type The Slice type ID.
     * @return The class instance created for the given type, or nil if the factory is unable to
     *     create the value.
     */
    Value create(String type);
}
