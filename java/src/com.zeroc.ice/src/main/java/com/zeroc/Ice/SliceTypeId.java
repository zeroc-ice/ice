// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

/**
 * Assigns a Slice type ID to a class or to an interface.
 *
 * <p>The Slice compiler assigns Slice type IDs to the classes and interfaces it generates from Slice classes,
 * exceptions, and interfaces.
 */
@Retention(RetentionPolicy.RUNTIME)
@Target({ElementType.TYPE})
public @interface SliceTypeId {
    /**
     * Returns the Slice type ID assigned to this class or interface.
     *
     * @return the Slice type ID
    */
    String value();
}
