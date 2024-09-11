// Copyright (c) ZeroC, Inc.

package com.zeroc.IceGridGUI.Application;

interface TemplateInstance
{
    Object rebuild(java.util.List<Editable> editables) throws UpdateFailedException;
    void restore(Object backupObj);
}
