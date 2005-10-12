// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid.TreeNode;

import javax.swing.JOptionPane;

import IceGrid.Model;
import IceGrid.TemplateDescriptor;


abstract class Templates extends EditableParent
{
    abstract void tryAdd(String newId, TemplateDescriptor descriptor)
	throws UpdateFailedException;
 
    abstract protected java.util.List findAllTemplateInstances(String templateId);
   
    protected Templates(String id, Model model)
    {
	super(false, id, model, false);
    }

    void tryUpdate(CommonBase child)
	throws UpdateFailedException
    {
	java.util.List instanceList = findAllTemplateInstances(child.getId());
	
	java.util.List parentList = new java.util.Vector();
	java.util.List backupList = new java.util.Vector();

	java.util.List editables = new java.util.LinkedList();

	java.util.Iterator p = instanceList.iterator();
	while(p.hasNext())
	{
	    CommonBase instance = (CommonBase)p.next();
	    Parent parent = (Parent)instance.getParent();
	    
	    try
	    {
		backupList.add(parent.rebuildChild(instance, editables));
	    }
	    catch(UpdateFailedException e)
	    {
		for(int i = backupList.size() - 1; i >= 0; --i)
		{
		    parent = (Parent)parentList.get(i);
		    parent.restoreChild((CommonBase)instanceList.get(i), backupList.get(i));
		}
		throw e;
	    }
	    
	    parentList.add(parent);
	}
	
	p = editables.iterator();
	while(p.hasNext())
	{
	    Editable editable = (Editable)p.next();
	    editable.markModified();
	}
    }
}
