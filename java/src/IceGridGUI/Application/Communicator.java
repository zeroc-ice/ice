// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGridGUI.Application;

import java.util.Enumeration;

import javax.swing.JOptionPane;
import javax.swing.tree.DefaultTreeModel;

import IceGrid.*;
import IceGridGUI.*;

//
// The base class for Server, Service, ServerTemplate and ServiceTemplate
//
abstract class Communicator extends TreeNode implements DescriptorHolder
{
    public Enumeration children()
    {
        return new Enumeration()
            {
                public boolean hasMoreElements()
                {
                    if(_p.hasNext())
                    {
                        return true;
                    }
                   
                    while(++_index < _childListArray.length)
                    {
                        _p = _childListArray[_index].iterator();
                        if(_p.hasNext())
                        {
                            return true;
                        }
                    }
                    return false;
                }

                public Object nextElement()
                {
                    try
                    {
                        return _p.next();
                    }
                    catch(java.util.NoSuchElementException nse)
                    {
                        if(hasMoreElements())
                        {
                            return _p.next();
                        }
                        else
                        {
                            throw nse;
                        }
                    }
                }
                
                private int _index = 0;
                private java.util.Iterator _p = _childListArray[0].iterator();
            };
    }
    
    public boolean getAllowsChildren()
    {
        return true;
    }
    
    public javax.swing.tree.TreeNode getChildAt(int childIndex)
    {
        if(childIndex < 0)
        {
            throw new ArrayIndexOutOfBoundsException(childIndex);
        }
        int offset = 0;
        for(int i = 0; i < _childListArray.length; ++i)
        {
            if(childIndex < offset + _childListArray[i].size())
            {
                return _childListArray[i].get(childIndex - offset);
            }
            else
            {
                offset += _childListArray[i].size();
            }
        }
        throw new ArrayIndexOutOfBoundsException(childIndex);
    }
   
    public int getChildCount()
    {
        int result = 0;
        for(int i = 0; i < _childListArray.length; ++i)
        {
            result += _childListArray[i].size();
        }
        return result;
    }
    
    public int getIndex(javax.swing.tree.TreeNode node)
    {
        int offset = 0;
        for(int i = 0; i < _childListArray.length; ++i)
        {
            int index = _childListArray[i].indexOf(node);
            if(index == -1)
            {
                offset += _childListArray[i].size();
            }
            else
            {
                return offset + index;
            }
        }
        return -1;
    }

    public boolean isLeaf()
    {
        for(int i = 0; i < _childListArray.length; ++i)
        {
            if(!_childListArray[i].isEmpty())
            {
                return false;
            }
        }
        return true;
    }

    //
    // Actions
    //

    public void newAdapter()
    {
        _adapters.newChild();
    }
    public void newDbEnv()
    {
        _dbEnvs.newChild();
    }
    public void newService()
    {
        _services.newChild();
    }
    public void newServiceFromTemplate()
    {
        _services.newServiceFromTemplate();
    }

    public void paste()
    {
        Object descriptor =  getCoordinator().getClipboard();

        if(descriptor instanceof AdapterDescriptor)
        {
            AdapterDescriptor d = (AdapterDescriptor)descriptor;
            _adapters.newAdapter(Adapter.copyDescriptor(d));
        }
        else if(descriptor instanceof DbEnvDescriptor)
        {
            DbEnvDescriptor d = (DbEnvDescriptor)descriptor;
            _dbEnvs.newDbEnv(DbEnv.copyDescriptor(d));
        }
        else if(descriptor instanceof ServiceInstanceDescriptor && _services.initialized())
        {
            ServiceInstanceDescriptor d = (ServiceInstanceDescriptor)descriptor;
            _services.newService(ServiceInstance.copyDescriptor(d));
        }
        else
        {
            ((TreeNode)_parent).paste();
        }
    }
    
    abstract CommunicatorDescriptor getCommunicatorDescriptor();
    abstract Editable getEnclosingEditable();

    //
    // When 'this' is a template, returns all instances of this template.
    // Otherwise, return just 'this'
    //
    java.util.List findInstances()
    {
        java.util.List result = new java.util.LinkedList();
        result.add(this);
        return result;
    }
  
    
    TreeNode findChildLike(TreeNode other)
    {
        if(other instanceof Adapter)
        {
            return _adapters.findChildById(other.getId());
        }
        else if(other instanceof DbEnv)
        {
            return _dbEnvs.findChildById(other.getId());
        }
        else if(other instanceof Service)
        {
            return _dbEnvs.findChildById(other.getId());
        }
        else
        {
            return null;
        }
    }

