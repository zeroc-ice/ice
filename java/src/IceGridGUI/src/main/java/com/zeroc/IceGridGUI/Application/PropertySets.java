// Copyright (c) ZeroC, Inc.

package com.zeroc.IceGridGUI.Application;

import com.zeroc.IceGrid.PropertyDescriptor;
import com.zeroc.IceGrid.PropertySetDescriptor;
import com.zeroc.IceGridGUI.ApplicationActions;
import com.zeroc.IceGridGUI.TreeNodeBase;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

import javax.swing.JPopupMenu;

class PropertySets extends ListTreeNode implements PropertySetParent {
    public static Map<String, PropertySetDescriptor> copyDescriptors(
            Map<String, PropertySetDescriptor> descriptors) {
        Map<String, PropertySetDescriptor> copy = new HashMap<>();
        for (Map.Entry<String, PropertySetDescriptor> p : descriptors.entrySet()) {
            copy.put(p.getKey(), PropertySet.copyDescriptor(p.getValue()));
        }
        return copy;
    }

    // Actions
    @Override
    public boolean[] getAvailableActions() {
        boolean[] actions = new boolean[ACTION_COUNT];

        Object descriptor = getCoordinator().getClipboard();
        if (descriptor != null) {
            actions[PASTE] = descriptor instanceof PropertySetDescriptor;
        }

        actions[NEW_PROPERTY_SET] = true;
        return actions;
    }

    @Override
    public JPopupMenu getPopupMenu() {
        ApplicationActions actions = getCoordinator().getActionsForPopup();
        if (_popup == null) {
            _popup = new JPopupMenu();
            _popup.add(actions.get(NEW_PROPERTY_SET));
        }
        actions.setTarget(this);
        return _popup;
    }

    @Override
    public void newPropertySet() {
        PropertySetDescriptor descriptor =
            new PropertySetDescriptor(
                new String[0], new LinkedList<PropertyDescriptor>());
        newPropertySet(descriptor);
    }

    @Override
    public void paste() {
        Object descriptor = getCoordinator().getClipboard();

        PropertySetDescriptor d = PropertySet.copyDescriptor((PropertySetDescriptor) descriptor);
        newPropertySet(d);
    }

    PropertySets(TreeNode parent, Map<String, PropertySetDescriptor> desc)
        throws UpdateFailedException {
        super(false, parent, "Property Sets");
        _descriptors = desc;

        for (Map.Entry<String, PropertySetDescriptor> p : _descriptors.entrySet()) {
            String id = p.getKey();
            insertChild(new PropertySet(false, this, id, id, p.getValue()), false);
        }
    }

    void update(Map<String, PropertySetDescriptor> updates, String[] removePropertySets)
        throws UpdateFailedException {
        //
        // Note: _descriptors is updated by Root
        //

        // One big set of removes
        removeChildren(removePropertySets);

        // One big set of updates, followed by inserts
        List<PropertySet> newChildren = new ArrayList<>();

        for (Map.Entry<String, PropertySetDescriptor> p : updates.entrySet()) {
            String id = p.getKey();
            PropertySetDescriptor psd = p.getValue();
            PropertySet child = (PropertySet) findChild(id);
            if (child == null) {
                newChildren.add(new PropertySet(false, this, id, id, psd));
            } else {
                child.rebuild(psd);
            }
        }
        insertChildren(newChildren, true);
    }

    Map<String, PropertySetDescriptor> getUpdates() {
        Map<String, PropertySetDescriptor> updates = new HashMap<>();
        for (TreeNodeBase p : _children) {
            PropertySet ps = (PropertySet) p;
            if (ps.getEditable().isNew() || ps.getEditable().isModified()) {
                updates.put(ps.getId(), (PropertySetDescriptor) ps.getDescriptor());
            }
        }
        return updates;
    }

    void commit() {
        _editable.commit();
        for (TreeNodeBase p : _children) {
            PropertySet ps = (PropertySet) p;
            ps.commit();
        }
    }

    @Override
    Object getDescriptor() {
        return _descriptors;
    }

    @Override
    public void tryAdd(String id, PropertySetDescriptor descriptor) throws UpdateFailedException {
        insertChild(new PropertySet(true, this, id, id, descriptor), true);

        _descriptors.put(id, descriptor);
    }

    @Override
    public void tryRename(String oldId, String oldId2, String newId) throws UpdateFailedException {
        PropertySet oldChild = (PropertySet) findChild(oldId);
        assert oldChild != null;
        removeChild(oldChild);
        PropertySetDescriptor descriptor = (PropertySetDescriptor) oldChild.getDescriptor();

        try {
            insertChild(new PropertySet(true, this, newId, newId, descriptor), true);
        } catch (UpdateFailedException ex) {
            try {
                insertChild(oldChild, true);
            } catch (UpdateFailedException ufe) {
                assert false;
            }
            throw ex;
        }

        _editable.removeElement(oldId, oldChild.getEditable(), PropertySet.class);
        _descriptors.remove(oldId);
        _descriptors.put(newId, descriptor);
    }

    @Override
    public void insertPropertySet(PropertySet nps, boolean fireEvent) throws UpdateFailedException {
        insertChild(nps, fireEvent);
    }

    @Override
    public void removePropertySet(PropertySet nps) {
        removeChild(nps);
    }

    @Override
    public void removeDescriptor(String id) {
        _descriptors.remove(id);
    }

    @Override
    public Editable getEditable() {
        return super.getEditable();
    }

    private void newPropertySet(PropertySetDescriptor descriptor) {
        String id = makeNewChildId("PropertySet");

        PropertySet propertySet = new PropertySet(this, id, descriptor);

        try {
            insertChild(propertySet, true);
        } catch (UpdateFailedException e) {
            assert false;
        }
        getRoot().setSelectedNode(propertySet);
    }

    private final Map<String, PropertySetDescriptor> _descriptors;
    private static JPopupMenu _popup;
}
