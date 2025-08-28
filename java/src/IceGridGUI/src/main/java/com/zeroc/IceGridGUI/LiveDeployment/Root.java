// Copyright (c) ZeroC, Inc.

package com.zeroc.IceGridGUI.LiveDeployment;

import com.zeroc.Ice.Identity;
import com.zeroc.Ice.LocalException;
import com.zeroc.Ice.ObjectPrx;
import com.zeroc.IceGrid.AdapterDynamicInfo;
import com.zeroc.IceGrid.AdapterInfo;
import com.zeroc.IceGrid.AdminPrx;
import com.zeroc.IceGrid.AdminSessionPrx;
import com.zeroc.IceGrid.ApplicationDescriptor;
import com.zeroc.IceGrid.ApplicationInfo;
import com.zeroc.IceGrid.ApplicationUpdateInfo;
import com.zeroc.IceGrid.DeploymentException;
import com.zeroc.IceGrid.FileIteratorPrx;
import com.zeroc.IceGrid.FileNotAvailableException;
import com.zeroc.IceGrid.NodeDescriptor;
import com.zeroc.IceGrid.NodeDynamicInfo;
import com.zeroc.IceGrid.NodeUpdateDescriptor;
import com.zeroc.IceGrid.ObjectExistsException;
import com.zeroc.IceGrid.ObjectInfo;
import com.zeroc.IceGrid.PropertySetDescriptor;
import com.zeroc.IceGrid.RegistryInfo;
import com.zeroc.IceGrid.RegistryNotExistException;
import com.zeroc.IceGrid.RegistryUnreachableException;
import com.zeroc.IceGrid.ReplicaGroupDescriptor;
import com.zeroc.IceGrid.ServerDynamicInfo;
import com.zeroc.IceGridGUI.Coordinator;
import com.zeroc.IceGridGUI.LiveActions;
import com.zeroc.IceGridGUI.TreeNodeBase;
import com.zeroc.IceGridGUI.Utils;

import java.awt.Component;
import java.text.DateFormat;
import java.util.Arrays;
import java.util.Date;
import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.SortedMap;
import java.util.TreeMap;
import java.util.concurrent.CompletableFuture;
import java.util.prefs.Preferences;

import javax.swing.Icon;
import javax.swing.JDialog;
import javax.swing.JOptionPane;
import javax.swing.JPopupMenu;
import javax.swing.JTree;
import javax.swing.SwingUtilities;
import javax.swing.event.TreeExpansionEvent;
import javax.swing.event.TreeWillExpandListener;
import javax.swing.tree.DefaultTreeCellRenderer;
import javax.swing.tree.DefaultTreeModel;
import javax.swing.tree.ExpandVetoException;

/** The Root node of the Live Deployment view */
public class Root extends Communicator {
    /** A custom tree model to filter tree views. */
    class FilteredTreeModel extends DefaultTreeModel {
        public FilteredTreeModel(TreeNode root) {
            super(root, true);
        }

        public void resetFilters() {
            Object[] path = {root};
            int[] childIndices = new int[root.getChildCount()];
            Object[] children = new Object[root.getChildCount()];
            for (int i = 0; i < root.getChildCount(); i++) {
                childIndices[i] = i;
                children[i] = root.getChildAt(i);
            }
            fireTreeStructureChanged(this, path, childIndices, children);
        }

        @Override
        public int getChildCount(Object parent) {
            if (!filterEnabled()) {
                return super.getChildCount(parent);
            }
            int p = super.getChildCount(parent);
            int q = 0;
            for (int j = 0; j < p; j++) {
                TreeNode node = (TreeNode) super.getChild(parent, j);
                if (matchFilter(node)) {
                    q++;
                }
            }
            return q;
        }

        @Override
        public Object getChild(Object parent, int index) {
            if (!filterEnabled()) {
                return super.getChild(parent, index);
            }
            Object child = null;
            int p = 0;
            int q = super.getChildCount(parent);
            for (int j = 0; j < q; j++) {
                TreeNode node = (TreeNode) super.getChild(parent, j);
                if (!matchFilter(node)) {
                    continue;
                }

                if (p == index) {
                    child = node;
                    break;
                }
                p++;
                if (p > index) {
                    break;
                }
            }
            return child;
        }
    }

