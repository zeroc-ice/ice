//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

import java.lang.annotation.Documented;
import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

public class Annotations
{
    @Documented
    @Retention(RetentionPolicy.RUNTIME)
    @Target({ElementType.TYPE, ElementType.TYPE_PARAMETER, ElementType.TYPE_USE})
    public static @interface Nullable
    {

    }

    @Documented
    @Retention(RetentionPolicy.RUNTIME)
    @Target({ElementType.TYPE, ElementType.TYPE_PARAMETER, ElementType.TYPE_USE})
    public static @interface NonNull
    {

    }

    @Documented
    @Retention(RetentionPolicy.RUNTIME)
    @Target({ElementType.TYPE, ElementType.TYPE_PARAMETER, ElementType.TYPE_USE})
    public static @interface LazyNonNull
    {

    }
}
