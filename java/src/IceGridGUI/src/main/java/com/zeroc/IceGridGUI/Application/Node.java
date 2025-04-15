// Copyright (c) ZeroC, Inc.

package com.zeroc.IceGridGUI.Application;

import com.zeroc.IceGridGUI.ApplicationActions;
import com.zeroc.IceGridGUI.TreeNodeBase;
import com.zeroc.IceGridGUI.Utils;
import com.zeroc.IceGridGUI.XMLWriter;

import java.awt.Component;
import java.io.IOException;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.TreeMap;
import java.util.Vector;

import javax.swing.Icon;
import javax.swing.JOptionPane;
import javax.swing.JPopupMenu;
import javax.swing.JTree;
import javax.swing.tree.DefaultTreeCellRenderer;
import javax.swing.tree.DefaultTreeModel;

import com.zeroc.IceGrid.BoxedString;
import com.zeroc.IceGrid.IceBoxDescriptor;
import com.zeroc.IceGrid.NodeDescriptor;
import com.zeroc.IceGrid.NodeUpdateDescriptor;
import com.zeroc.IceGrid.PropertyDescriptor;
import com.zeroc.IceGrid.PropertySetDescriptor;
import com.zeroc.IceGrid.ServerDescriptor;
import com.zeroc.IceGrid.ServerInstanceDescriptor;
import com.zeroc.IceGrid.TemplateDescriptor;

class Node extends TreeNode implements PropertySetParent {
    public static NodeDescriptor copyDescriptor(NodeDescriptor nd) {
        NodeDescriptor copy = nd.clone();

        copy.propertySets = PropertySets.copyDescriptors(copy.propertySets);

        copy.serverInstances = new LinkedList<>();
        for (ServerInstanceDescriptor p : nd.serverInstances) {
            copy.serverInstances.add(ServerInstance.copyDescriptor(p));
        }

        copy.servers = new LinkedList<>();
        for (ServerDescriptor p : nd.servers) {
            copy.servers.add(PlainServer.copyDescriptor(p));
        }

        return copy;
    }

    @Override
    public Enumeration<javax.swing.tree.TreeNode> children() {
        return new Enumeration<javax.swing.tree.TreeNode>() {
            @Override
            public boolean hasMoreElements() {
                return _p.hasNext() || _q.hasNext();
            }

            @Override
            public javax.swing.tree.TreeNode nextElement() {
                if (_p.hasNext()) {
                    return _p.next();
                } else {
                    return (javax.swing.tree.TreeNode) _q.next();
                }
            }

            private Iterator<PropertySet> _p = _propertySets.iterator();
            private Iterator<Server> _q = _servers.iterator();
        };
    }

    @Override
    public boolean getAllowsChildren() {
        return true;
    }

    @Override
    public javax.swing.tree.TreeNode getChildAt(int childIndex) {
        if (childIndex < 0) {
            throw new ArrayIndexOutOfBoundsException(childIndex);
        } else if (childIndex < _propertySets.size()) {
            return _propertySets.get(childIndex);
        } else if (childIndex < (_propertySets.size() + _servers.size())) {
            return (javax.swing.tree.TreeNode) _servers.get(childIndex - _propertySets.size());
        } else {
            throw new ArrayIndexOutOfBoundsException(childIndex);
        }
    }

    @Override
    public int getChildCount() {
        return _propertySets.size() + _servers.size();
    }

    @Override
    public int getIndex(javax.swing.tree.TreeNode node) {
        if (node instanceof PropertySet) {
            return _propertySets.indexOf(node);
        } else {
            int index = _servers.indexOf(node);
            if (index != -1) {
                index += _propertySets.size();
            }
            return index;
        }
    }

    @Override
    public boolean isLeaf() {
        return _propertySets.isEmpty() && _servers.isEmpty();
    }

    void removeServers(String[] childIds) {
        removeSortedChildren(childIds, _servers, getRoot().getTreeModel());
    }

    void removePropertySets(String[] childIds) {
        removeSortedChildren(childIds, _propertySets, getRoot().getTreeModel());
    }

