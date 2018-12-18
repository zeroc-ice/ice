// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package com.zeroc.IceGridGUI.Application;

interface DescriptorHolder
{
    Object saveDescriptor();
    void restoreDescriptor(Object descriptor);
}