    boolean isIceBox()
    {
        return false;
    }

    protected Communicator(TreeNode parent, String id)
    {
        super(parent, id);
    }

    abstract class ChildList
    {
        abstract TreeNode createChild(Object descriptor) throws UpdateFailedException;
        abstract void newChild();

        protected ChildList(boolean sorted)
        {
            _sorted = sorted;
        }

        void write(XMLWriter writer) throws java.io.IOException
        {
            java.util.Iterator p = _children.iterator();
            while(p.hasNext())
            {
                TreeNode child = (TreeNode)p.next();
                child.write(writer);
            }
        }

        //
        // Some list-like methods
        //
        java.util.Iterator iterator()
        {
            return _children.iterator();
        }

        TreeNode get(int index)
        {
            return (TreeNode)_children.get(index);
        }

        int indexOf(Object obj)
        {
            return _children.indexOf(obj);
        }
        
        int size()
        {
            return _children.size();
        }

        boolean isEmpty()
        {
            return _children.isEmpty();
        }
        
        //
        // Non-list methods
        //

        void init(java.util.List descriptors) throws UpdateFailedException
        {
            assert _descriptors == null;
            assert _children.isEmpty();

            _descriptors = descriptors;
            java.util.Iterator p = _descriptors.iterator();

            while(p.hasNext())
            {
                Object descriptor = p.next();
                TreeNode child = createChild(descriptor);
                addChild(child, false);
            }
        }

        boolean initialized()
        {
            return _descriptors != null;
        }

        void clear()
        {
            _descriptors = null;
            _children.clear();
        }

        TreeNode addNewChild(Object descriptor) throws UpdateFailedException
        {
            TreeNode child = createChild(descriptor);
            addChild(child, true);
          
            return child;
        }

        TreeNode findChildWithDescriptor(Object descriptor)
        {
            java.util.Iterator p = _children.iterator();
            while(p.hasNext())
            {
                TreeNode child = (TreeNode)p.next();
                if(child.getDescriptor() == descriptor)
                {
                    return child;
                }
            }
            return null;
        }

        TreeNode findChildById(String id)
        {
            java.util.Iterator p = _children.iterator();
            while(p.hasNext())
            {
                TreeNode child = (TreeNode)p.next();
                if(child.getId().equals(id))
                {
                    return child;
                }
            }
            return null;
        }

        void addChild(TreeNode child, boolean fireEvent) throws UpdateFailedException
        {
            if(_sorted)
            {
                DefaultTreeModel treeModel = fireEvent ?
                    getRoot().getTreeModel() : null;
                
                if(!insertSortedChild(child, _children, treeModel))
                {
                    throw new UpdateFailedException(Communicator.this, child.getId());
                }
            }
            else
            {
                //
                // Just add the child at the end of the list
                //
                _children.add(child);
                if(fireEvent)
                {
                    getRoot().getTreeModel().nodesWereInserted(Communicator.this, new int[]{getIndex(child)});
                }
            }
        }

        int removeChild(TreeNode child)
        {
            int index = getIndex(child);
            
            _children.remove(child);
            
            getRoot().getTreeModel().nodesWereRemoved(Communicator.this,
                                                    new int[]{index},
                                                    new Object[]{child});
            return index;
        } 

        void destroyChild(TreeNode child)
        {
            if(child.isEphemeral())
            {
                removeChild(child);
            }
            else
            {
                Object descriptor = child.getDescriptor();
                removeDescriptor(descriptor);
                getEnclosingEditable().markModified();
                getRoot().updated();
                removeChild(child);
            }
        }

        void addDescriptor(Object descriptor)
        {
            _descriptors.add(descriptor);
        }

        void removeDescriptor(Object descriptor)
        {
            //
            // A straight remove uses equals(), which is not the desired behavior
            //
            java.util.Iterator p = _descriptors.iterator();
            while(p.hasNext())
            {
                if(descriptor == p.next())
                {
                    p.remove();
                    break;
                }
            }
        }
        
        boolean canMove(TreeNode child, boolean up)
        {       
            int i = _children.indexOf(child);
            assert i != -1;
            return (up && i > 0) || (!up && i < _children.size() - 1);
        }

