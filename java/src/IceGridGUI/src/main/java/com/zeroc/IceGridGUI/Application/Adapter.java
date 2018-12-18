// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package com.zeroc.IceGridGUI.Application;

import java.awt.Component;
import javax.swing.JTree;
import javax.swing.tree.DefaultTreeCellRenderer;

import com.zeroc.IceGrid.*;
import com.zeroc.IceGridGUI.*;

class Adapter extends TreeNode implements DescriptorHolder
{
    static class AdapterCopy
    {
        AdapterDescriptor descriptor;
        java.util.Map<String, String> parentProperties;
    }

    static public AdapterDescriptor copyDescriptor(AdapterDescriptor d)
    {
        return d.clone();
    }

    static public java.util.List<AdapterDescriptor>
    copyDescriptors(java.util.List<AdapterDescriptor> descriptors)
    {
        java.util.List<AdapterDescriptor> copy = new java.util.LinkedList<>();
        for(AdapterDescriptor p : descriptors)
        {
            copy.add(copyDescriptor(p));
        }
        return copy;
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
        actions[PASTE] = clipboard != null && (clipboard instanceof AdapterCopy ||
            clipboard instanceof DbEnvDescriptor);

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
        AdapterCopy copy = new AdapterCopy();
        copy.descriptor = copyDescriptor(_descriptor);
        copy.parentProperties = ((Communicator)_parent).propertiesMap();

        getCoordinator().setClipboard(copy);
        getCoordinator().getActionsForMenu().get(PASTE).setEnabled(true);
    }

    @Override
    public void paste()
    {
        ((TreeNode)_parent).paste();
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
            _cellRenderer.setLeafIcon(Utils.getIcon("/icons/16x16/adapter_inactive.png"));
        }

        return _cellRenderer.getTreeCellRendererComponent(tree, value, sel, expanded, leaf, row, hasFocus);
    }

    @Override
    public Editor getEditor()
    {
        if(_editor == null)
        {
            _editor = (AdapterEditor)getRoot().getEditor(AdapterEditor.class, this);
        }
        _editor.show(this);
        return _editor;
    }

    @Override
    protected Editor createEditor()
    {
        return new AdapterEditor();
    }

    @Override
    public void destroy()
    {
        removeProperty(_descriptor.name + ".Endpoints");
        removeProperty(_descriptor.name + ".PublishedEndpoints");
        removeProperty(_descriptor.name + ".ProxyOptions");

        ((Communicator)_parent).getAdapters().destroyChild(this);
    }

    @Override
    Object getDescriptor()
    {
        return _descriptor;
    }

    @Override
    public Object saveDescriptor()
    {
        return copyDescriptor(_descriptor);
    }

    @Override
    public void restoreDescriptor(Object savedDescriptor)
    {
        AdapterDescriptor ad = (AdapterDescriptor)savedDescriptor;

        _descriptor.name = ad.name;
        _descriptor.id = ad.id;
        _descriptor.replicaGroupId = ad.replicaGroupId;
        _descriptor.description = ad.description;
        _descriptor.registerProcess = ad.registerProcess;
        _descriptor.serverLifetime = ad.serverLifetime;
        _descriptor.objects = ad.objects;
    }

    Adapter(Communicator parent, String adapterName, AdapterDescriptor descriptor,
            java.util.Map<String, String> parentProperties, boolean ephemeral)
    {
        super(parent, adapterName);
        _descriptor = descriptor;
        _ephemeral = ephemeral;
        _parentProperties = parentProperties;
    }

    @Override
    void write(XMLWriter writer)
        throws java.io.IOException
    {
        assert false;
    }

    void write(XMLWriter writer, java.util.List<PropertyDescriptor> properties)
        throws java.io.IOException
    {
        if(!_ephemeral)
        {
            java.util.List<String[]> attributes = new java.util.LinkedList<>();
            attributes.add(createAttribute("name", _descriptor.name));
            String oaPrefix = _descriptor.name + ".";

            String value = getProperty(oaPrefix + "Endpoints");
            if(value != null && value.length() > 0)
            {
                attributes.add(createAttribute("endpoints", value));
            }

            value = getProperty(oaPrefix + "ProxyOptions");
            if(value != null && value.length() > 0)
            {
                attributes.add(createAttribute("proxy-options", value));
            }

            attributes.add(createAttribute("id", _descriptor.id));
            if(_descriptor.registerProcess)
            {
                attributes.add(createAttribute("register-process", "true"));
            }
            if(_descriptor.replicaGroupId.length() > 0)
            {
                attributes.add(createAttribute("replica-group", _descriptor.replicaGroupId));
            }
            if(_descriptor.priority != null && _descriptor.priority.length() > 0)
            {
                attributes.add(createAttribute("priority", _descriptor.priority));
            }
            if(!_descriptor.serverLifetime)
            {
                attributes.add(createAttribute("server-lifetime", "false"));
            }

            if(_descriptor.description.length() == 0
               && _descriptor.objects.isEmpty()
               && _descriptor.allocatables.isEmpty())
            {
                writer.writeElement("adapter", attributes);
            }
            else
            {
                writer.writeStartTag("adapter", attributes);

                if(_descriptor.description.length() > 0)
                {
                    writer.writeElement("description", _descriptor.description);
                }
                writeObjects("object", writer, _descriptor.objects, properties);
                writeObjects("allocatable", writer, _descriptor.allocatables, properties);
                writer.writeEndTag("adapter");
            }
        }
    }

    String getProperty(String property)
    {
        if(_parentProperties != null)
        {
            return _parentProperties.get(property);
        }
        else
        {
            return ((Communicator)_parent).getProperty(property);
        }
    }

    String lookupPropertyValue(String val)
    {
        if(_parentProperties != null)
        {
            for(java.util.Map.Entry<String, String> p : _parentProperties.entrySet())
            {
                if(p.getValue().equals(val))
                {
                    return p.getKey();
                }
            }
            return "";
        }
        else
        {
            return ((Communicator)_parent).lookupPropertyValue(val);
        }
    }

    void setProperty(String property, String newValue)
    {
        ((Communicator)_parent).setProperty(property, newValue);
    }

    void removeProperty(String property)
    {
        ((Communicator)_parent).removeProperty(property);
    }

    String getDefaultAdapterId()
    {
        return getDefaultAdapterId(_id);
    }

    String getDefaultAdapterId(String name)
    {
        return (_parent instanceof Service ||
                _parent instanceof ServiceTemplate) ?
            "${server}.${service}." + name: "${server}." + name;
    }

    @Override
    public boolean isEphemeral()
    {
        return _ephemeral;
    }

    private final boolean _ephemeral;
    private final java.util.Map<String, String> _parentProperties; // set only when ephemeral == true;
    private AdapterDescriptor _descriptor;
    private AdapterEditor _editor;

    static private DefaultTreeCellRenderer _cellRenderer;
}
