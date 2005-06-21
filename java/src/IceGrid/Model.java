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
    // WARNING: The following methods are called by the Server thread pool (or client 
    // thread pool for bidir connections), not by the UI thread!
    //

    void registryInit(int serial, final ApplicationDescriptor[] applications, String[] nodesUp)
    {	
	assert(_latestSerial == -1);
	_latestSerial = serial;

	SwingUtilities.invokeLater(new Runnable() 
	    {
		public void run() 
		{
		    NodeViewRoot nodeViewRoot = 
			(NodeViewRoot)TreeModelI.getTreeModel(TreeModelI.NODE_VIEW).getRoot();
		    
		    nodeViewRoot.init(applications);

		    ApplicationViewRoot applicationViewRoot =
			(ApplicationViewRoot)TreeModelI.getTreeModel(TreeModelI.APPLICATION_VIEW).getRoot();

		    applicationViewRoot.init(applications);
		   
		}
	    });
    }

    void applicationAdded(int serial, ApplicationDescriptor desc)
    {
    }

    void applicationRemoved(int serial, String name)
    {
    }


    //
    // Notification from the SessionKeeper thread
    //
    void lostSession()
    {
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
