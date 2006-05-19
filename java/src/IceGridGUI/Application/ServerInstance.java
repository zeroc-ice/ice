// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGridGUI.Application;

import java.awt.Component;

import javax.swing.Icon;
import javax.swing.JTree;

import javax.swing.tree.TreeCellRenderer;
import javax.swing.tree.DefaultTreeCellRenderer;

import IceGrid.*;
import IceGridGUI.*;

class ServerInstance extends TreeNode implements Server
{ 
    static public ServerInstanceDescriptor
    copyDescriptor(ServerInstanceDescriptor sid)
    {
        ServerInstanceDescriptor copy = (ServerInstanceDescriptor)sid.clone();
	copy.propertySet = PropertySet.copyDescriptor(copy.propertySet);
	return copy;
    }
    
    //
    // Actions
    //
    public boolean[] getAvailableActions()
    {
	boolean[] actions = new boolean[ACTION_COUNT];
	actions[COPY] = true;
	
	Object clipboard = getCoordinator().getClipboard();
	if(clipboard != null && 
	   (clipboard instanceof ServerDescriptor
	    || clipboard instanceof ServerInstanceDescriptor))
	{
	    actions[PASTE] = true;
	}

	actions[DELETE] = true;
	if(!_ephemeral)
	{
	    actions[SHOW_VARS] = true;
	    actions[SUBSTITUTE_VARS] = true;
	}
	
	return actions;
    }
    
    public void copy()
    {
	getCoordinator().setClipboard(copyDescriptor(_descriptor));
	getCoordinator().getActionsForMenu().get(PASTE).setEnabled(true);
    }
    
    public Editor getEditor()
    {
	if(_editor == null)
	{
	    _editor = (ServerInstanceEditor)
		getRoot().getEditor(ServerInstanceEditor.class, this);
	}
	_editor.show(this);
	return _editor;
    }

    protected Editor createEditor()
    {
	return new ServerInstanceEditor();
    }
	
    public Component getTreeCellRendererComponent(
	    JTree tree,
	    Object value,
	    boolean sel,
	    boolean expanded,
	    boolean leaf,
	    int row,
	    boolean hasFocus) 
    {
	if(_cellRenderer == null)
	{
	    //
	    // Initialization
	    //
	    _cellRenderer = new DefaultTreeCellRenderer();

	    _serverIcon = Utils.getIcon("/icons/16x16/server_inactive.png");
	    _iceboxServerIcon = Utils.getIcon("/icons/16x16/icebox_server_inactive.png");
	}

	_cellRenderer.setLeafIcon(_isIceBox ? _iceboxServerIcon : _serverIcon);

	return _cellRenderer.getTreeCellRendererComponent(
	    tree, value, sel, expanded, leaf, row, hasFocus);
    }

    public void destroy()
    {
	Node node = (Node)_parent;
	
	if(_ephemeral)
	{
	    node.removeServer(this);
	}
	else
	{
	    node.removeDescriptor(_descriptor);
	    node.removeServer(this);
	    node.getEditable().removeElement(_id, _editable, Server.class);
	    getRoot().updated();
	}
    }


    public Object getDescriptor()
    {
	return _descriptor;
    }

    public Object saveDescriptor()
    {
	return _descriptor.clone();
    }

    public void restoreDescriptor(Object savedDescriptor)
    {
	ServerInstanceDescriptor copy = (ServerInstanceDescriptor)savedDescriptor;

	_descriptor.template = copy.template;
	_descriptor.parameterValues = copy.parameterValues;
	_descriptor.propertySet = copy.propertySet;	
    }

    //
    // Builds the server and all its sub-tree
    //
    ServerInstance(boolean brandNew, TreeNode parent, String serverId, 
		   Utils.Resolver resolver, 
		   ServerInstanceDescriptor instanceDescriptor,
		   boolean isIceBox) 
	throws UpdateFailedException
    {
	super(parent, serverId);
	_ephemeral = false;
	_editable = new Editable(brandNew);
	rebuild(resolver, instanceDescriptor, isIceBox);
    }

    ServerInstance(TreeNode parent, String serverId, 
		   ServerInstanceDescriptor instanceDescriptor)
    {
	super(parent, serverId);
	_ephemeral = true;
	_editable = null;
	rebuild(null, instanceDescriptor, false);
    }