        void tryAdd(Object descriptor) throws UpdateFailedException
        {
            addDescriptor(descriptor);
            try
            {
                addNewChild(descriptor);
            }
            catch(UpdateFailedException e)
            {
                removeDescriptor(descriptor);
                throw e;
            }
            getEnclosingEditable().markModified();
        }

        void tryUpdate(TreeNode child) throws UpdateFailedException
        {
            //
            // Child is an Adapter or DbEnv
            //
            assert _sorted;

            Object descriptor = child.getDescriptor();
            removeChild(child);
            try
            {
                addNewChild(descriptor);
            }
            catch(UpdateFailedException e)
            {
                addChild(child, true);
                throw e;
            }
            getEnclosingEditable().markModified();
        }

        protected String makeNewChildId(String base)
        {
            String id = base;
            int i = 0;
            while(findChildById(id) != null)
            {
                id = base + "-" + (++i);
            }
            return id;
        }

        protected java.util.List _children = new java.util.LinkedList();
        protected java.util.List _descriptors;
        protected boolean _sorted;
    }

    class Adapters extends ChildList
    {
        Adapters()
        {
            super(true);
        }

        void write(XMLWriter writer, java.util.List props) throws java.io.IOException
        {
            java.util.Iterator p = _children.iterator();
            while(p.hasNext())
            {
                Adapter child = (Adapter)p.next();
                child.write(writer, props);
            }
        }
        
        void newChild()
        {
            AdapterDescriptor descriptor = new AdapterDescriptor(
                "NewAdapter",
                "",
                null,
                "",
                "",
                false,
                true,
                new java.util.LinkedList(),
                new java.util.LinkedList()
                );   
   
            newAdapter(descriptor);
        }

        TreeNode createChild(Object descriptor)
        {
            AdapterDescriptor ad = (AdapterDescriptor)descriptor;
            String name = Utils.substitute(ad.name, getResolver());
            return new Adapter(Communicator.this, name, ad, false);
        }
        
        private void newAdapter(AdapterDescriptor descriptor)
        {
            descriptor.name = makeNewChildId(descriptor.name);
            Adapter adapter = new Adapter(Communicator.this, descriptor.name, descriptor, true);
            try
            {
                addChild(adapter, true);
            }
            catch(UpdateFailedException e)
            {
                assert false;
            }
            getRoot().setSelectedNode(adapter);
        }

    }

    class DbEnvs extends ChildList
    {
        DbEnvs()
        {
            super(true);
        }
        
        void newChild()
        {
            DbEnvDescriptor descriptor = new DbEnvDescriptor(
                "NewDbEnv",
                "",
                "",
                new java.util.LinkedList());   
            
            newDbEnv(descriptor);
        }
        
        TreeNode createChild(Object descriptor)
        {
            DbEnvDescriptor dd = (DbEnvDescriptor)descriptor;
            String name =  Utils.substitute(dd.name, getResolver());
            return new DbEnv(Communicator.this, name, dd, false);
        }

        private void newDbEnv(DbEnvDescriptor descriptor)
        {
            descriptor.name = makeNewChildId(descriptor.name);
            DbEnv dbEnv = new DbEnv(Communicator.this, descriptor.name, descriptor, true);
            try
            {
                addChild(dbEnv, true);
            }
            catch(UpdateFailedException e)
            {
                assert false;
            }
            getRoot().setSelectedNode(dbEnv);
        }

    }
    
    class Services extends ChildList
    {
        Services()
        {
            super(false);
        }

        void newChild()
        {
            ServiceDescriptor serviceDescriptor = 
                new ServiceDescriptor(new java.util.LinkedList(),
                                      new PropertySetDescriptor(new String[0], new java.util.LinkedList()),
                                      new java.util.LinkedList(),
                                      new String[0],
                                      "",
                                      "NewService",
                                      "");

            ServiceInstanceDescriptor descriptor = 
                new ServiceInstanceDescriptor("",
                                              new java.util.HashMap(),
                                              serviceDescriptor,
                                              new PropertySetDescriptor(new String[0], new java.util.LinkedList()));
            
            newService(descriptor);
        }

        void newServiceFromTemplate()
        {
            ServiceInstanceDescriptor descriptor = 
                new ServiceInstanceDescriptor("",
                                              new java.util.HashMap(),
                                              null,
                                              new PropertySetDescriptor(new String[0], new java.util.LinkedList()));
            newService(descriptor);
        }
        
