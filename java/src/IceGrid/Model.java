// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid;

import javax.swing.tree.TreeModel;
import javax.swing.SwingUtilities;

import IceGrid.TreeNode.NodeViewRoot;
import IceGrid.TreeNode.ApplicationViewRoot;

//
// Where all the data are kept
//

public class Model
{

    //
    // Application view:
    //
    // root
    //  |
    //  -- $application
    //         |
    //         -- server templates
    //         |        |
    //         |        -- $server template
    //         |              |
    //         |              -- adapters
    //         |              |    |
    //         |              |    -- $adapter
    //         |              |    
    //         |              -- databases
    //         |              |      |
    //         |              |      -- $database
    //         |              |  
    //         |              -- services (only for IceBox servers)
    //         |                    |
    //         |                    -- $service instance
    //         |                          |
    //         |                          -- adapters, databases (see above)
    //         |
    //         -- service templates
    //         |        |
    //         |        -- $service template
    //         |              |
    //         |              -- adapters
    //         |              |    |
    //         |              |    -- $adapter
    //         |              |    
    //         |              -- databases
    //         |                     |
    //         |                     -- $database
    //         |
    //         |
    //         -- server instances
    //         |         |
    //         |         -- $server instance
    //         |               |
    //         |             (like node view below)
    //         |
    //         -- node variables
    //                  |
    //                  -- $nodevar (leaf, shows per-application variables)
    //
    //
    // Node view:
    //
    // root
    //  |
    //  -- $node
    //        |
    //        -- $instance
    //              |
    //              -- adapters
    //              |    |
    //              |    -- $adapter
    //              |    
    //              -- databases
    //              |      |
    //              |      -- $database
    //              |  
    //              -- services (only for IceBox servers)
    //                     |
    //                     -- $service
    //                            |
    //                            -- adapters, databases (see above)


    //
    // All Model's methods run in the UI thread
    //

    
    public Ice.Communicator getCommunicator()
    {
	return _communicator;
    }

    public NodeViewRoot getNodeViewRoot()
    {
	return _nodeViewRoot;
    }

    public ApplicationViewRoot getApplicationViewRoot()
    {
	return _applicationViewRoot;
    }
    
    public TreeModelI getTreeModel(int view)
    {
	if(view == TreeModelI.NODE_VIEW)
	{
	    return _nodeModel;
	}
	else if(view == TreeModelI.APPLICATION_VIEW)
	{
	    return _applicationModel;
	}
	else
	{
	    return null;
	}
    }


    //
    // From the Registry observer:
    //
 
    void registryInit(int serial, final java.util.List applications)
    {	
	assert(_latestSerial == -1);
	_latestSerial = serial;

	_nodeViewRoot.init(applications);
	_applicationViewRoot.init(applications);
    }

    void applicationAdded(ApplicationDescriptor desc)
    {
	_nodeViewRoot.put(desc.name, desc.nodes, true);
	_applicationViewRoot.applicationAdded(desc);
    }

    void applicationRemoved(String name)
    {
	_nodeViewRoot.remove(name);
	_applicationViewRoot.applicationRemoved(name);
    }

    void applicationSynced(ApplicationDescriptor desc)
    {
	_nodeViewRoot.remove(desc.name);
	_nodeViewRoot.put(desc.name, desc.nodes, true);
	_applicationViewRoot.applicationSynced(desc);
    }

    void applicationUpdated(ApplicationUpdateDescriptor desc)
    {
	for(int i = 0; i < desc.removeNodes.length; ++i)
	{
	    _nodeViewRoot.remove(desc.name, desc.removeNodes[i]);
	}
	_nodeViewRoot.put(desc.name, desc.nodes, true);
	_nodeViewRoot.removeServers(desc.removeServers);
	
	_applicationViewRoot.applicationUpdated(desc);
    }
    
    boolean updateSerial(int serial)
    {
	if(serial == _latestSerial + 1)
	{
	    _latestSerial = serial;
	    return true;
	}
	else
	{
	    return false;
	}
    }


    //
    // From the Node observer:
    //

    void nodeUp(NodeDynamicInfo updatedInfo)
    {
	_nodeViewRoot.nodeUp(updatedInfo);
    }

    void nodeDown(String node)
    {
	_nodeViewRoot.nodeDown(node);
    }

    void updateServer(String node, ServerDynamicInfo updatedInfo)
    {
	_nodeViewRoot.updateServer(node, updatedInfo);
    }

    void updateAdapter(String node, AdapterDynamicInfo updatedInfo)
    {
	_nodeViewRoot.updateAdapter(node, updatedInfo);
    }


    //
    // Other methods
    //

    void sessionLost()
    {
	_latestSerial = -1;
	_nodeViewRoot.clear();
	_applicationViewRoot.clear();
    }

    boolean save()
    {
	return true;
    }

    boolean updated()
    {
	return true;
    }  

    Model(Ice.Communicator communicator)
    {
	_communicator = communicator;
	
	_nodeViewRoot = new NodeViewRoot(this);
	_nodeModel = new TreeModelI(_nodeViewRoot);

	_applicationViewRoot = new ApplicationViewRoot(this);
	_applicationModel = new TreeModelI(_applicationViewRoot);
    }

    

    private Ice.Communicator _communicator;

    private NodeViewRoot _nodeViewRoot;
    private ApplicationViewRoot _applicationViewRoot;
    private TreeModelI _nodeModel;
    private TreeModelI _applicationModel;

    private int _latestSerial = -1;
}