    void write(XMLWriter writer) throws java.io.IOException
    {
	if(!_ephemeral)
	{
	    TemplateDescriptor templateDescriptor 
		= getRoot().findServerTemplateDescriptor(_descriptor.template);
	    
	    java.util.LinkedList attributes = parameterValuesToAttributes(
		_descriptor.parameterValues, templateDescriptor.parameters);
	    attributes.addFirst(createAttribute("template", _descriptor.template));
		
	    if(_descriptor.propertySet.references.length == 0 &&
	       _descriptor.propertySet.properties.size() == 0)
	    {
		writer.writeElement("server-instance", attributes);
	    }
	    else
	    {
		writer.writeStartTag("server-instance", attributes);
		writePropertySet(writer, "", _descriptor.propertySet, null);
		writer.writeEndTag("server-instance");
	    }
	}
    }


    boolean isIceBox()
    {
	return _isIceBox;
    }

    void isIceBox(boolean newValue)
    {
	_isIceBox = newValue;
    }
    
    static private class Backup
    {
	Backup(Editable ne)
	{
	    nodeEditable = ne;
	}

	Editable nodeEditable;
	java.util.Map parameterValues;
    }

    public Object rebuild(java.util.List editables) throws UpdateFailedException
    {
	Backup backup = new Backup(((Node)_parent).getEditable().save());
	Node node = (Node)_parent;
	
	TemplateDescriptor templateDescriptor 
	    = getRoot().findServerTemplateDescriptor(_descriptor.template);

	java.util.Set parameters = new java.util.HashSet(templateDescriptor.parameters);
	if(!parameters.equals(_descriptor.parameterValues.keySet()))
	{
	    backup.parameterValues = _descriptor.parameterValues;
	    _descriptor.parameterValues = EditorBase.makeParameterValues(
		_descriptor.parameterValues, templateDescriptor.parameters);
	}
	ServerInstance newServer = node.createServer(false, _descriptor);
	

	if(_id.equals(newServer.getId()))
	{
	    //
	    // A simple update. We can't simply rebuild server because 
	    // we need to keep a backup
	    //
	    if(_editable.isModified())
	    {
		newServer.getEditable().markModified();
	    }

	    node.removeServer(this);	    
	    try
	    {
		node.insertServer(newServer, true);
	    }
	    catch(UpdateFailedException e)
	    {
		assert false; // impossible, we just removed a child with
		              // this id
	    }

	    if(backup.parameterValues != null)
	    {
		editables.add(newServer.getEditable());
	    }
	}
	else
	{
	    newServer.getEditable().markNew();
	    node.removeServer(this);
	    node.getEditable().removeElement(_id, _editable, Server.class);
	    try
	    {
		node.insertServer(newServer, true);
	    }
	    catch(UpdateFailedException e)
	    {
		restore(backup);
		throw e;
	    }
	}
	
	return backup;
    }

    public void restore(Object backupObj)
    {
	Backup backup = (Backup)backupObj;
	Node node = (Node)_parent;
	
	node.getEditable().restore(backup.nodeEditable);

	if(backup.parameterValues != null)
	{
	    _descriptor.parameterValues = backup.parameterValues;
	}

	TreeNode badServer = node.findChildWithDescriptor(_descriptor);

	if(badServer != null)
	{
	    node.removeServer(badServer);
	}

	try
	{
	    node.insertServer(this, true);
	}
	catch(UpdateFailedException e)
	{
	    assert false; // impossible
	}
    }

    //
    // Update the server
    //
    void rebuild(Utils.Resolver resolver,  
		 ServerInstanceDescriptor instanceDescriptor,
		 boolean isIceBox)
    {
	_resolver = resolver;
	_isIceBox = isIceBox;
	_descriptor = instanceDescriptor;
    }

    Utils.Resolver getResolver()
    {
	return _resolver;
    }
 
    public Editable getEditable()
    {
	return _editable;
    }

    public boolean isEphemeral()
    {
	return _ephemeral;
    }

    public String toString()
    {
	if(_ephemeral)
	{
	    return super.toString();
	}
	else
	{
	    return _id + ": " + _descriptor.template + "<>";
	}
    }

    private ServerInstanceDescriptor _descriptor;
    private final boolean _ephemeral;
    private boolean _isIceBox;

    private ServerInstanceEditor _editor;

    private Utils.Resolver _resolver;
    private Editable _editable;
    
    static private DefaultTreeCellRenderer _cellRenderer;
    static private Icon _serverIcon;
    static private Icon _iceboxServerIcon;

}
