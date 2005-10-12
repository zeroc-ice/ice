// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid.TreeNode;

import IceGrid.DbEnvDescriptor;
import IceGrid.Model;
import IceGrid.SimpleInternalFrame;
import IceGrid.Utils;

class DbEnv extends Leaf
{
    static public DbEnvDescriptor copyDescriptor(DbEnvDescriptor d)
    {
	return (DbEnvDescriptor)d.clone();
    }

    //
    // Actions
    //
    public boolean[] getAvailableActions()
    {
	boolean[] actions = new boolean[ACTION_COUNT];
	actions[COPY] = true;
	
	if(_parent.getAvailableActions()[PASTE])
	{
	    actions[PASTE] = true;
	}
	if(isEditable())
	{
	    actions[DELETE] = true;
	}
	
	if(_resolver != null && !_ephemeral)
	{
	    actions[SUBSTITUTE_VARS] = true;
	}
	return actions;
    }

    public void copy()
    {
	_model.setClipboard(copyDescriptor(_descriptor));
	if(_parent.getAvailableActions()[PASTE])
	{
	    _model.getActions()[PASTE].setEnabled(true);
	}
    }
    public void paste()
    {
	_parent.paste();
    }

    public void displayProperties()
    {
	SimpleInternalFrame propertiesFrame = _model.getPropertiesFrame();
	
	propertiesFrame.setTitle("Properties for " + _id);
	if(_editor == null)
	{
	    _editor = new DbEnvEditor(_model.getMainFrame());
	}
	
	_editor.show(this);
	propertiesFrame.setContent(_editor.getComponent());

	propertiesFrame.validate();
	propertiesFrame.repaint();
    }


    public Object getDescriptor()
    {
	return _descriptor;
    }

    public Object copyDescriptor()
    {
	return copyDescriptor(_descriptor);
    }

    public Object saveDescriptor()
    {
	return copyDescriptor(_descriptor);
    }

    public void restoreDescriptor(Object savedDescriptor)
    {
	DbEnvDescriptor clone = (DbEnvDescriptor)savedDescriptor;
	_descriptor.name = clone.name;
	_descriptor.dbHome = clone.dbHome;
	_descriptor.properties = clone.properties;
    }

    public boolean destroy()
    {
	return _parent == null ? false : 
	    ((ListParent)_parent).destroyChild(this);
    }

    public boolean isEphemeral()
    {
	return _ephemeral;
    }

    DbEnv(String dbEnvName, DbEnvDescriptor descriptor, 
	  Utils.Resolver resolver, Model model)
    {
	super(dbEnvName, model);
	_ephemeral = false;
	_descriptor = descriptor;
	_resolver = resolver;
    }

    DbEnv(String dbEnvName, DbEnvDescriptor descriptor, Model model)
    {
	super(dbEnvName, model);
	_ephemeral = true;
	_descriptor = descriptor;
    }

    Utils.Resolver getResolver()
    {
	return _resolver;
    }

    boolean isEditable()
    {
	if(_parent == null)
	{
	    return false;
	}
	else
	{
	    return ((DbEnvs)_parent).isEditable();
	}

    }

    private DbEnvDescriptor _descriptor;
    private Utils.Resolver _resolver;
    private final boolean _ephemeral; 

    static private DbEnvEditor _editor;
}
