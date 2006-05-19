// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGridGUI;

import javax.swing.*;

import java.awt.Container;
import java.awt.Frame;
import java.awt.Component;
import java.awt.Cursor;

import java.awt.event.ActionEvent;
import java.awt.event.ItemEvent;
import java.awt.event.ActionListener;
import java.awt.event.ItemListener;

import com.jgoodies.forms.builder.DefaultFormBuilder;
import com.jgoodies.forms.factories.Borders;
import com.jgoodies.forms.factories.ButtonBarFactory;
import com.jgoodies.forms.factories.DefaultComponentFactory;
import com.jgoodies.forms.layout.FormLayout;
import com.jgoodies.forms.util.LayoutStyle;

import java.util.prefs.Preferences;

import IceGrid.*;

//
// The SessionKeeper is responsible for establishing sessions (one at a time)
// and keeping them alive.
//

class SessionKeeper
{
    static public class LoginInfo
    {
	LoginInfo(Preferences connectionPrefs, Ice.Communicator communicator)
	{
	    _connectionPrefs = connectionPrefs;
	    Ice.Properties properties = communicator.getProperties();

	    //
	    // Registry properties
	    //
	    String defaultLocator = properties.getProperty("Ice.Default.Locator");
	    Ice.ObjectPrx defaultLocatorProxy = null;
	    if(!defaultLocator.equals(""))
	    {
		try
		{
		    defaultLocatorProxy = communicator.stringToProxy(defaultLocator);
		}
		catch(Ice.LocalException e)
		{
		    // Ignored, keep null defaultLocatorProxy
		}
	    }
	    if(defaultLocatorProxy != null)
	    {
		//
		// Set new defaults
		//
		registryInstanceName = defaultLocatorProxy.ice_getIdentity().category;
		registryEndpoints = "";
		Ice.Endpoint[] endpoints = defaultLocatorProxy.ice_getEndpoints();
		for(int i = 0; i < endpoints.length; ++i)
		{
		    if(i > 0)
		    {
			registryEndpoints += ":";
		    }
		    registryEndpoints += endpoints[i].toString();
		}
	    }
	    else
	    {
		registryInstanceName = 
		    _connectionPrefs.get("registry.instanceName", registryInstanceName);
		registryEndpoints = 
		    _connectionPrefs.get("registry.endpoints", registryEndpoints);
	    }

	    
	    //
	    // Glacier2 properties
	    //
	    String defaultRouter = properties.getProperty("Ice.Default.Router");
	    Ice.ObjectPrx defaultRouterProxy = null;
	    if(!defaultRouter.equals(""))
	    {
		try
		{
		    defaultRouterProxy = communicator.stringToProxy(defaultRouter);
		}
		catch(Ice.LocalException e)
		{
		    // Ignored, keep null defaultRouterProxy
		}
	    }
	    if(defaultRouterProxy != null)
	    {
		//
		// Set new defaults
		//
		routerInstanceName = defaultRouterProxy.ice_getIdentity().category;
		routerEndpoints = "";
		Ice.Endpoint[] endpoints = defaultRouterProxy.ice_getEndpoints();
		for(int i = 0; i < endpoints.length; ++i)
		{
		    if(i > 0)
		    {
			routerEndpoints += ":";
		    }
		    routerEndpoints += endpoints[i].toString();
		}
	    }
	    else
	    {
		routerInstanceName = 
		    _connectionPrefs.get("router.instanceName", routerInstanceName);
		routerEndpoints = 
		    _connectionPrefs.get("router.endpoints", routerEndpoints);
	    }

	    registryUsername = _connectionPrefs.get("registry.username", registryUsername);
	    routerUsername = _connectionPrefs.get("router.username", routerUsername);

	    routed = _connectionPrefs.getBoolean("routed", routed);
	}

