// Copyright (c) ZeroC, Inc.

package com.zeroc.IceGridGUI.Application;

import com.zeroc.IceGrid.CommunicatorDescriptor;
import com.zeroc.IceGrid.IceBoxDescriptor;
import com.zeroc.IceGrid.ServerDescriptor;
import com.zeroc.IceGrid.ServiceInstanceDescriptor;
import com.zeroc.IceGrid.TemplateDescriptor;
import com.zeroc.IceGridGUI.ApplicationActions;
import com.zeroc.IceGridGUI.Utils;
import com.zeroc.IceGridGUI.XMLWriter;

import java.awt.Component;
import java.io.IOException;
import java.util.LinkedList;
import java.util.List;

import javax.swing.Icon;
import javax.swing.JPopupMenu;
import javax.swing.JTree;
import javax.swing.tree.DefaultTreeCellRenderer;

class ServerTemplate extends Communicator {
    public static TemplateDescriptor copyDescriptor(TemplateDescriptor templateDescriptor) {
        TemplateDescriptor copy = templateDescriptor.clone();

        copy.descriptor = PlainServer.copyDescriptor((ServerDescriptor) copy.descriptor);
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
            _plainIcon = Utils.getIcon("/icons/16x16/server_template.png");
            _iceboxIcon = Utils.getIcon("/icons/16x16/icebox_server_template.png");
        }

        if (_templateDescriptor.descriptor instanceof IceBoxDescriptor) {
            if (expanded) {
                _cellRenderer.setOpenIcon(_iceboxIcon);
            } else {
                _cellRenderer.setClosedIcon(_iceboxIcon);
            }
        } else {
            if (expanded) {
                _cellRenderer.setOpenIcon(_plainIcon);
            } else {
                _cellRenderer.setClosedIcon(_plainIcon);
            }
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
        } else {
            Object clipboard = getCoordinator().getClipboard();
            actions[PASTE] =
                clipboard != null
                    && ((isIceBox() && (clipboard instanceof ServiceInstanceDescriptor))
                    || (!isIceBox() && (clipboard instanceof Adapter.AdapterCopy)));
        }

        actions[DELETE] = true;

