// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceGridGUI.Application;

import java.util.Enumeration;

import com.zeroc.IceGrid.*;
import com.zeroc.IceGridGUI.*;

public abstract class TreeNode extends TreeNodeBase
{
    abstract public Editor getEditor();
    abstract protected Editor createEditor();
    abstract Object getDescriptor();

    abstract void write(XMLWriter writer) throws java.io.IOException;

    //
    // Ephemeral objects are destroyed when discard their changes
    //
    public boolean isEphemeral()
    {
        return false;
    }

    //
    // Destroys this node
    //
    public void destroy()
    {
        assert false;
    }

    TreeNode(TreeNode parent, String id)
    {
        super(parent, id);
    }

    public Root getRoot()
    {
        assert _parent != null;
        return ((TreeNode)_parent).getRoot();
    }

    TreeNode findChildLike(TreeNode other)
    {
        //
        // Default implementation just use id; not always appropriate
        //
        return (TreeNode)findChild(other.getId());
    }

    //
    // Get variable-resolver
    //
    Utils.Resolver getResolver()
    {
        if(isEphemeral())
        {
            return null;
        }
        else
        {
            return ((TreeNode)_parent).getResolver();
        }
    }

    //
    // Find child whose descriptor == the given descriptor
    //
    TreeNode findChildWithDescriptor(Object descriptor)
    {
        Enumeration p = children();
        while(p.hasMoreElements())
        {
            TreeNode node = (TreeNode)p.nextElement();
            if(node.getDescriptor() == descriptor)
            {
                return node;
            }
        }
        return null;
    }

    static String[] createAttribute(String name, String value)
    {
        return new String[]{name, value};
    }

    static void writeVariables(XMLWriter writer, java.util.Map<String, String> variables)
        throws java.io.IOException
    {
        for(java.util.Map.Entry<String, String> p : variables.entrySet())
        {
            java.util.List<String[]> attributes = new java.util.LinkedList<>();
            attributes.add(createAttribute("name", p.getKey()));
            attributes.add(createAttribute("value", p.getValue()));

            writer.writeElement("variable", attributes);
        }
    }

    static void writePropertySet(XMLWriter writer, PropertySetDescriptor psd,
                                 java.util.List<AdapterDescriptor> adapters, String[] logs)
        throws java.io.IOException
    {
        writePropertySet(writer, "", "", psd, adapters, logs);
    }

    static void writePropertySet(XMLWriter writer, String id, String idAttrName,
                                 PropertySetDescriptor psd,
                                 java.util.List<AdapterDescriptor> adapters, String[] logs)
        throws java.io.IOException
    {
        if(id.length() == 0 && psd.references.length == 0 && psd.properties.size() == 0)
        {
            return;
        }

        //
        // We don't show the .Endpoint of adapters,
        // since they already appear in the Adapter descriptors
        //
        java.util.Set<String> hiddenPropertyNames = new java.util.HashSet<>();
        java.util.Set<String> hiddenPropertyValues = new java.util.HashSet<>();

        if(adapters != null)
        {
            for(AdapterDescriptor p : adapters)
            {
                hiddenPropertyNames.add(p.name + ".Endpoints");
                hiddenPropertyNames.add(p.name + ".ProxyOptions");

                for(ObjectDescriptor q : p.objects)
                {
                    hiddenPropertyValues.add(com.zeroc.Ice.Util.identityToString(q.id, com.zeroc.Ice.ToStringMode.Unicode));
                }
                for(ObjectDescriptor q : p.allocatables)
                {
                    hiddenPropertyValues.add(com.zeroc.Ice.Util.identityToString(q.id, com.zeroc.Ice.ToStringMode.Unicode));
                }
            }
        }

        if(logs != null)
        {
            for(String log : logs)
            {
                hiddenPropertyValues.add(log);
            }
        }

        java.util.List<String[]> attributes = new java.util.LinkedList<>();
        if(id.length() > 0)
        {
            attributes.add(createAttribute(idAttrName, id));
        }
        if(psd.references.length == 0 && psd.properties.size() == 0)
        {
            writer.writeElement("properties", attributes);
        }
        else
        {
            writer.writeStartTag("properties", attributes);

            for(String ref : psd.references)
            {
                attributes.clear();
                attributes.add(createAttribute("refid", ref));
                writer.writeElement("properties", attributes);
            }

            for(PropertyDescriptor p : psd.properties)
            {
                if(hiddenPropertyNames.contains(p.name))
                {
                    //
                    // We hide only the first occurence
                    //
                    hiddenPropertyNames.remove(p.name);
                }
                else if(hiddenPropertyValues.contains(p.value))
                {
                    hiddenPropertyValues.remove(p.value);
                }
                else
                {
                    attributes.clear();
                    attributes.add(createAttribute("name", p.name));
                    attributes.add(createAttribute("value", p.value));
                    writer.writeElement("property", attributes);
                }
            }
            writer.writeEndTag("properties");
        }
    }

    static void writeLogs(XMLWriter writer, String[] logs, java.util.List<PropertyDescriptor> properties)
        throws java.io.IOException
    {
        for(String log : logs)
        {
            java.util.List<String[]> attributes = new java.util.LinkedList<>();
            attributes.add(createAttribute("path", log));
            String prop = lookupName(log, properties);
            if(prop != null)
            {
                attributes.add(createAttribute("property", prop));
            }
            writer.writeElement("log", attributes);
        }
    }

