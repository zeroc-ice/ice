// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGridGUI;

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

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.File;

import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.ButtonGroup;
import javax.swing.JButton;
import javax.swing.JCheckBoxMenuItem;
import javax.swing.JComponent;
import javax.swing.JEditorPane;
import javax.swing.JFileChooser;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JToggleButton;
import javax.swing.JToolBar;
import javax.swing.JTree;
import javax.swing.KeyStroke;
import javax.swing.SwingConstants;
import javax.swing.SwingUtilities;

import javax.swing.border.EmptyBorder;
import javax.swing.filechooser.FileFilter;

import com.jgoodies.looks.Options;
import com.jgoodies.looks.HeaderStyle;
import com.jgoodies.looks.BorderStyle;
import com.jgoodies.looks.plastic.PlasticLookAndFeel;

import IceGrid.*;

//
// This class coordinates the communications between the various objects
// that make up the IceGrid GUI.
// It also provides both the menu bar and tool bar.
//
public class Coordinator
{
    private class StatusBarI extends JPanel implements StatusBar
    {
	StatusBarI()
	{
	    super(new BorderLayout());
	    setBorder(new EmptyBorder(0, 13, 10, 13));

	    _text = new JLabel();
	    _text.setHorizontalAlignment(SwingConstants.LEFT);
	    add(_text, BorderLayout.LINE_START);

	    _connectedLabel = new JLabel("Working Offline");
	    _connectedLabel.setHorizontalAlignment(SwingConstants.RIGHT);
	    add(_connectedLabel, BorderLayout.LINE_END);
	}

	public void setText(String text)
	{
	    _text.setText(text);
	}

	public void setConnected(boolean connected)
	{
	    if(connected)
	    {
		_connectedLabel.setText("Working Online");
	    }
	    else
	    {
		_connectedLabel.setText("Working Offline");
	    }
	}

	JLabel _connectedLabel;
	JLabel _text;
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
	    fileMenu.add(_newMenu);
	    _newMenu.add(_newApplication);
	    _newMenu.add(_newApplicationWithDefaultTemplates);
	    _newMenu.addSeparator();

	    _newMenu.add(_appActionsForMenu.get(IceGridGUI.Application.TreeNode.NEW_ADAPTER));
	    _newMenu.add(_appActionsForMenu.get(IceGridGUI.Application.TreeNode.NEW_DBENV));
	    _newMenu.add(_appActionsForMenu.get(IceGridGUI.Application.TreeNode.NEW_NODE));
	    _newMenu.add(_appActionsForMenu.get(IceGridGUI.Application.TreeNode.NEW_PROPERTY_SET));
	    _newMenu.add(_appActionsForMenu.get(IceGridGUI.Application.TreeNode.NEW_REPLICA_GROUP));
	    

	    //
	    // Open sub-menu
	    //
	    JMenu openMenu = new JMenu("Open");
	    openMenu.add(_openApplicationFromFile);
	    openMenu.add(_openApplicationFromRegistry);
	    

	    //
	    // New server sub-sub-menu
	    //
	    _newServerMenu = new JMenu("Server");
	    _newServerMenu.setEnabled(false);
	    _newMenu.add(_newServerMenu);
	    _newServerMenu.add(_appActionsForMenu.get(IceGridGUI.Application.TreeNode.NEW_SERVER));
	    _newServerMenu.add(_appActionsForMenu.get(IceGridGUI.Application.TreeNode.NEW_SERVER_ICEBOX));
	    _newServerMenu.add(_appActionsForMenu.get(IceGridGUI.Application.TreeNode.NEW_SERVER_FROM_TEMPLATE));
	    
	    //
	    // New service sub-sub-menu
	    //
	    _newServiceMenu = new JMenu("Service");
	    _newServiceMenu.setEnabled(false);
	    _newMenu.add(_newServiceMenu);
	    _newServiceMenu.add(_appActionsForMenu.get(IceGridGUI.Application.TreeNode.NEW_SERVICE));
	    _newServiceMenu.add(_appActionsForMenu.get(IceGridGUI.Application.TreeNode.NEW_SERVICE_FROM_TEMPLATE));

	    //
	    // New template sub-sub-menu
	    //
	    _newTemplateMenu = new JMenu("Template");
	    _newTemplateMenu.setEnabled(false);
	    _newMenu.add(_newTemplateMenu);
	    _newTemplateMenu.add(_appActionsForMenu.get(IceGridGUI.Application.TreeNode.NEW_TEMPLATE_SERVER));
	    _newTemplateMenu.add(_appActionsForMenu.get(IceGridGUI.Application.TreeNode.NEW_TEMPLATE_SERVER_ICEBOX));
	    _newTemplateMenu.add(_appActionsForMenu.get(IceGridGUI.Application.TreeNode.NEW_TEMPLATE_SERVICE));
	    
	    fileMenu.addSeparator();
	    fileMenu.add(_login);
	    fileMenu.add(_logout);
	    fileMenu.addSeparator();
	    fileMenu.add(_acquireExclusiveWriteAccess);
	    fileMenu.add(_releaseExclusiveWriteAccess);
	    fileMenu.addSeparator();
	    fileMenu.add(openMenu);
	    fileMenu.add(_closeApplication);
	    fileMenu.add(_save);
	    fileMenu.add(_saveToFile);
	    fileMenu.add(_saveToRegistry);
	    fileMenu.addSeparator();
	    fileMenu.add(_discardUpdates);
	    fileMenu.addSeparator();
	    fileMenu.add(_exit);
	   
	    //
	    // Edit menu
	    //
	    JMenu editMenu = new JMenu("Edit");
	    editMenu.setMnemonic(java.awt.event.KeyEvent.VK_E);
	    add(editMenu);
	    editMenu.add(_appActionsForMenu.get(IceGridGUI.Application.TreeNode.COPY));
	    editMenu.add(_appActionsForMenu.get(IceGridGUI.Application.TreeNode.PASTE));
	    editMenu.addSeparator();
	    editMenu.add(_appActionsForMenu.get(IceGridGUI.Application.TreeNode.DELETE));
	    editMenu.addSeparator();
	    editMenu.add(_appActionsForMenu.get(IceGridGUI.Application.TreeNode.MOVE_UP));
	    editMenu.add(_appActionsForMenu.get(IceGridGUI.Application.TreeNode.MOVE_DOWN));

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
	    _appMenu.add(_patchApplication);
	    _appMenu.setEnabled(false);
	 