    void childrenChanged(List children) {
        childrenChanged(children, getRoot().getTreeModel());
    }

    Server findServer(String id) {
        return (Server) find(id, _servers);
    }

    PropertySet findPropertySet(String id) {
        return (PropertySet) find(id, _propertySets);
    }

    void insertPropertySets(List<PropertySet> newChildren, boolean fireEvent)
        throws UpdateFailedException {
        DefaultTreeModel treeModel = fireEvent ? getRoot().getTreeModel() : null;

        String badChildId = insertSortedChildren(newChildren, _propertySets, treeModel);

        if (badChildId != null) {
            throw new UpdateFailedException(this, badChildId);
        }
    }

    void insertServer(TreeNode child, boolean fireEvent) throws UpdateFailedException {
        DefaultTreeModel treeModel = fireEvent ? getRoot().getTreeModel() : null;

        if (!insertSortedChild(child, _servers, treeModel)) {
            throw new UpdateFailedException(this, child.getId());
        }
    }

    void insertServers(List<Server> newChildren, boolean fireEvent)
        throws UpdateFailedException {
        DefaultTreeModel treeModel = fireEvent ? getRoot().getTreeModel() : null;

        String badChildId = insertSortedChildren(newChildren, _servers, treeModel);

        if (badChildId != null) {
            throw new UpdateFailedException(this, badChildId);
        }
    }

    void removeServer(TreeNode child) {
        int index = getIndex(child);
        _servers.remove(child);

        getRoot().getTreeModel().nodesWereRemoved(this, new int[]{index}, new Object[]{child});
    }

    @Override
    public void insertPropertySet(PropertySet child, boolean fireEvent)
        throws UpdateFailedException {
        DefaultTreeModel treeModel = fireEvent ? getRoot().getTreeModel() : null;

        if (!insertSortedChild(child, _propertySets, treeModel)) {
            throw new UpdateFailedException(this, child.getId());
        }
    }

    @Override
    public void removePropertySet(PropertySet child) {
        int index = getIndex(child);
        _propertySets.remove(child);

        getRoot().getTreeModel().nodesWereRemoved(this, new int[]{index}, new Object[]{child});
    }

    @Override
    public void removeDescriptor(String id) {
        _descriptor.propertySets.remove(id);
    }

    @Override
    public Editable getEditable() {
        return _editable;
    }

    @Override
    public boolean[] getAvailableActions() {
        boolean[] actions = new boolean[ACTION_COUNT];

        actions[COPY] = !_ephemeral;
        actions[DELETE] = true;

        Object descriptor = getCoordinator().getClipboard();
        if (descriptor != null) {
            actions[PASTE] =
                descriptor instanceof NodeDescriptor
                    || descriptor instanceof ServerInstanceDescriptor
                    || descriptor instanceof ServerDescriptor
                    || descriptor instanceof PropertySetDescriptor;
        }

        if (!_ephemeral) {
            actions[SHOW_VARS] = true;
            actions[SUBSTITUTE_VARS] = true;
            actions[NEW_PROPERTY_SET] = true;
            actions[NEW_SERVER] = true;
            actions[NEW_SERVER_ICEBOX] = true;
            actions[NEW_SERVER_FROM_TEMPLATE] = true;
        }
        return actions;
    }

    @Override
    public JPopupMenu getPopupMenu() {
        ApplicationActions actions = getCoordinator().getActionsForPopup();
        if (_popup == null) {
            _popup = new JPopupMenu();
            _popup.add(actions.get(NEW_PROPERTY_SET));
            _popup.addSeparator();
            _popup.add(actions.get(NEW_SERVER));
            _popup.add(actions.get(NEW_SERVER_ICEBOX));
            _popup.add(actions.get(NEW_SERVER_FROM_TEMPLATE));
        }
        actions.setTarget(this);
        return _popup;
    }

    @Override
    public void copy() {
        getCoordinator().setClipboard(copyDescriptor(_descriptor));
        getCoordinator().getActionsForMenu().get(PASTE).setEnabled(true);
    }

