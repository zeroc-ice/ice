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
import javax.swing.JTree;
import javax.swing.tree.DefaultTreeCellRenderer;

import IceGrid.*;
import IceGridGUI.*;

class PropertySet extends TreeNode
{
    static public PropertySetDescriptor 
    copyDescriptor(PropertySetDescriptor d)
    {
	PropertySetDescriptor psd = (PropertySetDescriptor)d.clone();
	psd.properties = new java.util.LinkedList(psd.properties);
	return psd;
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
	    _cellRenderer = new DefaultTreeCellRenderer();
	    _cellRenderer.setLeafIcon(
		Utils.getIcon("/icons/16x16/grid.png"));
	}

	return _cellRenderer.getTreeCellRendererComponent(
	    tree, value, sel, expanded, leaf, row, hasFocus);
    }
    
    //
    // Actions
    //
    public boolean[] getAvailableActions()
    {
	boolean[] actions = new boolean[ACTION_COUNT];
	actions[COPY] = true;
	
	if(((TreeNode)_parent).getAvailableActions()[PASTE])
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
    public void paste()
    {
	((TreeNode)_parent).paste();
    }

    public void destroy()
    {
	PropertySetParent parent = (PropertySetParent)_parent;
	parent.removePropertySet(this);

	if(!_ephemeral)
	{
	    parent.removeDescriptor(_id);
	    parent.getEditable().removeElement(_id, _editable, PropertySet.class);
	    getRoot().updated();
	}
    }

    public Editor getEditor()
    {
	if(_editor == null)
	{
	    _editor = (PropertySetEditor)getRoot().
		getEditor(PropertySetEditor.class, this);
	}
	_editor.show(this);
	return _editor;
    }

    protected Editor createEditor()
    {
	return new PropertySetEditor();
    }

    public boolean isEphemeral()
    {
	return _ephemeral;
    }

    Object getDescriptor()
    {
	return _descriptor;
    }
    
    Object saveDescriptor()
    {
	return _descriptor.clone();
    }
    void restoreDescriptor(Object savedDescriptor)
    {
	PropertySetDescriptor clone = (PropertySetDescriptor)savedDescriptor;
	_descriptor.references = clone.references;
	_descriptor.properties = clone.properties;
    }

    void commit()
    {
	_editable.commit();
    }
    
    Editable getEditable()
    {
	return _editable;
    }
   
    PropertySet(boolean brandNew,
		TreeNode parent,
		String id,
		PropertySetDescriptor descriptor)
    {
	super(parent, id);
	_ephemeral = false;
	_editable = new Editable(brandNew);
	rebuild(descriptor);
    }
    
    PropertySet(TreeNode parent, String id, PropertySetDescriptor descriptor)
    {
	super(parent, id);
	_ephemeral = true;
	_editable = null;
	rebuild(descriptor);
    }
    
    void write(XMLWriter writer) throws java.io.IOException
    {
	if(!_ephemeral)
	{
	    writePropertySet(writer, _id, _descriptor, null);
	}
    }

    void rebuild(PropertySetDescriptor descriptor)
    {
	_descriptor = descriptor;
    }

    private PropertySetDescriptor _descriptor;
    private final boolean _ephemeral;
    private final Editable _editable;
    private PropertySetEditor _editor;

    static private DefaultTreeCellRenderer _cellRenderer;  
}
