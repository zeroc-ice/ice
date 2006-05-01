// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGridGUI.LiveDeployment;

import IceGridGUI.*;
import IceGrid.*;

public abstract class Editor extends EditorBase
{
    //
    // An expanded property set (i.e. containing other property sets)
    //
    static public class ExpandedPropertySet
    {
	public ExpandedPropertySet[] references;
	public java.util.List properties;       // list of PropertyDescriptor 
    }

    static public java.util.Vector propertySetToVector(
	ExpandedPropertySet propertySet,
	ExpandedPropertySet instancePropertySet, // can be null
	Utils.Resolver resolver)
    {
	java.util.SortedMap toMap = new java.util.TreeMap();
	
	addSet(propertySet, resolver, toMap);
	if(instancePropertySet != null)
	{
	    addSet(instancePropertySet, resolver, toMap);
	}

	java.util.Vector result = new java.util.Vector(toMap.size());
	java.util.Iterator p = toMap.entrySet().iterator();
	while(p.hasNext())
	{
	    java.util.Map.Entry entry = (java.util.Map.Entry)p.next();
	    java.util.Vector row = new java.util.Vector(2);
	    row.add(entry.getKey());
	    row.add(entry.getValue());
	    result.add(row);
	}
	return result;
    }


    static private void addSet(ExpandedPropertySet set, Utils.Resolver resolver,
			       java.util.SortedMap toMap)
    {
	for(int i = 0; i < set.references.length; ++i)
	{
	    addSet(set.references[i], resolver, toMap);
	}
	
	java.util.Iterator p = set.properties.iterator();
	while(p.hasNext())
	{
	    PropertyDescriptor pd = (PropertyDescriptor)p.next();
	    String name = (String)pd.name;
	    String val = (String)pd.value;
	   
	    assert resolver != null;
	    name = resolver.substitute(pd.name);
	    val = resolver.substitute(pd.value);
	   
	    toMap.put(name, val);
	}
    }
}