	void save()
	{
	    _connectionPrefs.putBoolean("routed", routed);

	    if(routed)
	    {
		_connectionPrefs.put("router.username", routerUsername);
		_connectionPrefs.put("router.instanceName", routerInstanceName);
		_connectionPrefs.put("router.endpoints", routerEndpoints);
	    }
	    else
	    {
		_connectionPrefs.put("registry.username", registryUsername);
		_connectionPrefs.put("registry.instanceName", registryInstanceName);
		_connectionPrefs.put("registry.endpoints", registryEndpoints);
	    }
	}

	boolean routed = false;

	String registryUsername = System.getProperty("user.name");
	char[] registryPassword;
	String registryInstanceName = "IceGrid";
	String registryEndpoints = "";

	String routerUsername = System.getProperty("user.name");
	char[] routerPassword;
	String routerInstanceName = "Glacier2";
	String routerEndpoints = "";
	
	private Preferences _connectionPrefs;
    }




    private class LoginDialog extends JDialog
    {
	LoginDialog()
	{
	    super(_coordinator.getMainFrame(), "Login - IceGrid Admin", true);

	    setDefaultCloseOperation(JDialog.HIDE_ON_CLOSE);
	      
	    JButton okButton = new JButton("OK");
	    ActionListener okListener = new ActionListener()
		{
		    public void actionPerformed(ActionEvent e)
		    {
			writeInfo();

			if(login(LoginDialog.this))
			{
			    setVisible(false);
			}
			//
			// Otherwise go back to the dialog
			//
		    }
		};
	    okButton.addActionListener(okListener);
	    getRootPane().setDefaultButton(okButton);
	
	    JButton cancelButton = new JButton("Cancel");
	    ActionListener cancelListener = new ActionListener()
		{
		    public void actionPerformed(ActionEvent e)
		    {
			setVisible(false);
		    }
		};
	    cancelButton.addActionListener(cancelListener);

	    JPanel directPanel = null;
	    {
		FormLayout layout = new FormLayout("right:pref, 3dlu, pref", "");
		
		DefaultFormBuilder builder = new DefaultFormBuilder(layout);
		builder.setDefaultDialogBorder();
		builder.setRowGroupingEnabled(true);
		builder.setLineGapSize(LayoutStyle.getCurrent().getLinePad());
		
		builder.append("Username", _registryUsername);
		builder.nextLine();
		builder.append("Password", _registryPassword);
		builder.nextLine();
		builder.append("IceGrid Instance Name", _registryInstanceName);
		builder.nextLine();
		builder.append("IceGrid Registry Endpoint(s)", _registryEndpoints);
		builder.nextLine();
		
		directPanel = builder.getPanel();
	    }

	    
	    JPanel routedPanel = null;
	    {
		FormLayout layout = new FormLayout("right:pref, 3dlu, pref", "");
		
		DefaultFormBuilder builder = new DefaultFormBuilder(layout);
		builder.setDefaultDialogBorder();
		builder.setRowGroupingEnabled(true);
		builder.setLineGapSize(LayoutStyle.getCurrent().getLinePad());
		
		builder.append("Username", _routerUsername);
		builder.nextLine();
		builder.append("Password", _routerPassword);
		builder.nextLine();
		builder.append("Glacier2 Instance Name", _routerInstanceName);
		builder.nextLine();
		builder.append("Glacier2 Router Endpoint(s)", _routerEndpoints);
		builder.nextLine();
		
		routedPanel = builder.getPanel();
	    }
	    
	    _tabbedPane.addTab("Direct", directPanel);
	    _tabbedPane.addTab("Routed", routedPanel);
	    _tabbedPane.setBorder(Borders.DIALOG_BORDER);

	    JComponent buttonBar = 
		    ButtonBarFactory.buildOKCancelBar(okButton, cancelButton);
	    buttonBar.setBorder(Borders.DIALOG_BORDER);
		
	    Container contentPane = getContentPane();
	    contentPane.setLayout(new BoxLayout(contentPane, BoxLayout.Y_AXIS));
	    contentPane.add(_tabbedPane);
	    contentPane.add(buttonBar);

	    pack();
	    setResizable(false);
	}

	
	void showDialog()
	{
	    if(isVisible() == false)
	    {
		_tabbedPane.setSelectedIndex(_loginInfo.routed ? 1 : 0);

		_registryUsername.setText(_loginInfo.registryUsername);
		_registryInstanceName.setText(_loginInfo.registryInstanceName);
		_registryEndpoints.setText(_loginInfo.registryEndpoints);
	
		_routerUsername.setText(_loginInfo.routerUsername);
		_routerInstanceName.setText(_loginInfo.routerInstanceName);
		_routerEndpoints.setText(_loginInfo.routerEndpoints);

		setLocationRelativeTo(_coordinator.getMainFrame());
		setVisible(true);
	    }
	  
	    //
	    // Otherwise it was already on the screen!
	    //
	}
       
