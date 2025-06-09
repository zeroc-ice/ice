// Copyright (c) ZeroC, Inc.

package com.zeroc.IceGridGUI.LiveDeployment;

import com.zeroc.Ice.CommunicatorDestroyedException;
import com.zeroc.Ice.ConnectionRefusedException;
import com.zeroc.Ice.FacetNotExistException;
import com.zeroc.Ice.LocalException;
import com.zeroc.Ice.ObjectNotExistException;
import com.zeroc.IceGridGUI.LiveActions;
import com.zeroc.IceGridGUI.Utils;
import com.zeroc.IceMX.MetricsAdminPrx;
import com.zeroc.IceMX.MetricsFailures;

import java.awt.Component;
import java.util.concurrent.CompletableFuture;

import javax.swing.Icon;
import javax.swing.JOptionPane;
import javax.swing.JPopupMenu;
import javax.swing.JTree;
import javax.swing.SwingUtilities;
import javax.swing.tree.DefaultTreeCellRenderer;

class MetricsView extends TreeNode {
    @Override
    public Editor getEditor() {
        return _editor;
    }

    // Actions
    @Override
    public boolean[] getAvailableActions() {
        boolean[] actions = new boolean[TreeNode.ACTION_COUNT];
        actions[ENABLE_METRICS_VIEW] = !_enabled;
        actions[DISABLE_METRICS_VIEW] = _enabled;
        return actions;
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
            _cellRenderer = new DefaultTreeCellRenderer();

            _enabledIcon = Utils.getIcon("/icons/16x16/metrics.png");
            _disabledIcon = Utils.getIcon("/icons/16x16/metrics_disabled.png");
        }

