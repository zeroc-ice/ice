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
import javax.swing.JPopupMenu;

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

    Editor getEditor();

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



    //
    // Actions
    //
    static final int NEW_ADAPTER = 0;
    static final int NEW_DBENV = 1;
    static final int NEW_NODE = 2;
    static final int NEW_REPLICA_GROUP = 3;
    static final int NEW_SERVER = 4;
    static final int NEW_SERVER_ICEBOX = 5;
    static final int NEW_SERVER_FROM_TEMPLATE = 6;
    static final int NEW_SERVICE = 7;
    static final int NEW_SERVICE_FROM_TEMPLATE = 8;
    static final int NEW_TEMPLATE_SERVER = 9;
    static final int NEW_TEMPLATE_SERVER_ICEBOX = 10;
    static final int NEW_TEMPLATE_SERVICE = 11;
  
    static final int COPY = 12;
    static final int PASTE = 13;
    static final int DELETE = 14;

    static final int SHOW_VARS = 15;
    static final int SUBSTITUTE_VARS = 16;

    static final int MOVE_UP = 17;
    static final int MOVE_DOWN =18;
    static final int START = 19;
    static final int STOP = 20;
    static final int ENABLE = 21;
    static final int DISABLE = 22;

    static final int SHUTDOWN_NODE = 23;

    static final int APPLICATION_INSTALL_DISTRIBUTION = 24;
    static final int SERVER_INSTALL_DISTRIBUTION = 25;

    static public final int ACTION_COUNT = 26;

    boolean[] getAvailableActions();
    
    void newAdapter();
    void newDbEnv();
    void newNode();
    void newReplicaGroup();
    void newServer();
    void newServerIceBox();
    void newServerFromTemplate();
    void newService();
    void newServiceFromTemplate();
    void newTemplateServer();
    void newTemplateServerIceBox();
    void newTemplateService();
    
    void copy();
    void paste();
    void delete();
   
    void moveUp();
    void moveDown();
    void start();
    void stop();
    void enable();
    void disable();

    void shutdownNode();

    void applicationInstallDistribution();
    void serverInstallDistribution();

    JPopupMenu getPopupMenu();
}
