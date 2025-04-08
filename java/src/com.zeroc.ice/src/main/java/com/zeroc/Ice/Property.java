// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

record Property(
        String pattern,
        boolean usesRegex,
        String defaultValue,
        boolean deprecated,
        PropertyArray propertyArray) {
}
