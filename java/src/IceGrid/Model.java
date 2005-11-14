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

import java.awt.BorderLayout;
import java.awt.Cursor;
import java.awt.Dimension;
import java.awt.Frame;
import java.awt.Rectangle;
import java.awt.Container;
import java.awt.Component;
import java.awt.Toolkit;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;
import java.awt.event.KeyEvent;

import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.ButtonGroup;
import javax.swing.JButton;
import javax.swing.JCheckBoxMenuItem;
import javax.swing.JEditorPane;
import javax.swing.JFrame;
import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JOptionPane;
import javax.swing.JScrollPane;
import javax.swing.JToggleButton;
import javax.swing.JToolBar;
import javax.swing.JTree;
import javax.swing.KeyStroke;
import javax.swing.SwingUtilities;

import javax.swing.event.TreeSelectionEvent;
import javax.swing.event.TreeSelectionListener;
import javax.swing.tree.TreeModel;
import javax.swing.tree.TreePath;

import com.jgoodies.looks.Options;
import com.jgoodies.looks.HeaderStyle;
import com.jgoodies.looks.BorderStyle;
import com.jgoodies.looks.plastic.PlasticLookAndFeel;

import IceGrid.TreeNode.Application;
import IceGrid.TreeNode.CommonBase;
import IceGrid.TreeNode.Editor;
import IceGrid.TreeNode.Root;

//
// The class that holds all the data and more!
//

public class Model
{
    private class SelectionListener implements TreeSelectionListener
    {
	public void valueChanged(TreeSelectionEvent e)
	{
	    if(_selectionListenerEnabled)
	    {
		TreePath path = null;
		if(e.isAddedPath())
		{
		    path = e.getPath();
		}
		
		if(path == null)
		{
		    show(_root);
		}
		else
		{
		    CommonBase newNode = (CommonBase)path.getLastPathComponent();
		    assert newNode.getParent() != null;
		    
		    if(_currentNode != null)
		    {
			if(_currentNode.isEphemeral())
			{
			    _currentNode.destroy();
			}
			else if(_currentNode.getParent() != null)
			{
			    _previousNodes.add(_currentNode);
			    while(_previousNodes.size() >= HISTORY_MAX_SIZE)
			    {
				_previousNodes.removeFirst();
			    }
			    _back.setEnabled(true);
			}
		    }
		    _nextNodes.clear();
		    _forward.setEnabled(false);

		    _currentNode = newNode;
		    show(_currentNode);
		}
	    }
	}
    }

    private class MenuBar extends JMenuBar
    {
	private MenuBar()
	{
	    putClientProperty(Options.HEADER_STYLE_KEY, HeaderStyle.BOTH);
	    putClientProperty(PlasticLookAndFeel.BORDER_STYLE_KEY, BorderStyle.SEPARATOR);

	    //
	    // File menu
	    // 
	    JMenu fileMenu = new JMenu("File");
	    fileMenu.setMnemonic(java.awt.event.KeyEvent.VK_F);
	    add(fileMenu);

	    //
	    // New sub-menu
	    //
	    _newMenu = new JMenu("New");
	    _newMenu.setEnabled(false);
	    fileMenu.add(_newMenu);
	    _newMenu.add(_newApplicationWithDefaultTemplates);
	    _newMenu.add(_newApplication);
	    _newMenu.addSeparator();
	    _newMenu.add(_actions[CommonBase.NEW_ADAPTER]);
	    _newMenu.add(_actions[CommonBase.NEW_DBENV]);
	    _newMenu.add(_actions[CommonBase.NEW_NODE]);
	    _newMenu.add(_actions[CommonBase.NEW_REPLICA_GROUP]);
	    
	    //
	    // New server sub-sub-menu
	    //
	    _newServerMenu = new JMenu("Server");
	    _newServerMenu.setEnabled(false);
	    _newMenu.add(_newServerMenu);
	    _newServerMenu.add(_actions[CommonBase.NEW_SERVER]);
	    _newServerMenu.add(_actions[CommonBase.NEW_SERVER_ICEBOX]);
	    _newServerMenu.add(_actions[CommonBase.NEW_SERVER_FROM_TEMPLATE]);
	    
	    //
	    // New service sub-sub-menu
	    //
	    _newServiceMenu = new JMenu("Service");
	    _newServiceMenu.setEnabled(false);
	    _newMenu.add(_newServiceMenu);
	    _newServiceMenu.add(_actions[CommonBase.NEW_SERVICE]);
	    _newServiceMenu.add(_actions[CommonBase.NEW_SERVICE_FROM_TEMPLATE]);

	    //
	    // New template sub-sub-menu
	    //
	    _newTemplateMenu = new JMenu("Template");
	    _newTemplateMenu.setEnabled(false);
	    _newMenu.add(_newTemplateMenu);
	    _newTemplateMenu.add(_actions[CommonBase.NEW_TEMPLATE_SERVER]);
	    _newTemplateMenu.add(_actions[CommonBase.NEW_TEMPLATE_SERVER_ICEBOX]);
	    _newTemplateMenu.add(_actions[CommonBase.NEW_TEMPLATE_SERVICE]);
	    
	    fileMenu.addSeparator();
	    fileMenu.add(_login);
	    fileMenu.add(_logout);
	    fileMenu.addSeparator();
	    fileMenu.add(_save);
	    fileMenu.add(_discard);
	    fileMenu.addSeparator();
	    fileMenu.add(_exit);
	   
	    //
	    // Edit menu
	    //
	    JMenu editMenu = new JMenu("Edit");
	    editMenu.setMnemonic(java.awt.event.KeyEvent.VK_E);
	    add(editMenu);
	    editMenu.add(_actions[CommonBase.COPY]);
	    editMenu.add(_actions[CommonBase.PASTE]);
	    editMenu.addSeparator();
	    editMenu.add(_actions[CommonBase.DELETE]);

	    //
	    // View menu
	    //
	    JMenu viewMenu = new JMenu("View");
	    viewMenu.setMnemonic(java.awt.event.KeyEvent.VK_V);
	    add(viewMenu);
	    viewMenu.add(_showVarsMenuItem);
	    viewMenu.add(_substituteMenuItem);
	    viewMenu.addSeparator();
	    viewMenu.add(_back);
	    viewMenu.add(_forward);

	    //
	    // Tools menu
	    //
	    JMenu toolsMenu = new JMenu("Tools");
	    toolsMenu.setMnemonic(java.awt.event.KeyEvent.VK_T);
	    add(toolsMenu);
	    
	    //
	    // Application sub-menu
	    //
	    _appMenu = new JMenu("Application");
	    _appMenu.setEnabled(false);
	    toolsMenu.add(_appMenu);
	    _appMenu.add(_actions[CommonBase.APPLICATION_INSTALL_DISTRIBUTION]);
	 
	    //
	    // Node sub-menu
	    //
	    _nodeMenu = new JMenu("Node");
	    _nodeMenu.setEnabled(false);
	    toolsMenu.add(_nodeMenu);
	    _nodeMenu.add(_actions[CommonBase.SHUTDOWN_NODE]);

	    //
	    // Server sub-menu
	    //
	    _serverMenu = new JMenu("Server");
	    _serverMenu.setEnabled(false);
	    toolsMenu.add(_serverMenu);
	    _serverMenu.add(_actions[CommonBase.START]);
	    _serverMenu.add(_actions[CommonBase.STOP]);
	    _serverMenu.addSeparator();
	    _serverMenu.add(_actions[CommonBase.ENABLE]);
	    _serverMenu.add(_actions[CommonBase.DISABLE]);
	    _serverMenu.addSeparator();
	    _serverMenu.add(_actions[CommonBase.SERVER_INSTALL_DISTRIBUTION]);

	    //
	    // Service sub-menu
	    //
	    _serviceMenu = new JMenu("Service");
	    _serviceMenu.setEnabled(false);
	    toolsMenu.add(_serviceMenu);
	    _serviceMenu.add(_actions[CommonBase.MOVE_UP]);
	    _serviceMenu.add(_actions[CommonBase.MOVE_DOWN]);

	    //
	    // Help menu
	    //
	    JMenu helpMenu = new JMenu("Help");
	    helpMenu.setMnemonic(java.awt.event.KeyEvent.VK_H);
	    add(helpMenu);

	    helpMenu.add(_helpContents);
	    if(GPL_BUILD)
	    {
		helpMenu.addSeparator();
		helpMenu.add(_copying);
		helpMenu.add(_warranty);
	    }

	    helpMenu.addSeparator();
	    helpMenu.add(_about);
	}
    }


