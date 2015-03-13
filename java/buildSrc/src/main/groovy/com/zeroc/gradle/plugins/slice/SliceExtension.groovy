// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.gradle.plugins.slice;

import org.gradle.api.NamedDomainObjectContainer

class SliceExtension {
    final NamedDomainObjectContainer<Java> java;
    def iceHome = null
    def srcDist = false
    def output

    SliceExtension(java) {
        this.java = java
    }

    def java(Closure closure) {
        try {
            java.configure(closure)
        }
        catch(MissingPropertyException ex) {
            java.create('default', closure)
        }
    }
}
