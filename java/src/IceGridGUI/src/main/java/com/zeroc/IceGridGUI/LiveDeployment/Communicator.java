// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package com.zeroc.IceGridGUI.LiveDeployment;

import java.util.Enumeration;

import javax.swing.SwingUtilities;
import com.zeroc.IceGridGUI.*;

abstract public class Communicator extends TreeNode
{
    Communicator(TreeNode parent, String id, int arraySize)
    {
        super(parent, id);
        _childrenArray = new java.util.List[arraySize];
    }

    //
    // Children-related overrides
    //
    @Override
    @SuppressWarnings("unchecked")
    public Enumeration<javax.swing.tree.TreeNode> children()
    {
        return new Enumeration<javax.swing.tree.TreeNode>()
            {
                @Override
                public boolean hasMoreElements()
                {
                    if(_p.hasNext())
                    {
                        return true;
                    }

                    while(++_index < _childrenArray.length)
                    {
                        _p = _childrenArray[_index].iterator();
                        if(_p.hasNext())
                        {
                            return true;
                        }
                    }
                    return false;
                }

                @Override
                public javax.swing.tree.TreeNode nextElement()
                {
                    try
                    {
                        return _p.next();
                    }
                    catch(java.util.NoSuchElementException nse)
                    {
                        if(hasMoreElements())
                        {
                            return _p.next();
                        }
                        else
                        {
                            throw nse;
                        }
                    }
                }

                private int _index = 0;
                private java.util.Iterator<javax.swing.tree.TreeNode> _p = _childrenArray[0].iterator();
            };
    }

    @Override
    public boolean getAllowsChildren()
    {
        return true;
    }

    @Override
    public javax.swing.tree.TreeNode getChildAt(int childIndex)
    {
        if(childIndex < 0)
        {
            throw new ArrayIndexOutOfBoundsException(childIndex);
        }
        int offset = 0;
        for(java.util.List l : _childrenArray)
        {
            if(childIndex < offset + l.size())
            {
                return (javax.swing.tree.TreeNode)l.get(childIndex - offset);
            }
            else
            {
                offset += l.size();
            }
        }
        throw new ArrayIndexOutOfBoundsException(childIndex);
    }

    @Override
    public int getChildCount()
    {
        int result = 0;
        for(java.util.List l : _childrenArray)
        {
            result += l.size();
        }
        return result;
    }

    @Override
    public int getIndex(javax.swing.tree.TreeNode node)
    {
        int offset = 0;
        for(java.util.List l : _childrenArray)
        {
            int index = l.indexOf(node);
            if(index == -1)
            {
                offset += l.size();
            }
            else
            {
                return offset + index;
            }
        }
        return -1;
    }

    @Override
    public boolean isLeaf()
    {
        for(java.util.List l : _childrenArray)
        {
            if(!l.isEmpty())
            {
                return false;
            }
        }
        return true;
    }

    //
    // TreeNode overrides
    //
    @Override
    public void retrieveIceLog()
    {
        if(_showIceLogDialog == null)
        {
            final String prefix = "Opening Ice Log file for " + getServerDisplayName() + "...";

            provideAdmin(prefix, (admin) ->
                {
                    final com.zeroc.Ice.LoggerAdminPrx loggerAdmin =
                        com.zeroc.Ice.LoggerAdminPrx.uncheckedCast(admin.ice_facet("Logger"));
                    final String title = getDisplayName() + " Ice log";
                    final String defaultFileName = getDefaultFileName();

                    SwingUtilities.invokeLater(() ->
                        {
                            success(prefix);
                            if(_showIceLogDialog == null)
                            {
                                _showIceLogDialog = new ShowIceLogDialog(Communicator.this, title, loggerAdmin,
                                                                         defaultFileName,
                                                                         getRoot().getLogMaxLines(),
                                                                         getRoot().getLogInitialLines());
                            }
                            else
                            {
                                _showIceLogDialog.toFront();
                            }
                        });
                });
        }
        else
        {
            _showIceLogDialog.toFront();
        }
    }

    @Override
    public void clearShowIceLogDialog()
    {
        _showIceLogDialog = null;
    }

