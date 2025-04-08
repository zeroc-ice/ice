// Copyright (c) ZeroC, Inc.

package com.zeroc.IceGridGUI.Application;

import com.zeroc.IceGrid.*;
import com.zeroc.IceGridGUI.ApplicationActions;
import com.zeroc.IceGridGUI.TreeNodeBase;
import com.zeroc.IceGridGUI.Utils;

import javax.swing.JPopupMenu;

import java.util.*;

class ServiceTemplates extends Templates {
    public static Map<String, TemplateDescriptor> copyDescriptors(
            Map<String, TemplateDescriptor> descriptors) {
        Map<String, TemplateDescriptor> copy = new HashMap<>();
        for (Map.Entry<String, TemplateDescriptor> p : descriptors.entrySet()) {
            copy.put(p.getKey(), ServiceTemplate.copyDescriptor(p.getValue()));
        }
        return copy;
    }

    // Actions
    @Override
    public boolean[] getAvailableActions() {
        boolean[] actions = new boolean[ACTION_COUNT];
        actions[NEW_TEMPLATE_SERVICE] = true;

        Object clipboard = getCoordinator().getClipboard();
        if (clipboard != null && clipboard instanceof TemplateDescriptor) {
            TemplateDescriptor d = (TemplateDescriptor) clipboard;
            actions[PASTE] = d.descriptor instanceof ServiceDescriptor;
        }
        return actions;
    }

    @Override
    public JPopupMenu getPopupMenu() {
        ApplicationActions actions = getCoordinator().getActionsForPopup();
        if (_popup == null) {
            _popup = new JPopupMenu();
            _popup.add(actions.get(NEW_TEMPLATE_SERVICE));
        }
        actions.setTarget(this);
        return _popup;
    }

    @Override
    public void newTemplateService() {
        ServiceDescriptor sd =
            new ServiceDescriptor(
                new LinkedList<AdapterDescriptor>(),
                new PropertySetDescriptor(
                    new String[0], new LinkedList<PropertyDescriptor>()),
                new String[0],
                "",
                "",
                "");

        newServiceTemplate(
            new TemplateDescriptor(
                sd,
                new LinkedList<String>(),
                new TreeMap<String, String>()));
    }

    @Override
    public void paste() {
        Object descriptor = getCoordinator().getClipboard();
        TemplateDescriptor td = (TemplateDescriptor) descriptor;
        newServiceTemplate(td);
    }

    ServiceTemplates(Root parent, Map<String, TemplateDescriptor> descriptors)
        throws UpdateFailedException {
        super(parent, "Service templates");

        _descriptors = descriptors;

        for (Map.Entry<String, TemplateDescriptor> p : _descriptors.entrySet()) {
            insertChild(new ServiceTemplate(false, this, p.getKey(), p.getValue()), false);
        }
    }

    // Variable resolution does not make sense for templates / template children
    @Override
    Utils.Resolver getResolver() {
        return null;
    }

    void newServiceTemplate(TemplateDescriptor descriptor) {
        String id = makeNewChildId("NewServiceTemplate");

        ServiceTemplate t = new ServiceTemplate(this, id, descriptor);
        try {
            insertChild(t, true);
        } catch (UpdateFailedException e) {
            assert false;
        }
        getRoot().setSelectedNode(t);
    }

    @Override
    void tryAdd(String newId, TemplateDescriptor descriptor) throws UpdateFailedException {
        insertChild(new ServiceTemplate(true, this, newId, descriptor), true);
        _descriptors.put(newId, descriptor);
    }

    Map<String, TemplateDescriptor> getUpdates() {
        Map<String, TemplateDescriptor> updates = new HashMap<>();
        for (TreeNodeBase p : _children) {
            ServiceTemplate t = (ServiceTemplate) p;
            if (t.getEditable().isNew() || t.getEditable().isModified()) {
                updates.put(t.getId(), (TemplateDescriptor) t.getDescriptor());
            }
        }
        return updates;
    }

    void commit() {
        _editable.commit();
        for (TreeNodeBase p : _children) {
            ServiceTemplate st = (ServiceTemplate) p;
            st.commit();
        }
    }

    void update(Map<String, TemplateDescriptor> descriptors, String[] removeTemplates)
        throws UpdateFailedException {
        //
        // Note: _descriptors is updated by Application
        //

        // One big set of removes
        removeChildren(removeTemplates);

        // One big set of updates, followed by inserts
        List<TreeNodeBase> newChildren = new ArrayList<>();
        List<TreeNodeBase> updatedChildren = new LinkedList<>();

        for (Map.Entry<String, TemplateDescriptor> p : descriptors.entrySet()) {
            String name = p.getKey();
            TemplateDescriptor templateDescriptor = p.getValue();
            ServiceTemplate child = (ServiceTemplate) findChild(name);
            if (child == null) {
                newChildren.add(new ServiceTemplate(false, this, name, templateDescriptor));
            } else {
                child.rebuild(templateDescriptor);
                updatedChildren.add(child);
            }
        }

        childrenChanged(updatedChildren);
        insertChildren(newChildren, true);
    }

    void removeDescriptor(String id) {
        _descriptors.remove(id);
    }

    @Override
    Object getDescriptor() {
        return _descriptors;
    }

    private final Map<String, TemplateDescriptor> _descriptors;

    private static JPopupMenu _popup;
}
