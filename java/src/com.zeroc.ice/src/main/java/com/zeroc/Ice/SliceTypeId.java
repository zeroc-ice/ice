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
 * exceptions, and interfaces.</p>
 */
@Retention(RetentionPolicy.RUNTIME)
@Target({ElementType.TYPE})
public @interface SliceTypeId {
    /** The Slice type ID of this class or interface. */
    String value();
}
