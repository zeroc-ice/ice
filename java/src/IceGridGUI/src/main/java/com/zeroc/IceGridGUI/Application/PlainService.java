// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceGridGUI.Application;

import java.awt.Component;
import javax.swing.JPopupMenu;
import javax.swing.JTree;
import javax.swing.tree.DefaultTreeCellRenderer;

import com.zeroc.IceGrid.*;
import com.zeroc.IceGridGUI.*;

class PlainService extends Communicator implements Service, Cloneable
{
    static public ServiceDescriptor
    copyDescriptor(ServiceDescriptor sd)
    {
        ServiceDescriptor copy = sd.clone();
        copy.adapters = Adapter.copyDescriptors(copy.adapters);
        copy.dbEnvs = DbEnv.copyDescriptors(copy.dbEnvs);
        copy.propertySet = PropertySet.copyDescriptor(copy.propertySet);
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
            _cellRenderer.setOpenIcon(Utils.getIcon("/icons/16x16/service.png"));

            _cellRenderer.setClosedIcon(Utils.getIcon("/icons/16x16/service.png"));
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
        if(clipboard != null &&
           (clipboard instanceof ServiceInstanceDescriptor
            || clipboard instanceof Adapter.AdapterCopy
            || clipboard instanceof DbEnvDescriptor))
        {
            actions[PASTE] = true;
        }

        actions[DELETE] = true;
        actions[NEW_ADAPTER] = !_ephemeral;
        actions[NEW_DBENV] = !_ephemeral;

        if(_parent instanceof Server && !_ephemeral)
        {
            actions[SHOW_VARS] = true;
            actions[SUBSTITUTE_VARS] = true;
        }

        actions[MOVE_UP] = canMove(true);
        actions[MOVE_DOWN] = canMove(false);
        return actions;
    }

    @Override
    public JPopupMenu getPopupMenu()
    {
        ApplicationActions actions = getCoordinator().getActionsForPopup();
        if(_popup == null)
        {
            _popup = new JPopupMenu();
            _popup.add(actions.get(NEW_ADAPTER));
            _popup.add(actions.get(NEW_DBENV));
            _popup.addSeparator();
            _popup.add(actions.get(MOVE_UP));
            _popup.add(actions.get(MOVE_DOWN));
        }
        actions.setTarget(this);
        return _popup;
    }

    @Override
    public void copy()
    {
        getCoordinator().setClipboard(ServiceInstance.copyDescriptor(_descriptor));
        getCoordinator().getActionsForMenu().get(PASTE).setEnabled(true);
    }

    @Override
    public void moveUp()
    {
        move(true);
    }

    @Override
    public void moveDown()
    {
        move(false);
    }

    @Override
    public Object getDescriptor()
    {
        return _descriptor;
    }

    @Override
    public Object saveDescriptor()
    {
        return _descriptor.clone();
    }

    @Override
    public void restoreDescriptor(Object savedDescriptor)
    {
        ServiceInstanceDescriptor sid = (ServiceInstanceDescriptor)savedDescriptor;

        _descriptor.descriptor.propertySet = sid.descriptor.propertySet;
        _descriptor.descriptor.description = sid.descriptor.description;
        _descriptor.descriptor.name = sid.descriptor.name;
        _descriptor.descriptor.entry = sid.descriptor.entry;
    }

    @Override
    public void destroy()
    {
        ((Communicator)_parent).getServices().destroyChild(this);
    }

    @Override
    public Editor getEditor()
    {
        if(_editor == null)
        {
            _editor = (PlainServiceEditor)getRoot().getEditor(PlainServiceEditor.class, this);
        }
        _editor.show(this);
        return _editor;
    }

    @Override
    protected Editor createEditor()
    {
        return new PlainServiceEditor();
    }

    @Override
    Editable getEnclosingEditable()
    {
        return ((Communicator)_parent).getEnclosingEditable();
    }

    private boolean canMove(boolean up)
    {
        if(_ephemeral)
        {
            return false;
        }
        else
        {
            return ((Communicator)_parent).getServices().canMove(this, up);
        }
    }

    private void move(boolean up)
    {
        assert canMove(up);
        ((Communicator)_parent).getServices().move(this, up);
    }

    @Override
    public Object rebuild(java.util.List<Editable> editables)
        throws UpdateFailedException
    {
        Communicator communicator = (Communicator)_parent;
        Services services = communicator.getServices();
        PlainService newService = null;

        newService = (PlainService)services.createChild(_descriptor);

        Object backup = null;

        try
        {
            backup = clone();
        }
        catch(CloneNotSupportedException e)
        {
            assert false;
        }

        reset(newService);
        getRoot().getTreeModel().nodeChanged(this);
        return backup;
    }

    @Override
    public void restore(Object backupObj)
    {
        reset((PlainService)backupObj);
        getRoot().getTreeModel().nodeChanged(this);
    }

    private void reset(PlainService from)
    {
        _id = from._id;
        assert _parent == from._parent;

        _adapters = from._adapters;
        _dbEnvs = from._dbEnvs;
        _services = from._services;
        _childListArray = from._childListArray;

        _descriptor = from._descriptor;
        _resolver = from._resolver;
    }

    PlainService(Communicator parent, String name, ServiceInstanceDescriptor descriptor, Utils.Resolver resolver)
        throws UpdateFailedException
    {
        super(parent, name);
        _descriptor = descriptor;
        _ephemeral = false;
        _resolver = resolver;

        _adapters.init(_descriptor.descriptor.adapters);
        _dbEnvs.init(_descriptor.descriptor.dbEnvs);
    }

    //
    // New temporary object
    //
    PlainService(Communicator parent, ServiceInstanceDescriptor descriptor)
    {
        super(parent, descriptor.descriptor.name);
        _descriptor = descriptor;
        _ephemeral = true;
    }

    static java.util.List<String[]> createAttributes(ServiceDescriptor descriptor)
    {
        java.util.List<String[]> attributes = new java.util.LinkedList<String[]>();
        attributes.add(createAttribute("name", descriptor.name));
        attributes.add(createAttribute("entry", descriptor.entry));
        return attributes;
    }

    @Override
    void write(XMLWriter writer)
        throws java.io.IOException
    {
        if(!_ephemeral)
        {
            writer.writeStartTag("service", createAttributes(_descriptor.descriptor));

            if(_descriptor.descriptor.description.length() > 0)
            {
                writer.writeElement("description", _descriptor.descriptor.description);
            }

            writePropertySet(writer,  _descriptor.descriptor.propertySet,
                             _descriptor.descriptor.adapters, _descriptor.descriptor.logs);
            writeLogs(writer, _descriptor.descriptor.logs, _descriptor.descriptor.propertySet.properties);

            _adapters.write(writer, _descriptor.descriptor.propertySet.properties);
            _dbEnvs.write(writer);
            writer.writeEndTag("service");
        }
    }

    @Override
    CommunicatorDescriptor getCommunicatorDescriptor()
    {
        return _descriptor.descriptor;
    }

    @Override
    Utils.Resolver getResolver()
    {
        return _resolver;
    }

    @Override
    public boolean isEphemeral()
    {
        return _ephemeral;
    }

    private ServiceInstanceDescriptor _descriptor;

    private final boolean _ephemeral;

    private Utils.Resolver _resolver;
    private PlainServiceEditor _editor;

    static private DefaultTreeCellRenderer _cellRenderer;
    static private JPopupMenu _popup;
}