    @Override
    public void paste() {
        Object descriptor = getCoordinator().getClipboard();
        if (descriptor instanceof NodeDescriptor) {
            ((TreeNode) _parent).paste();
        } else if (descriptor instanceof PropertySetDescriptor) {
            newPropertySet(PropertySet.copyDescriptor((PropertySetDescriptor) descriptor));
        } else if (descriptor instanceof ServerInstanceDescriptor) {
            // Remove any extra parameters
            ServerInstanceDescriptor sid =
                ServerInstance.copyDescriptor((ServerInstanceDescriptor) descriptor);

            TemplateDescriptor td = getRoot().findServerTemplateDescriptor(sid.template);

            if (td != null) {
                sid.parameterValues.keySet().retainAll(td.parameters);
            }

            newServer(sid);
        } else {
            ServerDescriptor sd = PlainServer.copyDescriptor((ServerDescriptor) descriptor);
            if (sd instanceof IceBoxDescriptor) {
                if (!getRoot().pasteIceBox((IceBoxDescriptor) sd)) {
                    return;
                }
            }
            newServer(sd);
        }
    }

    @Override
    public void newPropertySet() {
        newPropertySet(
            new PropertySetDescriptor(
                new String[0], new LinkedList<PropertyDescriptor>()));
    }

    @Override
    public void newServer() {
        newServer(PlainServer.newServerDescriptor());
    }

    @Override
    public void newServerIceBox() {
        newServer(PlainServer.newIceBoxDescriptor());
    }

    @Override
    public void newServerFromTemplate() {
        ServerInstanceDescriptor descriptor =
            new ServerInstanceDescriptor(
                "",
                new HashMap<String, String>(),
                new PropertySetDescriptor(
                    new String[0], new LinkedList<PropertyDescriptor>()),
                new HashMap<String, PropertySetDescriptor>());

        newServer(descriptor);
    }

    @Override
    public void destroy() {
        Nodes nodes = (Nodes) _parent;
        if (_ephemeral) {
            nodes.removeChild(this);
        } else {
            nodes.removeChild(this);
            nodes.removeDescriptor(_id);
            nodes.getEditable().removeElement(_id, _editable, Node.class);
            getRoot().updated();
        }
    }

    @Override
    public Component getTreeCellRendererComponent(
            JTree tree,
            Object value,
            boolean sel,
            boolean expanded,
            boolean leaf,
            int row,
            boolean hasFocus) {
        if (_cellRenderer == null) {
            // Initialization
            _cellRenderer = new DefaultTreeCellRenderer();
            Icon nodeIcon = Utils.getIcon("/icons/16x16/node.png");
            _cellRenderer.setOpenIcon(nodeIcon);
            _cellRenderer.setClosedIcon(nodeIcon);
        }

        return _cellRenderer.getTreeCellRendererComponent(
            tree, value, sel, expanded, leaf, row, hasFocus);
    }

    @Override
    public Editor getEditor() {
        if (_editor == null) {
            _editor = (NodeEditor) getRoot().getEditor(NodeEditor.class, this);
        }
        _editor.show(this);
        return _editor;
    }

    @Override
    protected Editor createEditor() {
        return new NodeEditor();
    }

    @Override
    public boolean isEphemeral() {
        return _ephemeral;
    }

    @Override
    Object getDescriptor() {
        return _descriptor;
    }

    NodeDescriptor saveDescriptor() {
        return _descriptor.clone();
    }

    void restoreDescriptor(NodeDescriptor copy) {
        _descriptor.description = copy.description;
        _descriptor.loadFactor = copy.loadFactor;
        _descriptor.variables = copy.variables;
    }

    @Override
    void write(XMLWriter writer) throws IOException {
        if (!_ephemeral) {
            List<String[]> attributes = new LinkedList<>();
            attributes.add(createAttribute("name", _id));
            if (_descriptor.loadFactor.length() > 0) {
                attributes.add(createAttribute("load-factor", _descriptor.loadFactor));
            }

            writer.writeStartTag("node", attributes);

            if (_descriptor.description.length() > 0) {
                writer.writeElement("description", _descriptor.description);
            }
            writeVariables(writer, _descriptor.variables);

            for (PropertySet p : _propertySets) {
                p.write(writer);
            }

            for (Server p : _servers) {
                TreeNode server = (TreeNode) p;
                server.write(writer);
            }

            writer.writeEndTag("node");
        }
    }

