// Copyright (c) ZeroC, Inc.

package com.zeroc.IceGridGUI.LiveDeployment;

import com.zeroc.Ice.FacetNotExistException;
import com.zeroc.Ice.LocalException;
import com.zeroc.Ice.LoggerAdminPrx;
import com.zeroc.Ice.ObjectNotExistException;
import com.zeroc.Ice.ObjectPrx;
import com.zeroc.Ice.PropertiesAdminPrx;
import com.zeroc.IceMX.MetricsAdminPrx;

import java.util.ArrayList;
import java.util.Enumeration;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.NoSuchElementException;
import java.util.SortedMap;
import java.util.concurrent.CompletableFuture;
import java.util.function.Consumer;

import javax.swing.SwingUtilities;

public abstract class Communicator extends TreeNode {
    Communicator(TreeNode parent, String id, int arraySize) {
        super(parent, id);
        _childrenArray = new List[arraySize];
    }

    // Children-related overrides
    @Override
    @SuppressWarnings("unchecked")
    public Enumeration<javax.swing.tree.TreeNode> children() {
        return new Enumeration<javax.swing.tree.TreeNode>() {
            @Override
            public boolean hasMoreElements() {
                if (_p.hasNext()) {
                    return true;
                }

                while (++_index < _childrenArray.length) {
                    _p = _childrenArray[_index].iterator();
                    if (_p.hasNext()) {
                        return true;
                    }
                }
                return false;
            }

            @Override
            public javax.swing.tree.TreeNode nextElement() {
                try {
                    return _p.next();
                } catch (NoSuchElementException nse) {
                    if (hasMoreElements()) {
                        return _p.next();
                    } else {
                        throw nse;
                    }
                }
            }

            private int _index = 0;
            private Iterator<javax.swing.tree.TreeNode> _p = _childrenArray[0].iterator();
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
        }
        int offset = 0;
        for (List l : _childrenArray) {
            if (childIndex < offset + l.size()) {
                return (javax.swing.tree.TreeNode) l.get(childIndex - offset);
            } else {
                offset += l.size();
            }
        }
        throw new ArrayIndexOutOfBoundsException(childIndex);
    }

    @Override
    public int getChildCount() {
        int result = 0;
        for (List l : _childrenArray) {
            result += l.size();
        }
        return result;
    }

    @Override
    public int getIndex(javax.swing.tree.TreeNode node) {
        int offset = 0;
        for (List l : _childrenArray) {
            int index = l.indexOf(node);
            if (index == -1) {
                offset += l.size();
            } else {
                return offset + index;
            }
        }
        return -1;
    }

    @Override
    public boolean isLeaf() {
        for (List l : _childrenArray) {
            if (!l.isEmpty()) {
                return false;
            }
        }
        return true;
    }

    // TreeNode overrides
    @Override
    public void retrieveIceLog() {
        if (_showIceLogDialog == null) {
            final String prefix = "Opening Ice Log file for " + getServerDisplayName() + "...";

            provideAdmin(
                prefix,
                admin -> {
                    final LoggerAdminPrx loggerAdmin =
                        LoggerAdminPrx.uncheckedCast(
                            getAdminFacet(admin, "Logger"));
                    final String title = getDisplayName() + " Ice log";
                    final String defaultFileName = getDefaultFileName();

                    SwingUtilities.invokeLater(
                        () -> {
                            success(prefix);
                            if (_showIceLogDialog == null) {
                                _showIceLogDialog =
                                    new ShowIceLogDialog(
                                        Communicator.this,
                                        title,
                                        loggerAdmin,
                                        defaultFileName,
                                        getRoot().getLogMaxLines(),
                                        getRoot().getLogInitialLines());
                            } else {
                                _showIceLogDialog.toFront();
                            }
                        });
                });
        } else {
            _showIceLogDialog.toFront();
        }
    }

    @Override
    public void clearShowIceLogDialog() {
        _showIceLogDialog = null;
    }

    protected void showRuntimeProperties() {
        final String prefix = "Retrieving properties for " + getDisplayName() + "...";
        provideAdmin(
            prefix,
            admin -> {
                final PropertiesAdminPrx propertiesAdmin =
                    PropertiesAdminPrx.uncheckedCast(
                        getAdminFacet(admin, "Properties"));

                propertiesAdmin
                    .getPropertiesForPrefixAsync("")
                    .whenComplete(
                        (result, ex) -> {
                            if (ex == null) {
                                SwingUtilities.invokeLater(
                                    () -> {
                                        success(prefix);
                                        ((CommunicatorEditor) getEditor())
                                            .setRuntimeProperties(
                                                (SortedMap<
                                                    String,
                                                    String>)
                                                    result,
                                                Communicator.this);
                                    });
                            } else if (ex
                                instanceof ObjectNotExistException
                                || ex
                                instanceof FacetNotExistException) {
                                SwingUtilities.invokeLater(
                                    () ->
                                        getRoot()
                                            .getCoordinator()
                                            .getStatusBar()
                                            .setText(
                                                prefix
                                                    + " Admin not available."));
                            } else {
                                amiFailure(
                                    prefix,
                                    "Failed to retrieve the properties for "
                                        + getDisplayName(),
                                    ex);
                            }
                        });
            });
    }

