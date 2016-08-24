// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceGridGUI.LiveDeployment;

import java.awt.Component;
import java.awt.Cursor;

import javax.swing.JPopupMenu;
import javax.swing.JTree;
import javax.swing.tree.DefaultTreeCellRenderer;
import com.zeroc.IceGrid.*;
import com.zeroc.IceGridGUI.*;

class Slave extends TreeNode
{
    //
    // Actions
    //
    @Override
    public boolean[] getAvailableActions()
    {
        boolean[] actions = new boolean[com.zeroc.IceGridGUI.LiveDeployment.TreeNode.ACTION_COUNT];
        actions[SHUTDOWN_REGISTRY] = true;
        actions[RETRIEVE_STDOUT] = true;
        actions[RETRIEVE_STDERR] = true;
        return actions;
    }

    @Override
    public void shutdownRegistry()
    {
        final String prefix = "Shutting down registry '" + _id + "'...";
        getCoordinator().getStatusBar().setText(prefix);

        try
        {
            getCoordinator().getMainFrame().setCursor(Cursor.getPredefinedCursor(Cursor.WAIT_CURSOR));

            getCoordinator().getAdmin().shutdownRegistryAsync(_id).whenComplete((result, ex) ->
                {
                    amiComplete(prefix, "Failed to shutdown " + _id, ex);
                });

        }
        catch(com.zeroc.Ice.LocalException e)
        {
            failure(prefix, "Failed to shutdown " + _id, e.toString());
        }
        finally
        {
            getCoordinator().getMainFrame().setCursor(Cursor.getPredefinedCursor(Cursor.DEFAULT_CURSOR));
        }
    }

    @Override
    public void retrieveOutput(final boolean stdout)
    {
        getRoot().openShowLogFileDialog(new ShowLogFileDialog.FileIteratorFactory()
            {
                @Override
                public FileIteratorPrx open(int count)
                    throws FileNotAvailableException, RegistryNotExistException, RegistryUnreachableException
                {
                    AdminSessionPrx session = getCoordinator().getSession();

                    FileIteratorPrx result;
                    if(stdout)
                    {
                        result = session.openRegistryStdOut(_id, count);
                    }
                    else
                    {
                        result = session.openRegistryStdErr(_id, count);
                    }
                    return result;
                }

                @Override
                public String getTitle()
                {
                    return "Registry " + _title + " " + (stdout ? "stdout" : "stderr");
                }

                @Override
                public String getDefaultFilename()
                {
                    return _id + (stdout ? ".out" : ".err");
                }
            });
    }

    @Override
    public JPopupMenu getPopupMenu()
    {
        LiveActions la = getCoordinator().getLiveActionsForPopup();

        if(_popup == null)
        {
            _popup = new JPopupMenu();
            _popup.add(la.get(RETRIEVE_STDOUT));
            _popup.add(la.get(RETRIEVE_STDERR));
            _popup.addSeparator();
            _popup.add(la.get(SHUTDOWN_REGISTRY));
        }

        la.setTarget(this);
        return _popup;
    }

    @Override
    public Editor getEditor()
    {
        if(_editor == null)
        {
            _editor = new SlaveEditor();
        }
        _editor.show(_info);
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
        boolean hasFocus)
    {
        if(_cellRenderer == null)
        {
            //
            // TODO: separate icon for master
            //

            _cellRenderer = new DefaultTreeCellRenderer();
            _cellRenderer.setLeafIcon(Utils.getIcon("/icons/16x16/registry.png"));
        }

        return _cellRenderer.getTreeCellRendererComponent(tree, value, sel, expanded, leaf, row, hasFocus);
    }

    Slave(TreeNode parent, RegistryInfo info, String instanceName)
    {
        super(parent, info.name);
        _info = info;
        _title = instanceName + " (" + info.name + ")";
    }

    private final RegistryInfo _info;
    private final String _title;

    static private DefaultTreeCellRenderer _cellRenderer;
    static private SlaveEditor _editor;
    static private JPopupMenu _popup;
}
