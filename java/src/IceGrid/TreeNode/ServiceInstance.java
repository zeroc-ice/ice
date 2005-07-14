// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid.TreeNode;

import IceGrid.Model;
import IceGrid.ServiceDescriptor;
import IceGrid.ServiceInstanceDescriptor;
import IceGrid.TemplateDescriptor;


class ServiceInstance extends Parent
{
    ServiceInstance(String id,
		    String displayString,
		    ServiceInstanceDescriptor descriptor, 
		    ServiceDescriptor serviceDescriptor,
		    boolean editable, // False when the enclosing server instance
		                      // is a template-instance
		    java.util.Map serverParameters, // When the enclosing server instance
		                                    // is a template instance,
		                                    // this server's parameters (substituted);
                                                    // null otherwise
		    java.util.Map myParameters,     // When this is a template instance within
		                                    // a server instance, descriptor.parameterValues
		                                    // substituted using parameters + variables
		    java.util.Map nodeVariables,
		    java.util.Map appVariables,
		    Model model, 
		    Node node)
    {
	super(id, model);
	_displayString = displayString;
	_descriptor = descriptor;
	_editable = editable;

	if(serviceDescriptor != null)
	{
	    boolean childrenEditable = _editable && (descriptor.template.length() == 0);

	    //
	    // For the children, the proper parameters can be used as variables.
	    // Note that the fields of the service-instance itself should be
	    // substituted with parameters (if any) + variables.
	    //
	    java.util.Map[] variables = null;
	    if(node != null)
	    {
		variables = new java.util.Map[3];
		if(myParameters != null)
		{
		    variables[0] = myParameters;
		    assert(descriptor.template.length() > 0);
		}
		else
		{
		    variables[0] = serverParameters;
		    assert(descriptor.template.length() == 0);
		}
		variables[1] = nodeVariables;
		variables[2] = appVariables;
	    }
	    
	    _adapters = new Adapters(serviceDescriptor.adapters, 
				     childrenEditable,
				     variables, _model, node);
	    addChild(_adapters);
	    
	    _dbEnvs = new DbEnvs(serviceDescriptor.dbEnvs, childrenEditable,
				 variables, _model);
	    addChild(_dbEnvs);
	}
    }

    void unregisterAdapters()
    {
	_adapters.unregisterAll();
    }

    public String toString()
    {
	if(_displayString != null)
	{
	    return _displayString;
	}
	else
	{
	    return _id;
	}
    }

    private ServiceInstanceDescriptor _descriptor;
    private String _displayString;
    private boolean _editable;
    private Adapters _adapters;
    private DbEnvs _dbEnvs;
}