    static class Backup {
        Utils.Resolver resolver;
        List<Object> backupList;
        List<Server> servers;
    }

    // Try to rebuild this node; returns a backup object if rollback is later necessary We don't
    // rebuild the property sets since they don't depend on the variables.
    Backup rebuild(List<Editable> editables) throws UpdateFailedException {
        Root root = getRoot();
        Backup backup = new Backup();
        backup.resolver = _resolver;

        @SuppressWarnings("unchecked")
        Utils.Resolver resolver =
            new Utils.Resolver(
                new Map[]{_descriptor.variables, root.getVariables()});
        _resolver = resolver;

        _resolver.put("application", root.getId());
        _resolver.put("node", _id);

        backup.backupList = new Vector<>();
        backup.servers = new LinkedList<>(_servers);

        for (Server p : backup.servers) {
            try {
                backup.backupList.add(p.rebuild(editables));
            } catch (UpdateFailedException e) {
                restore(backup);
                throw e;
            }
        }
        return backup;
    }

    void commit() {
        _editable.commit();
        _origVariables = _descriptor.variables;
        _origDescription = _descriptor.description;
        _origLoadFactor = _descriptor.loadFactor;

        for (PropertySet p : _propertySets) {
            p.commit();
        }

        for (Server p : _servers) {
            p.getEditable().commit();
        }
    }

    void restore(Backup backup) {
        for (int i = backup.backupList.size() - 1; i >= 0; i--) {
            backup.servers.get(i).restore(backup.backupList.get(i));
        }
        _resolver = backup.resolver;
    }

    ServerInstance createServer(boolean brandNew, ServerInstanceDescriptor instanceDescriptor)
        throws UpdateFailedException {
        Root root = getRoot();

        // Find template
        TemplateDescriptor templateDescriptor =
            root.findServerTemplateDescriptor(instanceDescriptor.template);

        if (templateDescriptor == null) {
            throw new UpdateFailedException(
                "Cannot find template descriptor '"
                    + instanceDescriptor.template
                    + "' referenced by server-instance");
        }
        ServerDescriptor serverDescriptor = (ServerDescriptor) templateDescriptor.descriptor;

        assert serverDescriptor != null;
        boolean isIceBox = serverDescriptor instanceof IceBoxDescriptor;

        // Build resolver
        Utils.Resolver instanceResolver =
            new Utils.Resolver(
                _resolver,
                instanceDescriptor.parameterValues,
                templateDescriptor.parameterDefaults);

        String serverId = instanceResolver.substitute(serverDescriptor.id);
        instanceResolver.put("server", serverId);

        // Create server
        return new ServerInstance(
            brandNew, this, serverId, instanceResolver, instanceDescriptor, isIceBox);
    }

    PlainServer createServer(boolean brandNew, ServerDescriptor serverDescriptor)
        throws UpdateFailedException {
        // Build resolver
        Utils.Resolver instanceResolver = new Utils.Resolver(_resolver);
        String serverId = instanceResolver.substitute(serverDescriptor.id);
        instanceResolver.put("server", serverId);

        // Create server
        return new PlainServer(brandNew, this, serverId, instanceResolver, serverDescriptor);
    }

