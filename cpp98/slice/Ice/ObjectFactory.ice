//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[["cpp:dll-export:ICE_API"]]
[["cpp:doxygen:include:Ice/Ice.h"]]
[["cpp:header-ext:h"]]

[["ice-prefix"]]

[["js:module:ice"]]

[["objc:dll-export:ICE_API"]]
[["objc:header-dir:objc"]]

[["python:pkgdir:Ice"]]

#ifndef __SLICE2JAVA_COMPAT__
[["java:package:com.zeroc"]]
#endif

["objc:prefix:ICE"]
module Ice
{

#if !defined(__SLICE2PHP__) && !defined(__SLICE2SWIFT__)
/**
 *
 * A factory for objects. Object factories are used when receiving "objects by value".
 * An object factory must be implemented by the application writer and registered
 * with the communicator.
 *
 **/
["deprecate:ObjectFactory has been deprecated, use ValueFactory instead."]
local interface ObjectFactory
{
    /**
     *
     * Create a new object for a given object type. The type is the
     * absolute Slice type id, i.e., the id relative to the
     * unnamed top-level Slice module. For example, the absolute
     * Slice type id for interfaces of type <code>Bar</code> in the module
     * <code>Foo</code> is <code>"::Foo::Bar"</code>.
     *
     * <p class="Note">The leading "<code>::</code>" is required.
     *
     * @param type The object type.
     *
     * @return The object created for the given type, or nil if the
     * factory is unable to create the object.
     *
     **/
    Value create(string type);

    /**
     *
     * Called when the factory is removed from the communicator, or if
     * the communicator is destroyed.
     *
     * @see Communicator#destroy
     *
     **/
    void destroy();
}

#endif

}
