// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.gradle.plugins.slice;

import org.gradle.api.NamedDomainObjectContainer

class SliceExtension {
    final NamedDomainObjectContainer<Java> java;
    def slice2java = "slice2java"
    def slice2freezej = "slice2freezej"
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
