// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * Creates class and exception instances from Slice type IDs.
 */
@FunctionalInterface
public interface SliceLoader {
    /**
     * Creates an instance of a class mapped from a Slice class or exception based on a Slice type ID.
     *
     * @param typeId the Slice type ID or compact type ID
     * @return a new instance of the class or exception identified by {@code typeId}, or {@code null} if the
     *     implementation cannot find the corresponding class
     * @throws MarshalException if the corresponding class was found but its instantiation failed
     */
    java.lang.Object newInstance(String typeId);
}
