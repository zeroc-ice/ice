// Copyright (c) ZeroC, Inc.

package com.zeroc.IceGridGUI.Application;

import com.zeroc.IceGrid.ObjectDescriptor;
import com.zeroc.IceGrid.ReplicaGroupDescriptor;
import com.zeroc.IceGridGUI.ApplicationActions;
import com.zeroc.IceGridGUI.TreeNodeBase;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;

import javax.swing.JPopupMenu;

class ReplicaGroups extends ListTreeNode {
    public static List<ReplicaGroupDescriptor> copyDescriptors(
            List<ReplicaGroupDescriptor> descriptors) {
        List<ReplicaGroupDescriptor> copy = new LinkedList<>();
        for (ReplicaGroupDescriptor p : descriptors) {
            copy.add(ReplicaGroup.copyDescriptor(p));
        }
        return copy;
    }

    // Actions
    @Override
    public boolean[] getAvailableActions() {
        boolean[] actions = new boolean[ACTION_COUNT];

        Object descriptor = getCoordinator().getClipboard();
        if (descriptor != null) {
            actions[PASTE] = descriptor instanceof ReplicaGroupDescriptor;
        }

        actions[NEW_REPLICA_GROUP] = true;
        return actions;
    }

    @Override
    public JPopupMenu getPopupMenu() {
        ApplicationActions actions = getCoordinator().getActionsForPopup();
        if (_popup == null) {
            _popup = new JPopupMenu();
            _popup.add(actions.get(NEW_REPLICA_GROUP));
        }
        actions.setTarget(this);
        return _popup;
    }

    @Override
    public void newReplicaGroup() {
        ReplicaGroupDescriptor descriptor =
            new ReplicaGroupDescriptor(
                makeNewChildId("NewReplicaGroup"),
                null,
                "",
                new LinkedList<ObjectDescriptor>(),
                "",
                "");

        newReplicaGroup(descriptor);
    }

    @Override
    public void paste() {
        Object descriptor = getCoordinator().getClipboard();

        ReplicaGroupDescriptor d = ReplicaGroup.copyDescriptor((ReplicaGroupDescriptor) descriptor);
        d.id = makeNewChildId(d.id);
        newReplicaGroup(d);
    }

    ReplicaGroups(TreeNode parent, List<ReplicaGroupDescriptor> desc)
        throws UpdateFailedException {
        super(false, parent, "Replica Groups");
        _descriptors = desc;

        for (ReplicaGroupDescriptor p : _descriptors) {
            insertChild(new ReplicaGroup(false, this, p), false);
        }
    }

    LinkedList<ReplicaGroupDescriptor> getUpdates() {
        LinkedList<ReplicaGroupDescriptor> updates = new LinkedList<>();
        for (TreeNodeBase p : _children) {
            ReplicaGroup ra = (ReplicaGroup) p;
            if (ra.getEditable().isNew() || ra.getEditable().isModified()) {
                updates.add((ReplicaGroupDescriptor) ra.getDescriptor());
            }
        }
        return updates;
    }

    void commit() {
        _editable.commit();
        for (TreeNodeBase p : _children) {
            ReplicaGroup rg = (ReplicaGroup) p;
            rg.commit();
        }
    }

    void update(List<ReplicaGroupDescriptor> descriptors, String[] removeReplicaGroups) {
        _descriptors = descriptors;

        // One big set of removes
        removeChildren(removeReplicaGroups);

        // Updates and inserts
        List<TreeNodeBase> updatedChildren = new ArrayList<>();
        for (ReplicaGroupDescriptor p : descriptors) {
            ReplicaGroup child = (ReplicaGroup) findChild(p.id);

            if (child == null) {
                try {
                    insertChild(new ReplicaGroup(false, this, p), true);
                } catch (UpdateFailedException e) {
                    assert false;
                }
            } else {
                child.rebuild(p);
                updatedChildren.add(child);
            }
        }
        childrenChanged(updatedChildren);
    }

    @Override
    Object getDescriptor() {
        return _descriptors;
    }

    /* TODO
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

    void removeDescriptor(Object descriptor) {
        // A straight remove uses equals(), which is not the desired behavior
        Iterator<ReplicaGroupDescriptor> p = _descriptors.iterator();
        while (p.hasNext()) {
            if (descriptor == p.next()) {
                p.remove();
                break;
            }
        }
    }

    void tryAdd(ReplicaGroupDescriptor descriptor, boolean addDescriptor)
        throws UpdateFailedException {
        insertChild(new ReplicaGroup(true, this, descriptor), true);

        if (addDescriptor) {
            _descriptors.add(descriptor);
        }
    }

    private void newReplicaGroup(ReplicaGroupDescriptor descriptor) {
        ReplicaGroup replicaGroup = new ReplicaGroup(this, descriptor);

        try {
            insertChild(replicaGroup, true);
        } catch (UpdateFailedException e) {
            assert false;
        }
        getRoot().setSelectedNode(replicaGroup);
    }

    private List<ReplicaGroupDescriptor> _descriptors;
    private static JPopupMenu _popup;
}