    private class ToolBar extends JToolBar
    {
	private ToolBar()
	{
	    putClientProperty(Options.HEADER_STYLE_KEY, HeaderStyle.BOTH);
	    putClientProperty(PlasticLookAndFeel.BORDER_STYLE_KEY, BorderStyle.SEPARATOR);
	    setFloatable(false);
	    putClientProperty("JToolBar.isRollover", Boolean.TRUE);

	    JButton button = new JButton(_login);
	    button.setText(null);
	    button.setIcon(Utils.getIcon("/icons/24x24/login.png"));
	    add(button);

	    addSeparator();
	   	    
	    button = new JButton(_back);
	    button.setText(null);
	    button.setIcon(Utils.getIcon("/icons/24x24/back.png"));
	    add(button);
	    button = new JButton(_forward);
	    button.setText(null);
	    button.setIcon(Utils.getIcon("/icons/24x24/forward.png"));
	    add(button);

	    addSeparator();
	    
	    button = new JButton(_save);
	    button.setText(null);
	    button.setIcon(Utils.getIcon("/icons/24x24/save.png"));
	    add(button);
	    button = new JButton(_discard);
	    button.setText(null);
	    button.setIcon(Utils.getIcon("/icons/24x24/discard.png"));
	    add(button);

	    addSeparator();

	    button = new JButton(_actions[CommonBase.COPY]);
	    button.setText(null);
	    button.setIcon(Utils.getIcon("/icons/24x24/copy.png"));
	    add(button);
	    button = new JButton(_actions[CommonBase.PASTE]);
	    button.setText(null);
	    button.setIcon(Utils.getIcon("/icons/24x24/paste.png"));
	    add(button);
	    button = new JButton(_actions[CommonBase.DELETE]);
	    button.setText(null);
	    button.setIcon(Utils.getIcon("/icons/24x24/delete.png"));
	    add(button);

	    addSeparator();

	    add(_showVarsTool);
	    add(_substituteTool);
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
	    _statusBar.setText(
		"Last update: new application '" + desc.name + "'");
	    checkWriteSerial();
	}
    }

    void applicationRemoved(int serial, String name)
    {
	if(proceedWithUpdate(serial))
	{
	    _root.applicationRemoved(name);
	    _statusBar.setText(
		"Last update: application '" + name + "' was removed");
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
	    _statusBar.setText("Last update: application  '" + desc.name + "' was updated");
	    checkWriteSerial();
	    restore(path);
	}
    }
    
    boolean proceedWithUpdate(int serial)
    {
	if(_latestSerial == -1 || serial <= _latestSerial)
	{
	    //
	    // Ignore old messages and messages when we're logged out
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
		"Received message from IceGrid Registry out of order");
	    return false;
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
	CommonBase currentNode = getSelectedNode();
	if(currentNode != null)
	{
	    show(currentNode);
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
	    if(newNode == getSelectedNode())
	    {
		refreshDisplay();
	    }
	    else
	    {
		_tree.setSelectionPath(newNode.getPath());
	    }
	}
    }