    private boolean matchFilter(TreeNode n) {
        if (_applicationNameFilter == null) {
            return true;
        }

        if (n instanceof Server) {
            Server server = (Server) n;
            if (!_applicationNameFilter.equals(server.getApplication().name)) {
                return false;
            }
        } else if (n instanceof Node) {
            return ((Node) n).hasServersFromApplication(_applicationNameFilter);
        }
        return true;
    }

    private boolean filterEnabled() {
        return _applicationNameFilter != null;
    }

    public void setApplicationNameFilter(String name) {
        _applicationNameFilter = name;
        _label = _instanceName + " (" + _replicaName + ")";
        if (_applicationNameFilter != null) {
            _label += " - " + _applicationNameFilter;
        }
        _treeModel.resetFilters();
    }

    public String getApplicationNameFilter() {
        return _applicationNameFilter;
    }

    public Root(Coordinator coordinator) {
        super(null, "Root", 3);
        _coordinator = coordinator;
        _childrenArray[0] = _metrics;
        _childrenArray[1] = _slaves;
        _childrenArray[2] = _nodes;
        _messageSizeMax =
            computeMessageSizeMax(
                _coordinator.getProperties().getIcePropertyAsInt("Ice.MessageSizeMax"));

        _treeModel = new FilteredTreeModel(this);
        _tree = new JTree();
        _tree.setModel(_treeModel);
        _addObjectDialog = new ObjectDialog(this, false);
        _showObjectDialog = new ObjectDialog(this, true);

        _tree.addTreeWillExpandListener(
            new TreeWillExpandListener() {
                @Override
                public void treeWillExpand(TreeExpansionEvent event) {
                    // Fetch metrics when Communicator node is expanded.
                    TreeNode node = (TreeNode) event.getPath().getLastPathComponent();
                    if (node instanceof Communicator) {
                        ((Communicator) node).fetchMetricsViewNames();
                    }
                }

                @Override
                public void treeWillCollapse(TreeExpansionEvent event)
                    throws ExpandVetoException {
                    if (event.getPath().getLastPathComponent() == Root.this) {
                        throw new ExpandVetoException(event);
                    }
                }
            });

        loadLogPrefs();
    }

    @Override
    public boolean[] getAvailableActions() {
        boolean[] actions = new boolean[TreeNode.ACTION_COUNT];
        actions[ADD_OBJECT] = _coordinator.connectedToMaster();
        actions[SHUTDOWN_REGISTRY] = true;
        actions[RETRIEVE_ICE_LOG] = true;
        actions[RETRIEVE_STDOUT] = true;
        actions[RETRIEVE_STDERR] = true;
        return actions;
    }

    @Override
    public void shutdownRegistry() {
        final String prefix = "Shutting down registry '" + _replicaName + "'...";
        final String errorTitle = "Failed to shutdown " + _replicaName;
        _coordinator.getStatusBar().setText(prefix);
        try {
            final AdminPrx admin = _coordinator.getAdmin();
            admin.shutdownRegistryAsync(_replicaName)
                .whenComplete(
                    (result, ex) -> {
                        amiComplete(prefix, errorTitle, ex);
                    });
        } catch (LocalException ex) {
            failure(prefix, errorTitle, ex.toString());
        }
    }

    public ApplicationDescriptor getApplicationDescriptor(String name) {
        ApplicationInfo app = _infoMap.get(name);
        if (app == null) {
            return null;
        } else {
            return app.descriptor;
        }
    }

    public Object[] getApplicationNames() {
        return _infoMap.keySet().toArray();
    }

    public SortedMap<String, String> getApplicationMap() {
        SortedMap<String, String> r = new TreeMap<>();

        for (Map.Entry<String, ApplicationInfo> p : _infoMap.entrySet()) {
            ApplicationInfo app = p.getValue();

            r.put(
                p.getKey(),
                DateFormat.getDateTimeInstance()
                    .format(new Date(app.updateTime)));
        }
        return r;
    }

