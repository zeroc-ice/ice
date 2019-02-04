//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.IceGridGUI.Application;

import java.awt.Component;
import javax.swing.JTree;
import javax.swing.tree.DefaultTreeCellRenderer;

import com.zeroc.IceGrid.*;
import com.zeroc.IceGridGUI.*;

class DbEnv extends TreeNode implements DescriptorHolder
{
    static public DbEnvDescriptor copyDescriptor(DbEnvDescriptor d)
    {
        return d.clone();
    }

    static public java.util.List<DbEnvDescriptor> copyDescriptors(java.util.List<DbEnvDescriptor> list)
    {
        java.util.List<DbEnvDescriptor> copy = new java.util.LinkedList<>();
        for(DbEnvDescriptor p : list)
        {
            copy.add(copyDescriptor(p));
        }
        return copy;
    }

    @Override
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
            _cellRenderer.setLeafIcon(Utils.getIcon("/icons/16x16/database.png"));
        }
        return _cellRenderer.getTreeCellRendererComponent(tree, value, sel, expanded, leaf, row, hasFocus);
    }

    //
    // Actions
    //
    @Override
    public boolean[] getAvailableActions()
    {
        boolean[] actions = new boolean[ACTION_COUNT];
        actions[COPY] = !_ephemeral;

        Object clipboard = getCoordinator().getClipboard();
        actions[PASTE] = clipboard != null &&
            (clipboard instanceof Adapter.AdapterCopy
             || clipboard instanceof DbEnvDescriptor);
        actions[DELETE] = true;

        if(!_ephemeral)
        {
            boolean[] parentActions = ((TreeNode)_parent).getAvailableActions();
            actions[SHOW_VARS] = parentActions[SHOW_VARS];
            actions[SUBSTITUTE_VARS] = parentActions[SUBSTITUTE_VARS];
        }
        return actions;
    }

    @Override
    public void copy()
    {
        getCoordinator().setClipboard(copyDescriptor(_descriptor));
        getCoordinator().getActionsForMenu().get(PASTE).setEnabled(true);
    }

    @Override
    public void paste()
    {
        ((TreeNode)_parent).paste();
    }

    @Override
    public Editor getEditor()
    {
        if(_editor == null)
        {
            _editor = (DbEnvEditor)getRoot().getEditor(DbEnvEditor.class, this);
        }
        _editor.show(this);
        return _editor;
    }

    @Override
    protected Editor createEditor()
    {
        return new DbEnvEditor();
    }

    @Override
    public Object getDescriptor()
    {
        return _descriptor;
    }

    public Object copyDescriptor()
    {
        return copyDescriptor(_descriptor);
    }

    @Override
    public Object saveDescriptor()
    {
        return copyDescriptor(_descriptor);
    }

    @Override
    public void restoreDescriptor(Object savedDescriptor)
    {
        DbEnvDescriptor clone = (DbEnvDescriptor)savedDescriptor;
        _descriptor.name = clone.name;
        _descriptor.dbHome = clone.dbHome;
        _descriptor.description = clone.description;
        _descriptor.properties = clone.properties;
    }

    @Override
    public void destroy()
    {
        ((Communicator)_parent).getDbEnvs().destroyChild(this);
    }

    @Override
    public boolean isEphemeral()
    {
        return _ephemeral;
    }

    DbEnv(Communicator parent, String dbEnvName, DbEnvDescriptor descriptor, boolean ephemeral)
    {
        super(parent, dbEnvName);
        _descriptor = descriptor;
        _ephemeral = ephemeral;
    }

    static void writeDbProperties(XMLWriter writer, java.util.List<PropertyDescriptor> properties)
        throws java.io.IOException
    {
        for(PropertyDescriptor p : properties)
        {
            java.util.List<String[]> attributes = new java.util.LinkedList<>();
            attributes.add(createAttribute("name", p.name));
            attributes.add(createAttribute("value", p.value));
            writer.writeElement("dbproperty", attributes);
        }
    }

    @Override
    void write(XMLWriter writer)
        throws java.io.IOException
    {
        if(!_ephemeral)
        {
            java.util.List<String[]> attributes = new java.util.LinkedList<>();
            attributes.add(createAttribute("name", _descriptor.name));
            if(_descriptor.dbHome.length() > 0)
            {
                attributes.add(createAttribute("home", _descriptor.dbHome));
            }

            if(_descriptor.description.length() == 0 && _descriptor.properties.isEmpty())
            {
                writer.writeElement("dbenv", attributes);
            }
            else
            {
                writer.writeStartTag("dbenv", attributes);
                if(_descriptor.description.length() > 0)
                {
                    writer.writeElement("description", _descriptor.description);
                }
                writeDbProperties(writer, _descriptor.properties);
                writer.writeEndTag("dbenv");
            }
        }
    }

    private DbEnvDescriptor _descriptor;
    private final boolean _ephemeral;
    private DbEnvEditor _editor;

    static private DefaultTreeCellRenderer _cellRenderer;
}
