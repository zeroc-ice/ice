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
import javax.swing.JTree;
import javax.swing.SwingUtilities;
import javax.swing.tree.TreePath;

import IceGrid.TreeNode.Root;
import com.jgoodies.uif_lite.panel.SimpleInternalFrame;

//
// Where all the data are kept
//

public class Model
{
    //
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
    //         -- nodes
    //             |
    //             -- $node
    //                  |
    //                   -- $instance
    //                          |
    //                          -- adapters
    //                          |    |
    //                          |    -- $adapter
    //                          |    
    //                          -- databases
    //                          |      |
    //                          |      -- $database
    //                          |  
    //                          -- services (only for IceBox servers)
    //                                |
    //                                 -- $service
    //                                       |
    //                                       -- adapters, databases (see above)

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

 
    public Root getRoot()
    {
	return _root;
    }
    
    public TreeModelI getTreeModel()
    {
	return _treeModel;
    }


    //
    // From the Registry observer:
    //
 
    void registryInit(int serial, final java.util.List applications)
    {	
	assert(_latestSerial == -1);
	_latestSerial = serial;

	_root.init(applications);
    }

    void applicationAdded(ApplicationDescriptor desc)
    {
	_root.applicationAdded(desc);
    }

    void applicationRemoved(String name)
    {
	_root.applicationRemoved(name);
    }

    void applicationUpdated(ApplicationUpdateDescriptor desc)
    {
	_root.applicationUpdated(desc);
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
	_root.nodeUp(updatedInfo);
    }

    void nodeDown(String node)
    {
	_root.nodeDown(node);
    }

    void updateServer(String node, ServerDynamicInfo updatedInfo)
    {
	_root.updateServer(node, updatedInfo);
    }

    void updateAdapter(String node, AdapterDynamicInfo updatedInfo)
    {
	_root.updateAdapter(node, updatedInfo);
    }


    //
    // Other methods
    //

    void sessionLost()
    {
	_latestSerial = -1;
	_root.clear();
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

    void toggleSubstitute()
    {
	_substitute = !_substitute;
	//
	// TODO: need to send a notification to redisplay current properties
	//
    }

    public boolean substitute()
    {
	return _substitute;
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

    public void setTree(JTree tree)
    {
	_tree = tree;
    }

    public JTree getTree()
    {
	return _tree;
    }

    public void setPropertiesFrame(SimpleInternalFrame frame)
    {
	_propertiesFrame = frame;
    }

    public SimpleInternalFrame getPropertiesFrame()
    {
	return _propertiesFrame;
    }

    Model(Ice.Communicator communicator, StatusBar statusBar)
    {
	_communicator = communicator;
	_statusBar = statusBar;
	
	_root = new Root(this);
	_treeModel = new TreeModelI(_root);
    }


    private Ice.Communicator _communicator;
    private StatusBar _statusBar;
    private SessionManagerPrx _sessionManager;
    private AdminPrx _admin;

    private Root _root;
    private TreeModelI _treeModel;

    private int _latestSerial = -1;

    private JTree _tree;

    private boolean _substitute = false;

    private SimpleInternalFrame _propertiesFrame;
}
