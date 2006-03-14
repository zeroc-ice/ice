// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid.TreeNode;

import javax.swing.AbstractListModel;
import javax.swing.JMenuItem;
import javax.swing.JPopupMenu;

import IceGrid.ReplicaGroupDescriptor;
import IceGrid.Model;
import IceGrid.Utils;

class ReplicaGroups extends EditableParent
{
    static public java.util.LinkedList
    copyDescriptors(java.util.List descriptors)
    {
	java.util.LinkedList copy = new java.util.LinkedList();
	java.util.Iterator p = descriptors.iterator();
	while(p.hasNext())
	{
	    copy.add(ReplicaGroup.copyDescriptor(
			 (ReplicaGroupDescriptor)p.next()));
	}
	return copy;
    }

    //
    // Actions
    //
    public boolean[] getAvailableActions()
    {
	boolean[] actions = new boolean[ACTION_COUNT];

	Object descriptor =  _model.getClipboard();
	if(descriptor != null)
	{
	    actions[PASTE] = descriptor instanceof ReplicaGroupDescriptor;
	}

	actions[NEW_REPLICA_GROUP] = true;
	return actions;
    }

    public JPopupMenu getPopupMenu()
    {
	if(_popup == null)
	{
	    _popup = new PopupMenu(_model);
	    JMenuItem item = new JMenuItem(_model.getActions()[NEW_REPLICA_GROUP]);
	    item.setText("New replica group");
	    _popup.add(item);
	}
	return _popup;
    }
    
    public void newReplicaGroup()
    {
	ReplicaGroupDescriptor descriptor = new
	    ReplicaGroupDescriptor(
		makeNewChildId("NewReplicaGroup"),
		null,
		new java.util.LinkedList(),
		"");

	newReplicaGroup(descriptor);
    }

    public void paste()
    {
	Object descriptor =  _model.getClipboard();
	
	ReplicaGroupDescriptor d = ReplicaGroup.copyDescriptor(
	    (ReplicaGroupDescriptor)descriptor);
	d.id = makeNewChildId(d.id);
	newReplicaGroup(d);
    }

    ReplicaGroups(java.util.List descriptors, Model model)
	throws UpdateFailedException
    {
	super(false, "Replica Groups", model);
	_descriptors = descriptors;

	java.util.Iterator p = _descriptors.iterator();
	while(p.hasNext())
	{
	    ReplicaGroupDescriptor descriptor 
		= (ReplicaGroupDescriptor)p.next();
	    
	    addChild(new ReplicaGroup(false, descriptor, _model));
	}
    }

    java.util.LinkedList getUpdates()
    {
	java.util.LinkedList updates = new java.util.LinkedList();
	java.util.Iterator p = _children.iterator();
	while(p.hasNext())
	{
	    ReplicaGroup ra = (ReplicaGroup)p.next();
	    if(ra.isNew() || ra.isModified())
	    {
		updates.add(ra.getDescriptor());
	    }
	}
	return updates;
    }

    void update(java.util.List descriptors, String[] removeAdapters)
	throws UpdateFailedException
    {
	_descriptors = descriptors;

	//
	// One big set of removes
	//
	removeChildren(removeAdapters);

	//
	// One big set of updates, followed by inserts
	//
	java.util.Vector newChildren = new java.util.Vector();
	java.util.Vector updatedChildren = new java.util.Vector();
	
	java.util.Iterator p = descriptors.iterator();
	while(p.hasNext())
	{
	    ReplicaGroupDescriptor descriptor =
		(ReplicaGroupDescriptor)p.next();
	    
	    ReplicaGroup child 
		= (ReplicaGroup)findChild(descriptor.id);
	    if(child == null)
	    {
		newChildren.add(new ReplicaGroup(false, descriptor,
						      _model));
	    }
	    else
	    {
		child.rebuild(descriptor);
		updatedChildren.add(child);
	    }
	}
	
	updateChildren((CommonBaseI[])updatedChildren.toArray
		       (new CommonBaseI[0]));
	addChildren((CommonBaseI[])newChildren.toArray(new CommonBaseI[0]));
    }

  

    void removeDescriptor(Object descriptor)
    {
	//
	// A straight remove uses equals(), which is not the desired behavior
	//
	java.util.Iterator p = _descriptors.iterator();
	while(p.hasNext())
	{
	    if(descriptor == p.next())
	    {
		p.remove();
		break;
	    }
	}
    }

    void tryAdd(ReplicaGroupDescriptor descriptor, boolean addDescriptor)
	throws UpdateFailedException
    {
	try
	{
	    addChild(createReplicaGroup(true, descriptor), true);
	}
	catch(UpdateFailedException e)
	{
	    e.addParent(this);
	    throw e;
	}

	if(addDescriptor)
	{
	    _descriptors.add(descriptor);
	}
    }


    private ReplicaGroup createReplicaGroup(boolean brandNew, 
					    ReplicaGroupDescriptor descriptor)
    {
	return new ReplicaGroup(brandNew, descriptor, _model);
    }

    private void newReplicaGroup(ReplicaGroupDescriptor descriptor)
    {
	ReplicaGroup replicaGroup = new ReplicaGroup(descriptor, _model);
	try
	{
	    addChild(replicaGroup, true);
	}
	catch(UpdateFailedException e)
	{
	    assert false;
	}
	_model.setSelectionPath(replicaGroup.getPath());
    }

    private java.util.List _descriptors;
    static private JPopupMenu _popup;
}