    NodeUpdateDescriptor getUpdate() {
        NodeUpdateDescriptor update = new NodeUpdateDescriptor();
        update.name = _id;

        // First: property sets
        if (_editable.isNew()) {
            update.removePropertySets = new String[0];
            update.propertySets = _descriptor.propertySets;
        } else {
            update.removePropertySets = _editable.removedElements(PropertySet.class);
            update.propertySets = new HashMap<>();

            for (PropertySet p : _propertySets) {
                if (p.getEditable().isNew() || p.getEditable().isModified()) {
                    update.propertySets.put(p.getId(), (PropertySetDescriptor) p.getDescriptor());
                }
            }
        }

        // Then: servers
        if (_editable.isNew()) {
            update.removeServers = new String[0];
        } else {
            update.removeServers = _editable.removedElements(Server.class);
        }

        update.serverInstances = new LinkedList<>();
        update.servers = new LinkedList<>();

        for (Server p : _servers) {
            if (_editable.isNew() || p.getEditable().isModified() || p.getEditable().isNew()) {
                if (p instanceof PlainServer) {
                    update.servers.add((ServerDescriptor) p.getDescriptor());
                } else {
                    update.serverInstances.add((ServerInstanceDescriptor) p.getDescriptor());
                }
            }
        }

        // Anything in this update?
        if (!_editable.isNew()
            && !_editable.isModified()
            && update.removePropertySets.length == 0
            && update.propertySets.isEmpty()
            && update.removeServers.length == 0
            && update.servers.isEmpty()
            && update.serverInstances.isEmpty()) {
            return null;
        }

        if (_editable.isNew()) {
            update.variables = _descriptor.variables;
            update.removeVariables = new String[0];
            update.loadFactor = new BoxedString(_descriptor.loadFactor);
            update.description = new BoxedString(_descriptor.description);
        } else {
            if (!_descriptor.description.equals(_origDescription)) {
                update.description = new BoxedString(_descriptor.description);
            }

            if (!_descriptor.loadFactor.equals(_origLoadFactor)) {
                update.loadFactor = new BoxedString(_descriptor.loadFactor);
            }

            // Diff variables (TODO: avoid duplication with same code in Root)
            update.variables = new TreeMap<>(_descriptor.variables);
            List<String> removeVariables = new LinkedList<>();

            for (Map.Entry<String, String> p : _origVariables.entrySet()) {
                String key = p.getKey();
                String newValue = update.variables.get(key);
                if (newValue == null) {
                    removeVariables.add(key);
                } else {
                    String value = p.getValue();
                    if (newValue.equals(value)) {
                        update.variables.remove(key);
                    }
                }
            }
            update.removeVariables = removeVariables.toArray(new String[0]);
        }

        return update;
    }

