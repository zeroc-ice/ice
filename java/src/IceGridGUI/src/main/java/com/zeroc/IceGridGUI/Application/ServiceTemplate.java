// Copyright (c) ZeroC, Inc.

package com.zeroc.IceGridGUI.Application;

import com.zeroc.IceGrid.CommunicatorDescriptor;
import com.zeroc.IceGrid.ServiceDescriptor;
import com.zeroc.IceGrid.TemplateDescriptor;
import com.zeroc.IceGridGUI.ApplicationActions;
import com.zeroc.IceGridGUI.Utils;
import com.zeroc.IceGridGUI.XMLWriter;

import java.awt.Component;
import java.io.IOException;
import java.util.LinkedList;
import java.util.List;

import javax.swing.JPopupMenu;
import javax.swing.JTree;
import javax.swing.tree.DefaultTreeCellRenderer;

class ServiceTemplate extends Communicator {
    public static TemplateDescriptor copyDescriptor(TemplateDescriptor templateDescriptor) {
        TemplateDescriptor copy = templateDescriptor.clone();
        copy.descriptor = PlainService.copyDescriptor((ServiceDescriptor) copy.descriptor);
        return copy;
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
            _cellRenderer.setOpenIcon(Utils.getIcon("/icons/16x16/service_template.png"));
            _cellRenderer.setClosedIcon(Utils.getIcon("/icons/16x16/service_template.png"));
        }

        return _cellRenderer.getTreeCellRendererComponent(
            tree, value, sel, expanded, leaf, row, hasFocus);
    }

    // Actions
    @Override
    public boolean[] getAvailableActions() {
        boolean[] actions = new boolean[ACTION_COUNT];

        if (((TreeNode) _parent).getAvailableActions()[PASTE]) {
            actions[PASTE] = true;
        } else {
            Object clipboard = getCoordinator().getClipboard();
            actions[PASTE] = clipboard != null && (clipboard instanceof Adapter.AdapterCopy);
        }

        actions[DELETE] = true;

        if (!_ephemeral) {
            actions[COPY] = true;
            actions[NEW_ADAPTER] = true;
        }

        return actions;
    }

    @Override
    public void copy() {
        getCoordinator().setClipboard(copyDescriptor(_templateDescriptor));
        getCoordinator().getActionsForMenu().get(PASTE).setEnabled(true);
    }

    @Override
    public JPopupMenu getPopupMenu() {
        ApplicationActions actions = getCoordinator().getActionsForPopup();
        if (_popup == null) {
            _popup = new JPopupMenu();
            _popup.add(actions.get(NEW_ADAPTER));
        }
        actions.setTarget(this);
        return _popup;
    }

    @Override
    public Editor getEditor() {
        if (_editor == null) {
            _editor =
                (ServiceTemplateEditor) getRoot().getEditor(ServiceTemplateEditor.class, this);
        }
        _editor.show(this);
        return _editor;
    }

    @Override
    protected Editor createEditor() {
        return new ServiceTemplateEditor();
    }

    ServiceTemplate(
            boolean brandNew, ServiceTemplates parent, String name, TemplateDescriptor descriptor)
        throws UpdateFailedException {
        super(parent, name);
        _editable = new Editable(brandNew);
        _ephemeral = false;
        rebuild(descriptor);
    }

    ServiceTemplate(ServiceTemplates parent, String name, TemplateDescriptor descriptor) {
        super(parent, name);
        _ephemeral = true;
        _editable = null;
        _templateDescriptor = descriptor;
    }

    @Override
    void write(XMLWriter writer) throws IOException {
        if (!_ephemeral) {
            List<String[]> attributes = new LinkedList<String[]>();
            attributes.add(createAttribute("id", _id));
            writer.writeStartTag("service-template", attributes);
            writeParameters(
                writer, _templateDescriptor.parameters, _templateDescriptor.parameterDefaults);

            ServiceDescriptor descriptor = (ServiceDescriptor) _templateDescriptor.descriptor;

            writer.writeStartTag("service", PlainService.createAttributes(descriptor));

            if (descriptor.description.length() > 0) {
                writer.writeElement("description", descriptor.description);
            }

            writePropertySet(writer, descriptor.propertySet, descriptor.adapters, descriptor.logs);
            writeLogs(writer, descriptor.logs, descriptor.propertySet.properties);
            _adapters.write(writer, descriptor.propertySet.properties);
            writer.writeEndTag("service");
            writer.writeEndTag("service-template");
        }
    }

    void rebuild(TemplateDescriptor descriptor) throws UpdateFailedException {
        _templateDescriptor = descriptor;

        _adapters.clear();

        if (!_ephemeral) {
            _adapters.init(_templateDescriptor.descriptor.adapters);
        }
    }

    void commit() {
        _editable.commit();
    }

    @Override
    public Object getDescriptor() {
        return _templateDescriptor;
    }

    @Override
    CommunicatorDescriptor getCommunicatorDescriptor() {
        return _templateDescriptor.descriptor;
    }

    @Override
    public boolean isEphemeral() {
        return _ephemeral;
    }

    @Override
    public void destroy() {
        ServiceTemplates serviceTemplates = (ServiceTemplates) _parent;

        if (_ephemeral) {
            serviceTemplates.removeChild(this);
        } else {
            serviceTemplates.removeDescriptor(_id);
            getRoot().removeServiceInstances(_id);
            serviceTemplates.removeChild(this);
            serviceTemplates.getEditable().removeElement(_id, _editable, ServiceTemplate.class);
            getRoot().updated();
        }
    }

    @Override
    List<? extends TemplateInstance> findInstances() {
        return getRoot().findServiceInstances(_id);
    }

    Editable getEditable() {
        return _editable;
    }

    @Override
    Editable getEnclosingEditable() {
        return _editable;
    }

    @Override
    public Object saveDescriptor() {
        // Shallow copy
        TemplateDescriptor clone = _templateDescriptor.clone();
        clone.descriptor = (ServiceDescriptor) _templateDescriptor.descriptor.clone();
        return clone;
    }

    @Override
    public void restoreDescriptor(Object savedDescriptor) {
        TemplateDescriptor clone = (TemplateDescriptor) savedDescriptor;
        // Keep the same object
        _templateDescriptor.parameters = clone.parameters;

        ServiceDescriptor sd = (ServiceDescriptor) _templateDescriptor.descriptor;
        ServiceDescriptor csd = (ServiceDescriptor) clone.descriptor;

        sd.propertySet = csd.propertySet;
        sd.description = csd.description;
        sd.name = csd.name;
        sd.entry = csd.entry;
    }

    private TemplateDescriptor _templateDescriptor;
    private final boolean _ephemeral;
    private Editable _editable;
    private ServiceTemplateEditor _editor;

    private static DefaultTreeCellRenderer _cellRenderer;
    private static JPopupMenu _popup;
}
