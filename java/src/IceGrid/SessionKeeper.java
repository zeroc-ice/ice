// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid;

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

	    registryTimeout = properties.getPropertyAsInt("Ice.Override.Timeout");
	    routerTimeout = registryTimeout;
	    if(registryTimeout == 0)
	    {
		registryTimeout = _connectionPrefs.getInt("registry.timeout", 0);
		routerTimeout =  _connectionPrefs.getInt("router.timeout", 0);
	    }
	    
	    registryConnectTimeout = properties.getPropertyAsInt("Ice.Override.ConnectTimeout");
	    routerConnectTimeout = registryConnectTimeout;
	    if(registryConnectTimeout == 0)
	    {
		registryConnectTimeout = _connectionPrefs.getInt("registry.connectTimeout", 0);
		routerConnectTimeout = _connectionPrefs.getInt("router.connectTimeout", 0);
	    }
	  
	    registryUsername = _connectionPrefs.get("registry.username", registryUsername);
	    routerUsername = _connectionPrefs.get("router.username", routerUsername);

	    registryAutoconnect = _connectionPrefs.
		getBoolean("registry.autoconnect", registryAutoconnect);

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
		_connectionPrefs.putInt("router.timeout", routerTimeout);
		_connectionPrefs.putInt("router.connectTimeout", routerConnectTimeout);
	    }
	    else
	    {
		_connectionPrefs.put("registry.username", registryUsername);
		_connectionPrefs.putBoolean("registry.autoconnect", registryAutoconnect);
		_connectionPrefs.put("registry.instanceName", registryInstanceName);
		_connectionPrefs.put("registry.endpoints", registryEndpoints);
		_connectionPrefs.putInt("registry.timeout", registryTimeout);
		_connectionPrefs.putInt("registry.connectTimeout", registryConnectTimeout);
	    }
	}

	boolean routed = false;

	String registryUsername = System.getProperty("user.name");
	boolean registryAutoconnect = false;
	String registryInstanceName = "IceGrid";
	String registryEndpoints = "";
	int registryTimeout = 0;
	int registryConnectTimeout = 0;

	String routerUsername = System.getProperty("user.name");
	char[] routerPassword;
	String routerInstanceName = "Glacier2";
	String routerEndpoints = "";
	int routerTimeout = 0;
	int routerConnectTimeout = 0;
	
	private Preferences _connectionPrefs;
    }




    private class LoginDialog extends JDialog
    {
	LoginDialog()
	{
	    super(_model.getMainFrame(), "Login - IceGrid Admin", true);

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

	    _registryTimeout.setEditable(true);
	    _registryConnectTimeout.setEditable(true);

	    _routerTimeout.setEditable(true);
	    _routerConnectTimeout.setEditable(true);

	    JPanel directPanel = null;
	    {
		FormLayout layout = new FormLayout("right:pref, 3dlu, pref", "");
		
		DefaultFormBuilder builder = new DefaultFormBuilder(layout);
		builder.setDefaultDialogBorder();
		builder.setRowGroupingEnabled(true);
		builder.setLineGapSize(LayoutStyle.getCurrent().getLinePad());
		
		builder.append("Username", _registryUsername);
		builder.nextLine();
		builder.append("", _registryAutoconnect);
		builder.nextLine();
		builder.append("IceGrid Instance Name", _registryInstanceName);
		builder.nextLine();
		builder.append("IceGrid Registry Endpoint(s)", _registryEndpoints);
		builder.nextLine();
		
		builder.appendSeparator("Timeouts (in milliseconds)");
		builder.append("Connection", _registryTimeout);
		builder.nextLine();
		builder.append("Connection Establishment", _registryConnectTimeout);
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
		
		builder.appendSeparator("Timeouts (in milliseconds)");
		builder.append("Connection", _routerTimeout);
		builder.nextLine();
		builder.append("Connection Establishment", _routerConnectTimeout);
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

	//
	// Returns a copy; the new info is "installed" only after
	// a successful login.
	//
	boolean showDialog()
	{
	    if(isVisible() == false)
	    {
		_tabbedPane.setSelectedIndex(_loginInfo.routed ? 1 : 0);

		_registryUsername.setText(_loginInfo.registryUsername);
		_registryAutoconnect.setSelected(_loginInfo.registryAutoconnect);
		_registryInstanceName.setText(_loginInfo.registryInstanceName);
		_registryEndpoints.setText(_loginInfo.registryEndpoints);

		if(_loginInfo.registryTimeout == 0)
		{
		    _registryTimeout.setSelectedItem(TIMEOUT_NOT_SET);
		}
		else
		{
		    _registryTimeout.setSelectedItem(
			Integer.toString(_loginInfo.registryTimeout));
		}

		if(_loginInfo.registryConnectTimeout == 0)
		{
		    _registryConnectTimeout.setSelectedItem(CONNECT_TIMEOUT_NOT_SET);
		}
		else
		{
		    _registryConnectTimeout.setSelectedItem(
			Integer.toString(_loginInfo.registryConnectTimeout));
		}
		
		_routerUsername.setText(_loginInfo.routerUsername);
		_routerInstanceName.setText(_loginInfo.routerInstanceName);
		_routerEndpoints.setText(_loginInfo.routerEndpoints);

		if(_loginInfo.routerTimeout == 0)
		{
		    _routerTimeout.setSelectedItem(TIMEOUT_NOT_SET);
		}
		else
		{
		    _routerTimeout.setSelectedItem(
			Integer.toString(_loginInfo.routerTimeout));
		}

		if(_loginInfo.routerConnectTimeout == 0)
		{
		    _routerConnectTimeout.setSelectedItem(CONNECT_TIMEOUT_NOT_SET);
		}
		else
		{
		    _routerConnectTimeout.setSelectedItem(
			Integer.toString(_loginInfo.routerConnectTimeout));
		}


		setLocationRelativeTo(_model.getMainFrame());
		setVisible(true);
		return true;
	    }
	    else
	    {
		//
		// Otherwise it was already on the screen!
		//
		return false;
	    }
	}
       
	private void writeInfo()
	{
	    _loginInfo.routed = (_tabbedPane.getSelectedIndex() == 1);

	    _loginInfo.registryUsername = _registryUsername.getText();
	    _loginInfo.registryAutoconnect = _registryAutoconnect.isSelected();
	    _loginInfo.registryInstanceName = _registryInstanceName.getText();
	    _loginInfo.registryEndpoints = _registryEndpoints.getText();
	 
	    Object registryTimeout = _registryTimeout.getSelectedItem();
	    if(registryTimeout == TIMEOUT_NOT_SET)
	    {
		_loginInfo.registryTimeout = 0;
	    }
	    else
	    {
		_loginInfo.registryTimeout = 
		    Integer.valueOf(registryTimeout.toString()).intValue();
	    }
	    
	    Object registryConnectTimeout = _registryConnectTimeout.getSelectedItem();
	    if(registryConnectTimeout == CONNECT_TIMEOUT_NOT_SET)
	    {
		_loginInfo.registryConnectTimeout = 0;
	    }
	    else
	    {
		_loginInfo.registryConnectTimeout = 
		    Integer.valueOf(registryConnectTimeout.toString()).intValue();
	    }

	    
	    _loginInfo.routerUsername = _routerUsername.getText();
	    _loginInfo.routerPassword = _routerPassword.getPassword();
	    _loginInfo.routerInstanceName = _routerInstanceName.getText();
	    _loginInfo.routerEndpoints = _routerEndpoints.getText();

	    
	    Object routerTimeout = _routerTimeout.getSelectedItem();
	    if(routerTimeout == TIMEOUT_NOT_SET)
	    {
		_loginInfo.routerTimeout = 0;
	    }
	    else
	    {
		_loginInfo.routerTimeout = 
		    Integer.valueOf(routerTimeout.toString()).intValue();
	    }
	    
	    Object routerConnectTimeout = _routerConnectTimeout.getSelectedItem();
	    if(routerConnectTimeout == CONNECT_TIMEOUT_NOT_SET)
	    {
		_loginInfo.routerConnectTimeout = 0;
	    }
	    else
	    {
		_loginInfo.routerConnectTimeout = 
		    Integer.valueOf(routerConnectTimeout.toString()).intValue();
	    }
	}

	private JTabbedPane _tabbedPane = new JTabbedPane();
	private JTextField _registryUsername = new JTextField(30);
	private JCheckBox _registryAutoconnect 
	    = new JCheckBox("Automatically log in at startup");

	private JTextField _registryInstanceName = new JTextField(30);
	private JTextField _registryEndpoints = new JTextField(30);

	private JComboBox _registryTimeout = new JComboBox(new Object[]
	    {TIMEOUT_NOT_SET, "10000", "60000", "600000"});

	private JComboBox _registryConnectTimeout = new JComboBox(new Object[]
	    {CONNECT_TIMEOUT_NOT_SET, "3000", "10000"});
	
	
	private JTextField _routerUsername = new JTextField(30);
	private JPasswordField _routerPassword = new JPasswordField(30);
	private JTextField _routerInstanceName = new JTextField(30);
	private JTextField _routerEndpoints = new JTextField(30);

	private JComboBox _routerTimeout = new JComboBox(new Object[]
	    {TIMEOUT_NOT_SET, "10000", "60000", "600000"});
	
	private JComboBox _routerConnectTimeout = new JComboBox(new Object[]
	    {CONNECT_TIMEOUT_NOT_SET, "3000", "10000"});
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


    SessionKeeper(Model model)
    {
	_model = model;
	_loginDialog = new LoginDialog();
	_loginPrefs = model.getPrefs().node("Connection");
    }

    //
    // Runs in UI thread
    //
    void createSession(boolean autoconnectEnabled)
    {
	_loginInfo = new LoginInfo(_loginPrefs, 
				   _model.getCommunicator());
	boolean openDialog = true;
	if(autoconnectEnabled && !_loginInfo.routed && 
	   _loginInfo.registryAutoconnect)
	{
	    openDialog = !login(_model.getMainFrame());
	}

	if(openDialog)
	{
	    _loginDialog.showDialog();
	}
    }

    //
    // Runs in UI thread
    //
    void relog(boolean showDialog)
    {
	if(_loginInfo == null)
	{
	    createSession(!showDialog);
	}
	else
	{
	    if(showDialog || !login(_model.getMainFrame()))
	    {
		_loginDialog.showDialog();
	    }
	}
    }

    //
    // Runs in UI thread
    //
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
       
	    _session = _model.login(_loginInfo, parent);
	    if(_session == null)
	    {
		return false;
	    }
	    
	    _model.getStatusBar().setConnected(true);

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

    private void logout(boolean destroySession)
    {
	destroyObservers();
	releaseSession(destroySession);
	_model.sessionLost();
    }


    //
    // Runs in UI thread
    //
    void sessionLost(String message)
    {
	JOptionPane.showMessageDialog(
	    _model.getMainFrame(),
	    message,
	    "Session lost",
	    JOptionPane.ERROR_MESSAGE);
	
	logout(false);
	relog(true);
    }

    //
    // Runs in UI thread
    //
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
		_model.destroySession(_session);
	    }

	    _session = null;
	    _model.getStatusBar().setConnected(false);
	}
    }

    //
    // Runs in UI thread
    //
    private void registerObservers()
    {
	//
	// Create the object adapter for the observers
	//
	String uuid = Ice.Util.generateUUID();
	String category;

	Ice.RouterPrx router = _model.getCommunicator().getDefaultRouter();
	if(router == null)
	{
	    _observerAdapter = _model.getCommunicator().createObjectAdapterWithEndpoints(
		"Observers-" + uuid, "default -t 15000");
	    category = "observer";
	}
	else
	{
	    _observerAdapter = _model.getCommunicator().createObjectAdapter("Observers-" + uuid);
	    _observerAdapter.addRouter(router);
	    category = router.getServerProxy().ice_getIdentity().category;
	}
	
	//
	// Create servants and proxies
	//
	_registryObserverIdentity.name = "registry-" + uuid;
	_registryObserverIdentity.category = category;
	
	_nodeObserverIdentity.name = "node-" + uuid;
	_nodeObserverIdentity.category = category;
	
	RegistryObserverI registryObserverServant = new RegistryObserverI(_model);
	
	RegistryObserverPrx registryObserver = 
	    RegistryObserverPrxHelper.uncheckedCast(
		_observerAdapter.add(
		    registryObserverServant, _registryObserverIdentity));
	
	NodeObserverPrx nodeObserver =
	    NodeObserverPrxHelper.uncheckedCast(
		_observerAdapter.add(
		    new NodeObserverI(_model), _nodeObserverIdentity));
	
	_observerAdapter.activate();
	
	_session.setObservers(registryObserver, nodeObserver); 
	
	registryObserverServant.waitForInit();
    }

    //
    // Runs in UI thread
    //
    private void destroyObservers()
    {
	if(_observerAdapter != null)
	{
	    try
	    {
		_observerAdapter.remove(_registryObserverIdentity);
	    }
	    catch(Ice.NotRegisteredException e)
	    {
	    }
	    try
	    {
		_observerAdapter.remove(_nodeObserverIdentity);
	    }
	    catch(Ice.NotRegisteredException e)
	    {
	    }
	    _observerAdapter.deactivate();
	   
	    // TODO: waitForDeactivate once bug #535 is fixed
	    //
	    //  _observerAdapter.waitForDeactivate();
	    _observerAdapter = null;
	}
    }
    

    SessionPrx getSession()
    {
	return _session;
    }
   
    private LoginDialog _loginDialog;
    private LoginInfo _loginInfo;

    private Model _model;
    private Preferences _loginPrefs;
  
    private Pinger _thread;
    private SessionPrx _session;

    private Ice.ObjectAdapter _observerAdapter;
    private Ice.Identity _registryObserverIdentity = new Ice.Identity();
    private Ice.Identity _nodeObserverIdentity = new Ice.Identity();

    static private Object TIMEOUT_NOT_SET = new Object()
	{
	    public String toString()
	    {
		return "Use the timeout specified in each proxy's endpoint";
	    }
	};
    
    static private Object CONNECT_TIMEOUT_NOT_SET = new Object()
	{
	    public String toString()
	    {
		return "Use the connection timeout";
	    }
	};
}
