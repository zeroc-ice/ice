// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid.TreeNode;

import javax.swing.JMenuItem;
import javax.swing.JPopupMenu;

import IceGrid.DbEnvDescriptor;
import IceGrid.Model;
import IceGrid.Utils;

class DbEnvs extends ListParent
{
    static public java.util.LinkedList copyDescriptors(java.util.LinkedList list)
    {
	java.util.LinkedList copy = new java.util.LinkedList();
	java.util.Iterator p = list.iterator();
	while(p.hasNext())
	{
	    copy.add(DbEnv.copyDescriptor((DbEnvDescriptor)p.next()));
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
	    actions[PASTE] = _isEditable && 
		descriptor instanceof DbEnvDescriptor;
	}

	actions[NEW_DBENV] = _isEditable;
	return actions;
    }

    public JPopupMenu getPopupMenu()
    {
	if(_popup == null)
	{
	    _popup = new PopupMenu(_model);
	    JMenuItem item = new JMenuItem(_model.getActions()[NEW_DBENV]);
	    item.setText("New DbEnv");
	    _popup.add(item);
	}
	if(_isEditable)
	{
	    return _popup;
	}
	else
	{
	    return null;
	}
    }

    public void newDbEnv()
    {
	DbEnvDescriptor descriptor = new DbEnvDescriptor(
	    "NewDbEnv",
	    "",
	    "",
	    new java.util.LinkedList());   
	
	newDbEnv(descriptor);
    }
    public void paste()
    {
	Object descriptor =  _model.getClipboard();
	assert _isEditable && 
	    descriptor instanceof DbEnvDescriptor;

	DbEnvDescriptor d = DbEnv.copyDescriptor(
	    (DbEnvDescriptor)descriptor);
	d.name = makeNewChildId(d.name);
	newDbEnv(d);
    }


    CommonBase addNewChild(Object d) throws UpdateFailedException
    {
	DbEnvDescriptor descriptor = (DbEnvDescriptor)d;
       
	try
	{
	    DbEnv dbEnv = createDbEnv(descriptor);
	    addChild(dbEnv, true);
	    return dbEnv;
	}
	catch(UpdateFailedException e)
	{
	    e.addParent(this);
	    throw e;
	}
    }
    
    DbEnvs(java.util.List descriptors, 
	   boolean isEditable,
	   Utils.Resolver resolver,
	   Model model)
	throws UpdateFailedException
    {
	super("DbEnvs", model);
	_isEditable = isEditable;
	_descriptors = descriptors;
	_resolver = resolver;

	java.util.Iterator p = _descriptors.iterator();
	while(p.hasNext())
	{
	    addChild(createDbEnv((DbEnvDescriptor)p.next()));   
	}
    }

    boolean isEditable()
    {
	return _isEditable;
    }

    private DbEnv createDbEnv(DbEnvDescriptor descriptor)
    {
	String dbEnvName = Utils.substitute(descriptor.name, _resolver);
	return new DbEnv(dbEnvName, descriptor, _resolver, _model);
    }

    private void newDbEnv(DbEnvDescriptor descriptor)
    {
	descriptor.name = makeNewChildId(descriptor.name);
	DbEnv dbEnv = new DbEnv(descriptor.name, descriptor, _model);
	try
	{
	    addChild(dbEnv, true);
	}
	catch(UpdateFailedException e)
	{
	    assert false;
	}
	_model.setSelectionPath(dbEnv.getPath());

    }

    private final boolean _isEditable;
    private Utils.Resolver _resolver;

    static private JPopupMenu _popup;
}
