// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

/**
 * Assigns a compact Slice type ID to a class.
 *
 * <p>The Slice compiler assigns both a Slice type ID and a compact Slice type ID to the mapped class of a Slice class
 * that specifies a compact type ID.</p>
 */
@Retention(RetentionPolicy.RUNTIME)
@Target({ElementType.TYPE})
public @interface CompactSliceTypeId {
    /** The compact ID of this class.*/
    int value();
}
