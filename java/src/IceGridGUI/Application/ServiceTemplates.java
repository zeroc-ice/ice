// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGridGUI.Application;

import java.awt.event.ActionEvent;

import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.JMenuItem;
import javax.swing.JPopupMenu;

import IceGrid.*;
import IceGridGUI.*;

class ServiceTemplates extends Templates
{
    static public java.util.Map
    copyDescriptors(java.util.Map descriptors)
    {
        java.util.Map copy = new java.util.HashMap();
        java.util.Iterator p = descriptors.entrySet().iterator();
        while(p.hasNext())
        {
            java.util.Map.Entry entry = (java.util.Map.Entry)p.next();
            
            copy.put(entry.getKey(), 
                     ServiceTemplate.copyDescriptor(
                         (TemplateDescriptor)entry.getValue()));
        }
        return copy;
    }
    

    //
    // Actions
    //
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
    public void newTemplateService()
    {
        ServiceDescriptor sd = new ServiceDescriptor(
            new java.util.LinkedList(),
            new PropertySetDescriptor(new String[0], new java.util.LinkedList()),
            new java.util.LinkedList(),
            new String[0],
            "",
            "",
            "");
            
        newServiceTemplate(new TemplateDescriptor(sd, new java.util.LinkedList(), new java.util.TreeMap()));

    }
    public void paste()
    {
        Object descriptor = getCoordinator().getClipboard();
        TemplateDescriptor td = (TemplateDescriptor)descriptor;
        newServiceTemplate(td);
    }


    ServiceTemplates(Root parent, java.util.Map descriptors)
        throws UpdateFailedException
    {
        super(parent, "Service templates");

        _descriptors = descriptors;

        java.util.Iterator p = _descriptors.entrySet().iterator();
        
        while(p.hasNext())
        {
            java.util.Map.Entry entry = (java.util.Map.Entry)p.next();
            insertChild(new ServiceTemplate(false, this,
                                            (String)entry.getKey(),
                                            (TemplateDescriptor)entry.getValue()),
                        false);
        }
    }

    //
    // Variable resolution does not make sense for templates / template children
    //
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

    void tryAdd(String newId, TemplateDescriptor descriptor)
        throws UpdateFailedException
    {
        insertChild(new ServiceTemplate(true, this, newId, descriptor),
                    true);
        _descriptors.put(newId, descriptor);
    }

    java.util.Map getUpdates()
    {
        java.util.Map updates = new java.util.HashMap();
        java.util.Iterator p = _children.iterator();
        while(p.hasNext())
        {
            ServiceTemplate t = (ServiceTemplate)p.next();
            if(t.getEditable().isNew() || t.getEditable().isModified())
            {
                updates.put(t.getId(), t.getDescriptor());
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
            ServiceTemplate st = (ServiceTemplate)p.next();
            st.commit();
        }
    }
   

    void update(java.util.Map descriptors, String[] removeTemplates)
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
        java.util.Vector newChildren = new java.util.Vector();
        java.util.Vector updatedChildren = new java.util.Vector();
        
        java.util.Iterator p = descriptors.entrySet().iterator();
        while(p.hasNext())
        {
            java.util.Map.Entry entry = (java.util.Map.Entry)p.next();
            String name = (String)entry.getKey();
            TemplateDescriptor templateDescriptor 
                = (TemplateDescriptor)entry.getValue();
            ServiceTemplate child = (ServiceTemplate)findChild(name);
            if(child == null)
            {
                newChildren.add(new ServiceTemplate(false, this, name, 
                                                    templateDescriptor));
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

    Object getDescriptor()
    {
        return _descriptors;
    }

    private java.util.Map _descriptors;

    static private JPopupMenu _popup;
}
