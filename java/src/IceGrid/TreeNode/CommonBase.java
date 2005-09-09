// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid.TreeNode;

import javax.swing.tree.TreePath;
import javax.swing.tree.TreeCellRenderer;
import javax.swing.JPopupMenu;
import javax.swing.JPanel;

import com.jgoodies.uif_lite.panel.SimpleInternalFrame;

import IceGrid.Model;

//
// CommonBase is similar to javax.swing.tree.TreeNode
//

public interface CommonBase extends TreeCellRenderer
{
    Object getChildAt(int childIndex);
    int getChildCount();
    int getIndex(Object child);
    boolean isLeaf();

    CommonBase findChild(String id);

    //
    // Unique within the scope of each parent
    //
    String getId();

    Model getModel();

    //
    // Unregister this element with the enclosing application;
    // often no-op
    //
    void unregister();


    //
    // Destroys this node, no-op when destruction not allowed
    //
    // void remove();

    //
    // Set this node as a parent, and recursively update
    // the path of all children.
    // This method has no effect on the parent->children relationship,
    // only child->parent.
    // 
    void setParent(CommonBase newParent);

    //
    // Get this node's parent
    // This is used by nodes to create events
    // pointing to themselves
    //
    CommonBase getParent();

    //
    // The path to this node
    // typically used by children to create TreeModelEvents
    //
    TreePath getPath();

    //
    // Get pop-up menu
    //
    JPopupMenu getPopupMenu();

    void displayProperties();

    //
    // Get properties
    //
    PropertiesHolder getPropertiesHolder();

    //
    // The enclosing editable
    //
    Editable getEditable();
}
