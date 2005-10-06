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
import javax.swing.JPanel;

import IceGrid.Actions;
import IceGrid.Model;
import IceGrid.SimpleInternalFrame;

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
    // Ephemeral objects are destroyed when you switch selection
    // without "apply"ing the changes.
    //
    boolean isEphemeral();

    //
    // Destroys this node, returns true when destroyed
    //
    boolean destroy();

    //
    // Get this node's parent;
    // null when the node is not attached to the root
    //
    CommonBase getParent();

    //
    // The path to this node;
    // null when the node is not attached to the root
    // typically used by children to create TreeModelEvents
    //
    TreePath getPath();

    //
    // Actions associated with this object
    //
    Actions getActions();

    void displayProperties();

    //
    // Get properties
    //
    PropertiesHolder getPropertiesHolder();

    //
    // The enclosing editable
    //
    Editable getEditable();

    //
    // The enclosing Application
    //
    Application getApplication();

    //
    // Gets the associated descriptor
    //
    Object getDescriptor();

    //
    // Find child whose descriptor == the given descriptor
    //
    CommonBase findChildWithDescriptor(Object descriptor);

    //
    // Save & restore the descriptor
    // How much needs to be copied depends on how what the corresponding
    // editor writes.
    //
    Object saveDescriptor();
    void restoreDescriptor(Object savedDescriptor);
    
    //
    // Set this child's parent
    //
    void setParent(CommonBase parent);
    void clearParent();

    //
    // Find all instances of this child (including this child)
    //
    java.util.List findAllInstances(CommonBase child);
}
