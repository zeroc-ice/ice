// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceGridGUI.LiveDeployment;

import java.awt.Component;

import javax.swing.Icon;
import javax.swing.JTree;
import javax.swing.tree.DefaultTreeCellRenderer;
import javax.swing.SwingUtilities;
import javax.swing.JOptionPane;
import javax.swing.JPopupMenu;

import com.zeroc.IceGridGUI.*;

class MetricsView extends TreeNode
{
    @Override
    public Editor getEditor()
    {
        return _editor;
    }

    //
    // Actions
    //
    @Override
    public boolean[] getAvailableActions()
    {
        boolean[] actions = new boolean[com.zeroc.IceGridGUI.LiveDeployment.TreeNode.ACTION_COUNT];
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
        boolean hasFocus)
    {
        if(_cellRenderer == null)
        {
            _cellRenderer = new DefaultTreeCellRenderer();

            _enabledIcon = Utils.getIcon("/icons/16x16/metrics.png");
            _disabledIcon = Utils.getIcon("/icons/16x16/metrics_disabled.png");
        }

        Icon icon = _enabled ?  _enabledIcon : _disabledIcon;
        _cellRenderer.setLeafIcon(icon);
        return _cellRenderer.getTreeCellRendererComponent(tree, value, sel, expanded, leaf, row, hasFocus);
    }

    MetricsView(TreeNode parent, String name, com.zeroc.IceMX.MetricsAdminPrx admin, boolean enabled)
    {
        super(parent, name);
        _name = name;
        _admin = admin;
        _editor = new MetricsViewEditor(getRoot());
        _enabled = enabled;
    }

    @Override
    public void enableMetricsView(boolean enabled)
    {
        if(_admin != null)
        {
            if(enabled)
            {
                _admin.enableMetricsViewAsync(_name).whenComplete((result, ex) ->
                    {
                        if(ex == null)
                        {
                            SwingUtilities.invokeLater(() ->
                                {
                                    _enabled = true;
                                    getRoot().getTreeModel().nodeChanged(MetricsView.this);
                                    getRoot().getCoordinator().showActions(MetricsView.this);
                                    if(getRoot().getTree().getLastSelectedPathComponent() == MetricsView.this)
                                    {
                                        //
                                        // If the metrics view is selected when enabled success,
                                        // we must start the refresh thread to pull updates.
                                        //
                                        MetricsViewEditor.startRefresh(MetricsView.this);
                                    }
                                });
                        }
                        else
                        {
                            MetricsViewEditor.stopRefresh();
                            SwingUtilities.invokeLater(() ->
                                {
                                    if(ex instanceof com.zeroc.Ice.ObjectNotExistException ||
                                       ex instanceof com.zeroc.Ice.ConnectionRefusedException)
                                    {
                                        // Server is down.
                                    }
                                    else if(!(ex instanceof com.zeroc.Ice.CommunicatorDestroyedException))
                                    {
                                        ex.printStackTrace();
                                        JOptionPane.showMessageDialog(getCoordinator().getMainFrame(),
                                                                      "Error: " + ex.toString(), "Error",
                                                                      JOptionPane.ERROR_MESSAGE);
                                    }
                                });
                        }
                    });
            }
            else
            {
                _admin.disableMetricsViewAsync(_name).whenComplete((result, ex) ->
                    {
                        if(ex == null)
                        {
                            SwingUtilities.invokeLater(() ->
                                {
                                    _enabled = false;
                                    _editor.show(MetricsView.this, null, 0);
                                    getRoot().getTreeModel().nodeChanged(MetricsView.this);
                                    getRoot().getCoordinator().showActions(MetricsView.this);
                                    if(getRoot().getTree().getLastSelectedPathComponent() == MetricsView.this)
                                    {
                                        //
                                        // If the metrics view is selected when disabled success,
                                        // we stop the refresh.
                                        //
                                        MetricsViewEditor.stopRefresh();
                                    }
                                });
                        }
                        else
                        {
                            MetricsViewEditor.stopRefresh();
                            SwingUtilities.invokeLater(() ->
                                {
                                    if(ex instanceof com.zeroc.Ice.ObjectNotExistException ||
                                       ex instanceof com.zeroc.Ice.ConnectionRefusedException)
                                    {
                                        // Server is down.
                                    }
                                    else if(!(ex instanceof com.zeroc.Ice.CommunicatorDestroyedException))
                                    {
                                        ex.printStackTrace();
                                        JOptionPane.showMessageDialog(getCoordinator().getMainFrame(),
                                                                      "Error: " + ex.toString(), "Error",
                                                                      JOptionPane.ERROR_MESSAGE);
                                    }
                                });
                        }
                    });
            }
        }
    }

