// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGridGUI.Application;

import javax.swing.JOptionPane;

import IceGrid.*;
import IceGridGUI.*;

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
        java.util.List instanceList = child.findInstances();
        java.util.List backupList = new java.util.Vector();

        java.util.List editables = new java.util.LinkedList();

        java.util.Iterator p = instanceList.iterator();
        while(p.hasNext())
        {
            TemplateInstance instance = (TemplateInstance)p.next();
            
            try
            {
                backupList.add(instance.rebuild(editables));
            }
            catch(UpdateFailedException e)
            {
                for(int i = backupList.size() - 1; i >= 0; --i)
                {
                    instance = (TemplateInstance)instanceList.get(i);
                    instance.restore(backupList.get(i));
                }
                throw e;
            }
        }
        
        p = editables.iterator();
        while(p.hasNext())
        {
            Editable editable = (Editable)p.next();
            editable.markModified();
        }
    }
}
