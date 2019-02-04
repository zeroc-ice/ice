//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.IceGridGUI;

import java.awt.Component;
import javax.swing.JTree;
import javax.swing.tree.TreeCellRenderer;
import javax.swing.tree.DefaultTreeCellRenderer;

class CellRenderer implements TreeCellRenderer
{
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
        TreeCellRenderer node = (TreeCellRenderer)value;
        Component result = node.getTreeCellRendererComponent(tree, value, sel, expanded, leaf, row, hasFocus);

        if(result == null)
        {
            result = _defaultRenderer.getTreeCellRendererComponent(tree, value, sel, expanded, leaf, row, hasFocus);
        }
        return result;
    }

    private TreeCellRenderer _defaultRenderer = new DefaultTreeCellRenderer();
}