    @Override
    public Editor getEditor() {
        if (_editor == null) {
            _editor = new RegistryEditor();
        }
        _editor.show(this);
        return _editor;
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

            Icon icon = Utils.getIcon("/icons/16x16/registry.png");
            _cellRenderer.setOpenIcon(icon);
            _cellRenderer.setClosedIcon(icon);
        }

        return _cellRenderer.getTreeCellRendererComponent(
            tree, value, sel, expanded, leaf, row, hasFocus);
    }

    public void applicationInit(
            String instanceName, String replicaName, List<ApplicationInfo> applications) {
        _instanceName = instanceName;
        _replicaName = replicaName;
        _label = instanceName + " (" + _replicaName + ")";
        _tree.setRootVisible(true);

        for (ApplicationInfo p : applications) {
            applicationAdded(p);
        }
        _treeModel.resetFilters();
    }

    // Called when the session to the IceGrid Registry is closed
    public void clear() {
        _adapters.clear();
        _objects.clear();
        _replicaName = null;
        _applicationNameFilter = null;

        _infoMap.clear();
        _nodes.clear();
        _slaves.clear();
        _metrics.clear();
        _metricsRetrieved = false;
        _treeModel.nodeStructureChanged(this);
        _tree.setRootVisible(false);
        _editor = null;
    }

    public void showApplicationDetails(String appName) {
        ApplicationInfo app = _infoMap.get(appName);
        if (app != null) {
            if (_applicationDetailsDialog == null) {
                _applicationDetailsDialog = new ApplicationDetailsDialog(this);
            }

            _applicationDetailsDialog.showDialog(app);
        }
    }

    // From the Registry Observer:
    public void applicationAdded(ApplicationInfo info) {
        _infoMap.put(info.descriptor.name, info);

        for (Map.Entry<String, NodeDescriptor> p : info.descriptor.nodes.entrySet()) {
            String nodeName = p.getKey();
            NodeDescriptor nodeDesc = p.getValue();

            Node node = findNode(nodeName);
            if (node == null) {
                insertNode(new Node(this, info.descriptor, nodeName, nodeDesc));
            } else {
                node.add(info.descriptor, nodeDesc);
            }
        }
    }

    public void applicationRemoved(String name) {
        _infoMap.remove(name);

        List<Node> toRemove = new LinkedList<>();
        int[] toRemoveIndices = new int[_nodes.size()];

        int i = 0;
        for (Node node : _nodes) {
            if (node.remove(name)) {
                toRemove.add(node);
                toRemoveIndices[i++] = getIndex(node);
            }
        }

        if (!toRemove.isEmpty()) {
            _nodes.removeAll(toRemove);
            _treeModel.nodesWereRemoved(this, toRemoveIndices, toRemove.toArray());
        }
    }

    public void applicationUpdated(ApplicationUpdateInfo update) {
        ApplicationInfo app = _infoMap.get(update.descriptor.name);

        app.updateTime = update.updateTime;
        app.updateUser = update.updateUser;
        app.revision = update.revision;

        ApplicationDescriptor appDesc = app.descriptor;

        // Update various fields of appDesc
        if (update.descriptor.description != null) {
            appDesc.description = update.descriptor.description.value;
        }

        appDesc.variables
            .keySet()
            .removeAll(Arrays.asList(update.descriptor.removeVariables));
        appDesc.variables.putAll(update.descriptor.variables);
        boolean variablesChanged =
            update.descriptor.removeVariables.length > 0
                || !update.descriptor.variables.isEmpty();

        // Update only descriptors (no tree node shown in this view)
        appDesc.propertySets
            .keySet()
            .removeAll(Arrays.asList(update.descriptor.removePropertySets));
        appDesc.propertySets.putAll(update.descriptor.propertySets);

        for (String id : update.descriptor.removeReplicaGroups) {
            for (int i = 0; i < appDesc.replicaGroups.size(); i++) {
                ReplicaGroupDescriptor rgd = appDesc.replicaGroups.get(i);
                if (rgd.id.equals(id)) {
                    appDesc.replicaGroups.remove(i);
                    break; // for
                }
            }
        }

        for (ReplicaGroupDescriptor newRgd : update.descriptor.replicaGroups) {
            boolean replaced = false;
            int i = 0;
            while (i < appDesc.replicaGroups.size() && !replaced) {
                ReplicaGroupDescriptor oldRgd = appDesc.replicaGroups.get(i);

                if (newRgd.id.equals(oldRgd.id)) {
                    appDesc.replicaGroups.set(i, newRgd);
                    replaced = true;
                }
                i++;
            }

            if (!replaced) {
                appDesc.replicaGroups.add(newRgd);
            }
        }

        appDesc.serviceTemplates
            .keySet()
            .removeAll(Arrays.asList(update.descriptor.removeServiceTemplates));
        appDesc.serviceTemplates.putAll(update.descriptor.serviceTemplates);

        appDesc.serverTemplates
            .keySet()
            .removeAll(Arrays.asList(update.descriptor.removeServerTemplates));
        appDesc.serverTemplates.putAll(update.descriptor.serverTemplates);

        //
        // Nodes
        //

        // Removal
        appDesc.nodes.keySet().removeAll(Arrays.asList(update.descriptor.removeNodes));

        for (String name : update.descriptor.removeNodes) {
            Node node = findNode(name);
            if (node.remove(update.descriptor.name)) {
                int index = getIndex(node);
                _nodes.remove(node);
                _treeModel.nodesWereRemoved(this, new int[]{index}, new Object[]{node});
            }
        }

        // Add/update
        Set<Node> freshNodes = new HashSet<>();
        for (NodeUpdateDescriptor desc : update.descriptor.nodes) {
            String nodeName = desc.name;

            Node node = findNode(nodeName);
            if (node == null) {
                node = new Node(this, appDesc, desc);
                insertNode(node);
            } else {
                node.update(
                    appDesc,
                    desc,
                    variablesChanged,
                    update.descriptor.serviceTemplates.keySet(),
                    update.descriptor.serverTemplates.keySet());
            }
            freshNodes.add(node);
        }

        // Notify non-fresh nodes if needed
        if (variablesChanged
            || !update.descriptor.serviceTemplates.isEmpty()
            || !update.descriptor.serverTemplates.isEmpty()) {
            for (Node p : _nodes) {
                if (!freshNodes.contains(p)) {
                    p.update(
                        appDesc,
                        null,
                        variablesChanged,
                        update.descriptor.serviceTemplates.keySet(),
                        update.descriptor.serverTemplates.keySet());
                }
            }
        }
    }

    public void adapterInit(AdapterInfo[] adapters) {
        for (AdapterInfo info : adapters) {
            _adapters.put(info.id, info);
        }
    }

    public void adapterAdded(AdapterInfo info) {
        _adapters.put(info.id, info);
    }

    public void adapterUpdated(AdapterInfo info) {
        _adapters.put(info.id, info);
    }

    public void adapterRemoved(String id) {
        _adapters.remove(id);
    }

    public void objectInit(ObjectInfo[] objects) {
        for (ObjectInfo info : objects) {
            _objects.put(
                info.proxy.ice_getCommunicator().identityToString(info.proxy.ice_getIdentity()),
                info);
        }
    }

    public void objectAdded(ObjectInfo info) {
        _objects.put(
            info.proxy.ice_getCommunicator().identityToString(info.proxy.ice_getIdentity()),
            info);
    }

    public void objectUpdated(ObjectInfo info) {
        _objects.put(
            info.proxy.ice_getCommunicator().identityToString(info.proxy.ice_getIdentity()),
            info);
    }

    public void objectRemoved(Identity id) {
        _objects.remove(_coordinator.getCommunicator().identityToString(id));
    }

    // From the Registry Observer:
    public void registryUp(RegistryInfo info) {
        if (info.name.equals(_replicaName)) {
            _info = info;
            fetchMetricsViewNames();
        } else {
            Slave newSlave = new Slave(this, info, _replicaName);
            insertSortedChild(newSlave, _slaves, _treeModel);
        }
    }

    public void registryDown(String name) {
        TreeNodeBase registry = find(name, _slaves);
        if (registry != null) {
            int index = getIndex(registry);
            _slaves.remove(registry);
            _treeModel.nodesWereRemoved(this, new int[]{index}, new Object[]{registry});
        }
    }

    // From the Node Observer:
    public void nodeUp(NodeDynamicInfo updatedInfo) {
        Node node = findNode(updatedInfo.info.name);
        if (node != null) {
            node.up(updatedInfo, true);
        } else {
            insertNode(new Node(this, updatedInfo));
        }
    }

    public void nodeDown(String nodeName) {
        Node node = findNode(nodeName);
        if (node != null) {
            if (node.down()) {
                int index = getIndex(node);
                _nodes.remove(node);
                _treeModel.nodesWereRemoved(this, new int[]{index}, new Object[]{node});
            }
        }
    }

    public void updateServer(String nodeName, ServerDynamicInfo updatedInfo) {
        Node node = findNode(nodeName);
        if (node != null) {
            node.updateServer(updatedInfo);
        }
    }

    public void updateAdapter(String nodeName, AdapterDynamicInfo updatedInfo) {
        Node node = findNode(nodeName);
        if (node != null) {
            node.updateAdapter(updatedInfo);
        }
    }

    @Override
    public JPopupMenu getPopupMenu() {
        LiveActions la = _coordinator.getLiveActionsForPopup();

        if (_popup == null) {
            _popup = new JPopupMenu();
            _popup.add(la.get(ADD_OBJECT));
            _popup.addSeparator();
            _popup.add(la.get(RETRIEVE_ICE_LOG));
            _popup.add(la.get(RETRIEVE_STDOUT));
            _popup.add(la.get(RETRIEVE_STDERR));
            _popup.addSeparator();
            _popup.add(la.get(SHUTDOWN_REGISTRY));
        }

        la.setTarget(this);
        return _popup;
    }

    public void setSelectedNode(TreeNode node) {
        _tree.setSelectionPath(node.getPath());
    }

    public JTree getTree() {
        return _tree;
    }

    public DefaultTreeModel getTreeModel() {
        return _treeModel;
    }

    @Override
    public Coordinator getCoordinator() {
        return _coordinator;
    }

    @Override
    public String toString() {
        return _label;
    }

    // Check that this node is attached to the tree
    public boolean hasNode(TreeNode node) {
        while (node != this) {
            TreeNode parent = (TreeNode) node.getParent();
            if (parent.getIndex(node) == -1) {
                return false;
            } else {
                node = parent;
            }
        }
        return true;
    }

    @Override
    public void addObject() {
        _addObjectDialog.showDialog();
    }

    public void showObject(String proxy, String type) {
        _showObjectDialog.showDialog(proxy, type);
    }

    @Override
    Root getRoot() {
        return this;
    }

    String getInstanceName() {
        return _instanceName;
    }

    SortedMap<String, ObjectInfo> getObjects() {
        return _objects;
    }

    SortedMap<String, AdapterInfo> getAdapters() {
        return _adapters;
    }

    RegistryInfo getRegistryInfo() {
        return _info;
    }

    void addObject(String strProxy, final String type, final JDialog dialog) {
        ObjectPrx proxy = null;

        try {
            proxy = ObjectPrx.createProxy(_coordinator.getCommunicator(), strProxy);
        } catch (LocalException e) {
            JOptionPane.showMessageDialog(
                _coordinator.getMainFrame(),
                "Cannot parse proxy '" + strProxy + "'",
                "addObject failed",
                JOptionPane.ERROR_MESSAGE);
        }

        assert proxy != null;

        String strIdentity =
            _coordinator.getCommunicator().identityToString(proxy.ice_getIdentity());

        final String prefix = "Adding well-known object '" + strIdentity + "'...";
        final AdminPrx admin = _coordinator.getAdmin();

        _coordinator.getStatusBar().setText(prefix);
        try {
            CompletableFuture<Void> r;
            if (type == null) {
                r = admin.addObjectAsync(proxy);
            } else {
                r = admin.addObjectWithTypeAsync(proxy, type);
            }
            r.whenComplete(
                (result, ex) -> {
                    if (ex == null) {
                        amiSuccess(prefix);

                        SwingUtilities.invokeLater(() -> dialog.setVisible(false));
                    } else if (ex instanceof ObjectExistsException) {
                        amiFailure(
                            prefix,
                            "addObject failed",
                            "An object with this identity is already registered as a"
                                + " well-known object");
                    } else if (ex instanceof DeploymentException) {
                        amiFailure(
                            prefix,
                            "addObject failed",
                            "Deployment exception: " + ((DeploymentException) ex).reason);
                    } else {
                        amiFailure(prefix, "addObject failed", ex.toString());
                    }
                });
        } catch (LocalException ex) {
            failure(prefix, "addObject failed", ex.toString());
        }
    }

    void removeObject(String strProxy) {
        var proxy = ObjectPrx.createProxy(_coordinator.getCommunicator(), strProxy);
        Identity identity = proxy.ice_getIdentity();
        final String strIdentity = _coordinator.getCommunicator().identityToString(identity);

        final String prefix = "Removing well-known object '" + strIdentity + "'...";
        final String errorTitle = "Failed to remove object '" + strIdentity + "'";

        _coordinator.getStatusBar().setText(prefix);
        try {
            final AdminPrx admin = _coordinator.getAdmin();
            admin.removeObjectAsync(identity)
                .whenComplete(
                    (result, ex) -> {
                        amiComplete(prefix, errorTitle, ex);
                    });
        } catch (LocalException ex) {
            failure(prefix, errorTitle, ex.toString());
        }
    }

    void removeAdapter(final String adapterId) {
        final String prefix = "Removing adapter '" + adapterId + "'...";
        final String errorTitle = "Failed to remove adapter '" + adapterId + "'";
        _coordinator.getStatusBar().setText(prefix);
        try {
            final AdminPrx admin = _coordinator.getAdmin();
            admin.removeAdapterAsync(adapterId)
                .whenComplete(
                    (result, ex) -> {
                        amiComplete(prefix, errorTitle, ex);
                    });
        } catch (LocalException ex) {
            failure(prefix, errorTitle, ex.toString());
        }
    }

    //
    // Implement Communicator abstract methods
    //

    @Override
    protected CompletableFuture<ObjectPrx> getAdminAsync() {
        return _coordinator.getAdmin().getRegistryAdminAsync(_replicaName);
    }

    @Override
    protected String getDisplayName() {
        return "Registry";
    }

    @Override
    protected String getDefaultFileName() {
        return "registry-" + _instanceName + "-" + _replicaName;
    }

    @Override
    public void retrieveOutput(final boolean stdout) {
        getRoot()
            .openShowLogFileDialog(
                new ShowLogFileDialog.FileIteratorFactory() {
                    @Override
                    public FileIteratorPrx open(int count)
                        throws FileNotAvailableException,
                        RegistryNotExistException,
                        RegistryUnreachableException {
                        AdminSessionPrx session = _coordinator.getSession();

                        FileIteratorPrx result;
                        if (stdout) {
                            result = session.openRegistryStdOut(_replicaName, count);
                        } else {
                            result = session.openRegistryStdErr(_replicaName, count);
                        }
                        return result;
                    }

                    @Override
                    public String getTitle() {
                        return "Registry " + _label + " " + (stdout ? "stdout" : "stderr");
                    }

                    @Override
                    public String getDefaultFilename() {
                        return _replicaName + (stdout ? ".out" : ".err");
                    }
                });
    }

    PropertySetDescriptor findNamedPropertySet(String name, String applicationName) {
        ApplicationInfo app = _infoMap.get(applicationName);
        return app.descriptor.propertySets.get(name);
    }

    void addShowIceLogDialog(String title, ShowIceLogDialog dialog) {
        _showIceLogDialogMap.put(title, dialog);
    }

    void removeShowIceLogDialog(String title) {
        _showIceLogDialogMap.remove(title);
    }

    void openShowLogFileDialog(ShowLogFileDialog.FileIteratorFactory factory) {
        ShowLogFileDialog d = _showLogFileDialogMap.get(factory.getTitle());
        if (d == null) {
            d =
                new ShowLogFileDialog(
                    this,
                    factory,
                    _logMaxLines,
                    _logMaxSize,
                    _logInitialLines,
                    _logMaxReadSize,
                    _logPeriod);

            _showLogFileDialogMap.put(factory.getTitle(), d);
        } else {
            d.toFront();
        }
    }

    void removeShowLogFileDialog(String title) {
        _showLogFileDialogMap.remove(title);
    }

    public void closeAllShowLogDialogs() {
        for (ShowIceLogDialog p : _showIceLogDialogMap.values()) {
            p.close(false);
        }
        _showIceLogDialogMap.clear();

        for (ShowLogFileDialog p : _showLogFileDialogMap.values()) {
            p.close(false);
        }
        _showLogFileDialogMap.clear();
    }

    public int getMessageSizeMax() {
        return _messageSizeMax;
    }

    public void setLogPrefs(
            int maxLines, int maxSize, int initialLines, int maxReadSize, int period) {
        _logMaxLines = maxLines;
        _logMaxSize = maxSize;
        _logInitialLines = initialLines;
        _logMaxReadSize = maxReadSize;
        _logPeriod = period;

        storeLogPrefs();
    }

    public void setLogPrefs(int maxLines, int initialLines) {
        _logMaxLines = maxLines;
        _logInitialLines = initialLines;

        storeLogPrefs();
    }

    public int getLogMaxLines() {
        return _logMaxLines;
    }

    public int getLogInitialLines() {
        return _logInitialLines;
    }

    private void loadLogPrefs() {
        Preferences logPrefs = Coordinator.getPreferences().node("Log");
        _logMaxLines = logPrefs.getInt("maxLines", 1000);
        _logMaxSize = logPrefs.getInt("maxSize", 1000000);
        _logInitialLines = logPrefs.getInt("initialLines", 1000);
        _logMaxReadSize = logPrefs.getInt("maxReadSize", 1000000);
        _logPeriod = logPrefs.getInt("period", 300);

        if (_logMaxReadSize + 512 > _messageSizeMax) {
            _logMaxReadSize = _messageSizeMax - 512;
        }
    }

    private void storeLogPrefs() {
        Preferences logPrefs = Coordinator.getPreferences().node("Log");
        logPrefs.putInt("maxLines", _logMaxLines);
        logPrefs.putInt("maxSize", _logMaxSize);
        logPrefs.putInt("initialLines", _logInitialLines);
        logPrefs.putInt("maxReadSize", _logMaxReadSize);
        logPrefs.putInt("period", _logPeriod);
    }

    private Node findNode(String nodeName) {
        return (Node) find(nodeName, _nodes);
    }

    private void insertNode(Node node) {
        insertSortedChild(node, _nodes, _treeModel);
    }

    public static int computeMessageSizeMax(int num) {
        if (num <= 0) {
            num = 1024;
        }

        if (num > 0x7fffffff / 1024) {
            return 0x7fffffff;
        } else {
            return num * 1024; // num is in kilobytes, returned value in bytes
        }
    }

    private final Coordinator _coordinator;
    private String _instanceName = "";
    private String _replicaName;

    private final List<Node> _nodes = new LinkedList<>();
    private final List<Slave> _slaves = new LinkedList<>();

    // Maps application name to current application info
    private final Map<String, ApplicationInfo> _infoMap = new TreeMap<>();

    // Map AdapterId => AdapterInfo
    private final SortedMap<String, AdapterInfo> _adapters = new TreeMap<>();

    // Map stringified identity => ObjectInfo
    private final SortedMap<String, ObjectInfo> _objects = new TreeMap<>();

    // 'this' is the root of the tree
    private final JTree _tree;
    private final FilteredTreeModel _treeModel;

    private RegistryInfo _info;

    private String _label;

    private final ObjectDialog _addObjectDialog;
    private final ObjectDialog _showObjectDialog;

    // ShowLogFileDialog and ShowIceLogFileDialog
    private final int _messageSizeMax;

    private final Map<String, ShowIceLogDialog> _showIceLogDialogMap =
        new HashMap<>();
    private final Map<String, ShowLogFileDialog> _showLogFileDialogMap =
        new HashMap<>();

    int _logMaxLines;
    int _logMaxSize;
    int _logInitialLines;
    int _logMaxReadSize;
    int _logPeriod;

    private ApplicationDetailsDialog _applicationDetailsDialog;

    private static RegistryEditor _editor;
    private static JPopupMenu _popup;
    private static DefaultTreeCellRenderer _cellRenderer;

    // Application name to filter, if empty all applications are displayed.
    private String _applicationNameFilter;
}
