// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceGridGUI.Application;

import java.awt.Component;

import javax.swing.Icon;
import javax.swing.JPopupMenu;
import javax.swing.JTree;

import javax.swing.tree.DefaultTreeCellRenderer;

import com.zeroc.IceGrid.*;
import com.zeroc.IceGridGUI.*;

class PlainServer extends Communicator implements Server
{
    static public ServerDescriptor
    copyDescriptor(ServerDescriptor sd)
    {
        ServerDescriptor copy = sd.clone();

        copy.adapters = Adapter.copyDescriptors(copy.adapters);
        copy.dbEnvs = DbEnv.copyDescriptors(copy.dbEnvs);

        copy.propertySet = PropertySet.copyDescriptor(copy.propertySet);

        copy.distrib = copy.distrib.clone();

        if(copy instanceof IceBoxDescriptor)
        {
            IceBoxDescriptor ib = (IceBoxDescriptor)copy;
            ib.services = ServiceInstance.copyDescriptors(ib.services);
        }
        return copy;
    }

    static public void shallowRestore(ServerDescriptor from, ServerDescriptor into)
    {
        //
        // When editing a server or server template, if we update properties,
        // we replace the entire field
        //
        into.propertySet = from.propertySet;
        into.description = from.description;
        into.id = from.id;
        into.exe = from.exe;
        into.options = from.options;
        into.envs = from.envs;
        into.activation = from.activation;
        into.activationTimeout = from.activationTimeout;
        into.deactivationTimeout = from.deactivationTimeout;
        into.applicationDistrib = from.applicationDistrib;
        into.distrib.icepatch = from.distrib.icepatch;
        into.distrib.directories = from.distrib.directories;
    }

    static public ServerDescriptor newServerDescriptor()
    {
        return new ServerDescriptor(
            new java.util.LinkedList<AdapterDescriptor>(),
            new PropertySetDescriptor(new String[0], new java.util.LinkedList<PropertyDescriptor>()),
            new java.util.LinkedList<DbEnvDescriptor>(),
            new String[0],
            "",
            "NewServer",
            "",
            "",
            "",
            new java.util.LinkedList<String>(),
            new java.util.LinkedList<String>(),
            "manual",
            "",
            "",
            true,
            new DistributionDescriptor("", new java.util.LinkedList<String>()),
            false, // Allocatable
            "");
    }

    static public IceBoxDescriptor newIceBoxDescriptor()
    {
        return new IceBoxDescriptor(
            new java.util.LinkedList<AdapterDescriptor>(),
            new PropertySetDescriptor(new String[0], new java.util.LinkedList<PropertyDescriptor>()),
            new java.util.LinkedList<DbEnvDescriptor>(),
            new String[0],
            "",
            "NewIceBox",
            "",
            "",
            "",
            new java.util.LinkedList<String>(),
            new java.util.LinkedList<String>(),
            "manual",
            "",
            "",
            true,
            new DistributionDescriptor("", new java.util.LinkedList<String>()),
            false, // Allocatable
            "",
            new java.util.LinkedList<ServiceInstanceDescriptor>()
            );
    }

