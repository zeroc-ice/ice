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
import java.util.prefs.BackingStoreException;

import java.awt.Cursor;
import java.awt.Frame;
import java.awt.Rectangle;
import java.awt.Container;
import java.awt.Component;

import java.awt.event.ActionEvent;
import java.awt.event.ItemEvent;
import java.awt.event.ActionListener;
import java.awt.event.ItemListener;

import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.JFrame;
import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JOptionPane;
import javax.swing.JToggleButton;
import javax.swing.JToolBar;
import javax.swing.JTree;
import javax.swing.KeyStroke;
import javax.swing.SwingUtilities;

import javax.swing.tree.TreeModel;
import javax.swing.tree.TreePath;

import com.jgoodies.looks.Options;
import com.jgoodies.looks.HeaderStyle;
import com.jgoodies.looks.BorderStyle;
import com.jgoodies.looks.plastic.PlasticLookAndFeel;

import com.jgoodies.uif_lite.panel.SimpleInternalFrame;

import IceGrid.TreeNode.Application;
import IceGrid.TreeNode.CommonBase;
import IceGrid.TreeNode.Root;



//
// The class that holds all the data and more!
//

public class Model
{
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

    class MenuBar extends JMenuBar
    {
	MenuBar()
	{
	    putClientProperty(Options.HEADER_STYLE_KEY, HeaderStyle.BOTH);
	    putClientProperty(PlasticLookAndFeel.BORDER_STYLE_KEY, BorderStyle.SEPARATOR);

	    JMenu fileMenu = new JMenu("File");
	    fileMenu.setMnemonic(java.awt.event.KeyEvent.VK_F);
	    add(fileMenu);
	    
	    JMenu editMenu = new JMenu("Edit");
	    editMenu.setMnemonic(java.awt.event.KeyEvent.VK_E);
	    add(editMenu);
	    
	    JMenu helpMenu = new JMenu("Help");
	    helpMenu.setMnemonic(java.awt.event.KeyEvent.VK_H);
	    add(helpMenu);
	    
	    fileMenu.add(_connect);
	    fileMenu.addSeparator();
	    fileMenu.add(_save);
	    fileMenu.add(_discard);
	    fileMenu.addSeparator();
	    fileMenu.add(_exit);
	    
	    editMenu.add(_copy);
	    editMenu.add(_paste);
	    editMenu.addSeparator();
	    editMenu.add(_delete);
	    
	    helpMenu.add(_about);
	} 
    }
    
    class ToolBar extends JToolBar
    {
	ToolBar()
	{
	    putClientProperty(Options.HEADER_STYLE_KEY, HeaderStyle.BOTH);
	    putClientProperty(PlasticLookAndFeel.BORDER_STYLE_KEY, BorderStyle.SEPARATOR);
	    setFloatable(false);
	    putClientProperty("JToolBar.isRollover", Boolean.TRUE);

	    add(_connect);
	    addSeparator();
	    add(_save);
	    add(_discard);
	    addSeparator();
	    add(_copy);
	    add(_paste);
	    addSeparator();
	    add(_delete);
	    addSeparator();
	    add(new JToggleButton(_substituteVar));
	}
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

    void applicationAdded(int serial, ApplicationDescriptor desc)
    {
	if(proceedWithUpdate(serial))
	{
	    _root.applicationAdded(desc);
	    _statusBar.setText("Connected; application '" 
			       + desc.name + "' added (" + serial + ")");

	    checkWriteSerial();
	}
    }

    void applicationRemoved(int serial, String name)
    {
	if(proceedWithUpdate(serial))
	{
	    _root.applicationRemoved(name);
	    _statusBar.setText("Connected; application '" 
			       + name + "' removed (" + serial + ")"); 
	    checkWriteSerial();
	}
    }

    void applicationUpdated(int serial, ApplicationUpdateDescriptor desc)
    {
	if(proceedWithUpdate(serial))
	{
	    TreePath path = null;
	    CommonBase currentNode = (CommonBase)_tree.getLastSelectedPathComponent();
	    if(currentNode != null)
	    {
		path = currentNode.getPath();
	    }
	    _root.applicationUpdated(desc);
	    _statusBar.setText("Connected; application '" 
			       + desc.name + "' updated (" + serial + ")");
	    checkWriteSerial();
	    restore(path);
	}
    }
    
