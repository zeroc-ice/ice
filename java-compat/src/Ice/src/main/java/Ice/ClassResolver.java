//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package Ice;

/**
 *
 * A ClassResolver translates a Slice type Id into a Java class using
 * an implementation-defined algorithm.
 *
 **/
public interface ClassResolver
{
    /**
     * Resolve a Slice type Id into a Java class. The type Id corresponds to a
     * Slice value or user exception.
     *
     * @param typeId A string type ID (such as <code>"::Module::Class"</code>).
     * @return The Java class object corresponding to the Slice type ID, or null
     * if no class could be found.
     **/
    Class<?> resolveClass(String typeId);
}
