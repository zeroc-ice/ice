// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceGridGUI.Application;

import java.awt.Component;

import javax.swing.Icon;
import javax.swing.JPopupMenu;
import javax.swing.JTree;

import javax.swing.tree.DefaultTreeCellRenderer;

import IceGrid.*;
import IceGridGUI.*;

class ServerInstance extends ListTreeNode implements Server, PropertySetParent
{
    static public ServerInstanceDescriptor
    copyDescriptor(ServerInstanceDescriptor sid)
    {
        ServerInstanceDescriptor copy = sid.clone();
        copy.propertySet = PropertySet.copyDescriptor(copy.propertySet);
        return copy;
    }

    //
    // Overrides ListTreeNode
    //
    @Override
    public boolean getAllowsChildren()
    {
        return _isIceBox;
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
           (clipboard instanceof ServerDescriptor
            || clipboard instanceof ServerInstanceDescriptor
            || (_isIceBox && clipboard instanceof PropertySetDescriptor)))
        {
            actions[PASTE] = true;
        }

        actions[DELETE] = true;
        if(!_ephemeral)
        {
            actions[SHOW_VARS] = true;
            actions[SUBSTITUTE_VARS] = true;

            if(_isIceBox)
            {
                actions[NEW_PROPERTY_SET] = true;
            }
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
        if(_isIceBox)
        {
            Object descriptor =  getCoordinator().getClipboard();
            if(descriptor instanceof PropertySetDescriptor)
            {
                newPropertySet(PropertySet.copyDescriptor((PropertySetDescriptor)descriptor));
                return;
            }
        }

        ((TreeNode)_parent).paste();
    }

    @Override
    public void newPropertySet()
    {
        newPropertySet(new PropertySetDescriptor(new String[0], new java.util.LinkedList<PropertyDescriptor>()));
    }

    @Override
    public JPopupMenu getPopupMenu()
    {
        if(_isIceBox)
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
        else
        {
            return null;
        }
    }

    @Override
    public Editor getEditor()
    {
        if(_editor == null)
        {
            _editor = (ServerInstanceEditor)getRoot().getEditor(ServerInstanceEditor.class, this);
        }
        _editor.show(this);
        return _editor;
    }

    @Override
    protected Editor createEditor()
    {
        return new ServerInstanceEditor();
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

            _cellRenderer.setLeafIcon(_serverIcon);
            _cellRenderer.setOpenIcon(_iceboxServerIcon);
            _cellRenderer.setClosedIcon(_iceboxServerIcon);
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
        return _descriptor.clone();
    }

    @Override
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
    ServerInstance(boolean brandNew, TreeNode parent, String serverId, Utils.Resolver resolver,
                   ServerInstanceDescriptor instanceDescriptor, boolean isIceBox)
        throws UpdateFailedException
    {
        super(brandNew, parent, serverId);
        _ephemeral = false;
        rebuild(resolver, instanceDescriptor, isIceBox);
    }

    ServerInstance(TreeNode parent, String serverId, ServerInstanceDescriptor instanceDescriptor)
    {
        super(false, parent, serverId);
        _ephemeral = true;
        try
        {
            rebuild(null, instanceDescriptor, false);
        }
        catch(UpdateFailedException e)
        {
            assert false;
        }
    }

