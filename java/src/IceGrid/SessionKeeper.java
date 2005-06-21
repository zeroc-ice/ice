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

import java.awt.Frame;
import java.awt.Component;
import java.awt.Cursor;

import java.awt.event.ActionEvent;
import java.awt.event.ItemEvent;
import java.awt.event.ActionListener;
import java.awt.event.ItemListener;


import com.jgoodies.forms.builder.DefaultFormBuilder;
import com.jgoodies.forms.layout.FormLayout;
import com.jgoodies.forms.factories.ButtonBarFactory;

import java.util.prefs.Preferences;

//
// The SessionKeeper is responsible for establishing sessions (one at a time)
// and keeping them alive.
//

class SessionKeeper
{
    class ConnectDialog extends JDialog
    {
	ConnectDialog(ConnectInfo info)
	{
	    super(_parent, "New Connection - IceGrid Admin", true);
	    setDefaultCloseOperation(JDialog.DISPOSE_ON_CLOSE);
	    
	    _info = info;

	    _locatorProxy = new JTextField(30);
	    _locatorProxy.setText(_info.locatorProxy);
	    _username = new JTextField(30);
	    _username.setText(_info.username);
	    _passwordLabel = new JLabel("Password");
	    _password = new JPasswordField(30);

	    _adminIdentity = new JTextField(30);
	    _adminIdentity.setText(_info.adminIdentity);
	    _sessionManagerIdentity = new JTextField(30);
	    _sessionManagerIdentity.setText(_info.sessionManagerIdentity);

	    _useGlacier = new JCheckBox("Connect using Glacier");
	    _useGlacier.setSelected(_info.useGlacier);
	    _autoconnect = new JCheckBox("Autoconnect");
	    _autoconnect.setSelected(_info.autoconnect);

	    _autoconnect.setEnabled(!info.useGlacier);
	    _passwordLabel.setEnabled(info.useGlacier);
	    _password.setEnabled(info.useGlacier);

	    _useGlacier.addItemListener(new ItemListener() 
		{
		    public void itemStateChanged(ItemEvent e)
		    {
			boolean selected = _useGlacier.isSelected();
			_autoconnect.setEnabled(!selected);
			_autoconnect.setSelected(false);
			_passwordLabel.setEnabled(selected);
			_password.setEnabled(selected);
		    }
		});

	    
	    JButton okButton = new JButton("OK");
	    ActionListener okListener = new ActionListener()
		{
		    public void actionPerformed(ActionEvent e)
		    {
			_info.locatorProxy = _locatorProxy.getText();
			_info.username = _username.getText();
			_info.password = _password.getPassword();
			_info.useGlacier = _useGlacier.isSelected();
			_info.autoconnect = _autoconnect.isSelected();
			_info.adminIdentity = _adminIdentity.getText();
			_info.sessionManagerIdentity 
			    = _sessionManagerIdentity.getText();
			
			if(doConnect(ConnectDialog.this, _info))
			{
			    dispose();
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
			dispose();
		    }
		};
	    cancelButton.addActionListener(cancelListener);
	    //
	    // TODO: bind ESC to the cancel button, could be easier with an 
	    // Action.
	    //
	    
	    
	    //
	    // Build new content Panel with JGoodies's DefaultFormBuilder
	    //
	    FormLayout layout = new FormLayout("right:pref, 3dlu, pref", "");
	    
	    DefaultFormBuilder builder = new DefaultFormBuilder(layout);
	    builder.setDefaultDialogBorder();
	    
	    builder.append("IceGrid Locator proxy", _locatorProxy);
	    builder.nextLine();
	    builder.append("");
	    builder.nextLine();
	    builder.append("Username", _username);
	    builder.nextLine();
	    builder.append(_passwordLabel,_password);
	    builder.appendSeparator("Options");
	    builder.append("", _useGlacier);
	    builder.nextLine();
	    builder.append("", _autoconnect);
	    builder.appendSeparator("Advanced");
	    builder.append("Admin identity", _adminIdentity);
	    builder.nextLine();
	    builder.append("SessionManager identity", 
			   _sessionManagerIdentity);
	    builder.appendSeparator();
	    builder.append("");
	    builder.nextLine();
	    builder.append(ButtonBarFactory.buildOKCancelBar(okButton, 
							     cancelButton),
			   builder.getColumnCount());
	    
	    setContentPane(builder.getPanel());
	    pack();
	    setResizable(false);
	}

	void getSession()
	{
	    setLocationRelativeTo(_parent);
	    setVisible(true);
	}
	
	private ConnectInfo _info;

	private JTextField _locatorProxy;
	private JTextField _username;
	private JLabel _passwordLabel;
	private JPasswordField _password;
	private JCheckBox _useGlacier;
	private JCheckBox _autoconnect;
	private JTextField _adminIdentity;
	private JTextField _sessionManagerIdentity;
    }

    
    static class ConnectInfo
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
		    destroyObservers();
		    _model.lostSession();