    void update(
            NodeUpdateDescriptor update,
            Set<String> serverTemplates,
            Set<String> serviceTemplates)
        throws UpdateFailedException {
        Root root = getRoot();

        Vector<Server> newServers = new Vector<>();
        Vector<Server> updatedServers = new Vector<>();

        if (update != null) {
            // Description
            if (update.description != null) {
                _descriptor.description = update.description.value;
                _origDescription = _descriptor.description;
            }

            // Load factor
            if (update.loadFactor != null) {
                _descriptor.loadFactor = update.loadFactor.value;
                _origLoadFactor = _descriptor.loadFactor;
            }

            // Variables
            for (String name : update.removeVariables) {
                _descriptor.variables.remove(name);
            }
            _descriptor.variables.putAll(update.variables);

            // Property Sets
            removePropertySets(update.removePropertySets);
            for (String id : update.removePropertySets) {
                _descriptor.propertySets.remove(id);
            }

            Vector<PropertySet> newPropertySets = new Vector<>();
            Vector<PropertySet> updatedPropertySets = new Vector<>();

            for (Map.Entry<String, PropertySetDescriptor> p :
                    update.propertySets.entrySet()) {
                String id = p.getKey();
                PropertySetDescriptor psd = p.getValue();

                // Lookup named property set
                PropertySet ps = findPropertySet(id);
                if (ps != null) {
                    ps.rebuild(psd);
                    updatedPropertySets.add(ps);
                } else {
                    ps = new PropertySet(false, this, id, id, psd);
                    newPropertySets.add(ps);
                    _descriptor.propertySets.put(id, psd);
                }
            }
            childrenChanged(updatedPropertySets);
            insertPropertySets(newPropertySets, true);

            // Update _descriptor
            for (String id : update.removeServers) {
                Server server = findServer(id);
                removeDescriptor(server);
            }

            // One big set of removes
            removeServers(update.removeServers);

            // One big set of updates, followed by inserts
            for (ServerInstanceDescriptor p : update.serverInstances) {
                // Find template
                TemplateDescriptor templateDescriptor =
                    root.findServerTemplateDescriptor(p.template);

                assert templateDescriptor != null;

                ServerDescriptor serverDescriptor =
                    (ServerDescriptor) templateDescriptor.descriptor;

                assert serverDescriptor != null;

                // Build resolver
                Utils.Resolver instanceResolver =
                    new Utils.Resolver(
                        _resolver, p.parameterValues, templateDescriptor.parameterDefaults);

                String serverId = instanceResolver.substitute(serverDescriptor.id);
                instanceResolver.put("server", serverId);

                // Lookup servers
                ServerInstance server = (ServerInstance) findServer(serverId);
                if (server != null) {
                    removeDescriptor(server);
                    server.rebuild(
                        instanceResolver, p, serverDescriptor instanceof IceBoxDescriptor);
                    updatedServers.add(server);
                    _descriptor.serverInstances.add(p);
                } else {
                    server =
                        new ServerInstance(
                            false,
                            this,
                            serverId,
                            instanceResolver,
                            p,
                            serverDescriptor instanceof IceBoxDescriptor);
                    newServers.add(server);
                    _descriptor.serverInstances.add(p);
                }
            }

            // Plain servers
            for (ServerDescriptor p : update.servers) {
                // Build resolver
                Utils.Resolver instanceResolver = new Utils.Resolver(_resolver);
                String serverId = instanceResolver.substitute(p.id);
                instanceResolver.put("server", serverId);

                // Lookup server
                PlainServer server = (PlainServer) findServer(serverId);

                if (server != null) {
                    removeDescriptor(server);
                    server.rebuild(instanceResolver, p);
                    updatedServers.add(server);
                    _descriptor.servers.add(p);
                } else {
                    server = new PlainServer(false, this, serverId, instanceResolver, p);
                    newServers.add(server);
                    _descriptor.servers.add(p);
                }
            }
        }

        // Find servers affected by template updates
        Set<Server> serverSet = new HashSet<>();

        for (String p : serverTemplates) {
            List<ServerInstance> serverInstances = findServerInstances(p);
            for (ServerInstance q : serverInstances) {
                if (!updatedServers.contains(q) && !newServers.contains(q)) {
                    serverSet.add(q);
                }
            }
        }

        // Servers affected by service-template updates
        for (String p : serviceTemplates) {
            List<ServiceInstance> serviceInstances = findServiceInstances(p);
            for (ServiceInstance q : serviceInstances) {
                Server server = (Server) q.getParent().getParent();
                if (!updatedServers.contains(server) && !newServers.contains(server)) {
                    serverSet.add(server);
                }
            }
        }

        // Rebuild these servers
        for (Server p : serverSet) {
            if (p instanceof PlainServer) {
                PlainServer ps = (PlainServer) p;
                ServerDescriptor serverDescriptor = (ServerDescriptor) ps.getDescriptor();
                Utils.Resolver instanceResolver = new Utils.Resolver(_resolver);

                String serverId = instanceResolver.substitute(serverDescriptor.id);
                assert serverId.equals(ps.getId());

                ps.rebuild(instanceResolver, serverDescriptor);
            } else {
                ServerInstance si = (ServerInstance) p;
                ServerInstanceDescriptor instanceDescriptor =
                    (ServerInstanceDescriptor) si.getDescriptor();

                TemplateDescriptor templateDescriptor =
                    root.findServerTemplateDescriptor(instanceDescriptor.template);
                assert templateDescriptor != null;

                ServerDescriptor serverDescriptor =
                    (ServerDescriptor) templateDescriptor.descriptor;
                assert serverDescriptor != null;

                Utils.Resolver instanceResolver =
                    new Utils.Resolver(
                        _resolver,
                        instanceDescriptor.parameterValues,
                        templateDescriptor.parameterDefaults);

                String serverId = instanceResolver.substitute(serverDescriptor.id);
                assert serverId.equals(si.getId());

                si.rebuild(
                    instanceResolver,
                    instanceDescriptor,
                    serverDescriptor instanceof IceBoxDescriptor);
            }
            updatedServers.add(p);
        }

        childrenChanged(updatedServers);
        insertServers(newServers, true);
    }

