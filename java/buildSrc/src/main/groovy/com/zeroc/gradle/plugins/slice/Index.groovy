// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.gradle.plugins.slice;

class Index {
	def name
	def javaType
    def type
    def member
    def casesensitive = true

    Index(String n) {
        name = n
    }
}
