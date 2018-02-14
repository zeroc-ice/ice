// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceGridGUI.Application;

import javax.swing.JPopupMenu;

import IceGrid.*;
import IceGridGUI.*;

class ServiceTemplates extends Templates
{
    static public java.util.Map<String, TemplateDescriptor>
    copyDescriptors(java.util.Map<String, TemplateDescriptor> descriptors)
    {
        java.util.Map<String, TemplateDescriptor> copy = new java.util.HashMap<String, TemplateDescriptor>();
        for(java.util.Map.Entry<String, TemplateDescriptor> p : descriptors.entrySet())
        {
            copy.put(p.getKey(), ServiceTemplate.copyDescriptor(p.getValue()));
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
        actions[NEW_TEMPLATE_SERVICE] = true;

        Object clipboard = getCoordinator().getClipboard();
        if(clipboard != null && clipboard instanceof TemplateDescriptor)
        {
            TemplateDescriptor d = (TemplateDescriptor)clipboard;
            actions[PASTE] = d.descriptor instanceof ServiceDescriptor;
        }
        return actions;
    }

    @Override
    public JPopupMenu getPopupMenu()
    {
        ApplicationActions actions = getCoordinator().getActionsForPopup();
        if(_popup == null)
        {
            _popup = new JPopupMenu();
            _popup.add(actions.get(NEW_TEMPLATE_SERVICE));
        }
        actions.setTarget(this);
        return _popup;
    }

    @Override
    public void newTemplateService()
    {
        ServiceDescriptor sd = new ServiceDescriptor(
            new java.util.LinkedList<AdapterDescriptor>(),
            new PropertySetDescriptor(new String[0], new java.util.LinkedList<PropertyDescriptor>()),
            new java.util.LinkedList<DbEnvDescriptor>(),
            new String[0],
            "",
            "",
            "");

        newServiceTemplate(new TemplateDescriptor(sd, new java.util.LinkedList<String>(),
                                                  new java.util.TreeMap<String, String>()));
    }

    @Override
    public void paste()
    {
        Object descriptor = getCoordinator().getClipboard();
        TemplateDescriptor td = (TemplateDescriptor)descriptor;
        newServiceTemplate(td);
    }

    ServiceTemplates(Root parent, java.util.Map<String, TemplateDescriptor> descriptors)
        throws UpdateFailedException
    {
        super(parent, "Service templates");

        _descriptors = descriptors;

        for(java.util.Map.Entry<String, TemplateDescriptor> p : _descriptors.entrySet())
        {
            insertChild(new ServiceTemplate(false, this, p.getKey(), p.getValue()), false);
        }
    }

    //
    // Variable resolution does not make sense for templates / template children
    //
    @Override
    Utils.Resolver getResolver()
    {
        return null;
    }

    void newServiceTemplate(TemplateDescriptor descriptor)
    {
        String id = makeNewChildId("NewServiceTemplate");

        ServiceTemplate t = new ServiceTemplate(this, id, descriptor);
        try
        {
            insertChild(t, true);
        }
        catch(UpdateFailedException e)
        {
            assert false;
        }
        getRoot().setSelectedNode(t);
    }

    @Override
    void tryAdd(String newId, TemplateDescriptor descriptor)
        throws UpdateFailedException
    {
        insertChild(new ServiceTemplate(true, this, newId, descriptor), true);
        _descriptors.put(newId, descriptor);
    }

    java.util.Map<String, TemplateDescriptor> getUpdates()
    {
        java.util.Map<String, TemplateDescriptor> updates = new java.util.HashMap<String, TemplateDescriptor>();
        for(TreeNodeBase p : _children)
        {
            ServiceTemplate t = (ServiceTemplate)p;
            if(t.getEditable().isNew() || t.getEditable().isModified())
            {
                updates.put(t.getId(), (TemplateDescriptor)t.getDescriptor());
            }
        }
        return updates;
    }

    void commit()
    {
        _editable.commit();
        for(TreeNodeBase p : _children)
        {
            ServiceTemplate st = (ServiceTemplate)p;
            st.commit();
        }
    }

    void update(java.util.Map<String, TemplateDescriptor> descriptors, String[] removeTemplates)
        throws UpdateFailedException
    {
        //
        // Note: _descriptors is updated by Application
        //

        //
        // One big set of removes
        //
        removeChildren(removeTemplates);

        //
        // One big set of updates, followed by inserts
        //
        java.util.List<TreeNodeBase> newChildren = new java.util.ArrayList<TreeNodeBase>();
        java.util.List<TreeNodeBase> updatedChildren = new java.util.LinkedList<TreeNodeBase>();

        for(java.util.Map.Entry<String, TemplateDescriptor> p : descriptors.entrySet())
        {
            String name = p.getKey();
            TemplateDescriptor templateDescriptor = p.getValue();
            ServiceTemplate child = (ServiceTemplate)findChild(name);
            if(child == null)
            {
                newChildren.add(new ServiceTemplate(false, this, name, templateDescriptor));
            }
            else
            {
                child.rebuild(templateDescriptor);
                updatedChildren.add(child);
            }
        }

        childrenChanged(updatedChildren);
        insertChildren(newChildren, true);
    }

    void removeDescriptor(String id)
    {
        _descriptors.remove(id);
    }

    @Override
    Object getDescriptor()
    {
        return _descriptors;
    }

    private java.util.Map<String, TemplateDescriptor> _descriptors;

    static private JPopupMenu _popup;
}
