// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceGridGUI.Application;

import IceGrid.*;

abstract class Templates extends ListTreeNode
{
    abstract void tryAdd(String newId, TemplateDescriptor descriptor)
        throws UpdateFailedException;

    protected Templates(Root parent, String id)
    {
        super(false, parent, id);
    }

    void tryUpdate(Communicator child)
        throws UpdateFailedException
    {
        java.util.List<? extends TemplateInstance> instanceList = child.findInstances();
        java.util.List<Object> backupList = new java.util.Vector<Object>();

        java.util.List<Editable> editables = new java.util.LinkedList<Editable>();

        for(TemplateInstance p : instanceList)
        {
            try
            {
                backupList.add(p.rebuild(editables));
            }
            catch(UpdateFailedException e)
            {
                for(int i = backupList.size() - 1; i >= 0; --i)
                {
                    TemplateInstance instance = instanceList.get(i);
                    instance.restore(backupList.get(i));
                }
                throw e;
            }
        }

        for(Editable p : editables)
        {
            p.markModified();
        }
    }
}
