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

class PropertiesHolder
{
    PropertiesHolder(CommunicatorDescriptor descriptor)
    {
	assert descriptor != null;

	_descriptor = descriptor;
    }

    String get(String key)
    {
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

    void set(String key, String newValue)
    {
	remove(key);
	_descriptor.properties.add(new PropertyDescriptor(key, newValue));
    }

    void remove(String key)
    {
	java.util.Iterator p = _descriptor.properties.iterator();
	while(p.hasNext())
	{
	    PropertyDescriptor pd = (PropertyDescriptor)p.next();
	    if(pd.name.equals(key))
	    {
		p.remove();
	    }
	}
    }

    private CommunicatorDescriptor _descriptor;
}
