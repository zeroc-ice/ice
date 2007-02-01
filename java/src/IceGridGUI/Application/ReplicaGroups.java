// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGridGUI.Application;

import javax.swing.AbstractListModel;
import javax.swing.JMenuItem;
import javax.swing.JPopupMenu;

import IceGrid.*;
import IceGridGUI.*;

class ReplicaGroups extends ListTreeNode
{
    static public java.util.List
    copyDescriptors(java.util.List descriptors)
    {
        java.util.List copy = new java.util.LinkedList();
        java.util.Iterator p = descriptors.iterator();
        while(p.hasNext())
        {
            copy.add(ReplicaGroup.copyDescriptor(
                         (ReplicaGroupDescriptor)p.next()));
        }
        return copy;
    }

    //
    // Actions
    //
    public boolean[] getAvailableActions()
    {
        boolean[] actions = new boolean[ACTION_COUNT];

        Object descriptor =  getCoordinator().getClipboard();
        if(descriptor != null)
        {
            actions[PASTE] = descriptor instanceof ReplicaGroupDescriptor;
        }

        actions[NEW_REPLICA_GROUP] = true;
        return actions;
    }

    public JPopupMenu getPopupMenu()
    {
        ApplicationActions actions = getCoordinator().getActionsForPopup();
        if(_popup == null)
        {
            _popup = new JPopupMenu();
            _popup.add(actions.get(NEW_REPLICA_GROUP));
        }
        actions.setTarget(this);
        return _popup;
    }
    
    public void newReplicaGroup()
    {
        ReplicaGroupDescriptor descriptor = new
            ReplicaGroupDescriptor(
                makeNewChildId("NewReplicaGroup"),
                null,
                new java.util.LinkedList(),
                "");

        newReplicaGroup(descriptor);
    }

    public void paste()
    {
        Object descriptor =  getCoordinator().getClipboard();
        
        ReplicaGroupDescriptor d = ReplicaGroup.copyDescriptor(
            (ReplicaGroupDescriptor)descriptor);
        d.id = makeNewChildId(d.id);
        newReplicaGroup(d);
    }

    
    ReplicaGroups(TreeNode parent, java.util.List desc) throws UpdateFailedException
    {
        super(false, parent, "Replica Groups");
        _descriptors = desc;
        
        java.util.Iterator p = _descriptors.iterator();
        while(p.hasNext())
        {
            ReplicaGroupDescriptor descriptor 
                = (ReplicaGroupDescriptor)p.next();
            
            insertChild(new ReplicaGroup(false, this, descriptor), false);
        }
    }

    java.util.LinkedList getUpdates()
    {
        java.util.LinkedList updates = new java.util.LinkedList();
        java.util.Iterator p = _children.iterator();
        while(p.hasNext())
        {
            ReplicaGroup ra = (ReplicaGroup)p.next();
            if(ra.getEditable().isNew() || ra.getEditable().isModified())
            {
                updates.add(ra.getDescriptor());
            }
        }
        return updates;
    }

    void commit()
    {
        _editable.commit();
        java.util.Iterator p = _children.iterator();
        while(p.hasNext())
        {
            ReplicaGroup rg = (ReplicaGroup)p.next();
            rg.commit();
        }
    }

    void update(java.util.List descriptors, String[] removeReplicaGroups)
    {
        _descriptors = descriptors;

        //
        // One big set of removes
        //
        removeChildren(removeReplicaGroups);

        //
        // Updates and inserts
        //
        java.util.List updatedChildren = new java.util.LinkedList();
        java.util.Iterator p = descriptors.iterator();
        while(p.hasNext())
        {
            ReplicaGroupDescriptor descriptor =
                (ReplicaGroupDescriptor)p.next();
            
            ReplicaGroup child 
                = (ReplicaGroup)findChild(descriptor.id);

            if(child == null)
            {
                try
                {
                    insertChild(
                        new ReplicaGroup(false, this, descriptor),
                        true);
                }
                catch(UpdateFailedException e)
                {
                    assert false;
                }
            }
            else
            {
                child.rebuild(descriptor);
                updatedChildren.add(child);
            }
        }
        childrenChanged(updatedChildren);
    }

    Object getDescriptor()
    {
        return _descriptors;
    }

    /*
    Object saveDescriptor()
    {
        assert false;
        return null;
    }
    void restoreDescriptor(Object savedDescriptor)
    {
        assert false;
    }
    */

    void removeDescriptor(Object descriptor)
    {
        //
        // A straight remove uses equals(), which is not the desired behavior
        //
        java.util.Iterator p = _descriptors.iterator();
        while(p.hasNext())
        {
            if(descriptor == p.next())
            {
                p.remove();
                break;
            }
        }
    }

    void tryAdd(ReplicaGroupDescriptor descriptor, boolean addDescriptor)
        throws UpdateFailedException
    {
        insertChild(
            new ReplicaGroup(true, this, descriptor),
            true);

        if(addDescriptor)
        {
            _descriptors.add(descriptor);
        }
    }

    private void newReplicaGroup(ReplicaGroupDescriptor descriptor)
    {
        ReplicaGroup replicaGroup =
            new ReplicaGroup(this, descriptor);

        try
        {
            insertChild(replicaGroup, true);
        }
        catch(UpdateFailedException e)
        {
            assert false;
        }
        getRoot().setSelectedNode(replicaGroup);
    }

    private java.util.List _descriptors;
    static private JPopupMenu _popup;
}
