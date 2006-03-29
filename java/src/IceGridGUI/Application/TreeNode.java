// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
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
    // Ephemeral objects are destroyed when you switch selection
    // without "apply"ing the changes.
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

    Root getRoot()
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


    boolean isEditable()
    {
	if(_parent == null)
	{
	    return true;
	}
	else
	{
	    return ((TreeNode)_parent).isEditable();
	}
    }

    //
    // Get variable-resolver
    //
    Utils.Resolver getResolver()
    {
	return ((TreeNode)_parent).getResolver();
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
			       java.util.SortedMap variables)
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

    static void writeProperties(XMLWriter writer,
				java.util.List properties)
	throws java.io.IOException
    {
	java.util.Iterator p = properties.iterator();
	while(p.hasNext())
	{
	    PropertyDescriptor pd = (PropertyDescriptor)p.next();
	    java.util.List attributes = new java.util.LinkedList();
	    attributes.add(createAttribute("name", pd.name));
	    attributes.add(createAttribute("value", pd.value));
	    writer.writeElement("property", attributes);
	}
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
    
    static void writeObjects(XMLWriter writer, java.util.List objects)
	throws java.io.IOException
    {
	java.util.Iterator p = objects.iterator();
	while(p.hasNext())
	{
	    ObjectDescriptor od = (ObjectDescriptor)p.next();
	    java.util.List attributes = new java.util.LinkedList();
	    attributes.add(createAttribute("identity", Ice.Util.identityToString(od.id)));
	    if(od.type.length() > 0)
	    {
		attributes.add(createAttribute("type", od.type));
	    }
	    writer.writeElement("object", attributes);
	}
    }	  

    static void writeParameters(XMLWriter writer, java.util.List parameters,
				java.util.Map defaultValues)
	throws java.io.IOException
    {
	java.util.Iterator p = parameters.iterator();
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

	java.util.Iterator p = parameters.iterator();
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
    public static final int NEW_REPLICA_GROUP = 3;
    public static final int NEW_SERVER = 4;
    public static final int NEW_SERVER_ICEBOX = 5;
    public static final int NEW_SERVER_FROM_TEMPLATE = 6;
    public static final int NEW_SERVICE = 7;
    public static final int NEW_SERVICE_FROM_TEMPLATE = 8;
    public static final int NEW_TEMPLATE_SERVER = 9;
    public static final int NEW_TEMPLATE_SERVER_ICEBOX = 10;
    public static final int NEW_TEMPLATE_SERVICE = 11;
  
    public static final int COPY = 12;
    public static final int PASTE = 13;
    public static final int DELETE = 14;

    public static final int SHOW_VARS = 15;
    public static final int SUBSTITUTE_VARS = 16;

    public static final int MOVE_UP = 17;
    public static final int MOVE_DOWN = 18;

    static public final int ACTION_COUNT = 19;

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
	destroy();
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