    public boolean isEnabled()
    {
        return _enabled;
    }

    public String name()
    {
        return _name;
    }

    com.zeroc.IceMX.MetricsAdminPrx getMetricsAdmin()
    {
        return _admin;
    }

    @Override
    public JPopupMenu getPopupMenu()
    {
        LiveActions la = getCoordinator().getLiveActionsForPopup();

        if(_popup == null)
        {
            _popup = new JPopupMenu();
            _popup.add(la.get(ENABLE_METRICS_VIEW));
            _popup.add(la.get(DISABLE_METRICS_VIEW));
        }

        la.setTarget(this);
        return _popup;
    }

    public java.util.concurrent.CompletableFuture<com.zeroc.IceMX.MetricsFailures> fetchMetricsFailures(String map,
                                                                                                        String id)
    {
        if(_admin != null)
        {
            try
            {
                return _admin.getMetricsFailuresAsync(_name, map, id);
            }
            catch(com.zeroc.Ice.LocalException e)
            {
                JOptionPane.showMessageDialog(getCoordinator().getMainFrame(), "Error: " + e.toString(), "Error",
                                              JOptionPane.ERROR_MESSAGE);
            }
        }
        return null;
    }

    public void fetchMetricsView()
    {
        if(_admin != null)
        {
            try
            {
                _admin.getMetricsViewAsync(_name).whenComplete((result, ex) ->
                    {
                        if(ex == null)
                        {
                            SwingUtilities.invokeLater(() ->
                                {
                                    _editor.show(MetricsView.this, result.returnValue, result.timestamp);
                                });
                        }
                        else
                        {
                            MetricsViewEditor.stopRefresh();
                            SwingUtilities.invokeLater(() ->
                                {
                                    if(ex instanceof com.zeroc.Ice.ObjectNotExistException ||
                                       ex instanceof com.zeroc.Ice.ConnectionRefusedException)
                                    {
                                        // Server is down.
                                    }
                                    else if(ex instanceof com.zeroc.Ice.FacetNotExistException)
                                    {
                                        // MetricsAdmin facet not present.
                                    }
                                    else if(!(ex instanceof com.zeroc.Ice.CommunicatorDestroyedException))
                                    {
                                        ex.printStackTrace();
                                        JOptionPane.showMessageDialog(getCoordinator().getMainFrame(),
                                                                      "Error: " + ex.toString(), "Error",
                                                                      JOptionPane.ERROR_MESSAGE);
                                    }
                                });
                        }
                    });
            }
            catch(com.zeroc.Ice.CommunicatorDestroyedException e)
            {
            }
            catch(com.zeroc.Ice.LocalException e)
            {
                MetricsViewEditor.stopRefresh();
                JOptionPane.showMessageDialog(getCoordinator().getMainFrame(), "Error: " + e.toString(), "Error",
                                              JOptionPane.ERROR_MESSAGE);
            }
        }
    }

    private String _name;
    private com.zeroc.IceMX.MetricsAdminPrx _admin;
    private MetricsViewEditor _editor;
    private boolean _enabled;
    static private JPopupMenu _popup;
    static private DefaultTreeCellRenderer _cellRenderer;
    static private Icon _enabledIcon;
    static private Icon _disabledIcon;
}
