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
    abstract boolean tryAdd(String newId, TemplateDescriptor descriptor);
 
    abstract protected java.util.List findAllTemplateInstances(String templateId);
   
    protected Templates(String id, Model model)
    {
	super(false, id, model, false);
    }

    protected Templates(Templates o)
    {
	super(o);
    }

    boolean tryUpdate(CommonBase child)
    {
	java.util.List instanceList = findAllTemplateInstances(child.getId());
	
	java.util.List parentList = new java.util.Vector();
	java.util.List backupList = new java.util.Vector();
	java.util.List oldInstanceList = new java.util.Vector();

	java.util.List editables = new java.util.LinkedList();

	java.util.Iterator p = instanceList.iterator();
	while(p.hasNext())
	{
	    CommonBase instance = (CommonBase)p.next();
	    InstanceParent parent = (InstanceParent)instance.getParent();
	    
	    try
	    {
		backupList.add(parent.rebuild(instance, editables));
	    }
	    catch(UpdateFailedException e)
	    {
		for(int i = backupList.size() - 1; i >= 0; --i)
		{
		    parent = (InstanceParent)parentList.get(i);
		    parent.restore((CommonBase)instanceList.get(i), backupList.get(i));
		}

		JOptionPane.showMessageDialog(
		    _model.getMainFrame(),
		    e.toString(),
		    "Apply failed",
		    JOptionPane.ERROR_MESSAGE);
		
		return false;
	    }
	    
	    oldInstanceList.add(instance);
	    parentList.add(parent);
	}
	
	p = editables.iterator();
	while(p.hasNext())
	{
	    Editable editable = (Editable)p.next();
	    editable.markModified();
	}
	
	((CommonBaseI)child).fireNodeChangedEvent(this);

	return true;
    }
}