    Node(boolean brandNew, TreeNode parent, String nodeName, NodeDescriptor descriptor)
        throws UpdateFailedException {
        super(parent, nodeName);
        _editable = new Editable(brandNew);

        _ephemeral = false;
        _descriptor = descriptor;

        _origVariables = _descriptor.variables;
        _origDescription = _descriptor.description;
        _origLoadFactor = _descriptor.loadFactor;

        @SuppressWarnings("unchecked")
        Utils.Resolver resolver =
            new Utils.Resolver(
                new Map[]{_descriptor.variables, getRoot().getVariables()});
        _resolver = resolver;

        _resolver.put("application", getRoot().getId());
        _resolver.put("node", _id);

        // Property Sets
        for (Map.Entry<String, PropertySetDescriptor> p :
                _descriptor.propertySets.entrySet()) {
            String id = p.getKey();
            insertPropertySet(new PropertySet(false, this, id, id, p.getValue()), false);
        }

        // Template instances
        for (ServerInstanceDescriptor p : _descriptor.serverInstances) {
            insertServer(createServer(false, p), false);
        }

        // Plain servers
        for (ServerDescriptor p : _descriptor.servers) {
            insertServer(createServer(false, p), false);
        }
    }

    Node(TreeNode parent, String nodeName, NodeDescriptor descriptor) {
        super(parent, nodeName);
        _editable = new Editable(false);
        _ephemeral = true;
        _descriptor = descriptor;
    }

    List<ServerInstance> findServerInstances(String template) {
        List<ServerInstance> result = new LinkedList<>();
        for (Server p : _servers) {
            if (p instanceof ServerInstance) {
                ServerInstanceDescriptor instanceDescriptor =
                    (ServerInstanceDescriptor) p.getDescriptor();

                if (instanceDescriptor.template.equals(template)) {
                    result.add((ServerInstance) p);
                }
            }
        }
        return result;
    }

    void removeServerInstances(String template) {
        List<String> toRemove = new LinkedList<>();

        for (Server p : _servers) {
            if (p instanceof ServerInstance) {
                ServerInstanceDescriptor instanceDescriptor =
                    (ServerInstanceDescriptor) p.getDescriptor();

                if (instanceDescriptor.template.equals(template)) {
                    //
                    // Remove instance
                    //
                    removeDescriptor(instanceDescriptor);
                    String id = ((TreeNode) p).getId();
                    _editable.removeElement(id, p.getEditable(), Server.class);
                    toRemove.add(id);
                }
            }
        }

        if (!toRemove.isEmpty()) {
            removeServers(toRemove.toArray(new String[0]));
        }
    }

    List<ServiceInstance> findServiceInstances(String template) {
        List<ServiceInstance> result = new LinkedList<>();
        for (Server p : _servers) {
            if (p instanceof PlainServer) {
                result.addAll(((PlainServer) p).findServiceInstances(template));
            }
        }
        return result;
    }

    void removeServiceInstances(String template) {
        for (Server p : _servers) {
            if (p instanceof PlainServer) {
                ((PlainServer) p).removeServiceInstances(template);
            }
        }
    }

    @Override
    Utils.Resolver getResolver() {
        return _resolver;
    }

    @Override
    public void tryAdd(String id, PropertySetDescriptor descriptor) throws UpdateFailedException {
        insertPropertySet(new PropertySet(true, this, id, id, descriptor), true);
        _descriptor.propertySets.put(id, descriptor);
    }

    @Override
    public void tryRename(String oldId, String oldId2, String newId) throws UpdateFailedException {
        PropertySet oldChild = findPropertySet(oldId);
        assert oldChild != null;
        removePropertySet(oldChild);
        PropertySetDescriptor descriptor = (PropertySetDescriptor) oldChild.getDescriptor();

        try {
            insertPropertySet(new PropertySet(true, this, newId, newId, descriptor), true);
        } catch (UpdateFailedException ex) {
            try {
                insertPropertySet(oldChild, true);
            } catch (UpdateFailedException ufe) {
                assert false;
            }
            throw ex;
        }

        _editable.removeElement(oldId, oldChild.getEditable(), PropertySet.class);
        _descriptor.propertySets.remove(oldId);
        _descriptor.propertySets.put(newId, descriptor);
    }

