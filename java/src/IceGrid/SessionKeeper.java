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
	ConnectDialog()
	{
	    super(_parent, "New Connection - IceGrid Admin", true);
	    setDefaultCloseOperation(JDialog.HIDE_ON_CLOSE);
	    
	    _locatorProxy = new JTextField(30);
	    _username = new JTextField(30);
	    _passwordLabel = new JLabel("Password");
	    _password = new JPasswordField(30);
	    _adminIdentity = new JTextField(30);
	    _sessionManagerIdentity = new JTextField(30);
	    _useGlacier = new JCheckBox("Connect using Glacier");
	    _autoconnect = new JCheckBox("Autoconnect");
	
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

	boolean showDialog(Model.ConnectInfo info)
	{
	    if(isVisible() == false)
	    {
		_info = info;
		_locatorProxy.setText(_info.locatorProxy);
		_username.setText(_info.username);
		_adminIdentity.setText(_info.adminIdentity);
		_sessionManagerIdentity.setText(_info.sessionManagerIdentity);

		_useGlacier.setSelected(_info.useGlacier);
		_autoconnect.setSelected(_info.autoconnect);
		_autoconnect.setEnabled(!info.useGlacier);
		_passwordLabel.setEnabled(info.useGlacier);
		_password.setEnabled(info.useGlacier);

		setLocationRelativeTo(_parent);
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
	
	private Model.ConnectInfo _info;

	private JTextField _locatorProxy;
	private JTextField _username;
	private JLabel _passwordLabel;
	private JPasswordField _password;
	private JCheckBox _useGlacier;
	private JCheckBox _autoconnect;
	private JTextField _adminIdentity;
	private JTextField _sessionManagerIdentity;
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
				sessionLost("Failed to contact the IceGrid registry: "  + e.toString());
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


    SessionKeeper(Frame parent, Model model, 
		  Preferences prefs)
    {
	_parent = parent;
	_connectDialog = new ConnectDialog();
	_model = model;
	_connectionPrefs = prefs.node("Connection");
    }

    //
    // Runs in UI thread
    //
    void createSession(boolean autoconnectEnabled)
    {
	Model.ConnectInfo connectInfo = new Model.ConnectInfo(_connectionPrefs, 
						  _model.getCommunicator());
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
	    _connectDialog.showDialog(connectInfo);
	}
    }

    //
    // Runs in UI thread
    //
    private boolean doConnect(Component parent, Model.ConnectInfo info)
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
	    destroyObservers();
	    releaseSession();
	    _model.sessionLost();
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
		if(!_model.setConnectInfo(info, parent, oldCursor))
		{
		    return false;
		}
		
		try
		{
		    _session = _model.getSessionManager().createLocalSession(info.username);
		}
		catch(Ice.LocalException e)
		{
		    _model.sessionLost();
		    parent.setCursor(oldCursor);
		    JOptionPane.showMessageDialog(parent,
						  "Could not create session: "
						  + e.toString(),
						  "Connection failed",
						  JOptionPane.ERROR_MESSAGE);
		    return false;
		}
	    }
	    
	    _model.getStatusBar().setText("Connected");

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
    void sessionLost(String message)
    {
	destroyObservers();
	releaseSession();
	_model.sessionLost();

	JOptionPane.showMessageDialog(
	    _parent,
	    message,
	    "Session lost",
	    JOptionPane.ERROR_MESSAGE);
	
	//
	// Offer new Connection dialog
	//
	createSession(false);
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
	    _model.getStatusBar().setText("Not connected");
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
	    _observerAdapter = _model.getCommunicator().createObjectAdapterWithEndpoints(
		"Observers-" + uuid, "tcp");

	    //
	    // Create servants and proxies
	    //
	    _registryObserverIdentity.name = uuid;
	    _registryObserverIdentity.category = "registryObserver";

	    _nodeObserverIdentity.name = uuid;
	    _nodeObserverIdentity.category = "nodeObserver";

	    RegistryObserverI registryObserverServant = new RegistryObserverI(
		this, _model);

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
    private ConnectDialog _connectDialog;

    private Model _model;
    private Preferences _connectionPrefs;
  
    private Pinger _thread;
    private SessionPrx _session;

    private Ice.ObjectAdapter _observerAdapter;
    private Ice.Identity _registryObserverIdentity = new Ice.Identity();
    private Ice.Identity _nodeObserverIdentity = new Ice.Identity();
}
