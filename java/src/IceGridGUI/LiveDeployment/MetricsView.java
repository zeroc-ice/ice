// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
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
import javax.swing.tree.TreeModel;
import javax.swing.tree.TreePath;
import javax.swing.SwingUtilities;
import javax.swing.JOptionPane;

import java.util.Map;
import java.util.Enumeration;

import IceGrid.*;
import IceGridGUI.*;

class MetricsView extends TreeNode
{
    public Editor getEditor()
    {
        _editor.show(this);
        return _editor;
    }

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
            _cellRenderer.setLeafIcon(Utils.getIcon("/icons/16x16/metrics.png"));
        }
        _cellRenderer.setToolTipText(_toolTip);
        return _cellRenderer.getTreeCellRendererComponent(tree, value, sel, expanded, leaf, row, hasFocus);
    }

    MetricsView(TreeNode parent, String name)
    {
        super(parent, name);
        _name = name;
        if(_editor == null)
        {
            _editor = new MetricsViewEditor(getRoot());
        }
        fetchMetricsView();
    }
    
    public String name()
    {
        return _name;
    }

    Ice.ObjectPrx getServerAdmin()
    {
        if(_parent instanceof Server)
        {
            return ((Server)_parent).getServerAdmin();
        }
        else
        {
            return null;
        }
    }

    public java.util.Map<java.lang.String, IceMX.Metrics[]> data()
    {
        return _data;
    }

    public void fetchMetricsFailures(String map, String id, IceMX.Callback_MetricsAdmin_getMetricsFailures cb)
    {
        Ice.ObjectPrx admin = getServerAdmin();
        if(admin == null)
        {
            return;
        }

        try
        {
            IceMX.MetricsAdminPrx metricsAdmin = 
                IceMX.MetricsAdminPrxHelper.uncheckedCast(admin.ice_facet("MetricsAdmin"));
            metricsAdmin.begin_getMetricsFailures(_name, map, id, cb);
        }
        catch(Ice.LocalException e)
        {
            JOptionPane.showMessageDialog(getCoordinator().getMainFrame(), "Error: " + e.toString(), "Error",
                                          JOptionPane.ERROR_MESSAGE);
        }
    }

    public void fetchMetricsView()
    {
        Ice.ObjectPrx admin = getServerAdmin();
        if(admin == null)
        {
            return;
        }

        IceMX.Callback_MetricsAdmin_getMetricsView cb = new IceMX.Callback_MetricsAdmin_getMetricsView()
            {
                public void response(final java.util.Map<java.lang.String, IceMX.Metrics[]> data)
                {
                    SwingUtilities.invokeLater(new Runnable()
                        {
                            public void run()
                            {
                                _data = data;
                                if(_editor != null && _editor.currentView() == MetricsView.this)
                                {
                                    _editor.show(MetricsView.this);
                                }
                            }
                        });
                }

                public void exception(final Ice.LocalException e)
                {
                    SwingUtilities.invokeLater(new Runnable()
                        {
                            public void run()
                            {
                                e.printStackTrace();
                                if(e instanceof Ice.ObjectNotExistException)
                                {
                                    // Server is down.
                                }
                                else if(e instanceof Ice.FacetNotExistException)
                                {
                                    // MetricsAdmin facet not present.
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

                public void exception(final Ice.UserException e)
                {
                    SwingUtilities.invokeLater(new Runnable()
                        {
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
            IceMX.MetricsAdminPrx metricsAdmin = 
                IceMX.MetricsAdminPrxHelper.uncheckedCast(admin.ice_facet("MetricsAdmin"));
            metricsAdmin.begin_getMetricsView(_name, cb);
        }
        catch(Ice.LocalException e)
        {
            JOptionPane.showMessageDialog(getCoordinator().getMainFrame(), "Error: " + e.toString(), "Error",
                                          JOptionPane.ERROR_MESSAGE);
        }
    }


    private String _name;
    private String _toolTip;
    private MetricsViewEditor _editor;
    private java.util.Map<java.lang.String, IceMX.Metrics[]> _data;

    static private DefaultTreeCellRenderer _cellRenderer;
}