    static String lookupName(String val, java.util.List<PropertyDescriptor> properties)
    {
        for(PropertyDescriptor p : properties)
        {
            if(p.value.equals(val))
            {
                return p.name;
            }
        }
        return null;
    }

    static void writeDistribution(XMLWriter writer, DistributionDescriptor descriptor)
        throws java.io.IOException
    {
        if(descriptor.icepatch.length() > 0)
        {
            java.util.List<String[]> attributes = new java.util.LinkedList<>();
            attributes.add(createAttribute("icepatch", descriptor.icepatch));

            if(descriptor.directories.isEmpty())
            {
                writer.writeElement("distrib", attributes);
            }
            else
            {
                writer.writeStartTag("distrib", attributes);
                for(String p : descriptor.directories)
                {
                    writer.writeElement("directory", p);
                }
                writer.writeEndTag("distrib");
            }
        }
    }

    static void writeObjects(String elt, XMLWriter writer, java.util.List<ObjectDescriptor> objects,
                             java.util.List<PropertyDescriptor> properties)
        throws java.io.IOException
    {
        for(ObjectDescriptor p : objects)
        {
            java.util.List<String[]> attributes = new java.util.LinkedList<>();
            String strId = com.zeroc.Ice.Util.identityToString(p.id, com.zeroc.Ice.ToStringMode.Unicode);
            attributes.add(createAttribute("identity", strId));
            if(p.type.length() > 0)
            {
                attributes.add(createAttribute("type", p.type));
            }
            if(properties != null)
            {
                String prop = lookupName(strId, properties);
                if(prop != null)
                {
                    attributes.add(createAttribute("property", prop));
                }
            }
            if(p.proxyOptions != null && !p.proxyOptions.equals(""))
            {
                attributes.add(createAttribute("proxy-options", p.proxyOptions));
            }
            writer.writeElement(elt, attributes);
        }
    }

    static void writeParameters(XMLWriter writer, java.util.List<String> parameters,
                                java.util.Map<String, String> defaultValues)
        throws java.io.IOException
    {
        for(String p : new java.util.LinkedHashSet<String>(parameters))
        {
            String val = defaultValues.get(p);
            java.util.List<String[]> attributes = new java.util.LinkedList<>();
            attributes.add(createAttribute("name", p));
            if(val != null)
            {
                attributes.add(createAttribute("default", val));
            }
            writer.writeElement("parameter", attributes);
        }
    }

    static java.util.LinkedList<String[]>
    parameterValuesToAttributes(java.util.Map<String, String> parameterValues, java.util.List<String> parameters)
    {
        java.util.LinkedList<String[]> result = new java.util.LinkedList<>();

        //
        // We use a LinkedHashSet to maintain order while eliminating duplicates
        //
        for(String p : new java.util.LinkedHashSet<String>(parameters))
        {
            String val = parameterValues.get(p);
            if(val != null)
            {
                result.add(createAttribute(p, val));
            }
        }
        return result;
    }

    //
    // Actions
    //
    public static final int NEW_ADAPTER = 0;
    public static final int NEW_DBENV = 1;
    public static final int NEW_NODE = 2;
    public static final int NEW_PROPERTY_SET = 3;
    public static final int NEW_REPLICA_GROUP = 4;
    public static final int NEW_SERVER = 5;
    public static final int NEW_SERVER_ICEBOX = 6;
    public static final int NEW_SERVER_FROM_TEMPLATE = 7;
    public static final int NEW_SERVICE = 8;
    public static final int NEW_SERVICE_FROM_TEMPLATE = 9;
    public static final int NEW_TEMPLATE_SERVER = 10;
    public static final int NEW_TEMPLATE_SERVER_ICEBOX = 11;
    public static final int NEW_TEMPLATE_SERVICE = 12;

    public static final int COPY = 13;
    public static final int PASTE = 14;
    public static final int DELETE = 15;

    public static final int SHOW_VARS = 16;
    public static final int SUBSTITUTE_VARS = 17;

    public static final int MOVE_UP = 18;
    public static final int MOVE_DOWN = 19;

    static public final int ACTION_COUNT = 20;

    public boolean[] getAvailableActions()
    {
        return new boolean[ACTION_COUNT];
    }
    public void newAdapter()
    {
        assert false;
    }
    public void newDbEnv()
    {
        assert false;
    }
    public void newNode()
    {
        assert false;
    }
    public void newPropertySet()
    {
        assert false;
    }
    public void newReplicaGroup()
    {
        assert false;
    }
    public void newServer()
    {
        assert false;
    }
    public void newServerIceBox()
    {
        assert false;
    }
    public void newServerFromTemplate()
    {
        assert false;
    }
    public void newService()
    {
        assert false;
    }
    public void newServiceFromTemplate()
    {
        assert false;
    }
    public void newTemplateServer()
    {
        assert false;
    }
    public void newTemplateServerIceBox()
    {
        assert false;
    }
    public void newTemplateService()
    {
        assert false;
    }
    public void copy()
    {
        assert false;
    }
    public void paste()
    {
        assert false;
    }
    public void delete()
    {
        boolean enabled = getRoot().isSelectionListenerEnabled();

        if(enabled)
        {
            getRoot().disableSelectionListener();
        }
        destroy();
        getCoordinator().getCurrentTab().showNode(null);
        if(enabled)
        {
            getRoot().enableSelectionListener();
        }

        if(_parent != null)
        {
            getRoot().setSelectedNode((TreeNode)_parent);
        }
    }
    public void moveUp()
    {
        assert false;
    }
    public void moveDown()
    {
        assert false;
    }
}