    protected void showRuntimeProperties()
    {
        final String prefix = "Retrieving properties for " + getDisplayName() + "...";
        provideAdmin(prefix, (admin) ->
            {
                final com.zeroc.Ice.PropertiesAdminPrx propertiesAdmin =
                    com.zeroc.Ice.PropertiesAdminPrx.uncheckedCast(admin.ice_facet("Properties"));

                propertiesAdmin.getPropertiesForPrefixAsync("").whenComplete((result, ex) ->
                    {
                        if(ex == null)
                        {
                            SwingUtilities.invokeLater(() ->
                                {
                                    success(prefix);
                                    ((CommunicatorEditor)getEditor()).setRuntimeProperties(
                                        (java.util.SortedMap<String, String>)result, Communicator.this);
                                });
                        }
                        else if(ex instanceof com.zeroc.Ice.ObjectNotExistException || ex instanceof com.zeroc.Ice.FacetNotExistException)
                        {
                            SwingUtilities.invokeLater(() -> getRoot().getCoordinator().getStatusBar().setText(prefix + " Admin not available."));
                        }
                        else
                        {
                            amiFailure(prefix, "Failed to retrieve the properties for " + getDisplayName(), ex);
                        }
                    });
            });
    }

    protected void fetchMetricsViewNames()
    {
        if(_metricsRetrieved)
        {
            return; // Already loaded.
        }
        _metricsRetrieved = true;

        final String prefix = "Retrieving metrics for " + getDisplayName() + "...";
        if(!provideAdmin(prefix, (admin) ->
            {
                final com.zeroc.IceMX.MetricsAdminPrx metricsAdmin =
                    com.zeroc.IceMX.MetricsAdminPrx.uncheckedCast(admin.ice_facet("Metrics"));

                metricsAdmin.getMetricsViewNamesAsync().whenComplete((result, ex) ->
                    {
                        if(ex == null)
                        {
                            SwingUtilities.invokeLater(() ->
                                {
                                    success(prefix);
                                    _metrics.clear();
                                    for(String name : result.returnValue)
                                    {
                                        insertSortedChild(
                                            new MetricsView(Communicator.this, name, metricsAdmin, true), _metrics, null);
                                    }
                                    for(String name : result.disabledViews)
                                    {
                                        insertSortedChild(
                                            new MetricsView(Communicator.this, name, metricsAdmin, false), _metrics, null);
                                    }
                                    getRoot().getTreeModel().nodeStructureChanged(Communicator.this);
                                });
                        }
                        else if(ex instanceof com.zeroc.Ice.ObjectNotExistException || ex instanceof com.zeroc.Ice.FacetNotExistException)
                        {
                            SwingUtilities.invokeLater(() -> getRoot().getCoordinator().getStatusBar().setText(prefix + " Admin not available."));
                        }
                        else
                        {
                            amiFailure(prefix, "Failed to retrieve the metrics for " + getDisplayName(), ex);
                        }
                    });
            }))
        {
            _metricsRetrieved = false;
        }
    }

    void updateMetrics()
    {
        _metricsRetrieved = false;
        if(getRoot().getTree().isExpanded(getPath()))
        {
            fetchMetricsViewNames();
        }
    }

    public java.util.List<MetricsView>
    getMetrics()
    {
        return new java.util.ArrayList<>(_metrics);
    }

    protected abstract java.util.concurrent.CompletableFuture<com.zeroc.Ice.ObjectPrx> getAdminAsync();
    protected abstract String getDisplayName();
    protected String getServerDisplayName()
    {
        return getDisplayName();
    }
    protected abstract String getDefaultFileName();

    private boolean provideAdmin(final String prefix, final java.util.function.Consumer<com.zeroc.Ice.ObjectPrx> consumer)
    {
        getRoot().getCoordinator().getStatusBar().setText(prefix);
        try
        {
            getAdminAsync().whenComplete((admin, adminEx) ->
                {
                    if(adminEx == null && admin != null)
                    {
                        try
                        {
                            consumer.accept(admin);
                        }
                        catch(com.zeroc.Ice.LocalException e)
                        {
                            SwingUtilities.invokeLater(() -> getRoot().getCoordinator().getStatusBar().setText(prefix + " " + e.toString() + "."));
                        }
                    }
                    else if(adminEx == null || adminEx instanceof com.zeroc.Ice.ObjectNotExistException)
                    {
                        SwingUtilities.invokeLater(() -> getRoot().getCoordinator().getStatusBar().setText(prefix + " Admin not available."));
                    }
                    else
                    {
                        amiFailure(prefix, "Failed to retrieve the Admin proxy for " + getServerDisplayName(), adminEx);
                    }
                });
        }
        catch(com.zeroc.Ice.LocalException e)
        {
            getRoot().getCoordinator().getStatusBar().setText(prefix + " " + e.toString() + ".");
            return false;
        }
        return true;
    }

    protected ShowIceLogDialog _showIceLogDialog;
    protected java.util.List<MetricsView> _metrics = new java.util.LinkedList<>();
    protected boolean _metricsRetrieved = false;

    protected final java.util.List[] _childrenArray;
}