	private void writeInfo()
	{
	    _loginInfo.routed = (_tabbedPane.getSelectedIndex() == 1);

	    _loginInfo.registryUsername = _registryUsername.getText();
	    _loginInfo.registryPassword = _registryPassword.getPassword();
	    _loginInfo.registryInstanceName = _registryInstanceName.getText();
	    _loginInfo.registryEndpoints = _registryEndpoints.getText();
	 
	    _loginInfo.routerUsername = _routerUsername.getText();
	    _loginInfo.routerPassword = _routerPassword.getPassword();
	    _loginInfo.routerInstanceName = _routerInstanceName.getText();
	    _loginInfo.routerEndpoints = _routerEndpoints.getText();
	}

	private JTabbedPane _tabbedPane = new JTabbedPane();
	private JTextField _registryUsername = new JTextField(30);
	private JPasswordField _registryPassword = new JPasswordField(30);
	private JTextField _registryInstanceName = new JTextField(30);
	private JTextField _registryEndpoints = new JTextField(30);
	
	private JTextField _routerUsername = new JTextField(30);
	private JPasswordField _routerPassword = new JPasswordField(30);
	private JTextField _routerInstanceName = new JTextField(30);
	private JTextField _routerEndpoints = new JTextField(30);
    }

   
    //
    // We create a brand new Pinger thread for each session
    //
    class Pinger extends Thread
    {
	Pinger(long period)
	{
	    _period = period;
	}

	public synchronized void run()
	{
	    do
	    {
		try
		{
		    _session.keepAlive();
		}
		catch(final Ice.LocalException e)
		{
		    _done = true;
		  
		    SwingUtilities.invokeLater(new Runnable() 
			{
			    public void run() 
			    {
				sessionLost("Failed to contact the IceGrid registry: " 
					    + e.toString());
			    }
			});
		}
		
		if(!_done)
		{
		    try
		    {
			wait(_period);
		    }
		    catch(InterruptedException e)
		    {
			// Ignored
		    }
		}
	    } while(!_done);
	}
	
	public synchronized void done()
	{
	    if(!_done)
	    {
		_done = true;
		notify();
	    }
	}
	
	private long _period;
	private boolean _done = false;
    } 


    SessionKeeper(Coordinator coordinator)
    {
	_coordinator = coordinator;
	_loginDialog = new LoginDialog();
	_loginPrefs = coordinator.getPrefs().node("Connection");
    }

    //
    // Runs in UI thread
    //
    void createSession()
    {
	_loginInfo = new LoginInfo(_loginPrefs, 
				   _coordinator.getCommunicator());
	_loginDialog.showDialog();

    }
  
    void relog(boolean showDialog)
    {
	if(_loginInfo == null)
	{
	    createSession();
	}
	else
	{
	    if(showDialog || !login(_coordinator.getMainFrame()))
	    {
		_loginDialog.showDialog();
	    }
	}
    }
    
    void logout(boolean destroySession)
    {
	destroyObservers();
	releaseSession(destroySession);
	_coordinator.sessionLost();
    }

