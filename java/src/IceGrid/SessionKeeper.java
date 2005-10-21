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
		    _connectionPrefs.get("registryInstanceName", registryInstanceName);
		registryEndpoints = 
		    _connectionPrefs.get("registryEndpoints", registryEndpoints);
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
		    _connectionPrefs.get("routerInstanceName", routerInstanceName);
		routerEndpoints = 
		    _connectionPrefs.get("routerEndpoints", routerEndpoints);
	    }


	    timeout = properties.getPropertyAsInt("Ice.Override.Timeout");
	    if(timeout == 0)
	    {
		timeout = _connectionPrefs.getInt("timeout", 0);
		System.err.println("timeout == " + timeout);
	    }

	    connectTimeout = properties.getPropertyAsInt("Ice.Override.ConnectTimeout");
	    if(connectTimeout == 0)
	    {
		connectTimeout = _connectionPrefs.getInt("connectTimeout", 0);
		System.err.println("connectTimeout == " + connectTimeout);
	    }
	  
	    username = _connectionPrefs.get("username", username);
	    useGlacier = _connectionPrefs.
		getBoolean("useGlacier", useGlacier);
	    autoconnect = _connectionPrefs.
		getBoolean("autoconnect", autoconnect);
	}

	void save()
	{
	    _connectionPrefs.put("username", username);
	    _connectionPrefs.putBoolean("autoconnect", autoconnect);
	    
	    _connectionPrefs.put("registryInstanceName", registryInstanceName);
	    _connectionPrefs.put("registryEndpoints", registryEndpoints);
	    
	    _connectionPrefs.putBoolean("useGlacier", useGlacier);
	    _connectionPrefs.put("routerInstanceName", routerInstanceName);
	    _connectionPrefs.put("routerEndpoints", routerEndpoints);

	    _connectionPrefs.putInt("timeout", timeout);
	    _connectionPrefs.putInt("connectTimeout", connectTimeout);
	}

	String username = System.getProperty("user.name");
	char[] password;
	boolean autoconnect = false;

	String registryInstanceName = "IceGrid";
	String registryEndpoints = "";

	boolean useGlacier = false;
	String routerInstanceName = "Glacier2";
	String routerEndpoints = "";

	int timeout = 0;
	int connectTimeout = 0;

	private Preferences _connectionPrefs;
    }




    private class LoginDialog extends JDialog
    {
	LoginDialog()
	{
	    super(_model.getMainFrame(), "Login - IceGrid Admin", true);

	    setDefaultCloseOperation(JDialog.HIDE_ON_CLOSE);
	    
	    _useGlacier.addItemListener(new ItemListener() 
		{
		    public void itemStateChanged(ItemEvent e)
		    {
			enableDisableGlacier();
		    }
		});
	    
	    JButton okButton = new JButton("OK");
	    ActionListener okListener = new ActionListener()
		{
		    public void actionPerformed(ActionEvent e)
		    {
			_loginInfo.username = _username.getText();
			_loginInfo.password = _password.getPassword();
			_loginInfo.autoconnect = _autoconnect.isSelected();
			_loginInfo.registryInstanceName = 
			    _registryInstanceName.getText();
			_loginInfo.registryEndpoints = 
			    _registryEndpoints.getText();
			_loginInfo.useGlacier = _useGlacier.isSelected();
			_loginInfo.routerInstanceName = 
			    _routerInstanceName.getText();
			_loginInfo.routerEndpoints = 
			    _routerEndpoints.getText();
	
			Object timeout = _timeout.getSelectedItem();
			if(timeout == TIMEOUT_NOT_SET)
			{
			    _loginInfo.timeout = 0;
			}
			else
			{
			    _loginInfo.timeout = 
				Integer.valueOf(timeout.toString()).intValue();
			}

			Object connectTimeout = _connectTimeout.getSelectedItem();
			if(connectTimeout == CONNECT_TIMEOUT_NOT_SET)
			{
			    _loginInfo.connectTimeout = 0;
			}
			else
			{
			    _loginInfo.connectTimeout = 
				Integer.valueOf(connectTimeout.toString()).intValue();
			}

		
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

	    _timeout.setEditable(true);
	    _connectTimeout.setEditable(true);

	    FormLayout layout = new FormLayout("right:pref, 3dlu, pref", "");
	    
	    DefaultFormBuilder builder = new DefaultFormBuilder(layout);
	    builder.setDefaultDialogBorder();
	    builder.setRowGroupingEnabled(true);
	    builder.setLineGapSize(LayoutStyle.getCurrent().getLinePad());

	    builder.append("Username", _username);
	    builder.nextLine();
	    builder.append(_passwordLabel, _password);
	    builder.nextLine();
	    builder.append("", _autoconnect);
	    builder.nextLine();
	    
	    builder.appendSeparator("IceGrid Registry");
	    builder.append("Instance Name", _registryInstanceName);
	    builder.nextLine();
	    builder.append("Endpoint(s)", _registryEndpoints);
	    builder.nextLine();
	    
	    builder.appendSeparator("Glacier2 Router");
	    builder.append("", _useGlacier);
	    builder.append(_routerInstanceNameLabel, _routerInstanceName);
	    builder.nextLine();
	    builder.append(_routerEndpointsLabel, _routerEndpoints);

	    builder.appendSeparator("Timeouts (in milliseconds)");
	    builder.append("Connection", _timeout);
	    builder.nextLine();
	    builder.append("Connection Establishment", _connectTimeout);
	    builder.nextLine();

	    JComponent buttonBar = 
		ButtonBarFactory.buildOKCancelBar(okButton, cancelButton);
	    buttonBar.setBorder(Borders.DIALOG_BORDER);

	    Container contentPane = getContentPane();
	    contentPane.setLayout(new BoxLayout(contentPane, BoxLayout.Y_AXIS));
	    contentPane.add(builder.getPanel());
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
		_username.setText(_loginInfo.username);
		_autoconnect.setSelected(_loginInfo.autoconnect);

		_registryInstanceName.setText(_loginInfo.registryInstanceName);
		_registryEndpoints.setText(_loginInfo.registryEndpoints);

		_useGlacier.setSelected(_loginInfo.useGlacier);
		enableDisableGlacier();
		_routerInstanceName.setText(_loginInfo.routerInstanceName);
		_routerEndpoints.setText(_loginInfo.routerEndpoints);

		if(_loginInfo.timeout == 0)
		{
		    _timeout.setSelectedItem(TIMEOUT_NOT_SET);
		}
		else
		{
		    _timeout.setSelectedItem(Integer.toString(_loginInfo.timeout));
		}

		if(_loginInfo.connectTimeout == 0)
		{
		    _connectTimeout.setSelectedItem(CONNECT_TIMEOUT_NOT_SET);
		}
		else
		{
		    _connectTimeout.setSelectedItem(
			Integer.toString(_loginInfo.connectTimeout));
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
       
	private void enableDisableGlacier()
	{
	    boolean selected = _useGlacier.isSelected();

	    _autoconnect.setEnabled(!selected);
	    _autoconnect.setSelected(false);
	    
	    _passwordLabel.setEnabled(selected);
	    _password.setEnabled(selected);
	    _routerInstanceNameLabel.setEnabled(selected);
	    _routerInstanceName.setEnabled(selected); 
	    _routerEndpointsLabel.setEnabled(selected); 
	    _routerEndpoints.setEnabled(selected);
	}


	private JTextField _username = new JTextField(30);
	private JLabel _passwordLabel = new JLabel("Password");
	private JPasswordField _password = new JPasswordField(30);
	private JCheckBox _autoconnect 
	    = new JCheckBox("Automatically log in at startup");
	    
	private JTextField _registryInstanceName = new JTextField(30);
	private JTextField _registryEndpoints = new JTextField(30);
	
	private JCheckBox _useGlacier 
	    = new JCheckBox("Login through a Glacier2 Router");

	private JLabel _routerInstanceNameLabel = new JLabel("Instance Name");
	private JTextField _routerInstanceName = new JTextField(30);
	private JLabel _routerEndpointsLabel = new JLabel("Endpoint(s)");
	private  JTextField _routerEndpoints = new JTextField(30);

	private JComboBox _timeout = new JComboBox(new Object[]
	    {TIMEOUT_NOT_SET, "10000", "60000", "600000"});

	private JComboBox _connectTimeout = new JComboBox(new Object[]
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
	if(autoconnectEnabled && !_loginInfo.useGlacier && 
	   _loginInfo.autoconnect)
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
	    long period = _session.getTimeout() * 1000/ 2;
	    period = 500;
	    System.err.println("period == " + period + " ms");
	    
	    
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
		try
		{
		    _session.destroy();
		}
		catch(Ice.LocalException e)
		{
		    // Ignored
		}
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
	    _observerAdapter.waitForDeactivate();
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
