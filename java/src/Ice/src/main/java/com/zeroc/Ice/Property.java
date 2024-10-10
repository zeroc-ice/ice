//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

record Property(
        String pattern,
        boolean usesRegex,
        String defaultValue,
        boolean deprecated,
        PropertyArray propertyArray) {}
