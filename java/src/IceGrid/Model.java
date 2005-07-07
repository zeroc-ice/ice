// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid;

import java.util.prefs.Preferences;

import java.awt.Component;
import java.awt.Cursor;
import javax.swing.tree.TreeModel;
import javax.swing.JOptionPane;
import javax.swing.SwingUtilities;
import javax.swing.tree.TreePath;

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


    public static interface TreeNodeSelector
    {
	void selectNode(TreePath path, int view);
    }

    public static class ConnectInfo
    {
	ConnectInfo(Preferences connectionPrefs, 
		    Ice.Communicator communicator)
	{
	    String defaultLocator = communicator.getProperties().
		getProperty("Ice.Default.Locator");
	    if(defaultLocator.equals(""))
	    {
		defaultLocator = "IceGrid/Locator:ssl -h hostname -p port -t timeout";
	    }

	    _connectionPrefs = connectionPrefs;
	    locatorProxy = _connectionPrefs.
		get("locatorProxy", defaultLocator);
	    username = _connectionPrefs.get("username", username);
	    useGlacier = _connectionPrefs.
		getBoolean("useGlacier", useGlacier);
	    autoconnect = _connectionPrefs.
		getBoolean("autoconnect", autoconnect);
	    adminIdentity = _connectionPrefs.
		get("adminIdentity", adminIdentity);
	    sessionManagerIdentity = _connectionPrefs.
		get("sessionManagerIdentity", sessionManagerIdentity);
	}

	void save()
	{
	    _connectionPrefs.put("locatorProxy", locatorProxy);
	    _connectionPrefs.put("username", username);
	    _connectionPrefs.putBoolean("useGlacier", useGlacier);
	    _connectionPrefs.putBoolean("autoconnect", autoconnect);
	    _connectionPrefs.put("adminIdentity", adminIdentity);
	    _connectionPrefs.put("sessionManagerIdentity", 
				 sessionManagerIdentity);
	}

	String locatorProxy;
	String username = System.getProperty("user.name");
	char[] password;
	boolean useGlacier = false;
	boolean autoconnect = false;
	String adminIdentity = "IceGrid/Admin";
	String sessionManagerIdentity = "IceGrid/SessionManager";
	
	private Preferences _connectionPrefs;
    }




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
	_sessionManager = null;
	_admin = null;
    }
    
    boolean setConnectInfo(ConnectInfo info, Component parent, 
			   Cursor oldCursor)
    {	

	//
	// Default locator
	//
	Ice.LocatorPrx defaultLocator = null;
	try
	{
	    defaultLocator = Ice.LocatorPrxHelper.
		uncheckedCast(_communicator.stringToProxy(info.locatorProxy));
	    _communicator.setDefaultLocator(defaultLocator);
	}
	catch(Ice.LocalException e)
	{
	    sessionLost();
	    parent.setCursor(oldCursor);
		JOptionPane.showMessageDialog(
		    parent,
		    "The locator proxy is invalid: " + e.toString(),
		    "Invalid locator proxy",
		    JOptionPane.ERROR_MESSAGE);
		return false;
	}

	
	//
	// Session manager
	//
	try
	{
	    _sessionManager = SessionManagerPrxHelper.
		uncheckedCast(_communicator.stringToProxy(info.sessionManagerIdentity));
	}
	catch(Ice.LocalException e)
	{
	    sessionLost();
	    parent.setCursor(oldCursor);
		JOptionPane.showMessageDialog(
		    parent,
		    "The session manager identity is invalid: " + e.toString(),
		    "Invalid session manager",
		    JOptionPane.ERROR_MESSAGE);
		return false;
	}

	//
	// Admin
	//
	try
	{
	    _admin = AdminPrxHelper.
		uncheckedCast(_communicator.stringToProxy(info.adminIdentity));
	}
	catch(Ice.LocalException e)
	{
	    sessionLost();
	    parent.setCursor(oldCursor);
		JOptionPane.showMessageDialog(
		    parent,
		    "The admin identity is invalid: " + e.toString(),
		    "Invalid admin identity",
		    JOptionPane.ERROR_MESSAGE);
		return false;
	}

	return true;
    }
    
    boolean save()
    {
	return true;
    }

    boolean updated()
    {
	return true;
    }  

    public AdminPrx getAdmin()
    {
	return _admin;
    }

    public SessionManagerPrx getSessionManager()
    {
	return _sessionManager;
    }

    public StatusBar getStatusBar()
    {
	return _statusBar;
    }

    public void setTreeNodeSelector(TreeNodeSelector selector)
    {
	_treeNodeSelector = selector;
    }

    public TreeNodeSelector getTreeNodeSelector()
    {
	return _treeNodeSelector;
    }


    Model(Ice.Communicator communicator, StatusBar statusBar)
    {
	_communicator = communicator;
	_statusBar = statusBar;
	
	_nodeViewRoot = new NodeViewRoot(this);
	_nodeModel = new TreeModelI(_nodeViewRoot);

	_applicationViewRoot = new ApplicationViewRoot(this);
	_applicationModel = new TreeModelI(_applicationViewRoot);
    }

    

    private Ice.Communicator _communicator;
    private StatusBar _statusBar;
    private SessionManagerPrx _sessionManager;
    private AdminPrx _admin;


    private NodeViewRoot _nodeViewRoot;
    private ApplicationViewRoot _applicationViewRoot;
    private TreeModelI _nodeModel;
    private TreeModelI _applicationModel;

    private int _latestSerial = -1;

    private TreeNodeSelector _treeNodeSelector;
}
