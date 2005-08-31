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
    PropertiesHolder(CommunicatorDescriptor descriptor,
		     Editable editable)
    {
	assert _descriptor != null;

	_descriptor = descriptor;
	_editable = editable;
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

    void put(String key, String value)
    {
	assert _editable != null;
	
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
	_editable.markModified();
    }

    private CommunicatorDescriptor _descriptor;
    private Editable _editable;
}