        void move(TreeNode child, boolean up)
        {
            int index = getIndex(child);
            int listIndex = _children.indexOf(child);

            Object descriptor = child.getDescriptor();

            getEnclosingEditable().markModified();
            getRoot().updated();
        
            _descriptors.remove(listIndex);
            if(up)
            {
                _descriptors.add(listIndex - 1, descriptor);
            }
            else
            {
                _descriptors.add(listIndex + 1, descriptor);
            }
            
            _children.remove(listIndex);
            getRoot().getTreeModel().nodesWereRemoved(Communicator.this, 
                                                      new int[]{index},
                                                      new Object[]{child});
            if(up)
            {
                _children.add(listIndex - 1, child);
                getRoot().getTreeModel().nodesWereInserted(Communicator.this, 
                                                           new int[]{index - 1});
                
            }
            else
            {
                _children.add(listIndex + 1, child);
                getRoot().getTreeModel().nodesWereInserted(Communicator.this, 
                                                           new int[]{index + 1});
                
            }
            getRoot().setSelectedNode(child);
            getCoordinator().showActions(child);
        }

        TreeNode createChild(Object o) throws UpdateFailedException
        {
            ServiceInstanceDescriptor descriptor = (ServiceInstanceDescriptor)o;

            if(descriptor.descriptor == null)
            {
                TemplateDescriptor templateDescriptor 
                    = getRoot().findServiceTemplateDescriptor(descriptor.template);
                
                assert templateDescriptor != null;
                
                String serviceName = null;
                String displayString = null;
                Utils.Resolver serviceResolver = null;

                if(Communicator.this instanceof PlainServer)
                {
                    serviceResolver = new Utils.Resolver(getResolver(), 
                                                         descriptor.parameterValues,
                                                         templateDescriptor.parameterDefaults);

                    ServiceDescriptor serviceDescriptor = (ServiceDescriptor)templateDescriptor.descriptor;
                    assert serviceDescriptor != null;

                    serviceName = serviceResolver.substitute(serviceDescriptor.name);
                    serviceResolver.put("service", serviceName);
                    displayString = serviceName + ": " + descriptor.template + "<>";
                }
                else
                {
                    //
                    // Note: service names don't have to be unique
                    //
                    serviceName = descriptor.template + "<>";
                }

                return new ServiceInstance(Communicator.this,
                                           serviceName,
                                           displayString,
                                           descriptor, 
                                           serviceResolver);
            }
            else
            {
                ServiceDescriptor serviceDescriptor = descriptor.descriptor;

                String serviceName = null;
                Utils.Resolver serviceResolver = null;

                if(Communicator.this instanceof PlainServer)
                {
                    serviceResolver = new Utils.Resolver(getResolver());
                    serviceName = serviceResolver.substitute(serviceDescriptor.name);
                    serviceResolver.put("service", serviceName);
                }
                else
                {
                    serviceName = serviceDescriptor.name;
                }
            
                return new PlainService(Communicator.this,
                                        serviceName,
                                        descriptor,
                                        serviceResolver);
            }
        }
        
        void tryUpdate(TreeNode child) throws UpdateFailedException
        {
            //
            // Rebuilding a Service is quite different since the creation of a service can
            // trigger an UpdateFailedException
            //
            Object descriptor = child.getDescriptor();
            int listIndex = _children.indexOf(child);
            assert listIndex != -1;
            
            TreeNode newChild = createChild(descriptor);
            _children.set(listIndex, newChild);
            getRoot().getTreeModel().nodeChanged(newChild);
            
            getEnclosingEditable().markModified();
        }