    private boolean login(Component parent)
    {
	if(_session != null)
	{
	    logout(true);
	}
	assert _session == null;
	
	Cursor oldCursor = parent.getCursor();
	try
	{
	    parent.setCursor(Cursor.getPredefinedCursor(Cursor.WAIT_CURSOR));
       
	    _session = _coordinator.login(_loginInfo, parent);
	    if(_session == null)
	    {
		return false;
	    }
	    
	    _coordinator.getStatusBar().setConnected(true);

	    //
	    // Start thread
	    //
	    assert(_thread == null);

	    //
	    // When using Glacier2, we assume the session returns the
	    // the Glacier2 SessionTimeout.x
	    //
	    long period = _session.getTimeout() * 1000 / 2;
	    
	    _thread = new Pinger(period);
	    _thread.start();
	    
	    try
	    {
		registerObservers();
	    }
	    catch(Ice.LocalException e)
	    {
		logout(true);
		JOptionPane.showMessageDialog(parent,
					      "Could not register observers: "
					      + e.toString(),
					      "Login failed",
					      JOptionPane.ERROR_MESSAGE);
		return false;
	    }

	    _loginInfo.save();
	}
	finally
	{
	    parent.setCursor(oldCursor);
	}
	return true;
    }

    void sessionLost(String message)
    {
	JOptionPane.showMessageDialog(
	    _coordinator.getMainFrame(),
	    message,
	    "Session lost",
	    JOptionPane.ERROR_MESSAGE);
	
	logout(false);
	relog(true);
    }

    private void releaseSession(boolean destroySession)
    {
	if(_session != null)
	{
	    _thread.done();
	    
	    for(;;)
	    {
		try
		{
		    _thread.join();
		    break;
		}
		catch(InterruptedException e)
		{
		}
	    }
	    _thread = null;
	    
	    if(destroySession)
	    {
		_coordinator.destroySession(_session);
	    }
	    _session = null;
	    _coordinator.getStatusBar().setConnected(false);
	}
    }

    private void registerObservers()
    {
	//
	// Create the object adapter for the observers
	//
	String uuid = Ice.Util.generateUUID();
	String category;

	Glacier2.RouterPrx router = Glacier2.RouterPrxHelper.uncheckedCast(
	    _coordinator.getCommunicator().getDefaultRouter());
	if(router == null)
	{
	    category = "observer";
	}
	else
	{
	    category = router.getCategoryForClient();
	}
	
	//
	// Create servants and proxies
	//
	_registryObserverIdentity.name = "registry-" + uuid;
	_registryObserverIdentity.category = category;
	
	_nodeObserverIdentity.name = "node-" + uuid;
	_nodeObserverIdentity.category = category;
	
	Ice.ObjectAdapter adapter = _coordinator.getObjectAdapter();

	RegistryObserverI registryObserverServant = new RegistryObserverI(_coordinator);
	
	RegistryObserverPrx registryObserver = 
	    RegistryObserverPrxHelper.uncheckedCast(
		adapter.add(
		    registryObserverServant, _registryObserverIdentity));
	
	NodeObserverPrx nodeObserver =
	    NodeObserverPrxHelper.uncheckedCast(
		adapter.add(
		    new NodeObserverI(_coordinator), _nodeObserverIdentity));
	
	_session.setObservers(registryObserver, nodeObserver); 
	
	registryObserverServant.waitForInit();
    }

    //
    // Runs in UI thread
    //
    private void destroyObservers()
    {
	Ice.ObjectAdapter adapter = _coordinator.getObjectAdapter();

	try
	{
	    adapter.remove(_registryObserverIdentity);
	}
	catch(Ice.NotRegisteredException e)
	{
	}

	try
	{
	    adapter.remove(_nodeObserverIdentity);
	}
	catch(Ice.NotRegisteredException e)
	{
	}
    }
    

    AdminSessionPrx getSession()
    {
	return _session;
    }
   
    private LoginDialog _loginDialog;
    private LoginInfo _loginInfo;

    private Coordinator _coordinator;
    private Preferences _loginPrefs;
  
    private Pinger _thread;
    private AdminSessionPrx _session;

    private Ice.Identity _registryObserverIdentity = new Ice.Identity();
    private Ice.Identity _nodeObserverIdentity = new Ice.Identity();
}
