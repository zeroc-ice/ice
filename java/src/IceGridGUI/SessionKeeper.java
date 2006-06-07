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
import javax.swing.border.TitledBorder;

import java.io.FileInputStream;
import java.io.InputStream;
import java.io.File;
import java.security.KeyStore;

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
    //
    // An AdminSessionPrx and various objects associated with that session
    //
    private class Session
    {
	Session(AdminSessionPrx session, Component parent)
	{
	    _session = session;
	    
	    try
	    {
		_admin = session.getAdmin();
	    }
	    catch(Ice.LocalException e)
	    {
		logout(true);
		JOptionPane.showMessageDialog(
		    parent,
		    "Could not retrieve Admin proxy: " + e.toString(),
		    "Login failed",
		    JOptionPane.ERROR_MESSAGE);
		throw e;
	    }

	    long period = 0;

	    Glacier2.RouterPrx router = Glacier2.RouterPrxHelper.uncheckedCast(
		_coordinator.getCommunicator().getDefaultRouter());
	    if(router != null)
	    {
		period = router.getSessionTimeout() * 1000 / 2;
	    }
	    else
	    {
		period = session.getTimeout() * 1000 / 2;
	    }
	    
	    _thread = new Pinger(_session, period);
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
		throw e;
	    }
	}
	
	AdminSessionPrx getSession()
	{
	    return _session;
	}

	AdminPrx getAdmin()
	{
	    return _admin;
	}

	AdminPrx getRoutedAdmin()
	{
	    assert _admin != null;

	    if(_routedAdmin == null)
	    {
		//
		// Create a local Admin object used to route some operations to the real
		// Admin.
		// Routing admin calls is even necessary when we don't through Glacier2
		// since the Admin object provided by the registry is a well-known object
		// (indirect, locator-dependent).
		//
		Ice.ObjectAdapter adminRouterAdapter = _coordinator.getCommunicator().
		    createObjectAdapterWithEndpoints("IceGrid.AdminRouter", "tcp -h localhost");
		
		_routedAdmin = AdminPrxHelper.uncheckedCast(
		    adminRouterAdapter.addWithUUID(new AdminRouter(_admin)));
		
		adminRouterAdapter.activate();
	    }
	    return _routedAdmin;
	}

	void close(boolean destroySession)
	{
	    if(_thread != null)
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
	    }
	    
	    if(_adapter != null)
	    {
		try
		{
		    _adapter.remove(_registryObserverIdentity);
		}
		catch(Ice.NotRegisteredException e)
		{
		}
		
		try
		{
		    _adapter.remove(_nodeObserverIdentity);
		}
		catch(Ice.NotRegisteredException e)
		{
		}
	    }

	    if(destroySession)
	    {
		_coordinator.destroySession(_session);
	    }
	    _coordinator.getStatusBar().setConnected(false);
	}

	private void registerObservers()
	{
	    //
	    // Create the object adapter for the observers
	    //
	    String category;
	    
	    Glacier2.RouterPrx router = Glacier2.RouterPrxHelper.uncheckedCast(
		_coordinator.getCommunicator().getDefaultRouter());
	    if(router == null)
	    {
		category = "observer";

		_adapter = 
		    _coordinator.getCommunicator().createObjectAdapter("IceGrid.AdminGUI");
		_adapter.activate();
		_session.ice_getConnection().setAdapter(_adapter);
	    }
	    else
	    {
		category = router.getCategoryForClient();
		
		_adapter = 
		    _coordinator.getCommunicator().createObjectAdapterWithRouter("RoutedAdapter", router);
		_adapter.activate();
	    }
	    
	    //
	    // Create servants and proxies
	    //
	    _registryObserverIdentity.name = "registry";
	    _registryObserverIdentity.category = category;
	    _nodeObserverIdentity.name = "node";
	    _nodeObserverIdentity.category = category;
	    
	    RegistryObserverI registryObserverServant = new RegistryObserverI(
		_admin.ice_getIdentity().category, _coordinator);
	    
	    RegistryObserverPrx registryObserver = 
		RegistryObserverPrxHelper.uncheckedCast(
		    _adapter.add(
			registryObserverServant, _registryObserverIdentity));
	    
	    NodeObserverPrx nodeObserver =
		NodeObserverPrxHelper.uncheckedCast(
		    _adapter.add(
			new NodeObserverI(_coordinator), _nodeObserverIdentity));
	    
	    if(router == null)
	    {
		_session.setObservers(registryObserver, nodeObserver);
	    }
	    else
	    {
		_session.setObserversByIdentity(
		    _registryObserverIdentity, 
		    _registryObserverIdentity);
	    }
	    
	    registryObserverServant.waitForInit();
	}


	private final AdminSessionPrx _session;
	private Pinger _thread;
	
	private Ice.ObjectAdapter _adapter;
	private AdminPrx _admin;
	private AdminPrx _routedAdmin;
	private Ice.Identity _registryObserverIdentity = new Ice.Identity();
	private Ice.Identity _nodeObserverIdentity = new Ice.Identity();
    }


    static public class LoginInfo
    {
	LoginInfo(Preferences connectionPrefs, Coordinator coordinator)
	{
	    _connectionPrefs = connectionPrefs;

	    String prop = System.getProperty("java.net.ssl.keyStorePassword");
	    if(prop != null)
	    {
		keystorePassword = prop.toCharArray();
	    }
	    prop = System.getProperty("java.net.ssl.trustStorePassword");
	    if(prop != null)
	    {
		truststorePassword = prop.toCharArray();
	    }

	    Ice.Properties properties = coordinator.getProperties();

	    //
	    // Registry properties
	    //
	    String defaultLocator = properties.getProperty("Ice.Default.Locator");
	    Ice.ObjectPrx defaultLocatorProxy = null;
	    if(!defaultLocator.equals(""))
	    {
		try
		{
		    defaultLocatorProxy = coordinator.getCommunicator().stringToProxy(defaultLocator);
		}
		catch(Ice.LocalException e)
		{
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
		    defaultRouterProxy = coordinator.getCommunicator().stringToProxy(defaultRouter);
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
	    registryUseSSL = _connectionPrefs.getBoolean("registry.useSSL", registryUseSSL);

	    routerUsername = _connectionPrefs.get("router.username", routerUsername);
	    routerUseSSL = _connectionPrefs.getBoolean("router.useSSL", routerUseSSL);

	    routed = _connectionPrefs.getBoolean("routed", routed);

	    //
	    // SSL Configuration
	    //
	    String val = properties.getProperty("IceSSL.Keystore"); 
	    if(val.length() > 0)
	    {
		keystore = val;
	    }
	    else
	    {
		keystore = _connectionPrefs.get("keystore", keystore);
	    }

	    val = properties.getProperty("IceSSL.Alias");
	    if(val.length() > 0)
	    {
		alias = val;
	    }
	    else
	    {
		alias = _connectionPrefs.get("alias", "");
	    }

	    val = properties.getProperty("IceSSL.Truststore"); 
	    if(val.length() > 0)
	    {
		truststore = val;
	    }
	    else
	    {
		truststore = _connectionPrefs.get("truststore", keystore);
	    }
	}

	void save()
	{
	    _connectionPrefs.putBoolean("routed", routed);

	    if(routed)
	    {
		_connectionPrefs.put("router.username", routerUsername);
		_connectionPrefs.putBoolean("router.useSSL", routerUseSSL);
		_connectionPrefs.put("router.instanceName", routerInstanceName);
		_connectionPrefs.put("router.endpoints", routerEndpoints);
	    }
	    else
	    {
		_connectionPrefs.put("registry.username", registryUsername);
		_connectionPrefs.putBoolean("registry.useSSL", registryUseSSL);
		_connectionPrefs.put("registry.instanceName", registryInstanceName);
		_connectionPrefs.put("registry.endpoints", registryEndpoints);
	    }

	    //
	    // SSL Configuration
	    //
	    _connectionPrefs.put("keystore", keystore);
	    _connectionPrefs.put("alias", alias);
	    _connectionPrefs.put("truststore", keystore);
	}

	boolean routed = false;

	String registryUsername = System.getProperty("user.name");
	char[] registryPassword;
	boolean registryUseSSL = false;
	String registryInstanceName = "IceGrid";
	String registryEndpoints = "";

	String routerUsername = System.getProperty("user.name");
	char[] routerPassword;
	boolean routerUseSSL = false;
	String routerInstanceName = "Glacier2";
	String routerEndpoints = "";
	
	//
	// SSL Configuration
	//
	String keystore = System.getProperty("java.net.ssl.keyStore");
	char[] keyPassword;
	char[] keystorePassword;
	String alias;
	String truststore = System.getProperty("java.net.ssl.trustStore");
	char[] truststorePassword;
	private Preferences _connectionPrefs;
    }

    private class LoginDialog extends JDialog
    {
	LoginDialog()
	{
	    super(_coordinator.getMainFrame(), "Login - IceGrid Admin", true);
	    setDefaultCloseOperation(JDialog.HIDE_ON_CLOSE);

	    final File defaultDir = new java.io.File(_coordinator.getProperties().getProperty("IceSSL.DefaultDir"));
	    _keystoreType = 
		_coordinator.getProperties().getPropertyWithDefault("IceSSL.KeystoreType", 
								    java.security.KeyStore.getDefaultType());
	   
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

	    Action registryUseSSL = new AbstractAction("Use SSL for authentication")
		{
		    public void actionPerformed(ActionEvent e) 
		    {
			boolean selected = _registryUseSSL.isSelected();
			_registryUsername.setEnabled(!selected);
			_registryUsernameLabel.setEnabled(!selected);
			_registryPassword.setEnabled(!selected);
			_registryPasswordLabel.setEnabled(!selected);
		    }
		};
	    _registryUseSSL = new JCheckBox(registryUseSSL);
	    

	    Action routerUseSSL = new AbstractAction("Use SSL for authentication")
		{
		    public void actionPerformed(ActionEvent e) 
		    {
			boolean selected = _routerUseSSL.isSelected();
			_routerUsername.setEnabled(!selected);
			_routerUsernameLabel.setEnabled(!selected);
			_routerPassword.setEnabled(!selected);
			_routerPasswordLabel.setEnabled(!selected);
		    }
		};
	    _routerUseSSL = new JCheckBox(routerUseSSL);

	    _keystore.setEditable(false);
	    _advancedKeystore.setEditable(false);
	    Action chooseKeystore = new AbstractAction("...")
		{
		    public void actionPerformed(ActionEvent e) 
		    {
			String store = _keystore.getText();
			if(store == null || store.length() == 0)
			{
			    _fileChooser.setCurrentDirectory(defaultDir);
			}
			else
			{
			    File file = new File(store);
			    if(file.isAbsolute())
			    {
				_fileChooser.setSelectedFile(file);
			    }
			    else
			    {
				_fileChooser.setSelectedFile(new File(defaultDir, store));
			    }
			}

			int result = _fileChooser.showOpenDialog(LoginDialog.this);
			if(result == JFileChooser.APPROVE_OPTION)
			{
			    File file = _fileChooser.getSelectedFile();
			    _keystore.setText(file.getAbsolutePath());
			    updateAlias(file, _alias.getSelectedItem());
			}
		    }
		    private JFileChooser _fileChooser = new JFileChooser(); 
		};

	    _truststore.setEditable(false);
	    Action chooseTruststore = new AbstractAction("...")
		{
		    public void actionPerformed(ActionEvent e) 
		    {
			String store = _truststore.getText();
			if(store == null || store.length() == 0)
			{
			    _fileChooser.setCurrentDirectory(defaultDir);
			}
			else
			{
			    File file = new File(store);
			    if(file.isAbsolute())
			    {
				_fileChooser.setSelectedFile(file);
			    }
			    else
			    {
				_fileChooser.setSelectedFile(new File(defaultDir, store));
			    }
			}

			int result = _fileChooser.showOpenDialog(LoginDialog.this);
			if(result == JFileChooser.APPROVE_OPTION)
			{
			    File file = _fileChooser.getSelectedFile();
			    _truststore.setText(file.getAbsolutePath());
			}
		    }

		    private JFileChooser _fileChooser = new JFileChooser(); 
		};

	    
	    _registryUsername.setToolTipText("Your username");
	    _registryPassword.setToolTipText("Your password in this IceGrid registry");
	    _registryUseSSL.setToolTipText("Do you want to use SSL instead of username/password for authentication?");
	    _registryInstanceName.setToolTipText("The instance name of your IceGrid registry. For example: DemoIceGrid");
	    _registryEndpoints.setToolTipText("<html>Corresponds to the client endpoints of this IceGrid registry.<br>"
					      + "For example: tcp -h registry.domain.com -p 12000</html>");
	    
	    _routerUsername.setToolTipText("Your username");
	    _routerUsername.setToolTipText("Your password in this Glacier2 router");
	    _routerUseSSL.setToolTipText("Do you want to use SSL instead of username/password for authentication?");
	    _routerInstanceName.setToolTipText("The instance name of your Glacier2 router. For example: DemoGlacier2");
	    _routerEndpoints.setToolTipText("<html>Corresponds to client endpoints of this Glacier2 router.<br>"
					      + "For example: ssl -h glacier2router.domain.com -p 11000</html>");

	    _keystore.setToolTipText("SSL keystore file");
	    _keyPassword.setToolTipText("Password for keys in the selected keystore file");
	    _advancedKeystore.setToolTipText("SSL keystore file");
	    _advancedKeyPassword.setToolTipText("Password for keys in the selected keystore file");

	    _keystorePassword.setToolTipText("Password used to check the integrity of the keystore");
	    _alias.setToolTipText("Use this alias when authenticating IceGrid Admin with the IceGrid registry or Glacier2 router");
	    
	    _truststore.setToolTipText("SSL truststore file");
	    _truststorePassword.setToolTipText("Password used to check the integrity of the truststore");
	      

	    JPanel directPanel = null;
	    {
		FormLayout layout = new FormLayout("right:pref, 3dlu, pref", "");
		
		DefaultFormBuilder builder = new DefaultFormBuilder(layout);
		builder.setDefaultDialogBorder();
		builder.setRowGroupingEnabled(true);
		builder.setLineGapSize(LayoutStyle.getCurrent().getLinePad());
		
		_registryUsernameLabel = builder.append("Username", _registryUsername);
		builder.nextLine();
		_registryPasswordLabel = builder.append("Password", _registryPassword);
		builder.nextLine();
		builder.append("", _registryUseSSL);
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
		
		_routerUsernameLabel = builder.append("Username", _routerUsername);
		builder.nextLine();
		_routerPasswordLabel = builder.append("Password", _routerPassword);
		builder.nextLine();
		builder.append("", _routerUseSSL);
		builder.nextLine();
		builder.append("Glacier2 Instance Name", _routerInstanceName);
		builder.nextLine();
		builder.append("Glacier2 Router Endpoint(s)", _routerEndpoints);
		builder.nextLine();
		
		routedPanel = builder.getPanel();
	    }
	    
	    _mainPane.addTab("Direct", null, directPanel, "Log directly into the IceGrid registry");
	    _mainPane.addTab("Routed", null, routedPanel, "Log into the IceGrid registry through a Glacier2 router");
	    _mainPane.setBorder(Borders.DIALOG_BORDER);

	    JPanel basicSSLPanel = null;
	    {
		FormLayout layout = new FormLayout(
		    "right:pref, 3dlu, fill:pref:grow, 3dlu, pref", "");
		
		DefaultFormBuilder builder = new DefaultFormBuilder(layout);
		builder.setDefaultDialogBorder();
		builder.setRowGroupingEnabled(true);
		builder.setLineGapSize(LayoutStyle.getCurrent().getLinePad());
		
		builder.appendSeparator("Keystore");
		builder.append("File", _keystore);
		builder.append(new JButton(chooseKeystore));
		builder.nextLine();
		builder.append("Key Password");
		builder.append(_keyPassword, 3);
		builder.nextLine();
		
		basicSSLPanel = builder.getPanel();
	    }
	    
	    JPanel advancedSSLPanel = null;
	    {
		FormLayout layout = new FormLayout(
		    "right:pref, 3dlu, fill:pref:grow, 3dlu, pref", "");
		
		DefaultFormBuilder builder = new DefaultFormBuilder(layout);
		builder.setDefaultDialogBorder();
		builder.setRowGroupingEnabled(true);
		builder.setLineGapSize(LayoutStyle.getCurrent().getLinePad());
		
		builder.appendSeparator("Keystore");
		builder.append("File", _advancedKeystore);
		builder.append(new JButton(chooseKeystore));
		builder.nextLine();
		builder.append("Key Password");
		builder.append(_advancedKeyPassword, 3);
		builder.nextLine();
		builder.append("Integrity Password");
		builder.append(_keystorePassword, 3);
		builder.nextLine();
		builder.append("Alias");
		builder.append(_alias, 3);
		builder.nextLine();
		
		builder.appendSeparator("Truststore");
		builder.append("File", _truststore);
		builder.append(new JButton(chooseTruststore));;
		builder.nextLine();
		builder.append("Integrity Password");
		builder.append(_truststorePassword, 3);
		builder.nextLine();
	
		advancedSSLPanel = builder.getPanel();
	    }

	    JTabbedPane sslPane = new JTabbedPane();

	    sslPane.addTab("Basic", basicSSLPanel);
	    sslPane.addTab("Advanced", advancedSSLPanel);
	    TitledBorder titledBorder = BorderFactory.createTitledBorder(Borders.DIALOG_BORDER,
									 "SSL Configuration");
	    sslPane.setBorder(titledBorder);


	    JComponent buttonBar = 
		    ButtonBarFactory.buildOKCancelBar(okButton, cancelButton);
	    buttonBar.setBorder(Borders.DIALOG_BORDER);
	    


	    Container contentPane = getContentPane();
	    contentPane.setLayout(new BoxLayout(contentPane, BoxLayout.Y_AXIS));
	    contentPane.add(_mainPane);
	    contentPane.add(sslPane);
	    contentPane.add(buttonBar);

	    pack();
	    setResizable(false);
	}

	
	void showDialog()
	{
	    if(isVisible() == false)
	    {
		_mainPane.setSelectedIndex(_loginInfo.routed ? 1 : 0);

		_registryUsername.setText(_loginInfo.registryUsername);
		_registryUseSSL.setSelected(_loginInfo.registryUseSSL);
		_registryInstanceName.setText(_loginInfo.registryInstanceName);
		_registryEndpoints.setText(_loginInfo.registryEndpoints);
	
		_routerUsername.setText(_loginInfo.routerUsername);
		_routerUseSSL.setSelected(_loginInfo.routerUseSSL);
		_routerInstanceName.setText(_loginInfo.routerInstanceName);
		_routerEndpoints.setText(_loginInfo.routerEndpoints);

		_keystore.setText(_loginInfo.keystore);
		if(_loginInfo.keystore == null)
		{
		    clearAlias();
		}
		else
		{
		    updateAlias(new File(_loginInfo.keystore), _loginInfo.alias);
		}
		_truststore.setText(_loginInfo.truststore);

		setLocationRelativeTo(_coordinator.getMainFrame());
		setVisible(true);
	    }
	  
	    //
	    // Otherwise it was already on the screen!
	    //
	}
       
	private void writeInfo()
	{
	    _loginInfo.routed = (_mainPane.getSelectedIndex() == 1);

	    _loginInfo.registryUsername = _registryUsername.getText();
	    _loginInfo.registryPassword = _registryPassword.getPassword();
	    _loginInfo.registryUseSSL = _registryUseSSL.isSelected();
	    _loginInfo.registryInstanceName = _registryInstanceName.getText();
	    _loginInfo.registryEndpoints = _registryEndpoints.getText();
	 
	    _loginInfo.routerUsername = _routerUsername.getText();
	    _loginInfo.routerPassword = _routerPassword.getPassword();
	    _loginInfo.routerUseSSL = _routerUseSSL.isSelected();
	    _loginInfo.routerInstanceName = _routerInstanceName.getText();
	    _loginInfo.routerEndpoints = _routerEndpoints.getText();

	    _loginInfo.keystore = _keystore.getText();
	    _loginInfo.keyPassword = _keyPassword.getPassword();
	    _loginInfo.keystorePassword = _keystorePassword.getPassword();
	    if(_alias.getSelectedItem() == null)
	    {
		_loginInfo.alias = "";
	    }
	    else
	    {
		_loginInfo.alias = _alias.getSelectedItem().toString();
	    }
	    _loginInfo.truststore = _truststore.getText();
	    _loginInfo.truststorePassword = _truststorePassword.getPassword();
	}

	private void updateAlias(File file, Object selectedAlias)
	{
	    if(file.isFile())
	    {
		InputStream is = null;
		try
		{
		    is = new FileInputStream(file);
		}
		catch(java.io.IOException e)
		{
		    clearAlias();
		    return;
		}

		java.util.Vector aliasVector = new java.util.Vector();

		try
		{
		    KeyStore ks = KeyStore.getInstance(_keystoreType);
		    ks.load(is, null);
		    java.util.Enumeration p = ks.aliases();
		    while(p.hasMoreElements())
		    {
			aliasVector.add(p.nextElement());
		    }
		}
		catch(Exception e)
		{
		    clearAlias();
		    return;
		}
		finally
		{
		    try
		    {
			is.close();
		    }
		    catch(java.io.IOException e)
		    {}
		}
		_alias.setModel(new DefaultComboBoxModel(aliasVector));
		if(selectedAlias != null)
		{
		    _alias.setSelectedItem(selectedAlias);
		}
	    }
	    else
	    {
		clearAlias();
	    }
	}

	private void clearAlias()
	{
	    _alias.setModel(new DefaultComboBoxModel());
	}


	private JTabbedPane _mainPane = new JTabbedPane();
	private JTextField _registryUsername = new JTextField(30);
	private JLabel _registryUsernameLabel;
	private JPasswordField _registryPassword = new JPasswordField(30);
	private JLabel _registryPasswordLabel;
	private JCheckBox _registryUseSSL;
	private JTextField _registryInstanceName = new JTextField(30);
	private JTextField _registryEndpoints = new JTextField(30);
	
	private JTextField _routerUsername = new JTextField(30);
	private JLabel _routerUsernameLabel;
	private JPasswordField _routerPassword = new JPasswordField(30);
	private JLabel _routerPasswordLabel;
	private JCheckBox _routerUseSSL;
	private JTextField _routerInstanceName = new JTextField(30);
	private JTextField _routerEndpoints = new JTextField(30);

	private JTextField _keystore = new JTextField(30);
	private JPasswordField _keyPassword = new JPasswordField(30);

	private JTextField _advancedKeystore = new JTextField(
	    _keystore.getDocument(), null, 30);
	private JPasswordField _advancedKeyPassword = new JPasswordField(
	    _keyPassword.getDocument(), null, 30);
	
	private JPasswordField _keystorePassword = new JPasswordField(30);
	private JComboBox _alias = new JComboBox();

	private JTextField _truststore = new JTextField(30);
	private JPasswordField _truststorePassword = new JPasswordField(30);

	private String _keystoreType;
    }

   
    //
    // We create a brand new Pinger thread for each session
    //
    class Pinger extends Thread
    {
	Pinger(AdminSessionPrx session, long period)
	{
	    _session = session;
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
	
	private AdminSessionPrx _session;
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
	_loginInfo = new LoginInfo(_loginPrefs, _coordinator);
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
       
	    AdminSessionPrx session = _coordinator.login(_loginInfo, parent);
	    if(session == null)
	    {
		return false;
	    }
	    _coordinator.getStatusBar().setConnected(true);

	    try
	    {
		_session = new Session(session, parent);
	    }
	    catch(Ice.LocalException e)
	    {
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
    }

    void logout(boolean destroySession)
    {
	_session.close(destroySession);
	_coordinator.sessionLost();
	_session = null;
    }
   
    AdminSessionPrx getSession()
    {
	return _session == null ? null : _session.getSession();
    }
    
    AdminPrx getAdmin()
    {
	return _session == null ? null : _session.getAdmin();
    }

    AdminPrx getRoutedAdmin()
    {
	return _session == null ? null : _session.getRoutedAdmin();
    }
   
    private LoginDialog _loginDialog;
    private LoginInfo _loginInfo;

    private Coordinator _coordinator;
    private Preferences _loginPrefs;
  
    private Session _session;
}