    public boolean canUpdate()
    {
	if(_latestSerial == -1)
	{
	    JOptionPane.showMessageDialog(
		_mainFrame,
		"Updates are not allowed while working offline",
		"Working Offline",
		JOptionPane.ERROR_MESSAGE);
	    return false;
	}

	if(isUpdateInProgress())
	{
	    return true;
	}
	
	try
	{
	    _mainFrame.setCursor(Cursor.getPredefinedCursor(Cursor.WAIT_CURSOR));
	    _writeSerial = _sessionKeeper.getSession().startUpdate();
	}
	catch(AccessDeniedException e)
	{
	    _writeSerial = -1;
	    accessDenied(e);
	    return false;
	}
	finally
	{
	    _mainFrame.setCursor(Cursor.getPredefinedCursor(Cursor.DEFAULT_CURSOR));
	}
	showActions();

	if(_writeSerial > _latestSerial)
	{
	    //
	    // Warn user that his updates were lost
	    //
	    JOptionPane.showMessageDialog(
		_mainFrame,
		"Your view was not up-to-date;"
		+ " you now have exclusive write-access to the" +
		" IceGrid Registry, however your previous changes were lost.",
		"Concurrent update",
		JOptionPane.WARNING_MESSAGE);

	    //
	    // TODO: wait for my view to be up to date
	    //
	    return false;
	}
	else
	{
	    setUpdateInProgress();
	    return true;
	}
    }

    public boolean isUpdateInProgress()
    {
	return _writeSerial != -1 && 
	    _writeSerial >= _latestSerial;
    }


    private void checkWriteSerial()
    {
	if(_writeSerial != -1 && _writeSerial == _latestSerial)
	{
	    setUpdateInProgress();
	}
    }
    private void setUpdateInProgress()
    {
	_statusBar.setText("Update in progress");
	_save.setEnabled(true);
	_discard.setEnabled(true);
    }

