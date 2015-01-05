// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.gradle.plugins.slice;

class Java {
    def name
    def args = ""
    def files
    def srcDir = "src/main/slice"
    def include

    Java(String n) {
        name = n;
    }
}