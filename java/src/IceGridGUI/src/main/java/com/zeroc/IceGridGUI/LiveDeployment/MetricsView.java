// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceGridGUI.LiveDeployment;

import java.awt.Component;

import javax.swing.Icon;
import javax.swing.JTree;
import javax.swing.tree.DefaultTreeCellRenderer;
import javax.swing.SwingUtilities;
import javax.swing.JOptionPane;
import javax.swing.JPopupMenu;

import IceGridGUI.*;

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
        boolean[] actions = new boolean[IceGridGUI.LiveDeployment.TreeNode.ACTION_COUNT];
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

    MetricsView(TreeNode parent, String name, IceMX.MetricsAdminPrx admin, boolean enabled)
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
        IceMX.MetricsAdminPrx metricsAdmin = getMetricsAdmin();
        if(metricsAdmin != null)
        {
            if(enabled)
            {
                IceMX.Callback_MetricsAdmin_enableMetricsView cb = new IceMX.Callback_MetricsAdmin_enableMetricsView()
                    {
                        @Override
                        public void response()
                        {
                            SwingUtilities.invokeLater(new Runnable()
                            {
                                @Override
                                public void run()
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
                                }
                            });
                        }

                        @Override
                        public void exception(final Ice.LocalException e)
                        {
                            MetricsViewEditor.stopRefresh();
                            SwingUtilities.invokeLater(new Runnable()
                                {
                                    @Override
                                    public void run()
                                    {
                                        if(e instanceof Ice.ObjectNotExistException ||
                                           e instanceof Ice.ConnectionRefusedException)
                                        {
                                            // Server is down.
                                        }
                                        else if(e instanceof Ice.CommunicatorDestroyedException)
                                        {
                                        }
                                        else
                                        {
                                            e.printStackTrace();
                                            JOptionPane.showMessageDialog(getCoordinator().getMainFrame(), 
                                                                          "Error: " + e.toString(), "Error",
                                                                          JOptionPane.ERROR_MESSAGE);
                                        }
                                    }
                                });
                        }

                        @Override
                        public void exception(final Ice.UserException e)
                        {
                            MetricsViewEditor.stopRefresh();
                            SwingUtilities.invokeLater(new Runnable()
                                {
                                    @Override
                                    public void run()
                                    {
                                        e.printStackTrace();
                                        JOptionPane.showMessageDialog(getCoordinator().getMainFrame(), 
                                                                        "Error: " + e.toString(), "Error",
                                                                        JOptionPane.ERROR_MESSAGE);
                                    }
                                });
                        }
                    };
                metricsAdmin.begin_enableMetricsView(_name, cb);
            }
            else
            {
                IceMX.Callback_MetricsAdmin_disableMetricsView cb = new IceMX.Callback_MetricsAdmin_disableMetricsView()
                    {
                        @Override
                        public void response()
                        {
                            SwingUtilities.invokeLater(new Runnable()
                            {
                                @Override
                                public void run()
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
                                }
                            });
                        }

                        @Override
                        public void exception(final Ice.LocalException e)
                        {
                            MetricsViewEditor.stopRefresh();
                            SwingUtilities.invokeLater(new Runnable()
                                {
                                    @Override
                                    public void run()
                                    {
                                        if(e instanceof Ice.ObjectNotExistException ||
                                           e instanceof Ice.ConnectionRefusedException)
                                        {
                                            // Server is down.
                                        }
                                        else if(e instanceof Ice.CommunicatorDestroyedException)
                                        {
                                        }
                                        else
                                        {
                                            e.printStackTrace();
                                            JOptionPane.showMessageDialog(getCoordinator().getMainFrame(), 
                                                                          "Error: " + e.toString(), "Error",
                                                                          JOptionPane.ERROR_MESSAGE);
                                        }
                                    }
                                });
                        }

                        @Override
                        public void exception(final Ice.UserException e)
                        {
                            MetricsViewEditor.stopRefresh();
                            SwingUtilities.invokeLater(new Runnable()
                                {
                                    @Override
                                    public void run()
                                    {
                                        e.printStackTrace();
                                        JOptionPane.showMessageDialog(getCoordinator().getMainFrame(), 
                                                                        "Error: " + e.toString(), "Error",
                                                                        JOptionPane.ERROR_MESSAGE);
                                    }
                                });
                        }
                    };
                metricsAdmin.begin_disableMetricsView(_name, cb);
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

    IceMX.MetricsAdminPrx getMetricsAdmin()
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

    public void fetchMetricsFailures(String map, String id, IceMX.Callback_MetricsAdmin_getMetricsFailures cb)
    {
        IceMX.MetricsAdminPrx metricsAdmin = getMetricsAdmin();
        if(metricsAdmin != null)
        {
            try
            {
                metricsAdmin.begin_getMetricsFailures(_name, map, id, cb);
            }
            catch(Ice.LocalException e)
            {
                JOptionPane.showMessageDialog(getCoordinator().getMainFrame(), "Error: " + e.toString(), "Error",
                                              JOptionPane.ERROR_MESSAGE);
            }
        }
    }

    public void fetchMetricsView()
    {
        IceMX.MetricsAdminPrx metricsAdmin = getMetricsAdmin();
        if(metricsAdmin != null)
        {
            IceMX.Callback_MetricsAdmin_getMetricsView cb = new IceMX.Callback_MetricsAdmin_getMetricsView()
                {
                    @Override
                    public void response(final java.util.Map<java.lang.String, IceMX.Metrics[]> data,
                                         final long timestamp)
                    {
                        SwingUtilities.invokeLater(new Runnable()
                            {
                                @Override
                                public void run()
                                {
                                    _editor.show(MetricsView.this, data, timestamp);
                                }
                            });
                    }

                    @Override
                    public void exception(final Ice.LocalException e)
                    {
                        MetricsViewEditor.stopRefresh();
                        SwingUtilities.invokeLater(new Runnable()
                            {
                                @Override
                                public void run()
                                {
                                    if(e instanceof Ice.ObjectNotExistException ||
                                       e instanceof Ice.ConnectionRefusedException)
                                    {
                                        // Server is down.
                                    }
                                    else if(e instanceof Ice.FacetNotExistException)
                                    {
                                        // MetricsAdmin facet not present.
                                    }
                                    else if(e instanceof Ice.CommunicatorDestroyedException)
                                    {
                                    }
                                    else
                                    {
                                        e.printStackTrace();
                                        JOptionPane.showMessageDialog(getCoordinator().getMainFrame(), 
                                                                      "Error: " + e.toString(), "Error",
                                                                      JOptionPane.ERROR_MESSAGE);
                                    }
                                }
                            });
                    }

                    @Override
                    public void exception(final Ice.UserException e)
                    {
                        MetricsViewEditor.stopRefresh();
                        SwingUtilities.invokeLater(new Runnable()
                            {
                                @Override
                                public void run()
                                {
                                    e.printStackTrace();
                                    JOptionPane.showMessageDialog(getCoordinator().getMainFrame(), 
                                                                    "Error: " + e.toString(), "Error",
                                                                    JOptionPane.ERROR_MESSAGE);
                                }
                            });
                    }
                };
            try
            {
                metricsAdmin.begin_getMetricsView(_name, cb);
            }
            catch(Ice.CommunicatorDestroyedException e)
            {
            }
            catch(Ice.LocalException e)
            {
                MetricsViewEditor.stopRefresh();
                JOptionPane.showMessageDialog(getCoordinator().getMainFrame(), "Error: " + e.toString(), "Error",
                                              JOptionPane.ERROR_MESSAGE);
            }
        }
    }


    private String _name;
    private IceMX.MetricsAdminPrx _admin;
    private MetricsViewEditor _editor;
    private boolean _enabled;
    static private JPopupMenu _popup;
    static private DefaultTreeCellRenderer _cellRenderer;
    static private Icon _enabledIcon;
    static private Icon _disabledIcon;
}
