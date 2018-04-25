// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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
import com.zeroc.IceGrid.*;
import com.zeroc.IceGridGUI.*;

class Adapter extends TreeNode
{
    @Override
    public Editor getEditor()
    {
        if(_editor == null)
        {
            _editor = new AdapterEditor();
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
        boolean hasFocus)
    {
        if(_cellRenderer == null)
        {
            _cellRenderer = new DefaultTreeCellRenderer();
            _activeIcon = Utils.getIcon("/icons/16x16/adapter_active.png");
            _inactiveIcon = Utils.getIcon("/icons/16x16/adapter_inactive.png");
        }

        if(_currentEndpoints == null || _currentEndpoints.length() == 0)
        {
            _cellRenderer.setLeafIcon(_inactiveIcon);
        }
        else
        {
            _cellRenderer.setLeafIcon(_activeIcon);
        }

        _cellRenderer.setToolTipText(_toolTip);
        return _cellRenderer.getTreeCellRendererComponent(tree, value, sel, expanded, leaf, row, hasFocus);
    }

    Adapter(TreeNode parent, String adapterName, Utils.Resolver resolver, String adapterId,
            AdapterDescriptor descriptor, com.zeroc.Ice.ObjectPrx proxy)
    {
        super(parent, adapterName);
        _resolver = resolver;
        _adapterId = adapterId;
        _descriptor = descriptor;

        setCurrentEndpoints(proxy);
    }

    AdapterDescriptor getDescriptor()
    {
        return _descriptor;
    }

    Utils.Resolver getResolver()
    {
        return _resolver;
    }

    String getCurrentEndpoints()
    {
        return _currentEndpoints;
    }

    java.util.Map<String, String> getProperties()
    {
        if(_parent instanceof Server)
        {
            return ((Server)_parent).getProperties();
        }
        else
        {
            return ((Service)_parent).getProperties();
        }
    }

    boolean update(AdapterDynamicInfo info)
    {
        if(info == null)
        {
            setCurrentEndpoints(null);
            getRoot().getTreeModel().nodeChanged(this);
            return true;
        }
        else if(info.id.equals(_adapterId))
        {
            setCurrentEndpoints(info.proxy);
            getRoot().getTreeModel().nodeChanged(this);
            return true;
        }
        else
        {
            return false;
        }
    }

    boolean update(java.util.List<AdapterDynamicInfo> infoList)
    {
        for(AdapterDynamicInfo info : infoList)
        {
            if(update(info))
            {
                return true;
            }
        }
        return false;
    }

    private void setCurrentEndpoints(com.zeroc.Ice.ObjectPrx proxy)
    {
        if(proxy == null)
        {
            _currentEndpoints = null;
            _toolTip = "Inactive";
        }
        else
        {
            String str = proxy.toString();
            int index = str.indexOf(':');
            if(index == -1 || index == str.length() - 1)
            {
                _currentEndpoints = "";
            }
            else
            {
                _currentEndpoints = str.substring(index + 1);
            }
            _toolTip = "Published endpoints: " + _currentEndpoints;
        }
    }

    private AdapterDescriptor _descriptor;
    private Utils.Resolver _resolver;
    private String _adapterId;

    private String _currentEndpoints;
    private String _toolTip;

    static private DefaultTreeCellRenderer _cellRenderer;
    static private Icon _activeIcon;
    static private Icon _inactiveIcon;

    static private AdapterEditor _editor;
}