        private void newService(ServiceInstanceDescriptor descriptor)
        {
            if(descriptor.descriptor == null)
            {
                String name = makeNewChildId("NewService");
                
                //
                // Make sure descriptor.template points to a real template
                //
                ServiceTemplate t = getRoot().findServiceTemplate(descriptor.template);
                
                if(t == null)
                {
                    if(getRoot().getServiceTemplates().getChildCount() == 0)
                    {
                        JOptionPane.showMessageDialog(
                            getCoordinator().getMainFrame(),
                            "You need to create a service template before you can create a service from a template.",
                            "No Service Template",
                            JOptionPane.INFORMATION_MESSAGE);
                        return;
                    }
                    else
                    {
                        t = (ServiceTemplate)getRoot().getServiceTemplates().getChildAt(0);
                        descriptor.template = t.getId();
                        descriptor.parameterValues = new java.util.HashMap();
                    }
                }
                
                //
                // Validate/update parameterValues
                //
                TemplateDescriptor td = (TemplateDescriptor)t.getDescriptor();
                descriptor.parameterValues = Editor.makeParameterValues(descriptor.parameterValues,
                                                                        td.parameters);
                
                
                ServiceInstance service = new ServiceInstance(Communicator.this, name, descriptor);
                try
                {
                    addChild(service, true);
                }
                catch(UpdateFailedException e)
                {
                    assert false;
                }
                getRoot().setSelectedNode(service);
            }
            else
            {
                descriptor.descriptor.name = makeNewChildId(descriptor.descriptor.name);

                PlainService service = new PlainService(Communicator.this, descriptor);
                try
                {
                    addChild(service, true);
                }
                catch(UpdateFailedException e)
                {
                    assert false;
                }
                getRoot().setSelectedNode(service);
            }
        }
    }
    
    Adapters getAdapters()
    {
        return _adapters;
    }

    DbEnvs getDbEnvs()
    {
        return _dbEnvs;
    }

    Services getServices()
    {
        return _services;
    }

    java.util.List findServiceInstances(String template)
    { 
        java.util.List result = new java.util.LinkedList();
        java.util.Iterator p = _services.iterator();
        while(p.hasNext())
        {
            Object obj = p.next();
            if(obj instanceof ServiceInstance)
            {
                ServiceInstance service = (ServiceInstance)obj;
                ServiceInstanceDescriptor d = 
                    (ServiceInstanceDescriptor)service.getDescriptor();
                if(d.template.equals(template))
                {
                    result.add(service);
                }
            }
        }
        return result;
    }

    void removeServiceInstances(String template)
    {   
        java.util.Iterator p = _services.iterator();
        while(p.hasNext())
        {
            Object obj = p.next();
            if(obj instanceof ServiceInstance)
            {
                ServiceInstance service = (ServiceInstance)obj;
                ServiceInstanceDescriptor d = 
                    (ServiceInstanceDescriptor)service.getDescriptor();
                if(d.template.equals(template))
                {
                    p.remove();
                    _services.removeDescriptor(d);
                    getEnclosingEditable().markModified();
                }
            }
        }
    }

    void removeSortedChildren(String[] childIds, java.util.List fromChildren)
    {
        removeSortedChildren(childIds, fromChildren, getRoot().getTreeModel());
    }
    void childrenChanged(java.util.List children)
    {
        childrenChanged(children, getRoot().getTreeModel());
    }

    String getProperty(String key)
    {
        CommunicatorDescriptor descriptor = getCommunicatorDescriptor();
        java.util.Iterator p = descriptor.propertySet.properties.iterator();
        while(p.hasNext())
        {
            PropertyDescriptor pd = (PropertyDescriptor)p.next();
            if(pd.name.equals(key))
            {
                return pd.value;
            }
        }
        return null;
    }

    String lookupPropertyValue(String val)
    {
        CommunicatorDescriptor descriptor = getCommunicatorDescriptor();
        java.util.Iterator p = descriptor.propertySet.properties.iterator();
        while(p.hasNext())
        {
            PropertyDescriptor pd = (PropertyDescriptor)p.next();
            if(pd.value.equals(val))
            {
                return pd.name;
            }
        }
        return "";
    }


    void setProperty(String key, String newValue)
    {
        CommunicatorDescriptor descriptor = getCommunicatorDescriptor();
        removeProperty(key);
        ((java.util.LinkedList)descriptor.propertySet.properties).addFirst(new PropertyDescriptor(key, newValue));
    }

    void removeProperty(String key)
    {
        CommunicatorDescriptor descriptor = getCommunicatorDescriptor();
        java.util.Iterator p = descriptor.propertySet.properties.iterator();
        while(p.hasNext())
        {
            PropertyDescriptor pd = (PropertyDescriptor)p.next();
            if(pd.name.equals(key))
            {
                p.remove();
            }
        }
    }
    
    //
    // Children
    //
    protected Adapters _adapters = new Adapters();
    protected DbEnvs _dbEnvs = new DbEnvs();
    protected Services _services = new Services();
    protected ChildList[] _childListArray = new ChildList[]{_adapters, _dbEnvs, _services};
}
