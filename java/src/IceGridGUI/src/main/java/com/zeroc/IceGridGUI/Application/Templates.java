//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.IceGridGUI.Application;

import com.zeroc.IceGrid.*;

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
        java.util.List<Object> backupList = new java.util.Vector<>();

        java.util.List<Editable> editables = new java.util.LinkedList<>();

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
