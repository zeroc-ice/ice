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

class Model
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
    // The following methods all run in the UI thread
    //

 
    void registryInit(int serial, final java.util.List applications, String[] nodesUp)
    {	
	assert(_latestSerial == -1);
	_latestSerial = serial;

	NodeViewRoot nodeViewRoot = 
	    (NodeViewRoot)TreeModelI.getTreeModel(TreeModelI.NODE_VIEW).getRoot();
	nodeViewRoot.init(applications);
	
	ApplicationViewRoot applicationViewRoot =
	    (ApplicationViewRoot)TreeModelI.getTreeModel(TreeModelI.APPLICATION_VIEW).getRoot();
	applicationViewRoot.init(applications);
    }

    void applicationAdded(ApplicationDescriptor desc)
    {
	NodeViewRoot nodeViewRoot = 
	    (NodeViewRoot)TreeModelI.getTreeModel(TreeModelI.NODE_VIEW).getRoot();
	nodeViewRoot.put(desc.name, desc.nodes, true);

	ApplicationViewRoot applicationViewRoot =
	    (ApplicationViewRoot)TreeModelI.getTreeModel(TreeModelI.APPLICATION_VIEW).getRoot();
	applicationViewRoot.applicationAdded(desc);
    }

    void applicationRemoved(String name)
    {
	NodeViewRoot nodeViewRoot = 
	    (NodeViewRoot)TreeModelI.getTreeModel(TreeModelI.NODE_VIEW).getRoot();
	nodeViewRoot.remove(name);


	ApplicationViewRoot applicationViewRoot =
	    (ApplicationViewRoot)TreeModelI.getTreeModel(TreeModelI.APPLICATION_VIEW).getRoot();
	applicationViewRoot.applicationRemoved(name);
    }

    void applicationSynced(ApplicationDescriptor desc)
    {
	NodeViewRoot nodeViewRoot = 
	    (NodeViewRoot)TreeModelI.getTreeModel(TreeModelI.NODE_VIEW).getRoot();
	nodeViewRoot.remove(desc.name);
	nodeViewRoot.put(desc.name, desc.nodes, true);

	ApplicationViewRoot applicationViewRoot =
	    (ApplicationViewRoot)TreeModelI.getTreeModel(TreeModelI.APPLICATION_VIEW).getRoot();
	
	applicationViewRoot.applicationSynced(desc);
    }

    void applicationUpdated(ApplicationUpdateDescriptor desc)
    {
	NodeViewRoot nodeViewRoot = 
	    (NodeViewRoot)TreeModelI.getTreeModel(TreeModelI.NODE_VIEW).getRoot();
	

	for(int i = 0; i < desc.removeNodes.length; ++i)
	{
	    nodeViewRoot.remove(desc.name, desc.removeNodes[i]);
	}
	nodeViewRoot.put(desc.name, desc.nodes, true);
	nodeViewRoot.removeServers(desc.removeServers);

	ApplicationViewRoot applicationViewRoot =
	    (ApplicationViewRoot)TreeModelI.getTreeModel(TreeModelI.APPLICATION_VIEW).getRoot();
	
	applicationViewRoot.applicationUpdated(desc);
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


    void sessionLost()
    {
	_latestSerial = -1;

	NodeViewRoot nodeViewRoot = 
	    (NodeViewRoot)TreeModelI.getTreeModel(TreeModelI.NODE_VIEW).getRoot();
	
	nodeViewRoot.clear();
	
	ApplicationViewRoot applicationViewRoot =
	    (ApplicationViewRoot)TreeModelI.getTreeModel(TreeModelI.APPLICATION_VIEW).getRoot();
	
	applicationViewRoot.clear();
    }

	


    boolean save()
    {
	return true;
    }

    boolean updated()
    {
	return true;
    }  

    





    private int _latestSerial = -1;
}
