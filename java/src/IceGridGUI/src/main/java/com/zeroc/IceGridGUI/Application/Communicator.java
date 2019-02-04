//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.IceGridGUI.Application;

import java.util.Enumeration;

import javax.swing.JOptionPane;
import javax.swing.tree.DefaultTreeModel;

import com.zeroc.IceGrid.*;
import com.zeroc.IceGridGUI.*;

//
// The base class for Server, Service, ServerTemplate and ServiceTemplate
//
abstract class Communicator extends TreeNode implements DescriptorHolder
{
    @Override
    @SuppressWarnings("unchecked")
    public Enumeration<javax.swing.tree.TreeNode> children()
    {
        return new Enumeration<javax.swing.tree.TreeNode>()
            {
                @Override
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

                @Override
                public javax.swing.tree.TreeNode nextElement()
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
                private java.util.Iterator<javax.swing.tree.TreeNode> _p = _childListArray[0].iterator();
            };
    }

    @Override
    public boolean getAllowsChildren()
    {
        return true;
    }

    @Override
    public javax.swing.tree.TreeNode getChildAt(int childIndex)
    {
        if(childIndex < 0)
        {
            throw new ArrayIndexOutOfBoundsException(childIndex);
        }
        int offset = 0;
        for(ChildList childList : _childListArray)
        {
            if(childIndex < offset + childList.size())
            {
                return childList.get(childIndex - offset);
            }
            else
            {
                offset += childList.size();
            }
        }
        throw new ArrayIndexOutOfBoundsException(childIndex);
    }

    @Override
    public int getChildCount()
    {
        int result = 0;
        for(ChildList childList : _childListArray)
        {
            result += childList.size();
        }
        return result;
    }

    @Override
    public int getIndex(javax.swing.tree.TreeNode node)
    {
        int offset = 0;
        for(ChildList childList : _childListArray)
        {
            int index = childList.indexOf(node);
            if(index == -1)
            {
                offset += childList.size();
            }
            else
            {
                return offset + index;
            }
        }
        return -1;
    }

    @Override
    public boolean isLeaf()
    {
        for(ChildList childList : _childListArray)
        {
            if(!childList.isEmpty())
            {
                return false;
            }
        }
        return true;
    }

    //
    // Actions
    //

    @Override
    public void newAdapter()
    {
        _adapters.newChild();
    }

    @Override
    public void newDbEnv()
    {
        _dbEnvs.newChild();
    }

    @Override
    public void newService()
    {
        _services.newChild();
    }

    @Override
    public void newServiceFromTemplate()
    {
        _services.newServiceFromTemplate();
    }