    private void accessDenied(AccessDeniedException e)
    {
	JOptionPane.showMessageDialog(
	    _mainFrame,
	    "Another session (username = " + e.lockUserId + ") has exclusive write access to the registry",
	    "Access Denied",
	    JOptionPane.ERROR_MESSAGE);

    }

    
    public boolean removeApplication(String applicationName)
    {
	assert _writeSerial == _latestSerial;
	_mainFrame.setCursor(Cursor.getPredefinedCursor(Cursor.WAIT_CURSOR));

	try
	{
	    _sessionKeeper.getSession().removeApplication(applicationName);
	    return true;
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
		"Can't remove Application '" + e.name + "': it does not exist!",
		"Application does not exist",
		JOptionPane.ERROR_MESSAGE);
	    return true;
	}
	finally
	{
	    _mainFrame.setCursor(Cursor.getPredefinedCursor(Cursor.DEFAULT_CURSOR));
	}
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
			_sessionKeeper.getSession().addApplication(
			    (ApplicationDescriptor)application.getDescriptor());
			application.commit();
			//
			// Will ignore this update
			//
			_latestSerial++;
			_writeSerial = _latestSerial;
		    }
		    else
		    {
			ApplicationUpdateDescriptor updateDescriptor =
			    application.createUpdateDescriptor();
			if(updateDescriptor != null)
			{
			    _sessionKeeper.getSession().updateApplication(updateDescriptor);
			    application.commit();
			    //
			    // Will ignore this update
			    //
			    _latestSerial++;
			    _writeSerial = _latestSerial;
			}
		    }
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
			"Can't update Application '" + e.name + "': it does not exist!",
			"Application does not exist",
			JOptionPane.ERROR_MESSAGE);
		    return false;
		}
		catch(DeploymentException e)
		{
		    JOptionPane.showMessageDialog(
			_mainFrame,
			"Application '" + application.getId() + "': "+ e.reason,
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
	    showActions();

	    _statusBar.setText("Saving complete");
	    return true;
	}
	finally
	{
	    _mainFrame.setCursor(Cursor.getPredefinedCursor(Cursor.DEFAULT_CURSOR));
	}
    }
    
    
    private void discardUpdates()
    {
	assert _writeSerial == _latestSerial;

	int reallyDiscard = JOptionPane.showConfirmDialog(
	    _mainFrame,
	    "Do you want to discard all your updates?", 
	    "Discard Confirmation",
	    JOptionPane.YES_NO_OPTION);
	
	if(reallyDiscard == JOptionPane.YES_OPTION)
	{
	    _sessionKeeper.relog(false);
	}
    }


    //
    // From the Node observer:
    //

    void nodeUp(NodeDynamicInfo updatedInfo)
    {
	_root.nodeUp(updatedInfo);
	showActions();
	refreshCurrentStatus();
    }

    void nodeDown(String node)
    {
	_root.nodeDown(node);
	showActions();
	refreshCurrentStatus();
    }

    void updateServer(String node, ServerDynamicInfo updatedInfo)
    {
	_root.updateServer(node, updatedInfo);
	showActions();
	refreshCurrentStatus();
    }

    void updateAdapter(String node, AdapterDynamicInfo updatedInfo)
    {
	_root.updateAdapter(node, updatedInfo);
	showActions();
	refreshCurrentStatus();
    }

    //
    // Other methods
    //
    void sessionLost()
    {
	_latestSerial = -1;
	_writeSerial = -1;
	_admin = null;
	showActions();
	_logout.setEnabled(false);
    }
    
    SessionPrx login(SessionKeeper.LoginInfo info, Component parent)
    {	
	_root.clear();
	_newApplication.setEnabled(false);
	_newMenu.setEnabled(false);
	_newApplicationWithDefaultTemplates.setEnabled(false);

	_previousNodes.clear();
	_nextNodes.clear();
	_currentNode = null;
	_back.setEnabled(false);
	_forward.setEnabled(false);

	SessionPrx session = null;
	
	if(_routedAdapter != null)
	{
	    //
	    // Clean it up!
	    //
	    _routedAdapter.removeRouter(_communicator.getDefaultRouter());
	    _routedAdapter.deactivate();
	    _routedAdapter.waitForDeactivate();
	    _routedAdapter = null;
	}

	_communicator.setDefaultRouter(null);
	_communicator.setDefaultLocator(null);

	if(info.routed)
	{
	    //
	    // Router
	    //
	    
	    String str = info.routerInstanceName  + "/router";
	    if(!info.routerEndpoints.equals(""))
	    {
		str += ":" + info.routerEndpoints;
	    }
	    Glacier2.RouterPrx router = Glacier2.RouterPrxHelper.
		uncheckedCast(_communicator.stringToProxy(str));
	    
	    //
	    // The session must be routed through this router
	    //
	    _communicator.setDefaultRouter(router);
	    try
	    {
		Glacier2.SessionPrx s =
		    router.createSession(
			info.routerUsername, new String(info.routerPassword));
		
		session = SessionPrxHelper.uncheckedCast(s);
	    }
	    catch(Glacier2.PermissionDeniedException e)
	    {
		JOptionPane.showMessageDialog(parent,
					      "Permission denied: "
					      + e.reason,
					      "Login failed",
					      JOptionPane.ERROR_MESSAGE);
		return null;
	    }
	    catch(Glacier2.CannotCreateSessionException e)
	    {
		JOptionPane.showMessageDialog(parent,
					      "Could not create session: "
					      + e.reason,
					      "Login failed",
					      JOptionPane.ERROR_MESSAGE);
		return null;
	    }
	    catch(Ice.LocalException e)
	    {
		JOptionPane.showMessageDialog(parent,
					      "Could not create session: "
					      + e.toString(),
					      "Login failed",
					      JOptionPane.ERROR_MESSAGE);
		return null;
	    }
	}
	else
	{
	    //
	    // The client uses the locator only without routing
	    //
	    String str = info.registryInstanceName + "/Locator";
	    if(!info.registryEndpoints.equals(""))
	    {
		str += ":" + info.registryEndpoints;
	    }
	   
	    try
	    {
		Ice.LocatorPrx defaultLocator = Ice.LocatorPrxHelper.
		    checkedCast(_communicator.stringToProxy(str));
		_communicator.setDefaultLocator(defaultLocator);
	    }
	    catch(Ice.LocalException e)
	    {
		JOptionPane.showMessageDialog(
		    parent,
		    "Could not contact '" + str + "': " + e.toString(),
		    "Login failed",
		    JOptionPane.ERROR_MESSAGE);
		return null;
	    }

	    //
	    // Local session
	    //
	    str = info.registryInstanceName + "/SessionManager";
		
	    SessionManagerPrx sessionManager = SessionManagerPrxHelper.
		uncheckedCast(_communicator.stringToProxy(str));
	    
	    try
	    {
		session = sessionManager.createLocalSession(info.registryUsername);
	    }
	    catch(Ice.LocalException e)
	    {
		JOptionPane.showMessageDialog(parent,
					      "Could not create session: "
					      + e.toString(),
					      "Login failed",
					      JOptionPane.ERROR_MESSAGE);
		return null;
	    }
	}
	    
	//
	// Admin
	//
	try
	{
	    _admin = session.getAdmin();
	}
	catch(Ice.LocalException e)
	{
	    JOptionPane.showMessageDialog(
		parent,
		"Could not retrieve Admin proxy: " + e.toString(),
		"Login failed",
		JOptionPane.ERROR_MESSAGE);
	    destroySession(session);
	    return null;
	}
  
	_newApplication.setEnabled(true);
	_newApplicationWithDefaultTemplates.setEnabled(true);
	_newMenu.setEnabled(true);
	_logout.setEnabled(true);

	return session;
    }

    Ice.ObjectAdapter getObjectAdapter()
    {
	Ice.RouterPrx router = _communicator.getDefaultRouter();
	
	if(router == null)
	{
	    if(_localAdapter == null)
	    {
		_localAdapter = 
		    _communicator.createObjectAdapter("IceGrid.AdminGUI");
		_localAdapter.activate();
	    }
	    return _localAdapter;
	}
	else
	{
	    if(_routedAdapter == null)
	    {
		//
		// Needs a unique name since we destroy this adapter at
		// each new login
		//
		String name = "RoutedAdapter-" + Ice.Util.generateUUID();

		_routedAdapter =
		    _communicator.createObjectAdapter(name);
		_routedAdapter.addRouter(router);
		_routedAdapter.activate();
	    }
	    return _routedAdapter;
	}
    }


    void destroySession(SessionPrx session)
    {
	Ice.RouterPrx router = _communicator.getDefaultRouter();

	try
	{
	    if(router == null)
	    {
		session.destroy();
	    }
	    else
	    {
		Glacier2.RouterPrx gr 
		    = Glacier2.RouterPrxHelper.uncheckedCast(router);
		gr.destroySession();
	    }
	}
	catch(Glacier2.SessionNotExistException e)
	{
	    // Ignored
	}
	catch(Ice.LocalException e)
	{
	    // Ignored
	}
    }

    private void showVars()
    {
	substitute(false);
    }

    private void substituteVars()
    {
	substitute(true);
    }

    private void substitute(boolean newValue)
    {
	if(_substitute != newValue)
	{
	    _substitute = newValue;

	    if(_substitute)
	    {
		_substituteMenuItem.setSelected(true);
		_substituteTool.setSelected(true);
	    }
	    else
	    {
		_showVarsMenuItem.setSelected(true);
		_showVarsTool.setSelected(true);
	    }
	    refreshDisplay();
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

    public StatusBar getStatusBar()
    {
	return _statusBar;
    }

    public void setTree(JTree tree)
    {
	_tree = tree;
	_tree.getActionMap().put("copy", _actions[CommonBase.COPY]);
	_tree.getActionMap().put("paste",  _actions[CommonBase.PASTE]);

	SelectionListener appSelectionListener = new SelectionListener();
	tree.addTreeSelectionListener(
	    new SelectionListener());

    }

    public JTree getTree()
    {
	return _tree;
    }

    public SimpleInternalFrame getPropertiesFrame()
    {
	return _propertiesFrame;
    }

    public SimpleInternalFrame getCurrentStatusFrame()
    {
	return _currentStatusFrame;
    }

    public JFrame getMainFrame()
    {
	return _mainFrame;
    }

    static private Ice.Communicator createCommunicator(String[] args)
    {
	//
	// TODO: work-around bug #542 
	//

	Ice.Properties properties = Ice.Util.createProperties();
	properties.setProperty("Ice.Override.ConnectTimeout", "5000");
	properties.setProperty("IceGrid.AdminGUI.Endpoints", "tcp -t 10000");
	
	//
	// For Glacier
	//
	properties.setProperty("Ice.ACM.Client", "0");
	properties.setProperty("Ice.MonitorConnections", "5");
	properties.setProperty("Ice.RetryIntervals", "-1");

	return Ice.Util.initializeWithProperties(args, properties);
    }

    Model(JFrame mainFrame, String[] args, Preferences prefs, StatusBar statusBar)
    {	
	_mainFrame = mainFrame;
	_prefs = prefs;
	_statusBar = statusBar;
	_communicator = createCommunicator(args);

	_root = new Root(this);
	_treeModel = new TreeModelI(_root);

	_licenseDialog = new LicenseDialog(_mainFrame);

	_shutdownHook = new Thread("Shutdown hook")
	    {
		public void run()
		{
		    destroyCommunicator();
		}
	    };
	Runtime.getRuntime().addShutdownHook(_shutdownHook);
	
	//
	// Fixed height for current status frame
	//
	Dimension prefSize = new Dimension(0, 160);
	Dimension maxSize = new Dimension(Short.MAX_VALUE, 160);
	_currentStatusFrame.setMinimumSize(prefSize);
	_currentStatusFrame.setPreferredSize(prefSize);
	_currentStatusFrame.setMaximumSize(maxSize);
	_currentStatusFrame.getTitleLabel().setEnabled(false);
	_propertiesFrame.getTitleLabel().setEnabled(false);

	final int MENU_MASK = Toolkit.getDefaultToolkit().getMenuShortcutKeyMask();

	//
	// Common actions (nodes not involved)
	//
	_newApplication = new AbstractAction("Application without default templates")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    newApplication();
		}
	    };
	_newApplication.setEnabled(false);

	_newApplicationWithDefaultTemplates = 
	    new AbstractAction("Application")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    newApplicationWithDefaultTemplates();
		}
	    };
	_newApplicationWithDefaultTemplates.setEnabled(false);

	_login = new AbstractAction("Login...")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    login();
		}
	    };
	_login.putValue(Action.SHORT_DESCRIPTION, 
			"Log into an IceGrid Registry");

	_logout = new AbstractAction("Logout")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    logout();
		}
	    };
	_logout.putValue(Action.SHORT_DESCRIPTION, "Logout");
	_logout.setEnabled(false);

	_save = new AbstractAction("Save")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    saveUpdates();
		}
	    };
	_save.setEnabled(false);
	_save.putValue(Action.ACCELERATOR_KEY, 
		       KeyStroke.getKeyStroke(KeyEvent.VK_S, MENU_MASK));
	_save.putValue(Action.SHORT_DESCRIPTION, "Save to IceGrid Registry");
	
	_discard = new AbstractAction("Discard all updates...")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    discardUpdates();
		}
	    };
	_discard.setEnabled(false);
	_discard.putValue(Action.SHORT_DESCRIPTION, "Discard all updates");

	_exit = new AbstractAction("Exit")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    exit(0);
		}
	    };
	_exit.putValue(Action.ACCELERATOR_KEY, KeyStroke.getKeyStroke("alt F4"));


	_back = new AbstractAction("Go back to the previous node")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    back();
		}
	    };
	_back.setEnabled(false);
	_back.putValue(Action.SHORT_DESCRIPTION, "Go back to the previous node");

	_forward =  new AbstractAction("Go to the next node")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    forward();
		}
	    };
	_forward.setEnabled(false);
	_forward.putValue(Action.SHORT_DESCRIPTION, "Go to the next node");

	_helpContents = new AbstractAction("Contents")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    helpContents();
		}	
	    };

	_copying = new AbstractAction("Copying conditions")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    _licenseDialog.show("TOP");
		}
	    };

	_warranty = new AbstractAction("(Non)Warranty")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    _licenseDialog.show("WARRANTY");
		}
	    };
		
	_about = new AbstractAction("About")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    about();
		}
	    };


	//
	// Actions implemented by the nodes
	//
	_actions = new Action[CommonBase.ACTION_COUNT];

	_actions[CommonBase.NEW_ADAPTER] = new AbstractAction("Adapter")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    _actionsTarget.newAdapter();
		}
	    };

	_actions[CommonBase.NEW_DBENV] = new AbstractAction("Database environment")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    _actionsTarget.newDbEnv();
		}
	    };
	
	_actions[CommonBase.NEW_NODE] = new AbstractAction("Node")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    _actionsTarget.newNode();
		}
	    };

	_actions[CommonBase.NEW_REPLICA_GROUP] = new AbstractAction("Replica group")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    _actionsTarget.newReplicaGroup();
		}
	    };

	_actions[CommonBase.NEW_SERVER] = new AbstractAction("Server")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    _actionsTarget.newServer();
		}
	    };

	_actions[CommonBase.NEW_SERVER_ICEBOX] = new AbstractAction("IceBox server")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    _actionsTarget.newServerIceBox();
		}
	    };

	_actions[CommonBase.NEW_SERVER_FROM_TEMPLATE] = new AbstractAction("Server from template")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    _actionsTarget.newServerFromTemplate();
		}
	    };


	_actions[CommonBase.NEW_SERVICE] = new AbstractAction("Service")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    _actionsTarget.newService();
		}
	    };

	_actions[CommonBase.NEW_SERVICE_FROM_TEMPLATE] = new AbstractAction("Service from template")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    _actionsTarget.newServiceFromTemplate();
		}
	    };

	_actions[CommonBase.NEW_TEMPLATE_SERVER] = new AbstractAction("Server template")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    _actionsTarget.newTemplateServer();
		}
	    };
	
	_actions[CommonBase.NEW_TEMPLATE_SERVER_ICEBOX] = new AbstractAction("IceBox server template")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    _actionsTarget.newTemplateServerIceBox();
		}
	    };


	_actions[CommonBase.NEW_TEMPLATE_SERVICE] = new AbstractAction("Service template")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    _actionsTarget.newTemplateService();
		}
	    };
	
	
	_actions[CommonBase.COPY] = new AbstractAction("Copy")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    _actionsTarget.copy();
		}
	    };
	_actions[CommonBase.COPY].putValue(Action.ACCELERATOR_KEY, 
					   KeyStroke.getKeyStroke(KeyEvent.VK_C, MENU_MASK));
	_actions[CommonBase.COPY].putValue(Action.SHORT_DESCRIPTION, "Copy");

	_actions[CommonBase.PASTE] = new AbstractAction("Paste")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    _actionsTarget.paste();
		}
	    };
	_actions[CommonBase.PASTE].putValue(Action.ACCELERATOR_KEY, 
					    KeyStroke.getKeyStroke(KeyEvent.VK_V, MENU_MASK));
	_actions[CommonBase.PASTE].putValue(Action.SHORT_DESCRIPTION, "Paste");

	_actions[CommonBase.DELETE] = new AbstractAction("Delete")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    _actionsTarget.delete();
		}
	    };
	_actions[CommonBase.DELETE].putValue(Action.ACCELERATOR_KEY, KeyStroke.getKeyStroke("DELETE"));
	_actions[CommonBase.DELETE].putValue(Action.SHORT_DESCRIPTION, "Delete");
   
	_actions[CommonBase.SHOW_VARS] = new 
	    AbstractAction("Show variables")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    showVars();
		}
	    };
	_actions[CommonBase.SHOW_VARS].putValue(
	    Action.SHORT_DESCRIPTION, 
	    "Show variables and parameters in the Properties pane");


	_actions[CommonBase.SUBSTITUTE_VARS] = new 
	    AbstractAction("Substitute variables")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    substituteVars();
		}
	    };
	
	_actions[CommonBase.SUBSTITUTE_VARS].putValue(
	    Action.SHORT_DESCRIPTION, 
	    "Substitute variables and parameters with their values in the Properties pane");


	_showVarsMenuItem = new
	    JCheckBoxMenuItem(_actions[CommonBase.SHOW_VARS]);
	_showVarsTool = new 
	    JToggleButton(_actions[CommonBase.SHOW_VARS]);
	_showVarsTool.setIcon(Utils.getIcon("/icons/24x24/show_vars.png"));
	_showVarsTool.setText("");

	_substituteMenuItem = new
	    JCheckBoxMenuItem(_actions[CommonBase.SUBSTITUTE_VARS]);
	_substituteTool = new 
	    JToggleButton(_actions[CommonBase.SUBSTITUTE_VARS]);
	_substituteTool.setIcon(Utils.getIcon("/icons/24x24/substitute.png"));
	_substituteTool.setText("");


	ButtonGroup group = new ButtonGroup();
	group.add(_showVarsMenuItem);
	group.add(_substituteMenuItem);
	group = new ButtonGroup();
	group.add(_showVarsTool);
	group.add(_substituteTool);

	_showVarsMenuItem.setSelected(true);
	_showVarsTool.setSelected(true);
	
	_actions[CommonBase.MOVE_UP] = new AbstractAction("Move up")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    _actionsTarget.moveUp();
		}
	    };
			   
	_actions[CommonBase.MOVE_DOWN] = new AbstractAction("Move down")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    _actionsTarget.moveDown();
		}
	    };

	_actions[CommonBase.START] = new AbstractAction(
	    "Start", Utils.getIcon("/icons/16x16/start.png"))
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    _actionsTarget.start();
		}
	    };
	_actions[CommonBase.START].putValue(Action.SHORT_DESCRIPTION, 
					    "Start this server");


	_actions[CommonBase.STOP] = new AbstractAction(
	    "Stop", Utils.getIcon("/icons/16x16/stop.png"))
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    _actionsTarget.stop();
		}
	    };
	_actions[CommonBase.STOP].putValue(Action.SHORT_DESCRIPTION, 
					    "Stop this server");
	

	_actions[CommonBase.ENABLE] = new AbstractAction(
	    "Enable", Utils.getIcon("/icons/16x16/enable.png"))
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    _actionsTarget.enable();
		}
	    };
	_actions[CommonBase.ENABLE].putValue(Action.SHORT_DESCRIPTION, 
					     "Enable this server");
	
	_actions[CommonBase.DISABLE] = new AbstractAction(
	    "Disable", Utils.getIcon("/icons/16x16/disable.png"))
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    _actionsTarget.disable();
		}
	    };
	_actions[CommonBase.DISABLE].putValue(Action.SHORT_DESCRIPTION, 
					      "Disable this server");

	_actions[CommonBase.SHUTDOWN_NODE] = new AbstractAction("Shutdown")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    _actionsTarget.shutdownNode();
		}
	    };
	_actions[CommonBase.APPLICATION_INSTALL_DISTRIBUTION] = 
	    new AbstractAction("Patch")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    _actionsTarget.applicationInstallDistribution();
		}
	    };
	_actions[CommonBase.SERVER_INSTALL_DISTRIBUTION] = 
	    new AbstractAction("Patch")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    _actionsTarget.serverInstallDistribution();
		}
	    };
    }

    void show(CommonBase node)
    {
	if(_displayEnabled)
	{
	    showActions(node);

	    _currentEditor = node.getEditor();

	    Ice.StringHolder title = new Ice.StringHolder();	  
	    Component currentStatus = _currentEditor.getCurrentStatus(title);
	    if(title.value == null)
	    {
		title.value = "Current status";
	    }
	    _currentStatusFrame.setTitle(title.value);

	    if(currentStatus == null)
	    {
		_currentStatusFrame.setToolBar(null);
		Component oldContent = _currentStatusFrame.getContent();
		if(oldContent != null)
		{
		    _currentStatusFrame.remove(oldContent);
		}
		_currentStatusFrame.getTitleLabel().setEnabled(false);
	    }
	    else
	    {
		_currentStatusFrame.setToolBar(
		    _currentEditor.getCurrentStatusToolBar());
		_currentStatusFrame.setContent(currentStatus);
		_currentStatusFrame.getTitleLabel().setEnabled(true);
	    }
	    _currentStatusFrame.validate();
	    _currentStatusFrame.repaint();

	    Component currentProperties = _currentEditor.getProperties();

	    if(currentProperties == null)
	    {
		Component oldContent = _propertiesFrame.getContent();
		if(oldContent != null)
		{
		    _propertiesFrame.remove(oldContent);
		}
		_propertiesFrame.getTitleLabel().setEnabled(false);
	    }
	    else
	    {
		_propertiesFrame.setContent(currentProperties);
		_propertiesFrame.getTitleLabel().setEnabled(true);
	    }
	    _propertiesFrame.validate();
	    _propertiesFrame.repaint();
	}
    }

    void refreshCurrentStatus()
    {
	if(_currentEditor != null)
	{
	    _currentEditor.refreshCurrentStatus();
	}
    }

    //
    // New application action
    //
    private void newApplication()
    {
	_root.newApplication();
    }
    private void newApplicationWithDefaultTemplates()
    {
	_mainFrame.setCursor(Cursor.getPredefinedCursor(Cursor.WAIT_CURSOR));
	try
	{
	    ApplicationDescriptor descriptor = _admin.getDefaultApplicationDescriptor();
	    descriptor.name = "NewApplication";
	    _root.newApplication(descriptor);
	}
	catch(DeploymentException e)
	{
	    JOptionPane.showMessageDialog(
		_mainFrame,
		"The default application descriptor from the IceGrid Registry is invalid:\n"
		+ e.reason,
		"Deployment Exception",
		JOptionPane.ERROR_MESSAGE);
	}
	catch(Ice.LocalException e)
	{
	    JOptionPane.showMessageDialog(
		_mainFrame,
		"Could not retrieve the default application descriptor from the IceGrid Registry: \n"
		+ e.toString(),
		"Trouble with IceGrid Registry",
		JOptionPane.ERROR_MESSAGE);
	}
	finally
	{
	    _mainFrame.setCursor(Cursor.getPredefinedCursor(Cursor.DEFAULT_CURSOR));
	}
    }

    //
    // Login and logout action
    //
    private void login()
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
			_sessionKeeper.relog(true);
		    }
		    break;
		case JOptionPane.NO_OPTION:
		    _sessionKeeper.relog(true);
		    break;
		case JOptionPane.CANCEL_OPTION:
		    break;
		default:
		    assert false;
	    }
	}
	else
	{
	    _sessionKeeper.relog(true);
	}
    }

    private void logout()
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
			_sessionKeeper.logout(true);
		    }
		    break;
		case JOptionPane.NO_OPTION:
		    _sessionKeeper.logout(true);
		    break;
		case JOptionPane.CANCEL_OPTION:
		    break;
		default:
		    assert false;
	    }
	}
	else
	{
	    _sessionKeeper.logout(true);
	}
    }
    
    //
    // Navigation
    //

    private void cleanList(java.util.LinkedList list)
    {
        CommonBase previous = null;
	java.util.Iterator p = list.iterator();
	while(p.hasNext())
	{
	    CommonBase current = (CommonBase)p.next();
	    if(current == previous || current.getParent() == null)
	    {
		p.remove();
	    }
	    else
	    {
		previous = current;
	    }
	}
    }

    private void back()
    {
	cleanList(_previousNodes);

	if(_previousNodes.size() == 0)
	{
	    _back.setEnabled(false);
	}
	else
	{
	    CommonBase previousNode = (CommonBase)_previousNodes.removeLast();
	    if(_previousNodes.size() == 0)
	    {
		_back.setEnabled(false);
	    }
	    
	    if(previousNode != _currentNode)
	    {
		assert _currentNode != null;
		if(_currentNode.isEphemeral())
		{
		    _currentNode.destroy();
		}
		else
		{
		    assert _currentNode.getParent() != null;
		    _nextNodes.addFirst(_currentNode);
		    _forward.setEnabled(true);
		}
		
		_currentNode = previousNode;
		_selectionListenerEnabled = false;
		_tree.setSelectionPath(_currentNode.getPath());
		_selectionListenerEnabled = true;
		show(_currentNode);
	    }
	}
    }

    private void forward()
    {
	cleanList(_nextNodes);

	if(_nextNodes.size() == 0)
	{
	    _forward.setEnabled(false);
	}
	else
	{
	    CommonBase nextNode = (CommonBase)_nextNodes.removeFirst();	
	    if(_nextNodes.size() == 0)
	    {
		_forward.setEnabled(false);
	    }
	    
	    if(nextNode != _currentNode)
	    {
		assert _currentNode != null;
		assert !_currentNode.isEphemeral();
		assert _currentNode.getParent() != null;
		
		_previousNodes.add(_currentNode);
		_back.setEnabled(true);
		
		_currentNode = nextNode;
		_selectionListenerEnabled = false;
		_tree.setSelectionPath(_currentNode.getPath());
		_selectionListenerEnabled = true;
		show(_currentNode);
	    }
	}
    }

    private void helpContents()
    {
	BareBonesBrowserLaunch.openURL(
	    "http://www.zeroc.com/help/IceGridAdmin/");
    }

    private void about()
    {
	String text = "IceGrid Admin version " 
	    + IceUtil.Version.ICE_STRING_VERSION + "\n"
	    + "Copyright \u00A9 2005 ZeroC, Inc. All rights reserved.\n";
	    
	JOptionPane.showMessageDialog(
	    _mainFrame,
	    text,
	    "About - IceGrid Admin",
	    JOptionPane.INFORMATION_MESSAGE);
    }

    public void setClipboard(Object copy)
    {
	_clipboard = copy;
    }

    public Object getClipboard()
    {
	return _clipboard; 
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
	destroyCommunicator();
	_mainFrame.dispose();
	Runtime.getRuntime().removeShutdownHook(_shutdownHook);
	Runtime.getRuntime().exit(status);
    }
    
    //
    // Can be called by the shutdown hook thread
    //
    private void destroyCommunicator()
    {
	try	   
	{
	    _communicator.destroy();
	}
	catch(Ice.LocalException e)
	{
	    System.err.println("_communicator.destroy() raised "
			       + e.toString());
	}
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

    Preferences getPrefs()
    {
	return _prefs;
    }

    
    public Action[] getActions()
    {
	return _actions;
    }

    public void showActions()
    {
	showActions(_actionsTarget);
    }
    public void showActions(CommonBase node)
    {
	_actionsTarget = node;

	boolean[] availableActions;
	if(node == null)
	{
	    availableActions = new boolean[CommonBase.ACTION_COUNT];
	}
	else
	{   
	    availableActions = node.getAvailableActions();
	}

	for(int i = 0; i < _actions.length; ++i)
	{
	    _actions[i].setEnabled(availableActions[i]);
	}
	
	
	_newServerMenu.setEnabled(
	    availableActions[CommonBase.NEW_SERVER] ||
	    availableActions[CommonBase.NEW_SERVER_ICEBOX] ||
	    availableActions[CommonBase.NEW_SERVER_FROM_TEMPLATE]);

	_newServiceMenu.setEnabled(
	    availableActions[CommonBase.NEW_SERVICE] ||
	    availableActions[CommonBase.NEW_SERVICE_FROM_TEMPLATE]);
				  
	_newTemplateMenu.setEnabled(
	    availableActions[CommonBase.NEW_TEMPLATE_SERVER] ||
	    availableActions[CommonBase.NEW_TEMPLATE_SERVER_ICEBOX] ||
	    availableActions[CommonBase.NEW_TEMPLATE_SERVICE]);

	_appMenu.setEnabled(
	    availableActions[CommonBase.APPLICATION_INSTALL_DISTRIBUTION]);
	
	_nodeMenu.setEnabled(
	    availableActions[CommonBase.SHUTDOWN_NODE]);

	_serverMenu.setEnabled(
	    availableActions[CommonBase.START] ||
	    availableActions[CommonBase.STOP] ||
	    availableActions[CommonBase.ENABLE] ||
	    availableActions[CommonBase.DISABLE] ||
	    availableActions[CommonBase.SERVER_INSTALL_DISTRIBUTION]);

	_serviceMenu.setEnabled(
	    availableActions[CommonBase.MOVE_UP] ||
	    availableActions[CommonBase.MOVE_DOWN]);
    }
    
    void createMenuBar()
    {
	_mainFrame.setJMenuBar(new MenuBar());
    }

    void createToolBar()
    {
	_mainFrame.getContentPane().add(new ToolBar(),
					BorderLayout.PAGE_START);
    }


    private final Ice.Communicator _communicator;
    private Preferences _prefs;
    private StatusBar _statusBar;
    private AdminPrx _admin;
    
    private Ice.ObjectAdapter _localAdapter;
    private Ice.ObjectAdapter _routedAdapter;

    private Root _root;
    private TreeModelI _treeModel;

    private int _latestSerial = -1;
    private int _writeSerial = -1;

    private JTree _tree;

    private boolean _substitute = false;
    
    private boolean _displayEnabled = true;

    private SimpleInternalFrame _currentStatusFrame 
          = new SimpleInternalFrame("Current status");
    private SimpleInternalFrame _propertiesFrame 
          = new SimpleInternalFrame("Properties");
  
    private JFrame _mainFrame;
    private SessionKeeper _sessionKeeper;

    private Editor _currentEditor;

    private Object _clipboard;

    private LicenseDialog _licenseDialog;

    //
    // back/forward navigation
    //
    private java.util.LinkedList _previousNodes = new java.util.LinkedList();
    private CommonBase _currentNode;
    private java.util.LinkedList _nextNodes = new java.util.LinkedList();
    private boolean _selectionListenerEnabled = true;

    //
    // Actions
    //
    private Action _newApplication;
    private Action _newApplicationWithDefaultTemplates;
    private Action _login;
    private Action _logout;
    private Action _save;
    private Action _discard;
    private Action _exit;
    private Action _back;
    private Action _forward;
    private Action _helpContents;
    private Action _copying;
    private Action _warranty;
    private Action _about;


    private Action[] _actions;
    
    private JToggleButton _showVarsTool;
    private JToggleButton _substituteTool;
    private JCheckBoxMenuItem _substituteMenuItem;
    private JCheckBoxMenuItem _showVarsMenuItem;
    
    private CommonBase _actionsTarget;

    private JMenu _newMenu;
    private JMenu _newServerMenu;
    private JMenu _newServiceMenu;
    private JMenu _newTemplateMenu;
    private JMenu _appMenu;
    private JMenu _nodeMenu;
    private JMenu _serverMenu;
    private JMenu _serviceMenu;

    private final Thread _shutdownHook;

    static private final int HISTORY_MAX_SIZE = 20;

    //
    // TODO: should come from build system
    //
    static private final boolean GPL_BUILD = true;
}
