// Copyright (c) ZeroC, Inc.

package com.zeroc.IceGridGUI.Application;

interface Server extends TemplateInstance
{
    Editable getEditable();

    Object getDescriptor();

    Object saveDescriptor();
    void restoreDescriptor(Object descriptor);
}