    @Override
    public void paste()
    {
        Object descriptor =  getCoordinator().getClipboard();

        if(descriptor instanceof Adapter.AdapterCopy)
        {
            Adapter.AdapterCopy copy  = (Adapter.AdapterCopy)descriptor;
            _adapters.newAdapter(Adapter.copyDescriptor(copy.descriptor),
                                 new java.util.HashMap<>(copy.parentProperties));
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
    java.util.List<? extends TemplateInstance> findInstances()
    {
        java.util.List<TemplateInstance> result = new java.util.LinkedList<>();
        result.add((TemplateInstance)this);
        return result;
    }

    @Override
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

    abstract class ChildList<T>
    {
        abstract TreeNode createChild(T descriptor) throws UpdateFailedException;
        abstract void newChild();

        protected ChildList(boolean sorted)
        {
            _sorted = sorted;
        }

        void write(XMLWriter writer)
            throws java.io.IOException
        {
            for(TreeNode p : _children)
            {
                p.write(writer);
            }
        }

        //
        // Some list-like methods
        //
        java.util.Iterator<TreeNode> iterator()
        {
            return _children.iterator();
        }

        TreeNode get(int index)
        {
            return _children.get(index);
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

        void init(java.util.List<T> descriptors)
            throws UpdateFailedException
        {
            assert _descriptors == null;
            assert _children.isEmpty();

            _descriptors = descriptors;

            for(T descriptor : _descriptors)
            {
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

        TreeNode addNewChild(T descriptor)
            throws UpdateFailedException
        {
            TreeNode child = createChild(descriptor);
            addChild(child, true);

            return child;
        }

        TreeNode findChildWithDescriptor(T descriptor)
        {
            for(TreeNode p : _children)
            {
                if(p.getDescriptor() == descriptor)
                {
                    return p;
                }
            }
            return null;
        }

        TreeNode findChildById(String id)
        {
            for(TreeNode p : _children)
            {
                if(p.getId().equals(id))
                {
                    return p;
                }
            }
            return null;
        }

        void addChild(TreeNode child, boolean fireEvent)
            throws UpdateFailedException
        {
            if(_sorted)
            {
                DefaultTreeModel treeModel = fireEvent ?  getRoot().getTreeModel() : null;

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

            getRoot().getTreeModel().nodesWereRemoved(Communicator.this, new int[]{index}, new Object[]{child});
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
                @SuppressWarnings("unchecked")
                T descriptor = (T)child.getDescriptor();
                removeDescriptor(descriptor);
                getEnclosingEditable().markModified();
                getRoot().updated();
                removeChild(child);
            }
        }

        void addDescriptor(T descriptor)
        {
            _descriptors.add(descriptor);
        }

        void removeDescriptor(T descriptor)
        {
            //
            // A straight remove uses equals(), which is not the desired behavior
            //
            java.util.Iterator<T> p = _descriptors.iterator();
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

        void tryAdd(T descriptor)
            throws UpdateFailedException
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

        void tryUpdate(TreeNode child)
            throws UpdateFailedException
        {
            //
            // Child is an Adapter or DbEnv
            //
            assert _sorted;

            @SuppressWarnings("unchecked")
            T descriptor = (T)child.getDescriptor();
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

        protected java.util.List<TreeNode> _children = new java.util.LinkedList<>();
        protected java.util.List<T> _descriptors;
        protected boolean _sorted;
    }

    class Adapters extends ChildList<AdapterDescriptor>
    {
        Adapters()
        {
            super(true);
        }

        void write(XMLWriter writer, java.util.List<PropertyDescriptor> props)
            throws java.io.IOException
        {
            for(TreeNode p : _children)
            {
                ((Adapter)p).write(writer, props);
            }
        }

        @Override
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
                new java.util.LinkedList<ObjectDescriptor>(),
                new java.util.LinkedList<ObjectDescriptor>()
                );

            newAdapter(descriptor, null);
        }

        /*
        TreeNode createChild(Object descriptor)
        {
            AdapterDescriptor ad = (AdapterDescriptor)descriptor;
            String name = Utils.substitute(ad.name, getResolver());
            return new Adapter(Communicator.this, name, ad, null, false);
        }
        */
        @Override
        TreeNode createChild(AdapterDescriptor descriptor)
        {
            String name = Utils.substitute(descriptor.name, getResolver());
            return new Adapter(Communicator.this, name, descriptor, null, false);
        }

        private void newAdapter(AdapterDescriptor descriptor, java.util.Map<String, String> parentProperties)
        {
            String newName = makeNewChildId(descriptor.name);

            if(!newName.equals(descriptor.name) && parentProperties != null)
            {
                //
                // Adjust Endpoints and PublishedEnpoints
                //

                String key = descriptor.name + ".Endpoints";
                String val = parentProperties.remove(key);
                if(val != null)
                {
                    parentProperties.put(newName + ".Endpoints", val);
                }

                key = descriptor.name + ".PublishedEndpoints";
                val = parentProperties.remove(key);
                if(val != null)
                {
                    parentProperties.put(newName + ".PublishedEndpoints", val);
                }

                key = descriptor.name + ".ProxyOptions";
                val = parentProperties.remove(key);
                if(val != null)
                {
                    parentProperties.put(newName + ".ProxyOptions", val);
                }
            }

            descriptor.name = newName;

            Adapter adapter = new Adapter(Communicator.this, descriptor.name, descriptor, parentProperties, true);
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

    class DbEnvs extends ChildList<DbEnvDescriptor>
    {
        DbEnvs()
        {
            super(true);
        }

        @Override
        void newChild()
        {
            DbEnvDescriptor descriptor = new DbEnvDescriptor(
                "NewDbEnv",
                "",
                "",
                new java.util.LinkedList<PropertyDescriptor>());

            newDbEnv(descriptor);
        }

        //TreeNode createChild(Object descriptor)
        @Override
        TreeNode createChild(DbEnvDescriptor descriptor)
        {
            //DbEnvDescriptor dd = (DbEnvDescriptor)descriptor;
            //String name =  Utils.substitute(dd.name, getResolver());
            //return new DbEnv(Communicator.this, name, dd, false);
            String name =  Utils.substitute(descriptor.name, getResolver());
            return new DbEnv(Communicator.this, name, descriptor, false);
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

    class Services extends ChildList<ServiceInstanceDescriptor>
    {
        Services()
        {
            super(false);
        }

        @Override
        void newChild()
        {
            ServiceDescriptor serviceDescriptor =
                new ServiceDescriptor(new java.util.LinkedList<AdapterDescriptor>(),
                                      new PropertySetDescriptor(
                                          new String[0], new java.util.LinkedList<PropertyDescriptor>()),
                                      new java.util.LinkedList<DbEnvDescriptor>(),
                                      new String[0],
                                      "",
                                      "NewService",
                                      "");

            ServiceInstanceDescriptor descriptor =
                new ServiceInstanceDescriptor("",
                                              new java.util.HashMap<String, String>(),
                                              serviceDescriptor,
                                              new PropertySetDescriptor(
                                                  new String[0], new java.util.LinkedList<PropertyDescriptor>())
                                             );

            newService(descriptor);
        }

        void newServiceFromTemplate()
        {
            ServiceInstanceDescriptor descriptor =
                new ServiceInstanceDescriptor("",
                                              new java.util.HashMap<String, String>(),
                                              null,
                                              new PropertySetDescriptor(
                                                  new String[0],
                                                  new java.util.LinkedList<PropertyDescriptor>())
                                             );
            newService(descriptor);
        }

        void move(TreeNode child, boolean up)
        {
            int index = getIndex(child);
            int listIndex = _children.indexOf(child);

            ServiceInstanceDescriptor descriptor = (ServiceInstanceDescriptor)child.getDescriptor();

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
            getRoot().getTreeModel().nodesWereRemoved(Communicator.this, new int[]{index}, new Object[]{child});
            if(up)
            {
                _children.add(listIndex - 1, child);
                getRoot().getTreeModel().nodesWereInserted(Communicator.this, new int[]{index - 1});
            }
            else
            {
                _children.add(listIndex + 1, child);
                getRoot().getTreeModel().nodesWereInserted(Communicator.this, new int[]{index + 1});
            }
            getRoot().setSelectedNode(child);
            getCoordinator().showActions(child);
        }

        @Override
        TreeNode createChild(ServiceInstanceDescriptor descriptor)
            throws UpdateFailedException
        {
            //ServiceInstanceDescriptor descriptor = (ServiceInstanceDescriptor)o;

            if(descriptor.descriptor == null)
            {
                String serviceName = null;
                String displayString = null;
                Utils.Resolver serviceResolver = null;

                if(Communicator.this instanceof PlainServer)
                {
                    TemplateDescriptor templateDescriptor
                        = getRoot().findServiceTemplateDescriptor(descriptor.template);

                    if(templateDescriptor == null)
                    {
                        throw new UpdateFailedException("Cannot find template descriptor '" +
                                                        descriptor.template +
                                                        "' referenced by service-instance");
                    }

                    serviceResolver = new Utils.Resolver(getResolver(),
                                                         descriptor.parameterValues,
                                                         templateDescriptor.parameterDefaults);

                    ServiceDescriptor serviceDescriptor = (ServiceDescriptor)templateDescriptor.descriptor;

                    //
                    // If it's not null, it's a bug in the provider of this descriptor, e.g.
                    // the icegridadmin parsing code.
                    //
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

                return new ServiceInstance(Communicator.this, serviceName, displayString, descriptor, serviceResolver);
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

                return new PlainService(Communicator.this, serviceName, descriptor, serviceResolver);
            }
        }

        @Override
        void tryUpdate(TreeNode child)
            throws UpdateFailedException
        {
            //
            // Rebuilding a Service is quite different since the creation of a service can
            // trigger an UpdateFailedException
            //
            ServiceInstanceDescriptor descriptor = (ServiceInstanceDescriptor)child.getDescriptor();
            int listIndex = _children.indexOf(child);
            assert listIndex != -1;

            TreeNode newChild = createChild(descriptor);
            _children.set(listIndex, newChild);
            getRoot().getTreeModel().nodeStructureChanged(newChild);

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
                        descriptor.parameterValues = new java.util.HashMap<>();
                    }
                }

                //
                // Validate/update parameterValues
                //
                TemplateDescriptor td = (TemplateDescriptor)t.getDescriptor();
                descriptor.parameterValues = Editor.makeParameterValues(descriptor.parameterValues, td.parameters);

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

    java.util.List<ServiceInstance> findServiceInstances(String template)
    {
        java.util.List<ServiceInstance> result = new java.util.LinkedList<>();
        java.util.Iterator<TreeNode> p = _services.iterator();
        while(p.hasNext())
        {
            ServiceInstance obj = (ServiceInstance)p.next();
            if(obj instanceof ServiceInstance)
            {
                ServiceInstance service = obj;
                ServiceInstanceDescriptor d = (ServiceInstanceDescriptor)service.getDescriptor();
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
        boolean updated = false;

        java.util.Iterator<TreeNode> p = _services.iterator();
        while(p.hasNext())
        {
            TreeNode obj = p.next();
            if(obj instanceof ServiceInstance)
            {
                ServiceInstance service = (ServiceInstance)obj;
                ServiceInstanceDescriptor d = (ServiceInstanceDescriptor)service.getDescriptor();
                if(d.template.equals(template))
                {
                    p.remove();
                    _services.removeDescriptor(d);
                    getEnclosingEditable().markModified();
                    updated = true;
                }
            }
        }
        if(updated)
        {
            getRoot().getTreeModel().nodeStructureChanged(this);
        }
    }

    void removeSortedChildren(String[] childIds, java.util.List<TreeNodeBase> fromChildren)
    {
        removeSortedChildren(childIds, fromChildren, getRoot().getTreeModel());
    }

    void childrenChanged(java.util.List<TreeNodeBase> children)
    {
        childrenChanged(children, getRoot().getTreeModel());
    }

    String getProperty(String key)
    {
        CommunicatorDescriptor descriptor = getCommunicatorDescriptor();
        for(PropertyDescriptor p : descriptor.propertySet.properties)
        {
            if(p.name.equals(key))
            {
                return p.value;
            }
        }
        return null;
    }

    String lookupPropertyValue(String val)
    {
        CommunicatorDescriptor descriptor = getCommunicatorDescriptor();
        for(PropertyDescriptor p : descriptor.propertySet.properties)
        {
            if(p.value.equals(val))
            {
                return p.name;
            }
        }
        return "";
    }

    void setProperty(String key, String newValue)
    {
        CommunicatorDescriptor descriptor = getCommunicatorDescriptor();
        removeProperty(key);
        ((java.util.LinkedList<PropertyDescriptor>)descriptor.propertySet.properties).addFirst(
            new PropertyDescriptor(key, newValue));
    }

    void removeProperty(String key)
    {
        CommunicatorDescriptor descriptor = getCommunicatorDescriptor();
        java.util.Iterator<PropertyDescriptor> p = descriptor.propertySet.properties.iterator();
        while(p.hasNext())
        {
            PropertyDescriptor pd = p.next();
            if(pd.name.equals(key))
            {
                p.remove();
            }
        }
    }

    java.util.Map<String, String> propertiesMap()
    {
        java.util.Map<String, String> result = new java.util.HashMap<>();

        CommunicatorDescriptor descriptor = getCommunicatorDescriptor();
        for(PropertyDescriptor p : descriptor.propertySet.properties)
        {
            result.put(p.name, p.value);
        }
        return result;
    }

    //
    // Children
    //
    protected Adapters _adapters = new Adapters();
    protected DbEnvs _dbEnvs = new DbEnvs();
    protected Services _services = new Services();
    protected ChildList[] _childListArray = new ChildList[]{_adapters, _dbEnvs, _services};
}
