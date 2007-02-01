// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGridGUI.Application;

import javax.swing.JMenuItem;
import javax.swing.JPopupMenu;

import IceGrid.*;
import IceGridGUI.*;

class ServerTemplates extends Templates
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
                     ServerTemplate.copyDescriptor(
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
        actions[NEW_TEMPLATE_SERVER] = true;
        actions[NEW_TEMPLATE_SERVER_ICEBOX] = true;
        
        Object clipboard = getCoordinator().getClipboard();
        if(clipboard != null && clipboard instanceof TemplateDescriptor)
        {
            TemplateDescriptor d = (TemplateDescriptor)clipboard;
            actions[PASTE] = d.descriptor instanceof ServerDescriptor;
        }
        return actions;
    }
    public JPopupMenu getPopupMenu()
    {
        ApplicationActions actions = getCoordinator().getActionsForPopup();
        if(_popup == null)
        {
            _popup = new JPopupMenu();
            _popup.add(actions.get(NEW_TEMPLATE_SERVER));
            _popup.add(actions.get(NEW_TEMPLATE_SERVER_ICEBOX));
        }
        actions.setTarget(this);
        return _popup;
    }

    public void newTemplateServer()
    {
        newServerTemplate(new TemplateDescriptor(
                              PlainServer.newServerDescriptor(), 
                              new java.util.LinkedList(),
                              new java.util.TreeMap()));
    }
    public void newTemplateServerIceBox()
    {
        newServerTemplate(new TemplateDescriptor(
                              PlainServer.newIceBoxDescriptor(), 
                              new java.util.LinkedList(),
                              new java.util.TreeMap()));
    }
    public void paste()
    {
        Object descriptor =  getCoordinator().getClipboard();
        TemplateDescriptor td = (TemplateDescriptor)descriptor;
        newServerTemplate(td);
    }


    ServerTemplates(Root parent, java.util.Map descriptors)
        throws UpdateFailedException
    {
        super(parent, "Server templates");

        _descriptors = descriptors;

        java.util.Iterator p = _descriptors.entrySet().iterator();
        
        while(p.hasNext())
        {
            java.util.Map.Entry entry = (java.util.Map.Entry)p.next();
            insertChild(new ServerTemplate(false, this,
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

    java.util.Map getUpdates()
    {
        java.util.Map updates = new java.util.HashMap();
        java.util.Iterator p = _children.iterator();
        while(p.hasNext())
        {
            ServerTemplate t = (ServerTemplate)p.next();
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
            ServerTemplate st = (ServerTemplate)p.next();
            st.commit();
        }
    }

    java.util.List findServiceInstances(String template)
    {
        java.util.List result = new java.util.LinkedList();
        java.util.Iterator p = _children.iterator();
        while(p.hasNext())
        {
            ServerTemplate t = (ServerTemplate)p.next();
            result.addAll(t.findServiceInstances(template));
        }
        return result;
    }

    void removeServiceInstances(String template)
    {
        java.util.Iterator p = _children.iterator();
        while(p.hasNext())
        {
            ServerTemplate t = (ServerTemplate)p.next();
            t.removeServiceInstances(template);
        }
    }
    
    void newServerTemplate(TemplateDescriptor descriptor)
    {
        String id;
        if(descriptor.descriptor instanceof IceBoxDescriptor)
        {
            id = makeNewChildId("NewIceBoxTemplate");
        }
        else
        {
            id = makeNewChildId("NewServerTemplate");
        }
        
        ServerTemplate t = new ServerTemplate(this, id, descriptor);
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
        insertChild(new ServerTemplate(true, this, newId, descriptor), 
                    true);
        _descriptors.put(newId, descriptor);
    }

    void update(java.util.Map updates, String[] removeTemplates, 
                java.util.Set serviceTemplates)
        throws UpdateFailedException
    {
        //
        // Note: _descriptors is updated by Root
        //
        
        Root root = getRoot();

        //
        // One big set of removes
        //
        removeChildren(removeTemplates);

        //
        // One big set of updates, followed by inserts
        //
        java.util.Vector newChildren = new java.util.Vector();
        java.util.Vector updatedChildren = new java.util.Vector();
        
        java.util.Iterator p = updates.entrySet().iterator();
        while(p.hasNext())
        {
            java.util.Map.Entry entry = (java.util.Map.Entry)p.next();
            String name = (String)entry.getKey();
            TemplateDescriptor templateDescriptor 
                = (TemplateDescriptor)entry.getValue();
            ServerTemplate child = (ServerTemplate)findChild(name);
            if(child == null)
            {
                newChildren.add(new ServerTemplate(false, this, name, templateDescriptor));
            }
            else
            {
                child.rebuild(templateDescriptor);
                updatedChildren.add(child);
            }
        }
        
        //
        // Rebuild template affected by service template updates
        //
        p = serviceTemplates.iterator();
        while(p.hasNext())
        {
            java.util.List serviceInstances = 
                findServiceInstances((String)p.next());
            java.util.Iterator q = serviceInstances.iterator();
            while(q.hasNext())
            {
                ServiceInstance service = (ServiceInstance)q.next();
                ServerTemplate serverTemplate = 
                    (ServerTemplate)service.getParent();
                
                if(!updatedChildren.contains(serverTemplate) &&
                   !newChildren.contains(serverTemplate))
                {
                    serverTemplate.rebuild();
                    updatedChildren.add(serverTemplate);
                }
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
