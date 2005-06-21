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
import javax.swing.JPopupMenu;
import javax.swing.JPanel;

//
// CommonBase is similar to javax.swing.tree.TreeNode
//

public interface CommonBase
{
    Object getChildAt(int childIndex);
    int getChildCount();
    int getIndex(Object child);
    boolean isLeaf();

    //
    // Add this node as a parent. This operation does something
    // only when newParent a root node or was attached to a 
    // root node using addParent().
    // addParent() is propagated to the child of this node.
    // This method has no effect on the parent->children relationship,
    // only child->parent.
    // 
    void addParent(CommonBase newParent);

    //
    // Recursively destroys a sub-parent-tree: the children
    // forget this parent, but it does not affect the parents
    // knowledge about their children.
    //
    void removeParent(CommonBase parent);

    
    //
    // Adds this parent for this view. Used by the view-less addParent
    // to notify its children.
    //
    void addParent(CommonBase newParent, TreePath path, int view);

    //
    // Remove the parent for this view; used by the view-less 
    // removeParent to notify its children.
    //
    void removeParent(int view);

    //
    // Get the parent for the given view
    // This is used by nodes to create events
    // pointing to themselves
    //
    CommonBase getParent(int view);

    //
    // The path to this node for the given view;
    // typically used by children to create TreeModelEvents
    //
    TreePath getPath(int view);

    //
    // Get pop-up menu
    //
    JPopupMenu getPopupMenu();

    //
    // Get properties panel
    //
    JPanel getProperties(int view);

}