	    //
	    // Node sub-menu
	    //
	    _nodeMenu = new JMenu("Node");
	    _nodeMenu.setEnabled(false);
	    toolsMenu.add(_nodeMenu);
	    _nodeMenu.add(_liveActionsForMenu.get(IceGridGUI.LiveDeployment.TreeNode.SHUTDOWN_NODE));

	    //
	    // Registry sub-menu
	    //
	    _registryMenu = new JMenu("Registry");
	    _registryMenu.setEnabled(false);
	    toolsMenu.add(_registryMenu);
	    _registryMenu.add(_liveActionsForMenu.get(IceGridGUI.LiveDeployment.TreeNode.ADD_OBJECT));

	    //
	    // Server sub-menu
	    //
	    _serverMenu = new JMenu("Server");
	    _serverMenu.setEnabled(false);
	    toolsMenu.add(_serverMenu);
	    _serverMenu.add(_liveActionsForMenu.get(IceGridGUI.LiveDeployment.TreeNode.START));
	    _serverMenu.add(_liveActionsForMenu.get(IceGridGUI.LiveDeployment.TreeNode.STOP));
	    _serverMenu.addSeparator();
	    _serverMenu.add(_liveActionsForMenu.get(IceGridGUI.LiveDeployment.TreeNode.ENABLE));
	    _serverMenu.add(_liveActionsForMenu.get(IceGridGUI.LiveDeployment.TreeNode.DISABLE));
	    _serverMenu.addSeparator();
	    _serverMenu.add(_liveActionsForMenu.get(
				IceGridGUI.LiveDeployment.TreeNode.PATCH_SERVER));

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
	    button = new JButton(_logout);
	    button.setText(null);
	    button.setIcon(Utils.getIcon("/icons/24x24/logout.png"));
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
	    
	    button = new JButton(_openApplicationFromRegistry);
	    button.setText(null);
	    button.setIcon(Utils.getIcon("/icons/24x24/open_from_registry.png"));
	    add(button);
	    button = new JButton(_openApplicationFromFile);
	    button.setText(null);
	    button.setIcon(Utils.getIcon("/icons/24x24/open_from_file.png"));
	    add(button);

	    addSeparator();

	    button = new JButton(_save);
	    button.setText(null);
	    button.setIcon(Utils.getIcon("/icons/24x24/save.png"));
	    add(button);
	    button = new JButton(_saveToRegistry);
	    button.setText(null);
	    button.setIcon(Utils.getIcon("/icons/24x24/save_to_registry.png"));
	    add(button);
	    button = new JButton(_saveToFile);
	    button.setText(null);
	    button.setIcon(Utils.getIcon("/icons/24x24/save_to_file.png"));
	    add(button);
	    button = new JButton(_discardUpdates);
	    button.setText(null);
	    button.setIcon(Utils.getIcon("/icons/24x24/discard.png"));
	    add(button);

	    addSeparator();

	    button = new JButton(_appActionsForMenu.get(IceGridGUI.Application.TreeNode.COPY));
	    button.setText(null);
	    button.setIcon(Utils.getIcon("/icons/24x24/copy.png"));
	    add(button);
	    button = new JButton(_appActionsForMenu.get(IceGridGUI.Application.TreeNode.PASTE));
	    button.setText(null);
	    button.setIcon(Utils.getIcon("/icons/24x24/paste.png"));
	    add(button);
	    button = new JButton(_appActionsForMenu.get(IceGridGUI.Application.TreeNode.DELETE));
	    button.setText(null);
	    button.setIcon(Utils.getIcon("/icons/24x24/delete.png"));
	    add(button);

	    addSeparator();