    //
    // Actions
    //
    @Override
    public boolean[] getAvailableActions()
    {
        boolean[] actions = new boolean[ACTION_COUNT];

        Object clipboard = getCoordinator().getClipboard();
        if(clipboard != null &&
           (clipboard instanceof ServerDescriptor
            || clipboard instanceof ServerInstanceDescriptor
            || (isIceBox() && (clipboard instanceof ServiceInstanceDescriptor))
            || (!isIceBox() && (clipboard instanceof Adapter.AdapterCopy
                                || clipboard instanceof DbEnvDescriptor))))
        {
            actions[PASTE] = true;
        }

        actions[DELETE] = true;
        if(!_ephemeral)
        {
            actions[COPY] = true;
            actions[SHOW_VARS] = true;
            actions[SUBSTITUTE_VARS] = true;

            actions[NEW_ADAPTER] = !_services.initialized();
            actions[NEW_DBENV] = !_services.initialized();
            actions[NEW_SERVICE] = _services.initialized();
            actions[NEW_SERVICE_FROM_TEMPLATE] = _services.initialized();
        }
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
            _popup.add(actions.get(NEW_SERVICE));
            _popup.add(actions.get(NEW_SERVICE_FROM_TEMPLATE));
        }
        actions.setTarget(this);
        return _popup;
    }

    @Override
    public void copy()
    {
        getCoordinator().setClipboard(copyDescriptor(_descriptor));
        getCoordinator().getActionsForMenu().get(PASTE).setEnabled(true);
    }

    @Override
    public Editor getEditor()
    {
        if(_editor == null)
        {
            _editor = (PlainServerEditor)getRoot().getEditor(PlainServerEditor.class, this);
        }
        _editor.show(this);
        return _editor;
    }

    @Override
    protected Editor createEditor()
    {
        return new PlainServerEditor();
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
            //
            // Initialization
            //
            _cellRenderer = new DefaultTreeCellRenderer();

            _serverIcon = Utils.getIcon("/icons/16x16/server_inactive.png");
            _iceboxServerIcon = Utils.getIcon("/icons/16x16/icebox_server_inactive.png");
        }

        if(expanded)
        {
            _cellRenderer.setOpenIcon(isIceBox() ? _iceboxServerIcon : _serverIcon);
        }
        else
        {
            _cellRenderer.setClosedIcon(isIceBox() ? _iceboxServerIcon : _serverIcon);
        }
        return _cellRenderer.getTreeCellRendererComponent(tree, value, sel, expanded, leaf, row, hasFocus);
    }

    @Override
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

    @Override
    public Object getDescriptor()
    {
        return _descriptor;
    }

    @Override
    public Object saveDescriptor()
    {
        ServerDescriptor clone = _descriptor.clone();
        clone.distrib = clone.distrib.clone();
        return clone;
    }

    @Override
    public void restoreDescriptor(Object savedDescriptor)
    {
        shallowRestore((ServerDescriptor)savedDescriptor, _descriptor);
    }

    //
    // Builds the server and all its sub-tree
    //
    PlainServer(boolean brandNew, TreeNode parent, String serverId, Utils.Resolver resolver,
                ServerDescriptor serverDescriptor)
        throws UpdateFailedException
    {
        super(parent, serverId);
        _ephemeral = false;
        _editable = new Editable(brandNew);
        rebuild(resolver, serverDescriptor);
    }

    PlainServer(TreeNode parent, String serverId, ServerDescriptor serverDescriptor)
    {
        super(parent, serverId);
        _ephemeral = true;
        _editable = null;
        try
        {
            rebuild(null, serverDescriptor);
        }
        catch(UpdateFailedException e)
        {
            assert false;
        }
    }

    static java.util.List<String[]> createAttributes(ServerDescriptor descriptor)
    {
        java.util.List<String[]> attributes = new java.util.LinkedList<>();
        attributes.add(createAttribute("id", descriptor.id));
        if(descriptor.activation.length() > 0)
        {
            attributes.add(createAttribute("activation", descriptor.activation));
        }
        if(descriptor.activationTimeout.length() > 0)
        {
            attributes.add(createAttribute("activation-timeout", descriptor.activationTimeout));
        }
        if(!descriptor.applicationDistrib)
        {
            attributes.add(createAttribute("application-distrib", "false"));
        }
        if(descriptor.deactivationTimeout.length() > 0)
        {
            attributes.add(createAttribute("deactivation-timeout", descriptor.deactivationTimeout));
        }
        if(descriptor.exe.length() > 0)
        {
            attributes.add(createAttribute("exe", descriptor.exe));
        }
        if(descriptor.iceVersion.length() > 0)
        {
            attributes.add(createAttribute("ice-version", descriptor.iceVersion));
        }
        if(descriptor.pwd.length() > 0)
        {
            attributes.add(createAttribute("pwd", descriptor.pwd));
        }

        return attributes;
    }

    static void writeOptions(XMLWriter writer, java.util.List<String> options)
        throws java.io.IOException
    {
        for(String p : options)
        {
            writer.writeElement("option", p);
        }
    }

    static void writeEnvs(XMLWriter writer, java.util.List<String> envs)
        throws java.io.IOException
    {
        for(String p : envs)
        {
            writer.writeElement("env", p);
        }
    }

    @Override
    void write(XMLWriter writer)
        throws java.io.IOException
    {
        if(!_ephemeral)
        {
            if(isIceBox())
            {
                writer.writeStartTag("icebox", createAttributes(_descriptor));

                if(_descriptor.description.length() > 0)
                {
                    writer.writeElement("description", _descriptor.description);
                }
                writeOptions(writer, _descriptor.options);
                writeEnvs(writer, _descriptor.envs);

                writePropertySet(writer, "", "", _descriptor.propertySet, _descriptor.adapters, _descriptor.logs);
                writeLogs(writer, _descriptor.logs, _descriptor.propertySet.properties);
                writeDistribution(writer, _descriptor.distrib);

                _adapters.write(writer, _descriptor.propertySet.properties);
                _services.write(writer);
                writer.writeEndTag("icebox");
            }
            else
            {
                writer.writeStartTag("server", createAttributes(_descriptor));

                if(_descriptor.description.length() > 0)
                {
                    writer.writeElement("description", _descriptor.description);
                }

                writeOptions(writer, _descriptor.options);
                writeEnvs(writer, _descriptor.envs);

                writePropertySet(writer, _descriptor.propertySet, _descriptor.adapters, _descriptor.logs);
                writeLogs(writer, _descriptor.logs, _descriptor.propertySet.properties);
                writeDistribution(writer, _descriptor.distrib);

                _adapters.write(writer, _descriptor.propertySet.properties);
                _dbEnvs.write(writer);
                writer.writeEndTag("server");
            }
        }
    }

    @Override
    boolean isIceBox()
    {
        return _descriptor instanceof IceBoxDescriptor;
    }

    @Override
    public Object rebuild(java.util.List<Editable> editables)
        throws UpdateFailedException
    {
        Node node = (Node)_parent;
        PlainServer newServer = node.createServer(false, _descriptor);

        Object backup = null;

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
        }
        else
        {
            //
            // Typically, the id uses a variable whose definition changed
            //
            newServer.getEditable().markNew();
            node.removeServer(this);
            backup = node.getEditable().save();
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

    @Override
    public void restore(Object backupObj)
    {
        Editable backup = (Editable)backupObj;
        Node node = (Node)_parent;

        if(backup != null)
        {
            node.getEditable().restore(backup);
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

    void setServerDescriptor(ServerDescriptor descriptor)
    {
        _descriptor = descriptor;
    }

    //
    // Update the server and all its subtree
    //
    void rebuild(Utils.Resolver resolver, ServerDescriptor serverDescriptor)
        throws UpdateFailedException
    {
        assert serverDescriptor != null;
        _resolver = resolver;
        _descriptor = serverDescriptor;

        _adapters.clear();
        _dbEnvs.clear();
        _services.clear();

        if(!_ephemeral)
        {
            _adapters.init(_descriptor.adapters);
            if(isIceBox())
            {
                IceBoxDescriptor iceBoxDescriptor = (IceBoxDescriptor)_descriptor;
                _services.init(iceBoxDescriptor.services);

                //
                // IceBox has not dbEnv
                //
                assert _descriptor.dbEnvs.size() == 0;
            }
            else
            {
                _dbEnvs.init(_descriptor.dbEnvs);
            }
        }
    }

    @Override
    CommunicatorDescriptor getCommunicatorDescriptor()
    {
        return _descriptor;
    }

    @Override
    Utils.Resolver getResolver()
    {
        return _resolver;
    }

    @Override
    public Editable getEditable()
    {
        return _editable;
    }

    @Override
    Editable getEnclosingEditable()
    {
        return _editable;
    }

    @Override
    public boolean isEphemeral()
    {
        return _ephemeral;
    }

    private ServerDescriptor _descriptor;
    private final boolean _ephemeral;

    private PlainServerEditor _editor;

    private Utils.Resolver _resolver;
    private Editable _editable;

    static private DefaultTreeCellRenderer _cellRenderer;
    static private Icon _serverIcon;
    static private Icon _iceboxServerIcon;

    static private JPopupMenu _popup;
}