    boolean proceedWithUpdate(int serial)
    {
	if(serial <= _latestSerial)
	{
	    //
	    // Ignore old messages
	    //
	    return false;
	}
	else if(serial == _latestSerial + 1)
	{
	    boolean doIt = (_latestSerial == -1 || _writeSerial == -1 || 
			    _writeSerial > _latestSerial);
	    
	    if(_writeSerial == _latestSerial)
	    {
		_writeSerial = serial; 
	    }
	    _latestSerial = serial;
	    return doIt;
	}
	else
	{
	    _sessionKeeper.sessionLost(
		"Received message from registry out of order");
	    return false;
	}
    }

    private void checkWriteSerial()
    {
	if(_writeSerial != -1 && _writeSerial == _latestSerial)
	{
	    updateInProgress();
	}
    }
    

    public CommonBase getSelectedNode()
    {
	return (CommonBase)_tree.getLastSelectedPathComponent();
    }

    public CommonBase findNewNode(TreePath path)
    {
	CommonBase node = _root;
	for(int i = 1; i < path.getPathCount(); ++i)
	{
	    CommonBase child = node.findChild(((CommonBase)path.getPathComponent(i)).getId());
	    if(child == null)
	    {
		break;
	    }
	    else
	    {
		node = child;
	    }
	}
	return node;
    }

    public void setSelectionPath(TreePath path)
    {
	_tree.setSelectionPath(path);
    }

    public void refreshDisplay()
    {
	CommonBase currentNode = (CommonBase)_tree.getLastSelectedPathComponent();
	if(currentNode != null)
	{
	    currentNode.displayProperties();
	}
    }

    public boolean displayEnabled()
    {
	return _displayEnabled;
    }

    public void enableDisplay()
    {
	_displayEnabled = true;
    }
    
    public void disableDisplay()
    {
	_displayEnabled = false;
    }
    
    private void restore(TreePath path)
    {
	if(path != null)
	{
	    CommonBase newNode = findNewNode(path);
	    _tree.setSelectionPath(newNode.getPath());
	    newNode.displayProperties();
	}
    }

    public boolean canUpdate()
    {
	if(_writeSerial == _latestSerial)
	{
	    return true;
	}
	
	try
	{
	    _writeSerial = _sessionKeeper.getSession().startUpdate();
	}
	catch(AccessDeniedException e)
	{
	    _writeSerial = -1;
	    accessDenied(e);
	    return false;
	}
	
	if(_writeSerial > _latestSerial)
	{
	    //
	    // Warn user that his updates were lost
	    //
	    JOptionPane.showMessageDialog(
		_mainFrame,
		"Your view was not up-to-date;"
		+ " you now have exclusive write-access to the registry, however your previous changes were lost.",
		"Concurrent update",
		JOptionPane.WARNING_MESSAGE);
	    _mainFrame.setCursor(Cursor.getPredefinedCursor(Cursor.WAIT_CURSOR));
	    return false;
	}
	else
	{
	    updateInProgress();
	    return true;
	}
    }

    private void accessDenied(AccessDeniedException e)
    {
	JOptionPane.showMessageDialog(
	    _mainFrame,
	    "Another session (username = " + e.lockUserId + ") has exclusive write access to the registry",
	    "Access Denied",
	    JOptionPane.ERROR_MESSAGE);

    }

    private boolean saveUpdates()
    {
	assert _writeSerial == _latestSerial;
	_mainFrame.setCursor(Cursor.getPredefinedCursor(Cursor.WAIT_CURSOR));
	
	try
	{
	    for(int i = 0; i < _root.getChildCount(); ++i)
	    {
		Application application = (Application)_root.getChildAt(i);
		
		try
		{
		    if(application.isNew())
		    {
			_sessionKeeper.getSession().addApplication(application.getDescriptor());
			application.commit();
		    }
		    else
		    {
			ApplicationUpdateDescriptor updateDescriptor =
			    application.createUpdateDescriptor();
			if(updateDescriptor != null)
			{
			    _sessionKeeper.getSession().updateApplication(updateDescriptor);
			    application.commit();
			}
		    }
		    
		    //
		    // Will ignore these updates
		    //
		    _latestSerial++;
		    _writeSerial = _latestSerial;
		}
		catch(AccessDeniedException e)
		{
		    // Very unlikely
		    accessDenied(e);
		    return false;
		}
		catch(ApplicationNotExistException e)
		{
		    // Most likely an IceGrid bug, so no need to optimize!
		    //
		    JOptionPane.showMessageDialog(
			_mainFrame,
			"Can't update Application " + e.name + ": it does not exist!",
			"Application does not exist",
			JOptionPane.ERROR_MESSAGE);
		    return false;
		}
		catch(DeploymentException e)
		{
		    JOptionPane.showMessageDialog(
			_mainFrame,
			"Application " + application.getId() + ": "+ e.reason,
			"Deployment Exception",
			JOptionPane.ERROR_MESSAGE);
		    return false;
		}
	    }
	    
	    //
	    // Update completed entirely
	    //
	    try
	    {
		_sessionKeeper.getSession().finishUpdate();
	    }
	    catch(AccessDeniedException e)
	    {
		//
		// Very unlikely; we still consider the update a success.
		//
		accessDenied(e);
	    }
	    
	    _writeSerial = -1;
	    _save.setEnabled(false);
	    _discard.setEnabled(false);
	    _statusBar.setText("Saving complete");
	    return true;

	}
	finally
	{
	    _mainFrame.setCursor(Cursor.getPredefinedCursor(Cursor.DEFAULT_CURSOR));
	}
    }
    