        Icon icon = _enabled ? _enabledIcon : _disabledIcon;
        _cellRenderer.setLeafIcon(icon);
        return _cellRenderer.getTreeCellRendererComponent(
            tree, value, sel, expanded, leaf, row, hasFocus);
    }

    MetricsView(
            TreeNode parent,
            String name,
            MetricsAdminPrx admin,
            boolean enabled) {
        super(parent, name);
        _name = name;
        _admin = admin;
        _editor = new MetricsViewEditor(getRoot());
        _enabled = enabled;
    }

    @Override
    public void enableMetricsView(boolean enabled) {
        if (_admin != null) {
            if (enabled) {
                _admin.enableMetricsViewAsync(_name)
                    .whenComplete(
                        (result, ex) -> {
                            if (ex == null) {
                                SwingUtilities.invokeLater(
                                    () -> {
                                        _enabled = true;
                                        getRoot()
                                            .getTreeModel()
                                            .nodeChanged(MetricsView.this);
                                        getRoot()
                                            .getCoordinator()
                                            .showActions(MetricsView.this);
                                        if (getRoot()
                                            .getTree()
                                            .getLastSelectedPathComponent()
                                            == MetricsView.this) {
                                            // If the metrics view is selected when
                                            // enabled success, we must start the
                                            // refresh thread to pull updates.
                                            MetricsViewEditor.startRefresh(
                                                MetricsView.this);
                                        }
                                    });
                            } else {
                                MetricsViewEditor.stopRefresh();
                                SwingUtilities.invokeLater(
                                    () -> {
                                        if (ex
                                            instanceof ObjectNotExistException
                                            || ex
                                            instanceof ConnectionRefusedException) {
                                            // Server is down.
                                        } else if (!(ex
                                            instanceof CommunicatorDestroyedException)) {
                                            ex.printStackTrace();
                                            JOptionPane.showMessageDialog(
                                                getCoordinator().getMainFrame(),
                                                "Error: " + ex.toString(),
                                                "Error",
                                                JOptionPane.ERROR_MESSAGE);
                                        }
                                    });
                            }
                        });
            } else {
                _admin.disableMetricsViewAsync(_name)
                    .whenComplete(
                        (result, ex) -> {
                            if (ex == null) {
                                SwingUtilities.invokeLater(
                                    () -> {
                                        _enabled = false;
                                        _editor.show(MetricsView.this, null, 0);
                                        getRoot()
                                            .getTreeModel()
                                            .nodeChanged(MetricsView.this);
                                        getRoot()
                                            .getCoordinator()
                                            .showActions(MetricsView.this);
                                        if (getRoot()
                                            .getTree()
                                            .getLastSelectedPathComponent()
                                            == MetricsView.this) {
                                            // If the metrics view is selected when
                                            // disabled success, we stop the refresh.
                                            MetricsViewEditor.stopRefresh();
                                        }
                                    });
                            } else {
                                MetricsViewEditor.stopRefresh();
                                SwingUtilities.invokeLater(
                                    () -> {
                                        if (ex
                                            instanceof ObjectNotExistException
                                            || ex
                                            instanceof ConnectionRefusedException) {
                                            // Server is down.
                                        } else if (!(ex
                                            instanceof CommunicatorDestroyedException)) {
                                            ex.printStackTrace();
                                            JOptionPane.showMessageDialog(
                                                getCoordinator().getMainFrame(),
                                                "Error: " + ex.toString(),
                                                "Error",
                                                JOptionPane.ERROR_MESSAGE);
                                        }
                                    });
                            }
                        });
            }
        }
    }

    public boolean isEnabled() {
        return _enabled;
    }

    public String name() {
        return _name;
    }

    MetricsAdminPrx getMetricsAdmin() {
        return _admin;
    }

    @Override
    public JPopupMenu getPopupMenu() {
        LiveActions la = getCoordinator().getLiveActionsForPopup();

        if (_popup == null) {
            _popup = new JPopupMenu();
            _popup.add(la.get(ENABLE_METRICS_VIEW));
            _popup.add(la.get(DISABLE_METRICS_VIEW));
        }

        la.setTarget(this);
        return _popup;
    }

    public CompletableFuture<MetricsFailures> fetchMetricsFailures(String map, String id) {
        if (_admin != null) {
            try {
                return _admin.getMetricsFailuresAsync(_name, map, id);
            } catch (LocalException e) {
                JOptionPane.showMessageDialog(
                    getCoordinator().getMainFrame(),
                    "Error: " + e.toString(),
                    "Error",
                    JOptionPane.ERROR_MESSAGE);
            }
        }
        return null;
    }

    public void fetchMetricsView() {
        if (_admin != null) {
            try {
                _admin.getMetricsViewAsync(_name)
                    .whenComplete(
                        (result, ex) -> {
                            if (ex == null) {
                                SwingUtilities.invokeLater(
                                    () -> {
                                        _editor.show(
                                            MetricsView.this,
                                            result.returnValue,
                                            result.timestamp);
                                    });
                            } else {
                                MetricsViewEditor.stopRefresh();
                                SwingUtilities.invokeLater(
                                    () -> {
                                        if (ex
                                            instanceof ObjectNotExistException
                                            || ex
                                            instanceof ConnectionRefusedException) {
                                            // Server is down.
                                        } else if (ex
                                            instanceof FacetNotExistException) {
                                            // MetricsAdmin facet not present.
                                        } else if (!(ex
                                            instanceof CommunicatorDestroyedException)) {
                                            ex.printStackTrace();
                                            JOptionPane.showMessageDialog(
                                                getCoordinator().getMainFrame(),
                                                "Error: " + ex.toString(),
                                                "Error",
                                                JOptionPane.ERROR_MESSAGE);
                                        }
                                    });
                            }
                        });
            } catch (CommunicatorDestroyedException e) {} catch (LocalException e) {
                MetricsViewEditor.stopRefresh();
                JOptionPane.showMessageDialog(
                    getCoordinator().getMainFrame(),
                    "Error: " + e.toString(),
                    "Error",
                    JOptionPane.ERROR_MESSAGE);
            }
        }
    }

    private final String _name;
    private final MetricsAdminPrx _admin;
    private final MetricsViewEditor _editor;
    private boolean _enabled;
    private static JPopupMenu _popup;
    private static DefaultTreeCellRenderer _cellRenderer;
    private static Icon _enabledIcon;
    private static Icon _disabledIcon;
}
