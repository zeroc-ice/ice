// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceGridGUI.Application;

import javax.swing.JPopupMenu;

import com.zeroc.IceGrid.*;
import com.zeroc.IceGridGUI.*;

class ReplicaGroups extends ListTreeNode
{
    static public java.util.List<ReplicaGroupDescriptor>
    copyDescriptors(java.util.List<ReplicaGroupDescriptor> descriptors)
    {
        java.util.List<ReplicaGroupDescriptor> copy = new java.util.LinkedList<>();
        for(ReplicaGroupDescriptor p : descriptors)
        {
            copy.add(ReplicaGroup.copyDescriptor(p));
        }
        return copy;
    }

    //
    // Actions
    //
    @Override
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

    @Override
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

    @Override
    public void newReplicaGroup()
    {
        ReplicaGroupDescriptor descriptor = new
            ReplicaGroupDescriptor(
                makeNewChildId("NewReplicaGroup"),
                null,
                "",
                new java.util.LinkedList<ObjectDescriptor>(),
                "",
                "");

        newReplicaGroup(descriptor);
    }

    @Override
    public void paste()
    {
        Object descriptor =  getCoordinator().getClipboard();

        ReplicaGroupDescriptor d = ReplicaGroup.copyDescriptor((ReplicaGroupDescriptor)descriptor);
        d.id = makeNewChildId(d.id);
        newReplicaGroup(d);
    }

    ReplicaGroups(TreeNode parent, java.util.List<ReplicaGroupDescriptor> desc)
        throws UpdateFailedException
    {
        super(false, parent, "Replica Groups");
        _descriptors = desc;

        for(ReplicaGroupDescriptor p : _descriptors)
        {
            insertChild(new ReplicaGroup(false, this, p), false);
        }
    }

    java.util.LinkedList<ReplicaGroupDescriptor> getUpdates()
    {
        java.util.LinkedList<ReplicaGroupDescriptor> updates = new java.util.LinkedList<>();
        for(TreeNodeBase p : _children)
        {
            ReplicaGroup ra = (ReplicaGroup)p;
            if(ra.getEditable().isNew() || ra.getEditable().isModified())
            {
                updates.add((ReplicaGroupDescriptor)ra.getDescriptor());
            }
        }
        return updates;
    }

    void commit()
    {
        _editable.commit();
        for(TreeNodeBase p : _children)
        {
            ReplicaGroup rg = (ReplicaGroup)p;
            rg.commit();
        }
    }

    void update(java.util.List<ReplicaGroupDescriptor> descriptors, String[] removeReplicaGroups)
    {
        _descriptors = descriptors;

        //
        // One big set of removes
        //
        removeChildren(removeReplicaGroups);

        //
        // Updates and inserts
        //
        java.util.List<TreeNodeBase> updatedChildren = new java.util.ArrayList<>();
        for(ReplicaGroupDescriptor p : descriptors)
        {
            ReplicaGroup child = (ReplicaGroup)findChild(p.id);

            if(child == null)
            {
                try
                {
                    insertChild(new ReplicaGroup(false, this, p), true);
                }
                catch(UpdateFailedException e)
                {
                    assert false;
                }
            }
            else
            {
                child.rebuild(p);
                updatedChildren.add(child);
            }
        }
        childrenChanged(updatedChildren);
    }

    @Override
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
        java.util.Iterator<ReplicaGroupDescriptor> p = _descriptors.iterator();
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
        insertChild(new ReplicaGroup(true, this, descriptor), true);

        if(addDescriptor)
        {
            _descriptors.add(descriptor);
        }
    }

    private void newReplicaGroup(ReplicaGroupDescriptor descriptor)
    {
        ReplicaGroup replicaGroup = new ReplicaGroup(this, descriptor);

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

    private java.util.List<ReplicaGroupDescriptor> _descriptors;
    static private JPopupMenu _popup;
}
