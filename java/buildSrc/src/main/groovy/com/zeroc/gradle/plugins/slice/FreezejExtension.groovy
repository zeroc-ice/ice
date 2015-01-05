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

class FreezejExtension {
    final NamedDomainObjectContainer<Dict> dict;
	final NamedDomainObjectContainer<Index> index;

    def name
    def args = ""
    def files
    def srcDir
    def include

    FreezejExtension(dict, index) {
        this.dict = dict
        this.index = index
    }

    def dict(Closure closure) {
		dict.configure(closure)
    }
    def index(Closure closure) {
		index.configure(closure)
    }
}