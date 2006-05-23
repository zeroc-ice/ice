// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGridGUI.Application;

import javax.swing.AbstractListModel;
import javax.swing.JMenuItem;
import javax.swing.JPopupMenu;

import IceGrid.*;
import IceGridGUI.*;

class PropertySets extends ListTreeNode implements PropertySetParent
{
    static public java.util.Map
    copyDescriptors(java.util.Map descriptors)
    {
	java.util.Map copy = new java.util.HashMap();
	java.util.Iterator p = descriptors.entrySet().iterator();
	while(p.hasNext())
	{
	    java.util.Map.Entry entry = (java.util.Map.Entry)p.next();

	    copy.put(entry.getKey(), 
		     PropertySet.copyDescriptor(
			 (PropertySetDescriptor)entry.getValue()));
	}
	return copy;
    }

    //
    // Actions
    //
    public boolean[] getAvailableActions()
    {
	boolean[] actions = new boolean[ACTION_COUNT];

	Object descriptor =  getCoordinator().getClipboard();
	if(descriptor != null)
	{
	    actions[PASTE] = descriptor instanceof PropertySetDescriptor;
	}

	actions[NEW_PROPERTY_SET] = true;
	return actions;
    }

    public JPopupMenu getPopupMenu()
    {
	ApplicationActions actions = getCoordinator().getActionsForPopup();
	if(_popup == null)
	{
	    _popup = new JPopupMenu();
	    _popup.add(actions.get(NEW_PROPERTY_SET));
	}
	actions.setTarget(this);
	return _popup;
    }
    
    public void newPropertySet()
    {
	PropertySetDescriptor descriptor = new
	    PropertySetDescriptor(new String[0],
				  new java.util.LinkedList());

	newPropertySet(descriptor);
    }

    public void paste()
    {
	Object descriptor =  getCoordinator().getClipboard();
	
	PropertySetDescriptor d = PropertySet.copyDescriptor(
	    (PropertySetDescriptor)descriptor);
	newPropertySet(d);
    }

    
    PropertySets(TreeNode parent, java.util.Map desc) 
	throws UpdateFailedException
    {
	super(false, parent, "Property Sets");
	_descriptors = desc;
	
	java.util.Iterator p = _descriptors.entrySet().iterator();
	while(p.hasNext())
	{
	    java.util.Map.Entry entry = (java.util.Map.Entry)p.next();
	    
	    insertChild(new PropertySet(false, this, 
					(String)entry.getKey(),
					(PropertySetDescriptor)entry.getValue()), false);
	}
    }

    void update(java.util.Map updates, String[] removePropertySets)
	throws UpdateFailedException
    {
	//
	// Note: _descriptors is updated by Root
	//

	//
	// One big set of removes
	//
	removeChildren(removePropertySets);

	//
	// One big set of updates, followed by inserts
	//
	java.util.Vector newChildren = new java.util.Vector();
	
	java.util.Iterator p = updates.entrySet().iterator();
	while(p.hasNext())
	{
	    java.util.Map.Entry entry = (java.util.Map.Entry)p.next();
	    String id = (String)entry.getKey();
	    PropertySetDescriptor psd = (PropertySetDescriptor)entry.getValue();
	    PropertySet child = (PropertySet)findChild(id);
	    if(child == null)
	    {
		newChildren.add(
		    new PropertySet(false, this, id, psd));
	    }
	    else
	    {
		child.rebuild(psd);
	    }
	}
	insertChildren(newChildren, true);
    }

    java.util.Map getUpdates()
    {
	java.util.Map updates = new java.util.HashMap();
	java.util.Iterator p = _children.iterator();
	while(p.hasNext())
	{
	    PropertySet ps = (PropertySet)p.next();
	    if(ps.getEditable().isNew() || ps.getEditable().isModified())
	    {
		updates.put(ps.getId(), ps.getDescriptor());
	    }
	}
	return updates;
    }

    void commit()
    {
	_editable.commit();
	java.util.Iterator p = _children.iterator();
	while(p.hasNext())
	{
	    PropertySet ps = (PropertySet)p.next();
	    ps.commit();
	}
    }

    Object getDescriptor()
    {
	return _descriptors;
    }
   
    public void tryAdd(String id, PropertySetDescriptor descriptor)
	throws UpdateFailedException
    {
	insertChild(
	    new PropertySet(true, this, id, descriptor),
	    true);

	_descriptors.put(id, descriptor);
    }
    
    public void insertPropertySet(PropertySet nps, boolean fireEvent)
	throws UpdateFailedException
    {
	insertChild(nps, fireEvent);
    }

    public void removePropertySet(PropertySet nps)
    {
	removeChild(nps);
    }

    public void removeDescriptor(String id)
    {
	_descriptors.remove(id);
    }

    public Editable getEditable()
    {
	return super.getEditable();
    }

    private void newPropertySet(PropertySetDescriptor descriptor)
    {
	PropertySet propertySet =
	    new PropertySet(this, makeNewChildId("PropertySet"), descriptor);

	try
	{
	    insertChild(propertySet, true);
	}
	catch(UpdateFailedException e)
	{
	    assert false;
	}
	getRoot().setSelectedNode(propertySet);
    }

    private java.util.Map _descriptors;
    static private JPopupMenu _popup;
}