    private void discardUpdates(boolean showDialog)
    {
	assert _writeSerial == _latestSerial;
	//
	// Reestablish session, since we don't keep the old data around
	//
	_mainFrame.setCursor(Cursor.getPredefinedCursor(Cursor.WAIT_CURSOR));
	
	try
	{
	    _sessionKeeper.reconnect(showDialog);
	    _save.setEnabled(false);
	    _discard.setEnabled(false);
	}
	finally
	{
	    _mainFrame.setCursor(Cursor.getPredefinedCursor(Cursor.DEFAULT_CURSOR));
	}
    }


    private void updateInProgress()
    {
	_statusBar.setText("Update in progress");
	_mainFrame.setCursor(Cursor.getPredefinedCursor(Cursor.DEFAULT_CURSOR));
	_save.setEnabled(true);
	_discard.setEnabled(true);
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
	_writeSerial = -1;
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
	
	CommonBase node = (CommonBase)_tree.getLastSelectedPathComponent();
	if(node != null)
	{
	    node.displayProperties();
	}
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

    public JFrame getMainFrame()
    {
	return _mainFrame;
    }


    Model(JFrame mainFrame, String[] args, Preferences prefs, StatusBar statusBar)
    {	
	_mainFrame = mainFrame;
	_communicator = Ice.Util.initialize(args);
	_prefs = prefs;
	_statusBar = statusBar;
	
	_root = new Root(this);
	_treeModel = new TreeModelI(_root);

	//
	// Actions
	//
	_connect = new AbstractAction("Connect...", Utils.getIcon("/icons/connect.gif"))
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    if(_latestSerial != -1 && _latestSerial == _writeSerial)
		    {
			int saveFirst = JOptionPane.showConfirmDialog(
			    _mainFrame,
			    "Do you want to save your updates?", 
			    "Save Confirmation",
			    JOptionPane.YES_NO_CANCEL_OPTION);
			switch(saveFirst)
			{
			    case JOptionPane.YES_OPTION:
				if(saveUpdates())
				{
				    _sessionKeeper.createSession(false);
				}
				break;
			    case JOptionPane.NO_OPTION:
				discardUpdates(true);
				break;
			    case JOptionPane.CANCEL_OPTION:
				break;
			    default:
				assert false;
			}
		    }
		    else
		    {
			_sessionKeeper.reconnect(true);
		    }
		}
	    };
	_connect.putValue(Action.ACCELERATOR_KEY, KeyStroke.getKeyStroke("ctrl N"));    