	    add(_showVarsTool);
	    add(_substituteTool);
	}
    }

    //
    // All Coordinator's methods run in the UI thread
    //
    public Ice.Communicator getCommunicator()
    {
	if(_communicator == null)
	{
	    _communicator = createCommunicator(_properties);
	}
	return _communicator;
    }

    public Ice.Properties getProperties()
    {
	return _properties;
    }

    public Tab getCurrentTab()
    {
	return (Tab)_mainPane.getSelectedComponent();
    }
    
    public Action getBackAction()
    {
	return _back;
    }

    public Action getForwardAction()
    {
	return _forward;
    }

    public Action getCloseApplicationAction()
    {
	return _closeApplication;
    }

    public Action getSaveAction()
    {
	return _save;
    }

    public Action getSaveToRegistryAction()
    {
	return _saveToRegistry;
    }

    public Action getSaveToFileAction()
    {
	return _saveToFile;
    }

    public Action getDiscardUpdatesAction()
    {
	return _discardUpdates;
    }
    


    //
    // Open live application and select application tab
    //
    public ApplicationPane openLiveApplication(String applicationName)
    {
	ApplicationPane app = (ApplicationPane)_liveApplications.get(applicationName);
	if(app == null)
	{
	    ApplicationDescriptor desc = 
		_liveDeploymentRoot.getApplicationDescriptor(applicationName);
	    if(desc == null)
	    {
		JOptionPane.showMessageDialog(
		    _mainFrame,
		    "The application '" + applicationName + "' was not found in the registry.",
		    "No such application",
		    JOptionPane.ERROR_MESSAGE);
	    }
	    //
	    // Essential: deep-copy desc!
	    //
	    desc = IceGridGUI.Application.Root.copyDescriptor(desc);
	    app = new ApplicationPane(new IceGridGUI.Application.Root(this, desc, true, null));
	    _mainPane.addApplication(app);
	    _liveApplications.put(applicationName, app);
	}
	_mainPane.setSelectedComponent(app);
	return app;
    }
    
    public void removeLiveApplication(String name)
    {
	_liveApplications.remove(name);
    }
    
    public void addLiveApplication(IceGridGUI.Application.Root root)
    {
	ApplicationPane app = _mainPane.findApplication(root);
	assert app != null;
	_liveApplications.put(app.getRoot().getId(), app);
    }

    public ApplicationPane getLiveApplication(String name)
    {
	return (ApplicationPane)_liveApplications.get(name);
    }


    //
    // From the Registry observer:
    //
    void registryInit(String instanceName, int serial, java.util.List applications,
		      AdapterInfo[] adapters, ObjectInfo[] objects)
    {	
	assert _latestSerial == -1;
	_latestSerial = serial;

	_liveDeploymentRoot.init(instanceName, applications, adapters, objects);
	//
	// When we get this init, we can't have any live Application yet.
	//
    }

    void applicationAdded(int serial, ApplicationDescriptor desc)
    {
	_liveDeploymentRoot.applicationAdded(desc);
	_statusBar.setText(
	    "Last update: new application '" + desc.name + "'");
	updateSerial(serial);
    }

    void applicationRemoved(int serial, String name)
    {
	_liveDeploymentRoot.applicationRemoved(name);
	_statusBar.setText(
	    "Last update: application '" + name + "' was removed");

	ApplicationPane app = 
	    (ApplicationPane)_liveApplications.get(name);
	
	if(app != null)
	{
	    if(app.getRoot().kill())
	    {
		_mainPane.remove(app);
	    }
	    _liveApplications.remove(name);
	}
	updateSerial(serial);
    }

    void applicationUpdated(int serial, ApplicationUpdateDescriptor desc)
    {
	_liveDeploymentRoot.applicationUpdated(desc);
	_liveDeploymentPane.refresh();

	_statusBar.setText("Last update: application  '" + desc.name + "' was updated");
	
	ApplicationPane app =
	    (ApplicationPane)_liveApplications.get(desc.name);

	if(app != null)
	{
	    if(app.getRoot().update(desc))
	    {
		app.refresh();
	    }
	}
	updateSerial(serial);
    }

    void adapterAdded(int serial, AdapterInfo info)
    {
	_liveDeploymentRoot.adapterAdded(info);
	_liveDeploymentPane.refresh();
	updateSerial(serial);
    }    

    void adapterUpdated(int serial, AdapterInfo info)
    {
	_liveDeploymentRoot.adapterUpdated(info);
	_liveDeploymentPane.refresh();
	updateSerial(serial);
    }    

    void adapterRemoved(int serial, String id)
    {
	_liveDeploymentRoot.adapterRemoved(id);
	_liveDeploymentPane.refresh();
	updateSerial(serial);
    }    
    
    void objectAdded(int serial, ObjectInfo info)
    {
	_liveDeploymentRoot.objectAdded(info);
	_liveDeploymentPane.refresh();
	updateSerial(serial);
    }    

    void objectUpdated(int serial, ObjectInfo info)
    {
	_liveDeploymentRoot.objectUpdated(info);
	_liveDeploymentPane.refresh();
	updateSerial(serial);
    }    

    void objectRemoved(int serial, Ice.Identity id)
    {
	_liveDeploymentRoot.objectRemoved(id);
	_liveDeploymentPane.refresh();
	updateSerial(serial);
    } 
    
    public void accessDenied(AccessDeniedException e)
    {
	assert false;

	JOptionPane.showMessageDialog(
	    _mainFrame,
	    "Another session (username = " + e.lockUserId 
	    + ") has exclusive write access to the registry",
	    "Access Denied",
	    JOptionPane.ERROR_MESSAGE);
    }

    public void pasteApplication()
    {
	Object descriptor = getClipboard();
	ApplicationDescriptor desc = 
	    IceGridGUI.Application.Root.copyDescriptor((ApplicationDescriptor)descriptor);

	IceGridGUI.Application.Root root = new IceGridGUI.Application.Root(this, desc);
	ApplicationPane app = new ApplicationPane(root);
	_mainPane.addApplication(app);
	_mainPane.setSelectedComponent(app);
	root.setSelectedNode(root);
    }
    
    public void removeApplicationFromRegistry(String name)
    {
	_mainFrame.setCursor(Cursor.getPredefinedCursor(Cursor.WAIT_CURSOR));
	
	boolean acquired = false;
	try
	{
	    acquireExclusiveWriteAccess(null);
	    acquired = true;
	    _sessionKeeper.getAdmin().removeApplication(name);
	}
	catch(AccessDeniedException e)
	{
	    accessDenied(e);
	}
	catch(ApplicationNotExistException e)
	{
	    //
	    // Somebody else deleted this application at about the same time
	    //
	}
	catch(Ice.LocalException e)
	{
	    JOptionPane.showMessageDialog(
		_mainFrame,
		"Could not remove application '" + name + 
		"' from IceGrid registry:\n" + e.toString(),
		"Trouble with IceGrid registry",
		JOptionPane.ERROR_MESSAGE);
	}
	finally
	{
	    if(acquired)
	    {
		releaseExclusiveWriteAccess();
	    }
	    _mainFrame.setCursor(Cursor.getPredefinedCursor(Cursor.DEFAULT_CURSOR));
	}
    }

    public void acquireExclusiveWriteAccess(Runnable runnable)
	throws AccessDeniedException
    {
	System.err.println("acquireExclusiveWriteAccess");

	if(_writeSerial == -1)
	{
	    _mainFrame.setCursor(Cursor.getPredefinedCursor(Cursor.WAIT_CURSOR));
	    try
	    {
		System.err.println("startUpdate");
		_writeSerial = _sessionKeeper.getSession().startUpdate();
		System.err.println("write serial is now: " + _writeSerial);
	    }
	    finally
	    {
		_mainFrame.setCursor(Cursor.getPredefinedCursor(Cursor.DEFAULT_CURSOR));
	    }
	}

	assert _onExclusiveWrite == null;

	//
	// Must increment now since run() can call releaseExclusiveWriteAccess()
	//
	_writeAccessCount++;

	System.err.println("write accsess count is now: " + _writeAccessCount);

	if(runnable != null)
	{
	    if(_writeSerial <= _latestSerial)
	    {
		System.err.println("run update immediately");
		runnable.run();
	    }
	    else
	    {
		System.err.println("latest serial is: " + _latestSerial);
		System.err.println("waiting ...");

		_onExclusiveWrite = runnable;
		_mainFrame.setCursor(Cursor.getPredefinedCursor(Cursor.WAIT_CURSOR));
		//
		// TODO: start a thread to check we don't wait forever
		//
	    }
	}
    }

    public void releaseExclusiveWriteAccess()
    {
	if(--_writeAccessCount == 0)
	{
	    try
	    {
		_writeSerial = -1;
		_sessionKeeper.getSession().finishUpdate();
	    }
	    catch(AccessDeniedException e)
	    {
		accessDenied(e);
	    }
	    catch(Ice.ObjectNotExistException e)
	    {
		//
		// Ignored, the session is gone, and so is the exclusive access.
		//
	    }
	    catch(Ice.LocalException e)
	    {
		JOptionPane.showMessageDialog(
		    _mainFrame,
		    "Could not release exclusive write access on the IceGrid registry:\n"
		    + e.toString(),
		    "Trouble with IceGrid registry",
		    JOptionPane.ERROR_MESSAGE);
	    }
	}
    }

    private void updateSerial(int serial)
    {
	assert serial == _latestSerial + 1;
	_latestSerial = serial;
	
	System.err.println("updateSerial _latestSerial is now " + _latestSerial);
	
	if(_writeAccessCount > 0 && 
	   _writeSerial <= _latestSerial &&
	   _onExclusiveWrite != null)
	{
	    Runnable runnable = _onExclusiveWrite;
	    _onExclusiveWrite = null;
	    runnable.run();
	    _mainFrame.setCursor(Cursor.getPredefinedCursor(Cursor.DEFAULT_CURSOR));
	}
    }
  
    //
    // From the Node observer:
    //
    void nodeUp(NodeDynamicInfo updatedInfo)
    {
	_liveDeploymentRoot.nodeUp(updatedInfo);
	_liveDeploymentPane.refresh();
    }

    void nodeDown(String node)
    {
	_liveDeploymentRoot.nodeDown(node);
	_liveDeploymentPane.refresh();
    }

    void updateServer(String node, ServerDynamicInfo updatedInfo)
    {
	_liveDeploymentRoot.updateServer(node, updatedInfo);
	_liveDeploymentPane.refresh();
    }

    void updateAdapter(String node, AdapterDynamicInfo updatedInfo)
    {
	_liveDeploymentRoot.updateAdapter(node, updatedInfo);
	_liveDeploymentPane.refresh();
    }
    
    void sessionLost()
    {
	_latestSerial = -1;
	_writeSerial = -1;
	_writeAccessCount = 0;
	_onExclusiveWrite = null;
	_liveDeploymentRoot.clear();

	//
	// Kill all live applications
	//
	java.util.Iterator p = _liveApplications.values().iterator();
	while(p.hasNext())
	{
	    ApplicationPane app = (ApplicationPane)p.next();
	    if(app.getRoot().kill())
	    {
		_mainPane.remove(app);
	    }
	}
	_liveApplications.clear();
	
	_logout.setEnabled(false);
	_openApplicationFromRegistry.setEnabled(false);
	_patchApplication.setEnabled(false);
	_appMenu.setEnabled(false);
	_newApplicationWithDefaultTemplates.setEnabled(false);
	_acquireExclusiveWriteAccess.setEnabled(false);
	_releaseExclusiveWriteAccess.setEnabled(false);
	_saveToRegistry.setEnabled(false);
    }
    
    AdminSessionPrx login(SessionKeeper.LoginInfo info, Component parent)
    {	
	_liveDeploymentRoot.clear();

	AdminSessionPrx session = null;
	
	destroyCommunicator();
	//
	// Transform SSL info into properties
	//
	Ice.Properties properties = _properties._clone();
	properties.setProperty("IceSSL.Keystore", info.keystore);
	properties.setProperty("IceSSL.Password", new String(info.keyPassword));
	properties.setProperty("IceSSL.KeystorePassword", new String(info.keystorePassword));
	properties.setProperty("IceSSL.Alias", info.alias);
	properties.setProperty("IceSSL.Truststore", info.truststore);
	properties.setProperty("IceSSL.TruststorePassword", new String(info.truststorePassword));
	_communicator = createCommunicator(properties);

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
	    
	    try
	    {
		Glacier2.RouterPrx router = Glacier2.RouterPrxHelper.
		    uncheckedCast(_communicator.stringToProxy(str));

		//
		// The session must be routed through this router
		//
		_communicator.setDefaultRouter(router);

		Glacier2.SessionPrx s;
		if(info.routerUseSSL)
		{
		    s = router.createSessionFromSecureConnection();
		}
		else
		{
		    s = router.createSession(
			info.routerUsername, new String(info.routerPassword));
		}
		
		session = AdminSessionPrxHelper.uncheckedCast(s);
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
	    str = info.registryInstanceName + "/Registry";
		
	    RegistryPrx registry = RegistryPrxHelper.
		uncheckedCast(_communicator.stringToProxy(str));
	    
	    try
	    {
		if(info.registryUseSSL)
		{
		    session = AdminSessionPrxHelper.uncheckedCast(
			registry.createAdminSessionFromSecureConnection());
		}
		else
		{
		    session = AdminSessionPrxHelper.uncheckedCast(
			registry.createAdminSession(info.registryUsername, 
						    new String(info.registryPassword)));
		}
	    }
	    catch(IceGrid.PermissionDeniedException e)
	    {
		JOptionPane.showMessageDialog(parent,
					      "Permission denied: "
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
	
	_logout.setEnabled(true);
	_openApplicationFromRegistry.setEnabled(true);
	_patchApplication.setEnabled(true);
	_appMenu.setEnabled(true);
	_newApplicationWithDefaultTemplates.setEnabled(true);
	_acquireExclusiveWriteAccess.setEnabled(true);

	_mainPane.setSelectedComponent(_liveDeploymentPane);

	return session;
    }

    
    void destroySession(AdminSessionPrx session)
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

    void showVars()
    {
	substitute(false);
    }

    void substituteVars()
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
	    getCurrentTab().refresh();
	}
    }

    public boolean substitute()
    {
	return _substitute;
    }

    public AdminPrx getAdmin()
    {
	return _sessionKeeper.getAdmin();
    }

    public StatusBar getStatusBar()
    {
	return _statusBar;
    }

    public JFrame getMainFrame()
    {
	return _mainFrame;
    }

    public MainPane getMainPane()
    {
	return _mainPane;
    }

    public ApplicationDescriptor parseFile(File file)
    {
	if(_icegridadminProcess == null)
	{
	    //
	    // Start icegridadmin server
	    //
	    try
	    {
		_icegridadminProcess = Runtime.getRuntime().exec("icegridadmin --server");
	    }
	    catch(java.io.IOException e)
	     {
		JOptionPane.showMessageDialog(
		    _mainFrame,
		    "Failed to start icegridadmin subprocess: " + e.toString(),
		    "IO Exception",
		    JOptionPane.ERROR_MESSAGE);
		return null;
	    }
	    
	    try
	    {
		BufferedReader reader = 
		    new BufferedReader(new InputStreamReader(_icegridadminProcess.getInputStream(),
							     "US-ASCII"));
		
		String str = reader.readLine();
		reader.close();

		if(str == null || str.length() == 0)
		{
		    JOptionPane.showMessageDialog(
			_mainFrame,
			"The icegridadmin subprocess failed",
			"Subprocess failure",
			JOptionPane.ERROR_MESSAGE);
		    destroyIceGridAdmin();
		    return null;
		}
		_fileParser = str;
	    }
	    catch(java.io.UnsupportedEncodingException e)
	    {
		assert false;
	    }
	    catch(java.io.IOException e)
	    {
		JOptionPane.showMessageDialog(
		    _mainFrame,
		    "IO Exception: " + e.toString(),
		    "IO Exception",
		    JOptionPane.ERROR_MESSAGE);

		destroyIceGridAdmin();
		return null;

	    }
	}

	try
	{
	    FileParserPrx fileParser = FileParserPrxHelper.checkedCast(
		getCommunicator().stringToProxy(_fileParser));
	    return fileParser.parse(file.getAbsolutePath(), 
				    _sessionKeeper.getRoutedAdmin());
	}
	catch(ParseException e)
	{
	    JOptionPane.showMessageDialog(
		_mainFrame,
		"Failed to parse file '" + file.getAbsolutePath() + "':\n" + e.toString(),
		"Parse error",
		JOptionPane.ERROR_MESSAGE);
	    return null;
	}
	catch(Ice.LocalException e)
	{
	    JOptionPane.showMessageDialog(
		_mainFrame,
		"Operation on FileParser failed:\n" + e.toString(),
		"Communication error",
		JOptionPane.ERROR_MESSAGE);
	    destroyIceGridAdmin();
	    return null;
	}
    }

    private void destroyIceGridAdmin()
    {
	if(_icegridadminProcess != null)
	{
	    try
	    {
		_icegridadminProcess.destroy();
	    }
	    catch(Exception e)
	    {}
	    _icegridadminProcess = null;
	    _fileParser = null;
	}
    }


    public File saveToFile(boolean ask, IceGridGUI.Application.Root root,
			   File file)
    {
	if(ask || file == null)
	{
	    if(file != null)
	    {
		_fileChooser.setSelectedFile(file);
	    }
	    int result = _fileChooser.showSaveDialog(_mainFrame);
	    if(result == JFileChooser.APPROVE_OPTION)
	    {
		file = _fileChooser.getSelectedFile();
	    }
	    else
	    {
		file = null;
	    }
	}
	if(file != null)
	{
	    try
	    {
		XMLWriter writer = new XMLWriter(file);
		root.write(writer);
		writer.close();
		_statusBar.setText(
		    "Saved application '" + root.getId() + "' to "
		    + file.getAbsolutePath());
	    }
	    catch(java.io.FileNotFoundException e)
	    {
		JOptionPane.showMessageDialog(
		    _mainFrame,
		    "Cannot use the selected file for writing.",
		    "File Not Found",
		    JOptionPane.ERROR_MESSAGE);
		return null;
	    }
	    catch(java.io.IOException e)
	    {
		JOptionPane.showMessageDialog(
		    _mainFrame,
		    "IO Exception: " + e.toString(),
		    "IO Exception",
		    JOptionPane.ERROR_MESSAGE);
		return null;
	    }
	}
	return file;
    }

    
    static private Ice.Properties createProperties(Ice.StringSeqHolder args)
    {
	Ice.Properties properties = Ice.Util.createProperties(args);

	//
	// Set various default values
	//
	if(properties.getProperty("Ice.Override.ConnectTimeout").equals(""))
	{
	    properties.setProperty("Ice.Override.ConnectTimeout", "5000");
	}

	if(properties.getProperty("IceGrid.AdminGUI.Endpoints").equals(""))
	{
	    properties.setProperty("IceGrid.AdminGUI.Endpoints", "tcp -t 10000");
	}
	   
        //
        // For SSL with JDK 1.4
        //
	if(properties.getProperty("Ice.ThreadPerConnection").equals(""))
	{
	    properties.setProperty("Ice.ThreadPerConnection", "1");
	}

	//
	// For Glacier
	//
	if(properties.getProperty("Ice.ACM.Client").equals(""))
	{
	    properties.setProperty("Ice.ACM.Client", "0");
	}
	if(properties.getProperty("Ice.MonitorConnections").equals(""))
	{
	    properties.setProperty("Ice.MonitorConnections", "5");
	}
	
	//
	// Disable retries
	//
	properties.setProperty("Ice.RetryIntervals", "-1");
	return properties;
    }

    static private Ice.Communicator createCommunicator(Ice.Properties properties)
    {
	Ice.InitializationData initData = new Ice.InitializationData();
	initData.properties = properties;
	return Ice.Util.initialize(new String[0], initData);
    }

    Coordinator(JFrame mainFrame, Ice.StringSeqHolder args, Preferences prefs)
    {	
	_mainFrame = mainFrame;
	_prefs = prefs;
	_properties = createProperties(args);
	
	if(args.value.length > 0)
	{
	    //
	    // TODO: use proper logging
	    //
	    System.err.println("WARNING: extra command-line arguments");
	    for(int i = 0; i < args.value.length; ++i)
	    {
		System.err.println(args.value[i]);
	    }
	}

	_liveDeploymentRoot = new IceGridGUI.LiveDeployment.Root(this);

	_licenseDialog = new LicenseDialog(_mainFrame);

	_sessionKeeper = new SessionKeeper(this);

	_shutdownHook = new Thread("Shutdown hook")
	    {
		public void run()
		{
		    destroyIceGridAdmin();
		    destroyCommunicator();
		}
	    };
	Runtime.getRuntime().addShutdownHook(_shutdownHook);

	_fileChooser = new JFileChooser();
	_fileChooser.addChoosableFileFilter(new FileFilter()
	    {
		public boolean accept(File f)
		{
		    return f.isDirectory() || f.getName().endsWith(".xml");
		}
		
		public String getDescription()
		{
		    return ".xml files";
		}
	    });


	final int MENU_MASK = Toolkit.getDefaultToolkit().getMenuShortcutKeyMask();

	//
	// Common actions (nodes not involved)
	//
	_newApplication = new AbstractAction("Application")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    newApplication();
		}
	    };

	_newApplicationWithDefaultTemplates = 
	    new AbstractAction("Application with default templates from registry")
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
		    _sessionKeeper.relog(true);
		}
	    };
	_login.putValue(Action.SHORT_DESCRIPTION, 
			"Log into an IceGrid registry");

	_logout = new AbstractAction("Logout")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    _sessionKeeper.logout(true);
		}
	    };
	_logout.putValue(Action.SHORT_DESCRIPTION, "Logout");
	_logout.setEnabled(false);

	_acquireExclusiveWriteAccess = new AbstractAction("Acquire exclusive write access")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    try
		    {
			acquireExclusiveWriteAccess(null);
			_releaseExclusiveWriteAccess.setEnabled(true);
			_acquireExclusiveWriteAccess.setEnabled(false);
		    }
		    catch(AccessDeniedException ade)
		    {
			accessDenied(ade);
		    }
		}
	    };
	_acquireExclusiveWriteAccess.putValue(Action.SHORT_DESCRIPTION, 
					      "Acquire exclusive write access on the registry");
	_acquireExclusiveWriteAccess.setEnabled(false);

	
	_releaseExclusiveWriteAccess = new AbstractAction("Release exclusive write access")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    releaseExclusiveWriteAccess();
		    _acquireExclusiveWriteAccess.setEnabled(true);
		    _releaseExclusiveWriteAccess.setEnabled(false);
		}
	    };
	_releaseExclusiveWriteAccess.putValue(Action.SHORT_DESCRIPTION, 
					      "Release exclusive write access on the registry");
	_releaseExclusiveWriteAccess.setEnabled(false);

	_openApplicationFromFile = new AbstractAction("Application from file")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    int result = _fileChooser.showOpenDialog(_mainFrame);
		    if(result == JFileChooser.APPROVE_OPTION)
		    {
			File file = _fileChooser.getSelectedFile();

			ApplicationDescriptor desc = parseFile(file);

			if(desc != null)
			{
			    IceGridGUI.Application.Root root = 
				new IceGridGUI.Application.Root(Coordinator.this, desc, false, file);
			    ApplicationPane app = new ApplicationPane(root);
			    _mainPane.addApplication(app);
			    _mainPane.setSelectedComponent(app);
			    root.setSelectedNode(root);
			}
		    }
		}
	    };
	_openApplicationFromFile.putValue(Action.SHORT_DESCRIPTION, "Open application from file");
	_openApplicationFromFile.setEnabled(true);
	
	_openApplicationFromRegistry = new AbstractAction("Application from registry")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    Object[] applicationNames = _liveDeploymentRoot.getApplicationNames();
		    
		    if(applicationNames.length == 0)
		    {
			JOptionPane.showMessageDialog(
			    _mainFrame,
			    "The registry does not contain any application",
			    "Empty registry",
			    JOptionPane.INFORMATION_MESSAGE);
		    }
		    else
		    {
			String appName = (String)JOptionPane.showInputDialog(
			    _mainFrame, "Which Application do you want to open?", 
			    "Open Application from registry",	 
			    JOptionPane.QUESTION_MESSAGE, null,
			    applicationNames, applicationNames[0]);
			
			if(appName != null)
			{
			    IceGridGUI.Application.Root root = openLiveApplication(appName).getRoot();
			    if(root.getSelectedNode() == null)
			    {
				root.setSelectedNode(root);
			    }
			}
		    }
		}
	    };
	_openApplicationFromRegistry.putValue(Action.SHORT_DESCRIPTION, "Open application from registry");
	_openApplicationFromRegistry.setEnabled(false);

	_closeApplication = new AbstractAction("Close application")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    Tab tab = getCurrentTab();
		    if(tab.close())
		    {
			String id = ((ApplicationPane)tab).getRoot().getId();
			_liveApplications.remove(id);
		    }
		}
	    };
	_closeApplication.putValue(Action.SHORT_DESCRIPTION, "Close application");
	_closeApplication.setEnabled(false);

	_save = new AbstractAction("Save")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    getCurrentTab().save();
		}
	    };
	_save.setEnabled(false);
	_save.putValue(Action.ACCELERATOR_KEY, 
		       KeyStroke.getKeyStroke(KeyEvent.VK_S, MENU_MASK));
	_save.putValue(Action.SHORT_DESCRIPTION, "Save");

	
	_saveToRegistry = new AbstractAction("Save to registry")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    getCurrentTab().saveToRegistry();
		}
	    };
	_saveToRegistry.setEnabled(false);
	_saveToRegistry.putValue(Action.SHORT_DESCRIPTION, "Save to registry");

	
	_saveToFile = new AbstractAction("Save to file")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    getCurrentTab().saveToFile();
		}
	    };
	_saveToFile.setEnabled(false);
	_saveToFile.putValue(Action.SHORT_DESCRIPTION, "Save to file");

	
	_discardUpdates = new AbstractAction("Discard updates...")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    getCurrentTab().discardUpdates();
		}
	    };
	_discardUpdates.setEnabled(false);
	_discardUpdates.putValue(Action.SHORT_DESCRIPTION, "Discard updates");


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
		    getCurrentTab().back();
		}
	    };
	_back.setEnabled(false);
	_back.putValue(Action.SHORT_DESCRIPTION, "Go back to the previous node");

	_forward =  new AbstractAction("Go to the next node")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    getCurrentTab().forward();
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
	
	_patchApplication = new AbstractAction("Patch distribution")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    Object[] applicationNames = _liveDeploymentRoot.getPatchableApplicationNames();
		    
		    if(applicationNames.length == 0)
		    {
			JOptionPane.showMessageDialog(
			    _mainFrame,
			    "No application in this IceGrid registry can be patched",
			    "No application",
			    JOptionPane.INFORMATION_MESSAGE);
		    }
		    else
		    {
			String appName = (String)JOptionPane.showInputDialog(
			    _mainFrame, "Which Application do you want to patch?", 
			    "Patch application",	 
			    JOptionPane.QUESTION_MESSAGE, null,
			    applicationNames, applicationNames[0]);
			
			if(appName != null)
			{
			    _liveDeploymentRoot.patch(appName);
			}
		    }
		}
	    };
	_patchApplication.setEnabled(false);

	_showVarsMenuItem = new
	    JCheckBoxMenuItem(_appActionsForMenu.get(IceGridGUI.Application.TreeNode.SHOW_VARS));
	_showVarsTool = new 
	    JToggleButton(_appActionsForMenu.get(IceGridGUI.Application.TreeNode.SHOW_VARS));
	_showVarsTool.setIcon(Utils.getIcon("/icons/24x24/show_vars.png"));
	_showVarsTool.setText("");

	_substituteMenuItem = new
	    JCheckBoxMenuItem(_appActionsForMenu.get(IceGridGUI.Application.TreeNode.SUBSTITUTE_VARS));
	_substituteTool = new 
	    JToggleButton(_appActionsForMenu.get(IceGridGUI.Application.TreeNode.SUBSTITUTE_VARS));
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

	_mainFrame.setJMenuBar(new MenuBar());

	_mainFrame.getContentPane().add(new ToolBar(),
					BorderLayout.PAGE_START);
 
	_mainFrame.getContentPane().add((StatusBarI)_statusBar, 
					BorderLayout.PAGE_END);

	_liveDeploymentPane = new LiveDeploymentPane(_liveDeploymentRoot);
	_mainPane = new MainPane(this);
	_mainFrame.getContentPane().add(_mainPane, BorderLayout.CENTER);	
    }

    JComponent getLiveDeploymentPane()
    {
	return _liveDeploymentPane;
    }
    
    public IceGridGUI.LiveDeployment.Root getLiveDeploymentRoot()
    {
	return _liveDeploymentRoot;
    }

    private void newApplication()
    {
	ApplicationDescriptor desc =  new ApplicationDescriptor("NewApplication",
								 new java.util.TreeMap(),
								 new java.util.LinkedList(),
								 new java.util.HashMap(),
								 new java.util.HashMap(),
								 new java.util.HashMap(),
								 new IceGrid.DistributionDescriptor(
								     "", new java.util.LinkedList()),
								"",
								new java.util.HashMap());
	IceGridGUI.Application.Root root = new IceGridGUI.Application.Root(this, desc);
	ApplicationPane app = new ApplicationPane(root);
	_mainPane.addApplication(app);
	_mainPane.setSelectedComponent(app);
	root.setSelectedNode(root);
    }

    private void newApplicationWithDefaultTemplates()
    {
	_mainFrame.setCursor(Cursor.getPredefinedCursor(Cursor.WAIT_CURSOR));
	try
	{
	    ApplicationDescriptor descriptor = getAdmin().getDefaultApplicationDescriptor();
	    descriptor.name = "NewApplication";
	    IceGridGUI.Application.Root root = new IceGridGUI.Application.Root(this, descriptor);
	    ApplicationPane app = new ApplicationPane(root);
	    _mainPane.addApplication(app);
	    _mainPane.setSelectedComponent(app);
	    root.setSelectedNode(root);
	}
	catch(DeploymentException e)
	{
	    JOptionPane.showMessageDialog(
		_mainFrame,
		"The default application descriptor from the IceGrid registry is invalid:\n"
		+ e.reason,
		"Deployment Exception",
		JOptionPane.ERROR_MESSAGE);
	}
	catch(Ice.LocalException e)
	{
	    JOptionPane.showMessageDialog(
		_mainFrame,
		"Could not retrieve the default application descriptor from the IceGrid registry: \n"
		+ e.toString(),
		"Trouble with IceGrid registry",
		JOptionPane.ERROR_MESSAGE);
	}
	finally
	{
	    _mainFrame.setCursor(Cursor.getPredefinedCursor(Cursor.DEFAULT_CURSOR));
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
	    + "Copyright \u00A9 2005-2006 ZeroC, Inc. All rights reserved.\n";
	    
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
	destroyIceGridAdmin();
	destroyCommunicator();
	Runtime.getRuntime().removeShutdownHook(_shutdownHook);
	_mainFrame.dispose();
	Runtime.getRuntime().exit(status);
    }
    
    //
    // Can be called by the shutdown hook thread
    //
    private void destroyCommunicator()
    {
	if(_communicator != null)
	{
	    try	   
	    {
		_communicator.destroy();
	    }
	    catch(Ice.LocalException e)
	    {
		System.err.println("_communicator.destroy() raised "
				   + e.toString());
		e.printStackTrace();
	    }
	    _communicator = null;
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


    public AdminSessionPrx getSession()
    {
	return _sessionKeeper.getSession();
    }

    SessionKeeper getSessionKeeper()
    {
	return _sessionKeeper;
    }

    Preferences getPrefs()
    {
	return _prefs;
    }

    public LiveActions getLiveActionsForPopup()
    {
	return _liveActionsForPopup;
    }

    public LiveActions getLiveActionsForMenu()
    {
	return _liveActionsForMenu;
    }

    public ApplicationActions getActionsForMenu()
    {
	return _appActionsForMenu;
    }

    public ApplicationActions getActionsForPopup()
    {
	return _appActionsForPopup;
    }

    public void showActions(IceGridGUI.LiveDeployment.TreeNode node)
    {
	boolean[] availableActions = _liveActionsForMenu.setTarget(node);
	_appActionsForMenu.setTarget(null);
	
	_newServerMenu.setEnabled(false);
	_newServiceMenu.setEnabled(false);	  
	_newTemplateMenu.setEnabled(false);

	_appMenu.setEnabled(true);
	
	_nodeMenu.setEnabled(
	    availableActions[IceGridGUI.LiveDeployment.TreeNode.SHUTDOWN_NODE]);

	_registryMenu.setEnabled(
	    availableActions[IceGridGUI.LiveDeployment.TreeNode.ADD_OBJECT]);

	_serverMenu.setEnabled(
	    availableActions[IceGridGUI.LiveDeployment.TreeNode.START] ||
	    availableActions[IceGridGUI.LiveDeployment.TreeNode.STOP] ||
	    availableActions[IceGridGUI.LiveDeployment.TreeNode.ENABLE] ||
	    availableActions[IceGridGUI.LiveDeployment.TreeNode.DISABLE] ||
	    availableActions[IceGridGUI.LiveDeployment.TreeNode.PATCH_SERVER]);
    }
	
    public void showActions(IceGridGUI.Application.TreeNode node)
    {
	boolean[] availableActions = _appActionsForMenu.setTarget(node);
	_liveActionsForMenu.setTarget(null);
	
	_newServerMenu.setEnabled(
	    availableActions[IceGridGUI.Application.TreeNode.NEW_SERVER] ||
	    availableActions[IceGridGUI.Application.TreeNode.NEW_SERVER_ICEBOX] ||
	    availableActions[IceGridGUI.Application.TreeNode.NEW_SERVER_FROM_TEMPLATE]);

	_newServiceMenu.setEnabled(
	    availableActions[IceGridGUI.Application.TreeNode.NEW_SERVICE] ||
	    availableActions[IceGridGUI.Application.TreeNode.NEW_SERVICE_FROM_TEMPLATE]);
				  
	_newTemplateMenu.setEnabled(
	    availableActions[IceGridGUI.Application.TreeNode.NEW_TEMPLATE_SERVER] ||
	    availableActions[IceGridGUI.Application.TreeNode.NEW_TEMPLATE_SERVER_ICEBOX] ||
	    availableActions[IceGridGUI.Application.TreeNode.NEW_TEMPLATE_SERVICE]);

	_appMenu.setEnabled(false);
	_nodeMenu.setEnabled(false);
	_registryMenu.setEnabled(false);
	_serverMenu.setEnabled(false);
    }	


    private final Ice.Properties _properties;
    private Ice.Communicator _communicator;

    private Preferences _prefs;
    private StatusBarI _statusBar = new StatusBarI();
    
    private IceGridGUI.LiveDeployment.Root _liveDeploymentRoot;
    private LiveDeploymentPane _liveDeploymentPane;

    //
    // Maps application-name to ApplicationPane (only for 'live' applications)
    //
    private java.util.Map _liveApplications = new java.util.HashMap();

    private MainPane _mainPane;

    //
    // Keep tracks of serial number when viewing/editing application definitions
    // (not used for displaying live deployment)
    //
    private int _latestSerial = -1;
    private int _writeSerial = -1;
   
    private Runnable _onExclusiveWrite;
    private int _writeAccessCount = 0;

    private boolean _substitute = false;
    
    private JFrame _mainFrame;
    private SessionKeeper _sessionKeeper;

    private Object _clipboard;

    private LicenseDialog _licenseDialog;
    
    //
    // Actions
    //
    private Action _newApplication;
    private Action _newApplicationWithDefaultTemplates;
    private Action _login;
    private Action _logout;
    private Action _acquireExclusiveWriteAccess;
    private Action _releaseExclusiveWriteAccess;

    private Action _openApplicationFromFile;
    private Action _openApplicationFromRegistry;
    private Action _closeApplication;
    private Action _save;
    private Action _saveToRegistry;
    private Action _saveToFile;
    private Action _discardUpdates;
    private Action _exit;
    private Action _back;
    private Action _forward;
    private Action _helpContents;
    private Action _copying;
    private Action _warranty;
    private Action _about;
    private Action _patchApplication;

    //
    // Two sets of actions because the popup's target and the menu/toolbar's target
    // can be different.
    // 
    private LiveActions _liveActionsForMenu = new LiveActions();
    private LiveActions _liveActionsForPopup = new LiveActions();
    private ApplicationActions _appActionsForMenu = new ApplicationActions(false);
    private ApplicationActions _appActionsForPopup = new ApplicationActions(true);
    
    
    private JToggleButton _showVarsTool;
    private JToggleButton _substituteTool;
    private JCheckBoxMenuItem _substituteMenuItem;
    private JCheckBoxMenuItem _showVarsMenuItem;
    

    private JMenu _newMenu;
    private JMenu _newServerMenu;
    private JMenu _newServiceMenu;
    private JMenu _newTemplateMenu;
    private JMenu _appMenu;
    private JMenu _nodeMenu;
    private JMenu _registryMenu;
    private JMenu _serverMenu;

    private final Thread _shutdownHook;

    private JFileChooser _fileChooser;
    
    private Process _icegridadminProcess;
    private String _fileParser;

    static private final int HISTORY_MAX_SIZE = 20;

    //
    // TODO: should come from build system
    //
    static private final boolean GPL_BUILD = true;
}
