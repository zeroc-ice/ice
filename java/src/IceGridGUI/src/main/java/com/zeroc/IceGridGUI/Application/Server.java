// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceGridGUI.Application;

interface Server extends TemplateInstance
{
    Editable getEditable();

    Object getDescriptor();

    Object saveDescriptor();
    void restoreDescriptor(Object descriptor);
}
