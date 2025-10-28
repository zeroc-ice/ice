// Copyright (c) ZeroC, Inc.

package com.zeroc.IceGridGUI.Application;

import com.zeroc.IceGrid.PropertySetDescriptor;
import com.zeroc.IceGridGUI.Utils;
import com.zeroc.IceGridGUI.XMLWriter;

import java.awt.Component;
import java.io.IOException;
import java.util.LinkedList;

import javax.swing.JTree;
import javax.swing.tree.DefaultTreeCellRenderer;

class PropertySet extends TreeNode {
    public static PropertySetDescriptor copyDescriptor(PropertySetDescriptor d) {
        PropertySetDescriptor psd = d.clone();
        psd.properties = new LinkedList<>(psd.properties);
        return psd;
    }

    @Override
    public Component getTreeCellRendererComponent(
            JTree tree,
            Object value,
            boolean sel,
            boolean expanded,
            boolean leaf,
            int row,
            boolean hasFocus) {
        if (_cellRenderer == null) {
            _cellRenderer = new DefaultTreeCellRenderer();
            _cellRenderer.setLeafIcon(Utils.getIcon("/icons/16x16/property_set.png"));
        }

        return _cellRenderer.getTreeCellRendererComponent(
            tree, value, sel, expanded, leaf, row, hasFocus);
    }

    // Actions
    @Override
    public boolean[] getAvailableActions() {
        boolean[] actions = new boolean[ACTION_COUNT];
        actions[COPY] = !_ephemeral;

        if (((TreeNode) _parent).getAvailableActions()[PASTE]) {
            actions[PASTE] = true;
        }
        actions[DELETE] = true;

        if (!_ephemeral) {
            actions[SHOW_VARS] = true;
            actions[SUBSTITUTE_VARS] = true;
        }
        return actions;
    }

    @Override
    public void copy() {
        getCoordinator().setClipboard(copyDescriptor(_descriptor));
        getCoordinator().getActionsForMenu().get(PASTE).setEnabled(true);
    }

    @Override
    public void paste() {
        ((TreeNode) _parent).paste();
    }

    @Override
    public void destroy() {
        PropertySetParent parent = (PropertySetParent) _parent;
        parent.removePropertySet(this);

        if (!_ephemeral) {
            parent.removeDescriptor(_unsubstitutedId);
            if (_editable != null) {
                parent.getEditable().removeElement(_unsubstitutedId, _editable, PropertySet.class);
            } else {
                parent.getEditable().markModified();
            }
            getRoot().updated();
        }
    }

    @Override
    public Editor getEditor() {
        if (_editor == null) {
            if (_inServerInstance) {
                _editor =
                    (PropertySetEditor)
                        getRoot().getEditor(ServerInstancePropertySetEditor.class, this);
            } else {
                _editor = (PropertySetEditor) getRoot().getEditor(PropertySetEditor.class, this);
            }
        }
        _editor.show(_unsubstitutedId, this);
        return _editor;
    }

    @Override
    protected Editor createEditor() {
        if (_inServerInstance) {
            return new ServerInstancePropertySetEditor();
        } else {
            return new PropertySetEditor();
        }
    }

    @Override
    public boolean isEphemeral() {
        return _ephemeral;
    }

    public String unsubstitutedId() {
        return _unsubstitutedId;
    }

    @Override
    Object getDescriptor() {
        return _descriptor;
    }

    Object saveDescriptor() {
        return _descriptor.clone();
    }

    void restoreDescriptor(Object savedDescriptor) {
        PropertySetDescriptor clone = (PropertySetDescriptor) savedDescriptor;
        _descriptor.references = clone.references;
        _descriptor.properties = clone.properties;
    }

    void commit() {
        if (_editable != null) {
            _editable.commit();
        }
    }

    Editable getEditable() {
        if (_editable != null) {
            return _editable;
        } else {
            return ((PropertySetParent) _parent).getEditable();
        }
    }

    PropertySet(
            boolean brandNew,
            TreeNode parent,
            String id,
            String unsubstitutedId,
            PropertySetDescriptor descriptor) {
        super(parent, id);
        _unsubstitutedId = unsubstitutedId;
        _inServerInstance = parent instanceof ServerInstance;
        _ephemeral = false;
        _editable = new Editable(brandNew);
        rebuild(descriptor);
    }

    PropertySet(
            TreeNode parent, String id, String unsubstitutedId, PropertySetDescriptor descriptor) {
        super(parent, id);
        _unsubstitutedId = unsubstitutedId;
        _inServerInstance = parent instanceof ServerInstance;
        _ephemeral = false;
        _editable = null;
        rebuild(descriptor);
    }

    PropertySet(TreeNode parent, String id, PropertySetDescriptor descriptor) {
        super(parent, id);
        _unsubstitutedId = id;
        _inServerInstance = parent instanceof ServerInstance;
        _ephemeral = true;
        _editable = null;
        rebuild(descriptor);
    }

    @Override
    void write(XMLWriter writer) throws IOException {
        if (!_ephemeral) {
            writePropertySet(
                writer,
                _unsubstitutedId,
                _inServerInstance ? "service" : "id",
                _descriptor,
                null,
                null);
        }
    }

    void rebuild(PropertySetDescriptor descriptor) {
        _descriptor = descriptor;
    }

    private PropertySetDescriptor _descriptor;
    private String _unsubstitutedId;
    private final boolean _ephemeral;
    private final Editable _editable;
    private final boolean _inServerInstance;
    private PropertySetEditor _editor;

    private static DefaultTreeCellRenderer _cellRenderer;
}
