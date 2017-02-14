// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.Ice;

public interface NativePropertiesAdmin
{
    void addUpdateCallback(java.util.function.Consumer<java.util.Map<String, String>> callback);
    void removeUpdateCallback(java.util.function.Consumer<java.util.Map<String, String>> callback);
}
