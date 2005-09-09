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

    void replace(String oldKey, String newKey, String newValue)
    {
	boolean needPut = true;
	boolean needRemove = !oldKey.equals(newKey); // don't remove when oldKey == newKey

	java.util.Iterator p = _descriptor.properties.iterator();
	while(p.hasNext() && (needRemove || needPut))
	{
	    PropertyDescriptor pd = (PropertyDescriptor)p.next();
	    if(needRemove && pd.name.equals(oldKey))
	    {
		p.remove();
		needRemove = false;
	    }
	    if(needPut && pd.name.equals(newKey))
	    {
		pd.value = newValue;
		needPut = false;
	    }
	}

	if(needPut)
	{
	    _descriptor.properties.add(new PropertyDescriptor(newKey, newValue));
	}
    }
    private CommunicatorDescriptor _descriptor;
}