		    SwingUtilities.invokeLater(new Runnable() 
			{
			    public void run() 
			    {
				JOptionPane.showMessageDialog(
				    _parent,
				    "Failed to contact the IceGrid registry: "  + e.toString(),
				    "Session lost",
				    JOptionPane.ERROR_MESSAGE);
				
				//
				// Offer New Connection dialog
				//
				createSession(false);
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


    SessionKeeper(Frame parent, Ice.Communicator communicator, Model model, 
		  Preferences prefs, StatusBar statusBar)
    {
	_parent = parent;
	_communicator = communicator;
	_model = model;
	_connectionPrefs = prefs.node("Connection");
	_statusBar = statusBar;
    }

    //
    // Runs in UI thread
    //
    void createSession(boolean autoconnectEnabled)
    {
	ConnectInfo connectInfo = new ConnectInfo(_connectionPrefs, 
						  _communicator);
	boolean openDialog = true;
	if(autoconnectEnabled && !connectInfo.useGlacier && 
	   connectInfo.autoconnect)
	{
	    openDialog = !doConnect(_parent, connectInfo);
	}
	if(openDialog)
	{
	    //
	    // When the user presses OK on the connect dialog, doConnect
	    // is called
	    //
	    (new ConnectDialog(connectInfo)).getSession();
	}
    }

    //
    // Runs in UI thread
    //
    private boolean doConnect(Component parent, ConnectInfo info)
    {
	if(_session != null)
	{
	    //
	    // First take care of previous session!
	    //
	    
	    if(_model.updated())
	    {
		boolean saveFirst = true; // TODO: add real dialog!
		
		if(saveFirst)
		{
		    boolean saved = _model.save();
		    if(!saved)
		    {
			//
			// dismiss Connect dialog
			//
			return true;
		    }
		}
	    }
	    
	    //
	    // Discard session
	    //
	    _model.lostSession();
	    destroyObservers();
	    releaseSession();
	}
	

	Cursor oldCursor = parent.getCursor();

	try
	{
	    parent.setCursor(Cursor.getPredefinedCursor(Cursor.WAIT_CURSOR));

	    assert(_session == null);
	    
	    //
	    // Establish session
	    //
	    
	    if(info.useGlacier)
	    {
		//
		// Not yet implemented
		//
		assert false;
	    }
	    else
	    {
		
		Ice.LocatorPrx defaultLocator = null;
		try
		{
		    defaultLocator = Ice.LocatorPrxHelper.
			uncheckedCast(
			    _communicator.stringToProxy(info.locatorProxy));
		}
		catch(Ice.LocalException e)
		{
		    parent.setCursor(oldCursor);
		    JOptionPane.showMessageDialog(
			parent,
			"The locator proxy is invalid: " + e.toString(),
			"Invalid locator proxy",
			JOptionPane.ERROR_MESSAGE);
		    return false;
		}
		_communicator.setDefaultLocator(defaultLocator);
		
		//
		// TODO: timeout
		//
		
		SessionManagerPrx sessionManager = SessionManagerPrxHelper.
		    uncheckedCast(
			_communicator.stringToProxy(
			    info.sessionManagerIdentity));
		
		try
		{
		    _session = sessionManager.createLocalSession(info.username);
		}
		catch(Ice.LocalException e)
		{
		    parent.setCursor(oldCursor);
		    JOptionPane.showMessageDialog(parent,
						  "Could not create session: "
						  + e.toString(),
						  "Connection failed",
						  JOptionPane.ERROR_MESSAGE);
		    return false;
		}
	    }
	    
	    _statusBar.setText("Connected");

	    //
	    // Start thread
	    //
	    assert(_thread == null);
	    
	    //
	    // TODO: get period from session
	    //
	    long period = 10000;
	    
	    _thread = new Pinger(period);
	    _thread.start();
	    
	    try
	    {
		registerObservers();
	    }
	    catch(Ice.LocalException e)
	    {
		releaseSession();
		parent.setCursor(oldCursor);
		JOptionPane.showMessageDialog(parent,
					      "Could not register observers: "
					      + e.toString(),
					      "Connection failed",
					      JOptionPane.ERROR_MESSAGE);
		return false;
	    }

	    info.save();
	}
	finally
	{
	    parent.setCursor(oldCursor);
	}
	    
	return true;
    }

    //
    // Runs in UI thread
    //
    private void releaseSession()
    {
	if(_session != null)
	{
	    _thread.done();
	    
	    for(;;)
	    {
		try
		{
		    _thread.join();
		}
		catch(InterruptedException e)
		{
		}
		break;
	    }
	    _thread = null;
	    _session = null;
	    _statusBar.setText("Not connected");
	}
    }

    //
    // Runs in UI thread
    //
    private void registerObservers()
    {
	try
	{
	    //
	    // Create the object adapter for the observers
	    //
	    String uuid = Ice.Util.generateUUID();
	    _observerAdapter = _communicator.createObjectAdapterWithEndpoints(
		"Observers-" + uuid, "tcp");

	    //
	    // Create servants and proxies
	    //
	    _registryObserverIdentity.name = uuid;
	    _registryObserverIdentity.category = "registryObserver";

	    _nodeObserverIdentity.name = uuid;
	    _nodeObserverIdentity.category = "nodeObserver";

	    RegistryObserverI registryObserverServant = new RegistryObserverI(
		_model);

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
	catch(Ice.LocalException e)
	{
	    destroyObservers();
	    throw e;
	}
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
	    _observerAdapter = null;
	}
    }
    

    SessionPrx getSession()
    {
	return _session;
    }
   
    
    private Frame _parent;
    private Ice.Communicator _communicator;
    private Model _model;
    private Preferences _connectionPrefs;
    private StatusBar _statusBar;

    private Pinger _thread;
    private SessionPrx _session;

    private Ice.ObjectAdapter _observerAdapter;
    private Ice.Identity _registryObserverIdentity = new Ice.Identity();
    private Ice.Identity _nodeObserverIdentity = new Ice.Identity();
}
