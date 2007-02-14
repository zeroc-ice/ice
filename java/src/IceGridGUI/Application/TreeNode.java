// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGridGUI.Application;

import javax.swing.JOptionPane;
import javax.swing.JPopupMenu;
import javax.swing.SwingUtilities;

import java.util.Enumeration;

import IceGrid.*;
import IceGridGUI.*;

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

    static void writeVariables(XMLWriter writer, 
                               java.util.Map variables)
        throws java.io.IOException
    {
        java.util.Iterator p = variables.entrySet().iterator();
        while(p.hasNext())
        {
            java.util.Map.Entry entry = (java.util.Map.Entry)p.next();
            
            java.util.List attributes = new java.util.LinkedList();
            attributes.add(createAttribute("name", entry.getKey().toString()));
            attributes.add(createAttribute("value", entry.getValue().toString()));

            writer.writeElement("variable", attributes);
        }
    }

    static void writePropertySet(XMLWriter writer, PropertySetDescriptor psd, 
                                 java.util.List adapters, String[] logs)
        throws java.io.IOException
    {
        writePropertySet(writer, "", "", psd, adapters, logs);
    }

    static void writePropertySet(XMLWriter writer, String id, String idAttrName,
                                 PropertySetDescriptor psd, 
                                 java.util.List adapters, String[] logs)
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
        java.util.Set hiddenPropertyNames = new java.util.HashSet();
        java.util.Set hiddenPropertyValues = new java.util.HashSet();

        if(adapters != null)
        {
            java.util.Iterator p = adapters.iterator();
            while(p.hasNext())
            {
                AdapterDescriptor ad = (AdapterDescriptor)p.next();
                hiddenPropertyNames.add(ad.name + ".Endpoints");

                java.util.Iterator q = ad.objects.iterator();
                while(q.hasNext())
                {
                    ObjectDescriptor od = (ObjectDescriptor)q.next();
                    hiddenPropertyValues.add(Ice.Util.identityToString(od.id));
                }
                q = ad.allocatables.iterator();
                while(q.hasNext())
                {
                    ObjectDescriptor od = (ObjectDescriptor)q.next();
                    hiddenPropertyValues.add(Ice.Util.identityToString(od.id));
                }
            }
        }

        if(logs != null)
        {
            for(int i = 0; i < logs.length; ++i)
            {
                hiddenPropertyValues.add(logs[i]);
            }
        }

        java.util.List attributes = new java.util.LinkedList();
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
            
            for(int i = 0; i < psd.references.length; ++i)
            {
                attributes.clear();
                attributes.add(createAttribute("refid", psd.references[i])); 
                writer.writeElement("properties", attributes);
            }
            
            java.util.Iterator p = psd.properties.iterator();
            while(p.hasNext())
            {
                PropertyDescriptor pd = (PropertyDescriptor)p.next();
                if(hiddenPropertyNames.contains(pd.name))
                {
                    //
                    // We hide only the first occurence
                    //
                    hiddenPropertyNames.remove(pd.name);
                }
                else if(hiddenPropertyValues.contains(pd.value))
                {
                    hiddenPropertyValues.remove(pd.value);
                }
                else
                {
                    attributes.clear();
                    attributes.add(createAttribute("name", pd.name));
                    attributes.add(createAttribute("value", pd.value));
                    writer.writeElement("property", attributes);
                }
            }
            writer.writeEndTag("properties");
        }
    }

    static void writeLogs(XMLWriter writer, String[] logs, java.util.List properties)
        throws java.io.IOException
    {
        for(int i = 0; i < logs.length; ++i)
        {
            java.util.List attributes = new java.util.LinkedList();
            attributes.add(createAttribute("path", logs[i]));
            String prop = lookupName(logs[i], properties);
            if(prop != null)
            {
                attributes.add(createAttribute("property", prop));
            }
            writer.writeElement("log", attributes);
        }
    }

    static String lookupName(String val, java.util.List properties)
    {
        java.util.Iterator p = properties.iterator();
        while(p.hasNext())
        {
            PropertyDescriptor pd = (PropertyDescriptor)p.next();
            if(pd.value.equals(val))
            {
                return pd.name;
            }
        }
        return null;
    }


    static void writeDistribution(XMLWriter writer, 
                                  DistributionDescriptor descriptor)
        throws java.io.IOException
    {
        if(descriptor.icepatch.length() > 0)
        {
            java.util.List attributes = new java.util.LinkedList();
            attributes.add(createAttribute("icepatch", descriptor.icepatch));
        
            if(descriptor.directories.isEmpty())
            {
                writer.writeElement("distrib", attributes);
            }
            else
            {
                writer.writeStartTag("distrib", attributes);
                java.util.Iterator p = descriptor.directories.iterator();
                while(p.hasNext())
                {
                    writer.writeElement("directory", p.next().toString());
                }
                writer.writeEndTag("distrib");
            }
        }
    }
    
    static void writeObjects(String elt, XMLWriter writer, java.util.List objects, 
                             java.util.List properties)
        throws java.io.IOException
    {
        java.util.Iterator p = objects.iterator();
        while(p.hasNext())
        {
            ObjectDescriptor od = (ObjectDescriptor)p.next();
            java.util.List attributes = new java.util.LinkedList();
            String strId = Ice.Util.identityToString(od.id);
            attributes.add(createAttribute("identity", strId));
            if(od.type.length() > 0)
            {
                attributes.add(createAttribute("type", od.type));
            }
            if(properties != null)
            {
                String prop = lookupName(strId, properties);
                if(prop != null)
                {
                    attributes.add(createAttribute("property", prop));
                }
            }
            
            writer.writeElement(elt, attributes);
        }
    }     

    static void writeParameters(XMLWriter writer, java.util.List parameters,
                                java.util.Map defaultValues)
        throws java.io.IOException
    {
        java.util.Iterator p = new java.util.LinkedHashSet(parameters).iterator();
        while(p.hasNext())
        {
            String parameter = (String)p.next();
            String val = (String)defaultValues.get(parameter);
            java.util.List attributes = new java.util.LinkedList();
            attributes.add(createAttribute("name", parameter));
            if(val != null)
            {
                attributes.add(createAttribute("default", val));
            }
            writer.writeElement("parameter", attributes);
        }
    }

    static java.util.LinkedList 
    parameterValuesToAttributes(java.util.Map parameterValues,
                                java.util.List parameters)
    {
        java.util.LinkedList result = new java.util.LinkedList();

        //
        // We use a LinkedHashSet to maintain order while eliminating duplicates
        //
        java.util.Iterator p = new java.util.LinkedHashSet(parameters).iterator();
        while(p.hasNext())
        {
            String param = (String)p.next();
            String val = (String)parameterValues.get(param);
            if(val != null)
            {
                result.add(createAttribute(param, val));
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