    void tryAdd(ServerInstanceDescriptor instanceDescriptor, boolean addDescriptor)
        throws UpdateFailedException {
        insertServer(createServer(true, instanceDescriptor), true);

        if (addDescriptor) {
            _descriptor.serverInstances.add(instanceDescriptor);
        }
    }

    void tryAdd(ServerDescriptor serverDescriptor, boolean addDescriptor)
        throws UpdateFailedException {
        insertServer(createServer(true, serverDescriptor), true);

        if (addDescriptor) {
            _descriptor.servers.add(serverDescriptor);
        }
    }

    void removeDescriptor(Server server) {
        if (server instanceof ServerInstance) {
            removeDescriptor((ServerInstanceDescriptor) server.getDescriptor());
        } else {
            removeDescriptor((ServerDescriptor) server.getDescriptor());
        }
    }

    void removeDescriptor(ServerDescriptor sd) {
        // A straight remove uses equals(), which is not the desired behavior
        Iterator<ServerDescriptor> p = _descriptor.servers.iterator();
        while (p.hasNext()) {
            if (sd == p.next()) {
                p.remove();
                break;
            }
        }
    }

    void removeDescriptor(ServerInstanceDescriptor sd) {
        // A straight remove uses equals(), which is not the desired behavior
        Iterator<ServerInstanceDescriptor> p = _descriptor.serverInstances.iterator();
        while (p.hasNext()) {
            if (sd == p.next()) {
                p.remove();
                break;
            }
        }
    }

    private void newPropertySet(PropertySetDescriptor descriptor) {
        String id = makeNewChildId("PropertySet");

        PropertySet ps = new PropertySet(this, id, descriptor);
        try {
            insertPropertySet(ps, true);
        } catch (UpdateFailedException e) {
            assert false;
        }
        getRoot().setSelectedNode(ps);
    }

    private void newServer(ServerDescriptor descriptor) {
        descriptor.id = makeNewChildId(descriptor.id);

        PlainServer server = new PlainServer(this, descriptor.id, descriptor);
        try {
            insertServer(server, true);
        } catch (UpdateFailedException e) {
            assert false;
        }
        getRoot().setSelectedNode(server);
    }

    private void newServer(ServerInstanceDescriptor descriptor) {
        String id = makeNewChildId("NewServer");
        Root root = getRoot();

        // Make sure descriptor.template points to a real template
        ServerTemplate t = root.findServerTemplate(descriptor.template);

        if (t == null) {
            if (root.getServerTemplates().getChildCount() == 0) {
                JOptionPane.showMessageDialog(
                    getCoordinator().getMainFrame(),
                    "You need to create a server template before you can create a server from a template.",
                    "No Server Template",
                    JOptionPane.INFORMATION_MESSAGE);
                return;
            }

            t = (ServerTemplate) root.getServerTemplates().getChildAt(0);
            descriptor.template = t.getId();
            descriptor.parameterValues = new HashMap<>();
        }

        ServerInstance server = new ServerInstance(this, id, descriptor);
        try {
            insertServer(server, true);
        } catch (UpdateFailedException e) {
            assert false;
        }
        root.setSelectedNode(server);
    }

    private NodeDescriptor _descriptor;
    private Utils.Resolver _resolver;

    private Map<String, String> _origVariables;
    private String _origDescription;
    private String _origLoadFactor;

    private final boolean _ephemeral;
    private NodeEditor _editor;

    private LinkedList<PropertySet> _propertySets = new LinkedList<>();
    private LinkedList<Server> _servers = new LinkedList<>();

    private Editable _editable;

    private static DefaultTreeCellRenderer _cellRenderer;
    private static JPopupMenu _popup;
}