        if (!_ephemeral) {
            actions[NEW_ADAPTER] = !_services.initialized();
            actions[NEW_SERVICE] = _services.initialized();
            actions[NEW_SERVICE_FROM_TEMPLATE] = _services.initialized();
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
            _popup.add(actions.get(NEW_SERVICE));
            _popup.add(actions.get(NEW_SERVICE_FROM_TEMPLATE));
        }
        actions.setTarget(this);
        return _popup;
    }

    @Override
    public Editor getEditor() {
        if (_editor == null) {
            _editor = (ServerTemplateEditor) getRoot().getEditor(ServerTemplateEditor.class, this);
        }
        _editor.show(this);
        return _editor;
    }

    @Override
    protected Editor createEditor() {
        return new ServerTemplateEditor();
    }

    @Override
    public void destroy() {
        ServerTemplates serverTemplates = (ServerTemplates) _parent;

        if (_ephemeral) {
            serverTemplates.removeChild(this);
        } else {
            serverTemplates.removeDescriptor(_id);
            getRoot().removeServerInstances(_id);
            serverTemplates.removeChild(this);
            serverTemplates.getEditable().removeElement(_id, _editable, ServerTemplate.class);
            getRoot().updated();
        }
    }

    @Override
    public boolean isEphemeral() {
        return _ephemeral;
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
    public Object saveDescriptor() {
        // Shallow copy
        TemplateDescriptor clone = _templateDescriptor.clone();
        clone.descriptor = _templateDescriptor.descriptor.clone();
        return clone;
    }

    @Override
    public void restoreDescriptor(Object savedDescriptor) {
        TemplateDescriptor clone = (TemplateDescriptor) savedDescriptor;
        // Keep the same object
        _templateDescriptor.parameters = clone.parameters;

        PlainServer.shallowRestore(
            (ServerDescriptor) clone.descriptor,
            (ServerDescriptor) _templateDescriptor.descriptor);
    }

    // Application is needed to lookup service templates
    ServerTemplate(
            boolean brandNew, ServerTemplates parent, String name, TemplateDescriptor descriptor)
        throws UpdateFailedException {
        super(parent, name);
        _editable = new Editable(brandNew);
        _ephemeral = false;
        rebuild(descriptor);
    }

    ServerTemplate(ServerTemplates parent, String name, TemplateDescriptor descriptor) {
        super(parent, name);
        _ephemeral = true;
        try {
            rebuild(descriptor);
        } catch (UpdateFailedException e) {
            assert false;
        }
    }

    @Override
    void write(XMLWriter writer) throws IOException {
        if (!_ephemeral) {
            List<String[]> attributes = new LinkedList<String[]>();
            attributes.add(createAttribute("id", _id));
            writer.writeStartTag("server-template", attributes);
            writeParameters(
                writer, _templateDescriptor.parameters, _templateDescriptor.parameterDefaults);

            if (_templateDescriptor.descriptor instanceof IceBoxDescriptor) {
                IceBoxDescriptor descriptor = (IceBoxDescriptor) _templateDescriptor.descriptor;

                writer.writeStartTag("icebox", PlainServer.createAttributes(descriptor));

                if (descriptor.description.length() > 0) {
                    writer.writeElement("description", descriptor.description);
                }
                PlainServer.writeOptions(writer, descriptor.options);
                PlainServer.writeEnvs(writer, descriptor.envs);

                writePropertySet(
                    writer,
                    "",
                    "",
                    descriptor.propertySet,
                    descriptor.adapters,
                    descriptor.logs);
                writeLogs(writer, descriptor.logs, descriptor.propertySet.properties);

                _adapters.write(writer, descriptor.propertySet.properties);
                _services.write(writer);
                writer.writeEndTag("icebox");
            } else {
                ServerDescriptor descriptor = (ServerDescriptor) _templateDescriptor.descriptor;

                writer.writeStartTag("server", PlainServer.createAttributes(descriptor));

                if (descriptor.description.length() > 0) {
                    writer.writeElement("description", descriptor.description);
                }
                PlainServer.writeOptions(writer, descriptor.options);
                PlainServer.writeEnvs(writer, descriptor.envs);

                writePropertySet(
                    writer, descriptor.propertySet, descriptor.adapters, descriptor.logs);
                writeLogs(writer, descriptor.logs, descriptor.propertySet.properties);

                _adapters.write(writer, descriptor.propertySet.properties);
                writer.writeEndTag("server");
            }
            writer.writeEndTag("server-template");
        }
    }

    @Override
    boolean isIceBox() {
        return _templateDescriptor.descriptor instanceof IceBoxDescriptor;
    }

    void rebuild(TemplateDescriptor descriptor) throws UpdateFailedException {
        _templateDescriptor = descriptor;

        _adapters.clear();
        _services.clear();

        if (!_ephemeral) {
            _adapters.init(_templateDescriptor.descriptor.adapters);

            if (isIceBox()) {
                IceBoxDescriptor iceBoxDescriptor =
                    (IceBoxDescriptor) _templateDescriptor.descriptor;

                _services.init(iceBoxDescriptor.services);
            }
        }
    }

    void rebuild() throws UpdateFailedException {
        rebuild(_templateDescriptor);
    }

    void commit() {
        _editable.commit();
    }

    Editable getEditable() {
        return _editable;
    }

    @Override
    Editable getEnclosingEditable() {
        return _editable;
    }

    @Override
    List<? extends TemplateInstance> findInstances() {
        return getRoot().findServerInstances(_id);
    }

    private TemplateDescriptor _templateDescriptor;
    private final boolean _ephemeral;
    private Editable _editable;
    private ServerTemplateEditor _editor;

    private static DefaultTreeCellRenderer _cellRenderer;
    private static Icon _plainIcon;
    private static Icon _iceboxIcon;

    private static JPopupMenu _popup;
}
