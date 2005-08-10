// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid.TreeNode;

import IceGrid.CommunicatorDescriptor;
import IceGrid.Model;
import IceGrid.PropertyDescriptor;

class PropertiesHolder extends Parent
{
    String get(String key)
    {
	assert _descriptor != null;

	java.util.Iterator p = _descriptor.properties.iterator();
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

    void put(String key, String value)
    {
	//
	// TODO mark-modified?
	//

	assert _descriptor != null;
	
	java.util.Iterator p = _descriptor.properties.iterator();
	while(p.hasNext())
	{
	    PropertyDescriptor pd = (PropertyDescriptor)p.next();
	    if(pd.name.equals(key))
	    {
		pd.value = value;
		return;
	    }
	}
	_descriptor.properties.add(new PropertyDescriptor(key, value));
    }

    PropertiesHolder(String id, Model model)
    {
	super(id, model);
    }

    protected CommunicatorDescriptor _descriptor;

}