    protected void fetchMetricsViewNames() {
        if (_metricsRetrieved) {
            return; // Already loaded.
        }
        _metricsRetrieved = true;

        final String prefix = "Retrieving metrics for " + getDisplayName() + "...";
        if (!provideAdmin(
            prefix,
            admin -> {
                final MetricsAdminPrx metricsAdmin =
                    MetricsAdminPrx.uncheckedCast(
                        getAdminFacet(admin, "Metrics"));

                metricsAdmin
                    .getMetricsViewNamesAsync()
                    .whenComplete(
                        (result, ex) -> {
                            if (ex == null) {
                                SwingUtilities.invokeLater(
                                    () -> {
                                        success(prefix);
                                        _metrics.clear();
                                        for (String name : result.returnValue) {
                                            insertSortedChild(
                                                new MetricsView(
                                                    Communicator.this,
                                                    name,
                                                    metricsAdmin,
                                                    true),
                                                _metrics,
                                                null);
                                        }
                                        for (String name : result.disabledViews) {
                                            insertSortedChild(
                                                new MetricsView(
                                                    Communicator.this,
                                                    name,
                                                    metricsAdmin,
                                                    false),
                                                _metrics,
                                                null);
                                        }
                                        getRoot()
                                            .getTreeModel()
                                            .nodeStructureChanged(
                                                Communicator.this);
                                    });
                            } else if (ex
                                instanceof ObjectNotExistException
                                || ex
                                instanceof FacetNotExistException) {
                                SwingUtilities.invokeLater(
                                    () ->
                                        getRoot()
                                            .getCoordinator()
                                            .getStatusBar()
                                            .setText(
                                                prefix
                                                    + " Admin not available."));
                            } else {
                                amiFailure(
                                    prefix,
                                    "Failed to retrieve the metrics for "
                                        + getDisplayName(),
                                    ex);
                            }
                        });
            })) {
            _metricsRetrieved = false;
        }
    }

    void updateMetrics() {
        _metricsRetrieved = false;
        if (getRoot().getTree().isExpanded(getPath())) {
            fetchMetricsViewNames();
        }
    }

    public List<MetricsView> getMetrics() {
        return new ArrayList<>(_metrics);
    }

    protected abstract CompletableFuture<ObjectPrx> getAdminAsync();

    protected ObjectPrx getAdminFacet(ObjectPrx admin, String facet) {
        return admin != null ? admin.ice_facet(facet) : null;
    }

    protected abstract String getDisplayName();

    protected String getServerDisplayName() {
        return getDisplayName();
    }

    protected abstract String getDefaultFileName();

    private boolean provideAdmin(
            final String prefix,
            final Consumer<ObjectPrx> consumer) {
        getRoot().getCoordinator().getStatusBar().setText(prefix);
        try {
            getAdminAsync()
                .whenComplete(
                    (admin, adminEx) -> {
                        if (adminEx == null && admin != null) {
                            try {
                                consumer.accept(admin);
                            } catch (LocalException e) {
                                SwingUtilities.invokeLater(
                                    () ->
                                        getRoot()
                                            .getCoordinator()
                                            .getStatusBar()
                                            .setText(
                                                prefix
                                                    + " "
                                                    + e.toString()
                                                    + "."));
                            }
                        } else if (adminEx == null
                            || adminEx
                            instanceof ObjectNotExistException) {
                            SwingUtilities.invokeLater(
                                () ->
                                    getRoot()
                                        .getCoordinator()
                                        .getStatusBar()
                                        .setText(
                                            prefix
                                                + " Admin not available."));
                        } else {
                            amiFailure(
                                prefix,
                                "Failed to retrieve the Admin proxy for "
                                    + getServerDisplayName(),
                                adminEx);
                        }
                    });
        } catch (LocalException e) {
            getRoot().getCoordinator().getStatusBar().setText(prefix + " " + e.toString() + ".");
            return false;
        }
        return true;
    }

    protected ShowIceLogDialog _showIceLogDialog;
    protected List<MetricsView> _metrics = new LinkedList<>();
    protected boolean _metricsRetrieved;

    protected final List[] _childrenArray;
}
