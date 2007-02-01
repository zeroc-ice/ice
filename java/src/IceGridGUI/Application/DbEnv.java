// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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

class DbEnv extends TreeNode implements DescriptorHolder
{
    static public DbEnvDescriptor copyDescriptor(DbEnvDescriptor d)
    {
        return (DbEnvDescriptor)d.clone();
    }

    static public java.util.List copyDescriptors(java.util.List list)
    {
        java.util.List copy = new java.util.LinkedList();
        java.util.Iterator p = list.iterator();
        while(p.hasNext())
        {
            copy.add(copyDescriptor((DbEnvDescriptor)p.next()));
        }
        return copy;
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
            _cellRenderer.setLeafIcon(Utils.getIcon("/icons/16x16/database.png"));
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
        actions[COPY] = !_ephemeral;

        Object clipboard = getCoordinator().getClipboard();
        actions[PASTE] = clipboard != null && 
            (clipboard instanceof AdapterDescriptor
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

    public void copy()
    {
        getCoordinator().setClipboard(copyDescriptor(_descriptor));
        getCoordinator().getActionsForMenu().get(PASTE).setEnabled(true);
    }

    public void paste()
    {
        ((TreeNode)_parent).paste();
    }

    public Editor getEditor()
    {
        if(_editor == null)
        {
            _editor = (DbEnvEditor)getRoot().getEditor(DbEnvEditor.class, this);
        }
        _editor.show(this);
        return _editor;
    }

    protected Editor createEditor()
    {
        return new DbEnvEditor();
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
        _descriptor.description = clone.description;
        _descriptor.properties = clone.properties;
    }

    public void destroy()
    {
        ((Communicator)_parent).getDbEnvs().destroyChild(this);
    }

    public boolean isEphemeral()
    {
        return _ephemeral;
    }

    DbEnv(Communicator parent, String dbEnvName, DbEnvDescriptor descriptor, 
          boolean ephemeral)
    {
        super(parent, dbEnvName);
        _descriptor = descriptor;
        _ephemeral = ephemeral;
    }

    static void writeDbProperties(XMLWriter writer,
                                  java.util.List properties)
        throws java.io.IOException
    {
        java.util.Iterator p = properties.iterator();
        while(p.hasNext())
        {
            PropertyDescriptor pd = (PropertyDescriptor)p.next();
            java.util.List attributes = new java.util.LinkedList();
            attributes.add(createAttribute("name", pd.name));
            attributes.add(createAttribute("value", pd.value));
            writer.writeElement("dbproperty", attributes);
        }
    }

    void write(XMLWriter writer) throws java.io.IOException
    {
        if(!_ephemeral)
        {
            java.util.List attributes = new java.util.LinkedList();
            attributes.add(createAttribute("name", _descriptor.name));
            if(_descriptor.dbHome.length() > 0)
            {
                attributes.add(createAttribute("home", _descriptor.dbHome));
            }
                           
            if(_descriptor.description.length() == 0 && 
               _descriptor.properties.isEmpty())
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