	_save = new AbstractAction("Save", Utils.getIcon("/icons/save_edit.gif"))
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    saveUpdates();
		}
	    };
	_save.setEnabled(false);
		
	_discard = new AbstractAction("Discard all updates...", 
					    Utils.getIcon("/icons/undo_edit.gif"))
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    discardUpdates(false);
		}
	    };
	_discard.setEnabled(false);

	_exit = new AbstractAction("Exit")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    exit(0);
		}
	    };
	_exit.putValue(Action.ACCELERATOR_KEY, KeyStroke.getKeyStroke("alt F4"));

	_copy = new AbstractAction("Copy", Utils.getIcon("/icons/copy_edit.gif"))
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    CommonBase currentNode = (CommonBase)_tree.getLastSelectedPathComponent();
		    if(currentNode != null)
		    {
			_copiedDescriptor = currentNode.copy();
			if(_copiedDescriptor != null)
			{
			    _paste.setEnabled(true);
			}
		    }
		}
	    };
	_copy.putValue(Action.ACCELERATOR_KEY, KeyStroke.getKeyStroke("ctrl C"));
	
	_paste = new AbstractAction("Paste", Utils.getIcon("/icons/paste_edit.gif"))
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    CommonBase currentNode = (CommonBase)_tree.getLastSelectedPathComponent();
		    if(currentNode != null)
		    {
			currentNode.paste(_copiedDescriptor);
		    }
		}
	    };
	_paste.putValue(Action.ACCELERATOR_KEY, KeyStroke.getKeyStroke("ctrl V"));
	_paste.setEnabled(false);

	_delete = new AbstractAction("Delete", Utils.getIcon("/icons/delete_edit.gif"))
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    CommonBase currentNode = (CommonBase)_tree.getLastSelectedPathComponent();
		    if(currentNode != null)
		    {
			CommonBase parent = currentNode.getParent();
			CommonBase toSelect = null;
			if(parent != null)
			{
			    int index = parent.getIndex(currentNode);
			    toSelect = (CommonBase)parent.getChildAt(index + 1);
			    if(toSelect == null)
			    {
				if(index > 0)
				{
				    toSelect = (CommonBase)parent.getChildAt(0);
				}
				else
				{
				    toSelect = parent;
				}
			    }
			}
			if(toSelect != null)
			{
			    disableDisplay();
			}
			boolean destroyed = currentNode.destroy();
			if(toSelect != null)
			{
			    enableDisplay();
			    if(destroyed)
			    {
				toSelect = findNewNode(toSelect.getPath());
				_tree.setSelectionPath(toSelect.getPath()); 
			    }
			}
		    }
		}
	    };
	_delete.putValue(Action.ACCELERATOR_KEY, KeyStroke.getKeyStroke("DELETE"));    

	_substituteVar = new AbstractAction("${}")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    toggleSubstitute();
		}
	    };

	_substituteVar.putValue(Action.SHORT_DESCRIPTION, 
				     "Substitute variables and parameters in servers' properties");
    
	_about = new AbstractAction("About...")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    // TODO: implement
		}
	    };
    }

    void showMainFrame()
    {
	if(!loadWindowPrefs())
	{
	    _mainFrame.setLocation(100, 100);
	    _mainFrame.pack();
	}
	_mainFrame.setVisible(true);
    }

    void exit(int status)
    {
	storeWindowPrefs();
	try
	{
	    _communicator.destroy();
	}
	catch(Ice.LocalException e)
	{
	    // TODO: log error
	}
	_mainFrame.dispose();
	Runtime.getRuntime().exit(status);
    }


    private boolean loadWindowPrefs()
    {
	try
	{
	    if(!_prefs.nodeExists("Window"))
	    {
		return false;
	    }
	}
	catch(BackingStoreException e)
	{
	    return false;
	}

	Preferences windowPrefs = _prefs.node("Window");
	int x = windowPrefs.getInt("x", 0);
	int y = windowPrefs.getInt("y", 0);
	int width = windowPrefs.getInt("width", 0);
	int height = windowPrefs.getInt("height", 0);
	_mainFrame.setBounds(new Rectangle(x, y, width, height));
	if(windowPrefs.getBoolean("maximized", false))
	{
	    _mainFrame.setExtendedState(Frame.MAXIMIZED_BOTH);
	}
	return true;
    }

    private void storeWindowPrefs()
    {
	Preferences windowPrefs = _prefs.node("Window");
	Rectangle rect = _mainFrame.getBounds();
	windowPrefs.putInt("x", rect.x);
	windowPrefs.putInt("y", rect.y);
	windowPrefs.putInt("width", rect.width);
	windowPrefs.putInt("height", rect.height);
	windowPrefs.putBoolean("maximized",
			       _mainFrame.getExtendedState() == Frame.MAXIMIZED_BOTH);
    }


    SessionKeeper getSessionKeeper()
    {
	if(_sessionKeeper == null)
	{
	    _sessionKeeper = new SessionKeeper(this);
	}
	return _sessionKeeper;
    }

    JToolBar createToolBar()
    {
	return new ToolBar();
    }

    JMenuBar createMenuBar()
    {
	return new MenuBar();
    }

    Preferences getPrefs()
    {
	return _prefs;
    }

    private Ice.Communicator _communicator;
    private Preferences _prefs;
    private StatusBar _statusBar;
    private SessionManagerPrx _sessionManager;
    private AdminPrx _admin;

    private Root _root;
    private TreeModelI _treeModel;

    private int _latestSerial = -1;
    private int _writeSerial = -1;

    private JTree _tree;

    private boolean _substitute = false;
    
    private boolean _displayEnabled = true;

    private SimpleInternalFrame _propertiesFrame;

    private JFrame _mainFrame;
    private SessionKeeper _sessionKeeper;

    private Object _copiedDescriptor;

    //
    // Actions
    //
    private Action _connect;
    private Action _save;
    private Action _discard;
    private Action _exit;
    private Action _copy;
    private Action _paste;
    private Action _delete;
    private Action _about;
    private Action _substituteVar;
}