    @Override
    void write(XMLWriter writer)
        throws java.io.IOException
    {
        if(!_ephemeral)
        {
            TemplateDescriptor templateDescriptor = getRoot().findServerTemplateDescriptor(_descriptor.template);

            java.util.LinkedList<String[]> attributes = parameterValuesToAttributes(
                _descriptor.parameterValues, templateDescriptor.parameters);
            attributes.addFirst(createAttribute("template", _descriptor.template));

            if(_descriptor.propertySet.references.length == 0 && _descriptor.propertySet.properties.size() == 0 &&
               _children.size() == 0)
            {
                writer.writeElement("server-instance", attributes);
            }
            else
            {
                writer.writeStartTag("server-instance", attributes);
                writePropertySet(writer, _descriptor.propertySet, null, null);

                for(TreeNodeBase p : _children)
                {
                    PropertySet ps = (PropertySet)p;
                    ps.write(writer);
                }

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
        if(newValue != _isIceBox)
        {
            _isIceBox = newValue;
            _children.clear();
            getRoot().getTreeModel().nodeStructureChanged(this);
        }
    }

    static private class Backup
    {
        Backup(Editable ne)
        {
            nodeEditable = ne;
        }

        Editable nodeEditable;
        java.util.Map<String, String> parameterValues;
    }

    @Override
    public Object rebuild(java.util.List<Editable> editables)
        throws UpdateFailedException
    {
        Node node = (Node)_parent;
        Backup backup = new Backup(node.getEditable().save());

        TemplateDescriptor templateDescriptor = getRoot().findServerTemplateDescriptor(_descriptor.template);

        java.util.Set<String> parameters = new java.util.HashSet<String>(templateDescriptor.parameters);
        if(!parameters.equals(_descriptor.parameterValues.keySet()))
        {
            backup.parameterValues = _descriptor.parameterValues;
            _descriptor.parameterValues = Editor.makeParameterValues(
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

    @Override
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

    @Override
    public void tryAdd(String unsubstitutedId, PropertySetDescriptor descriptor)
        throws UpdateFailedException
    {
        insertPropertySet(new PropertySet(this,
                                          Utils.substitute(unsubstitutedId, _resolver),
                                          unsubstitutedId,
                                          descriptor),
                          true);
        _descriptor.servicePropertySets.put(unsubstitutedId, descriptor);
        _editable.markModified();
    }

    @Override
    public void tryRename(String oldId, String oldUnresolvedId, String newUnsubstitutedId)
        throws UpdateFailedException
    {
        PropertySet oldChild = (PropertySet)findChild(oldId);
        assert oldChild != null;
        removePropertySet(oldChild);
        PropertySetDescriptor descriptor = (PropertySetDescriptor)oldChild.getDescriptor();

        try
        {
            insertPropertySet(
                new PropertySet(this,
                                Utils.substitute(newUnsubstitutedId, _resolver),
                                newUnsubstitutedId, descriptor),
                true);
        }
        catch(UpdateFailedException ex)
        {
            try
            {
                insertPropertySet(oldChild, true);
            }
            catch(UpdateFailedException ufe)
            {
                assert false;
            }
            throw ex;
        }

        _editable.markModified();
        _descriptor.servicePropertySets.remove(oldUnresolvedId);
        _descriptor.servicePropertySets.put(newUnsubstitutedId, descriptor);
    }

    @Override
    public void insertPropertySet(PropertySet nps, boolean fireEvent)
        throws UpdateFailedException
    {
        insertChild(nps, fireEvent);
    }

    @Override
    public void removePropertySet(PropertySet nps)
    {
        removeChild(nps);
    }

    @Override
    public void removeDescriptor(String unsubstitutedId)
    {
        _descriptor.servicePropertySets.remove(unsubstitutedId);
    }

    @Override
    public Editable getEditable()
    {
        return _editable;
    }

    String[] getServiceNames()
    {
        assert _isIceBox;

        //
        // Retrieve the list of service instances
        //

        Communicator.ChildList services = getRoot().findServerTemplate(_descriptor.template).getServices();

        String[] result = new String[services.size()];
        int i = 0;

        java.util.Iterator p = services.iterator();
        while(p.hasNext())
        {
            TreeNode n = (TreeNode)p.next();
            ServiceInstanceDescriptor d = (ServiceInstanceDescriptor)n.getDescriptor();

            if(d.template.length() > 0)
            {
                TemplateDescriptor templateDescriptor =
                    getRoot().findServiceTemplateDescriptor(d.template);
                assert templateDescriptor != null;
                Utils.Resolver serviceResolver = new Utils.Resolver(_resolver,
                                                                    d.parameterValues,
                                                                    templateDescriptor.parameterDefaults);

                ServiceDescriptor serviceDescriptor = (ServiceDescriptor)templateDescriptor.descriptor;

                result[i++] = serviceResolver.substitute(serviceDescriptor.name);
            }
            else
            {
                result[i++] = _resolver.substitute(d.descriptor.name);
            }
        }
        return result;
    }

    //
    // Update the server and its children
    //
    void rebuild(Utils.Resolver resolver, ServerInstanceDescriptor instanceDescriptor, boolean isIceBox)
        throws UpdateFailedException
    {
        _resolver = resolver;
        _isIceBox = isIceBox;
        _descriptor = instanceDescriptor;

        _children.clear();

        for(java.util.Map.Entry<String, PropertySetDescriptor> p : _descriptor.servicePropertySets.entrySet())
        {
            String unsubstitutedId = p.getKey();
            insertPropertySet(new PropertySet(this,
                                              Utils.substitute(unsubstitutedId, _resolver),
                                              unsubstitutedId,
                                              p.getValue()),
                              false);
        }
    }

    private void newPropertySet(PropertySetDescriptor descriptor)
    {
        String id = makeNewChildId("Service");

        PropertySet ps = new PropertySet(this, id, descriptor);
        try
        {
            insertChild(ps, true);
        }
        catch(UpdateFailedException e)
        {
            assert false;
        }
        getRoot().setSelectedNode(ps);
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

    @Override
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

    static private DefaultTreeCellRenderer _cellRenderer;
    static private Icon _serverIcon;
    static private Icon _iceboxServerIcon;
    static private JPopupMenu _popup;
}
