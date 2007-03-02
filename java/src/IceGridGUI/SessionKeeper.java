// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
        Session(AdminSessionPrx session, long keepAliveperiod, Component parent)
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

            if(_coordinator.getCommunicator().getDefaultRouter() == null)
            {
                _admin = AdminPrxHelper.uncheckedCast(_admin.ice_endpoints(session.ice_getEndpoints()));
            }

            _thread = new Pinger(_session, keepAliveperiod);
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
        
        void logout(boolean destroySession)
        {
            close(destroySession);
            _coordinator.sessionLost();
            _connectedToMaster = false;
            _replicaName = "";
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
                    _adapter.remove(_applicationObserverIdentity);
                }
                catch(Ice.NotRegisteredException e)
                {
                }

                try
                {
                    _adapter.remove(_adapterObserverIdentity);
                }
                catch(Ice.NotRegisteredException e)
                {
                }

                try
                {
                    _adapter.remove(_objectObserverIdentity);
                }
                catch(Ice.NotRegisteredException e)
                {
                }
                
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
                    _coordinator.getCommunicator().createObjectAdapter("");
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
            _applicationObserverIdentity.name = "application-" + Ice.Util.generateUUID();
            _applicationObserverIdentity.category = category;
            _adapterObserverIdentity.name = "adapter-" + Ice.Util.generateUUID();
            _adapterObserverIdentity.category = category;
            _objectObserverIdentity.name = "object-" + Ice.Util.generateUUID();
            _objectObserverIdentity.category = category;
            _registryObserverIdentity.name = "registry-" + Ice.Util.generateUUID();
            _registryObserverIdentity.category = category;
            _nodeObserverIdentity.name = "node-" + Ice.Util.generateUUID();
            _nodeObserverIdentity.category = category;
            
            ApplicationObserverI applicationObserverServant = new ApplicationObserverI(
                _admin.ice_getIdentity().category, _coordinator);
            
            ApplicationObserverPrx applicationObserver = 
                ApplicationObserverPrxHelper.uncheckedCast(
                    _adapter.add(
                        applicationObserverServant, _applicationObserverIdentity));

            AdapterObserverPrx adapterObserver = 
                AdapterObserverPrxHelper.uncheckedCast(
                    _adapter.add(
                        new AdapterObserverI(_coordinator), _adapterObserverIdentity));

            ObjectObserverPrx objectObserver = 
                ObjectObserverPrxHelper.uncheckedCast(
                    _adapter.add(
                        new ObjectObserverI(_coordinator), _objectObserverIdentity));
            
            RegistryObserverPrx registryObserver =
                RegistryObserverPrxHelper.uncheckedCast(
                    _adapter.add(
                        new RegistryObserverI(_coordinator), _registryObserverIdentity));
            
            NodeObserverPrx nodeObserver =
                NodeObserverPrxHelper.uncheckedCast(
                    _adapter.add(
                        new NodeObserverI(_coordinator), _nodeObserverIdentity));

            try
            {
                if(router != null)
                {
                    _session.setObservers(registryObserver, 
                                          nodeObserver, 
                                          applicationObserver, 
                                          adapterObserver, 
                                          objectObserver); 
                }
                else
                {
                    _session.setObserversByIdentity(
                        _registryObserverIdentity,
                        _nodeObserverIdentity, 
                        _applicationObserverIdentity,
                        _adapterObserverIdentity,
                        _objectObserverIdentity);
                }
            }
            catch(ObserverAlreadyRegisteredException ex)
            {
                assert false; // We use UUIDs for the observer identities.
            }
            
            applicationObserverServant.waitForInit();
        }


        private final AdminSessionPrx _session;
        private Pinger _thread;
        
        private Ice.ObjectAdapter _adapter;
        private AdminPrx _admin;
        private AdminPrx _routedAdmin;
        private Ice.Identity _applicationObserverIdentity = new Ice.Identity();
        private Ice.Identity _adapterObserverIdentity = new Ice.Identity();
        private Ice.Identity _objectObserverIdentity = new Ice.Identity();
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
            Ice.ObjectPrx defaultLocatorProxy = null;
            try
            {
                defaultLocatorProxy = coordinator.getCommunicator().propertyToProxy("Ice.Default.Locator");
            }
            catch(Ice.LocalException e)
            {
                // Ignored, keep null defaultLocatorProxy
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
            connectToMaster = _connectionPrefs.getBoolean("registry.connectToMaster", connectToMaster);

            
            //
            // Glacier2 properties
            //
            Ice.ObjectPrx defaultRouterProxy = null;
            try
            {
                defaultRouterProxy = coordinator.getCommunicator().propertyToProxy("Ice.Default.Router");
            }
            catch(Ice.LocalException e)
            {
                // Ignored, keep null defaultRouterProxy
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
                routerInstanceName = _connectionPrefs.get("router.instanceName", routerInstanceName);
                routerEndpoints = _connectionPrefs.get("router.endpoints", routerEndpoints);
            }
            registrySSLEnabled = _connectionPrefs.getBoolean("routerSSLEnabled", registrySSLEnabled);

            //
            // Property, if defined, prevails
            //
            registryUsername = _connectionPrefs.get("registry.username", registryUsername);
            registryUsername = properties.getPropertyWithDefault("IceGridAdmin.Username", registryUsername);
            registryPassword = properties.getProperty("IceGridAdmin.Password").toCharArray();
                 
            registryUseSSL = _connectionPrefs.getBoolean("registry.useSSL", registryUseSSL);
            registryUseSSL = properties.getPropertyAsIntWithDefault("IceGridAdmin.AuthenticateUsingSSL",
                                                               registryUseSSL ? 1 : 0) > 0;

            registrySSLEnabled = registryUseSSL || _connectionPrefs.getBoolean("registry.sslEnabled", registrySSLEnabled);

            routerUsername = _connectionPrefs.get("router.username", routerUsername);
            routerUsername = properties.getPropertyWithDefault("IceGridAdmin.Username", routerUsername);
            routerPassword = properties.getProperty("IceGridAdmin.Password").toCharArray();

            routerUseSSL = _connectionPrefs.getBoolean("router.useSSL", routerUseSSL);
            routerUseSSL = properties.getPropertyAsIntWithDefault("IceGridAdmin.AuthenticateUsingSSL",
                                                             routerUseSSL ? 1 : 0) > 0;

            routerSSLEnabled = routerUseSSL || _connectionPrefs.getBoolean("router.sslEnabled", routerSSLEnabled);

            routed = _connectionPrefs.getBoolean("routed", routed);

            //
            // SSL Configuration
            //
            keystore = properties.getPropertyWithDefault("IceSSL.Keystore",
                                                         _connectionPrefs.get("keystore", keystore));      
            
            alias = properties.getPropertyWithDefault("IceSSL.Alias",
                                                      _connectionPrefs.get("alias", ""));

            truststore = properties.getPropertyWithDefault("IceSSL.Truststore",
                                                           _connectionPrefs.get("truststore", truststore));           
        }

        void save()
        {
            _connectionPrefs.putBoolean("routed", routed);

            if(routed)
            {
                _connectionPrefs.put("router.username", routerUsername);
                _connectionPrefs.putBoolean("router.useSSL", routerUseSSL);
                _connectionPrefs.putBoolean("router.sslEnabled", routerSSLEnabled);
                _connectionPrefs.put("router.instanceName", routerInstanceName);
                _connectionPrefs.put("router.endpoints", routerEndpoints);
            }
            else
            {
                _connectionPrefs.put("registry.username", registryUsername);
                _connectionPrefs.putBoolean("registry.useSSL", registryUseSSL);
                _connectionPrefs.putBoolean("registry.sslEnabled", registrySSLEnabled);
                _connectionPrefs.put("registry.instanceName", registryInstanceName);
                _connectionPrefs.put("registry.endpoints", registryEndpoints);
                _connectionPrefs.putBoolean("registry.connectToMaster", connectToMaster);
            }

            //
            // SSL Configuration
            //
            _connectionPrefs.put("keystore", keystore);
            _connectionPrefs.put("alias", alias);
            _connectionPrefs.put("truststore", truststore);
        }

        boolean routed = false;

        String registryUsername = System.getProperty("user.name");
        char[] registryPassword;
        boolean registryUseSSL = false;
        boolean registrySSLEnabled = false;
        String registryInstanceName = "IceGrid";
        String registryEndpoints = "";
        boolean connectToMaster = true;

        String routerUsername = System.getProperty("user.name");
        char[] routerPassword;
        boolean routerUseSSL = false;
        boolean routerSSLEnabled = false;
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
                        selectRegistryUseSSL(_registryUseSSL.isSelected());
                    }
                };
            _registryUseSSL = new JCheckBox(registryUseSSL);
            

            Action routerUseSSL = new AbstractAction("Use SSL for authentication")
                {
                    public void actionPerformed(ActionEvent e) 
                    {
                        selectRouterUseSSL(_routerUseSSL.isSelected());
                    }
                };
            _routerUseSSL = new JCheckBox(routerUseSSL);

            Action registrySSLEnabled = new AbstractAction("Enable IceSSL")
                {
                    public void actionPerformed(ActionEvent e) 
                    {
                        selectRegistrySSLEnabled(_registrySSLEnabled.isSelected());
                    }
                };
            _registrySSLEnabled = new JCheckBox(registrySSLEnabled);
            
            Action routerSSLEnabled = new AbstractAction("Enable IceSSL")
                {
                    public void actionPerformed(ActionEvent e) 
                    {
                        selectRouterSSLEnabled(_routerSSLEnabled.isSelected());
                    }
                };
            _routerSSLEnabled = new JCheckBox(routerSSLEnabled);


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
            _routerPassword.setToolTipText("Your password in this Glacier2 router");
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
                builder.append("", _registrySSLEnabled);
                builder.nextLine();
                builder.append("IceGrid Instance Name", _registryInstanceName);
                builder.nextLine();
                builder.append("IceGrid Registry Endpoint(s)", _registryEndpoints);
                builder.nextLine();
                builder.append("", _connectToMaster);
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
                builder.append("", _routerSSLEnabled);
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

            _mainPane.addChangeListener(new javax.swing.event.ChangeListener()
                {
                    public void stateChanged(javax.swing.event.ChangeEvent e)
                    {
                        if(_mainPane.getSelectedIndex() == 0)
                        {
                            directTabSelected();
                        }
                        else
                        {
                            routedTabSelected(); 
                        }
                    }
                });

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

            
            _sslPane.addTab("Basic", basicSSLPanel);
            _sslPane.addTab("Advanced", advancedSSLPanel);
            TitledBorder titledBorder = BorderFactory.createTitledBorder(Borders.DIALOG_BORDER,
                                                                         "SSL Configuration");
            _sslPane.setBorder(titledBorder);

            JComponent buttonBar = 
                    ButtonBarFactory.buildOKCancelBar(okButton, cancelButton);
            buttonBar.setBorder(Borders.DIALOG_BORDER);
            
            Container contentPane = getContentPane();
            contentPane.setLayout(new BoxLayout(contentPane, BoxLayout.Y_AXIS));
            contentPane.add(_mainPane);
            contentPane.add(_sslPane);
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
                _registryPassword.setText(new String(_loginInfo.registryPassword));

                selectRegistryUseSSL(_loginInfo.registryUseSSL);

                _registryInstanceName.setText(_loginInfo.registryInstanceName);
                _registryEndpoints.setText(_loginInfo.registryEndpoints);
                _registrySSLEnabled.setSelected(_loginInfo.registrySSLEnabled);
                _connectToMaster.setSelected(_loginInfo.connectToMaster);

                _routerUsername.setText(_loginInfo.routerUsername);
                _routerPassword.setText(new String(_loginInfo.routerPassword));
                selectRouterUseSSL(_loginInfo.routerUseSSL);
        
                _routerInstanceName.setText(_loginInfo.routerInstanceName);
                _routerEndpoints.setText(_loginInfo.routerEndpoints);
                _routerSSLEnabled.setSelected(_loginInfo.routerSSLEnabled);
                
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

                if(_loginInfo.routed)
                {
                    routedTabSelected();
                }
                else
                {
                    directTabSelected();
                }

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
            _loginInfo.registrySSLEnabled = _registrySSLEnabled.isSelected();
            _loginInfo.registryInstanceName = _registryInstanceName.getText();
            _loginInfo.registryEndpoints = _registryEndpoints.getText();
            _loginInfo.connectToMaster = _connectToMaster.isSelected();
         
            _loginInfo.routerUsername = _routerUsername.getText();
            _loginInfo.routerPassword = _routerPassword.getPassword();
            _loginInfo.routerUseSSL = _routerUseSSL.isSelected();
            _loginInfo.routerSSLEnabled = _routerSSLEnabled.isSelected();
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

        private void selectRegistryUseSSL(boolean selected)
        {
            _registryUseSSL.setSelected(selected);
            _registryUsername.setEnabled(!selected);
            _registryUsernameLabel.setEnabled(!selected);
            _registryPassword.setEnabled(!selected);
            _registryPasswordLabel.setEnabled(!selected);
            
            if(selected && _registrySSLEnabled.isSelected() == false)
            {
                selectRegistrySSLEnabled(selected);
            }
        }
        
        private void selectRouterUseSSL(boolean selected)
        {
            _routerUseSSL.setSelected(selected);
            _routerUsername.setEnabled(!selected);
            _routerUsernameLabel.setEnabled(!selected);
            _routerPassword.setEnabled(!selected);
            _routerPasswordLabel.setEnabled(!selected);
            
            if(selected && _routerSSLEnabled.isSelected() == false)
            {
                selectRouterSSLEnabled(selected);
            }
        }
        
        private void selectRegistrySSLEnabled(boolean selected)
        {
            _registrySSLEnabled.setSelected(selected);
            recursiveSetEnabled(_sslPane, selected);
            if(!selected && _registryUseSSL.isSelected())
            {
                selectRegistryUseSSL(selected);
            }
        }

        private void selectRouterSSLEnabled(boolean selected)
        {
            _routerSSLEnabled.setSelected(selected);
            recursiveSetEnabled(_sslPane, selected);
            if(!selected && _routerUseSSL.isSelected())
            {
                selectRouterUseSSL(selected);
            }
        }

        private void recursiveSetEnabled(java.awt.Container c, boolean enabled)
        {
            for(int i = 0; i < c.getComponentCount(); ++i)
            {
                java.awt.Component comp = c.getComponent(i);
                if(comp instanceof java.awt.Container)
                {
                    recursiveSetEnabled((java.awt.Container)comp, enabled);
                }
                comp.setEnabled(enabled);
            }
            c.setEnabled(enabled);
        }

        private void directTabSelected()
        {
            recursiveSetEnabled(_sslPane, _registrySSLEnabled.isSelected());
        }

        private void routedTabSelected()
        {
            recursiveSetEnabled(_sslPane, _routerSSLEnabled.isSelected());
        }

        private JTabbedPane _mainPane = new JTabbedPane();
        private JTextField _registryUsername = new JTextField(30);
        private JLabel _registryUsernameLabel;
        private JPasswordField _registryPassword = new JPasswordField(30);
        private JLabel _registryPasswordLabel;
        private JCheckBox _registryUseSSL;
        private JCheckBox _registrySSLEnabled;
        private JTextField _registryInstanceName = new JTextField(30);
        private JTextField _registryEndpoints = new JTextField(30);
        private JCheckBox _connectToMaster = new JCheckBox("Connect to Master Registry");

        private JTextField _routerUsername = new JTextField(30);
        private JLabel _routerUsernameLabel;
        private JPasswordField _routerPassword = new JPasswordField(30);
        private JLabel _routerPasswordLabel;
        private JCheckBox _routerUseSSL;
        private JCheckBox _routerSSLEnabled;
        private JTextField _routerInstanceName = new JTextField(30);
        private JTextField _routerEndpoints = new JTextField(30);

        private JTabbedPane _sslPane = new JTabbedPane();
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
       
            Ice.LongHolder keepAlivePeriodHolder = new Ice.LongHolder();
         
            AdminSessionPrx session = _coordinator.login(
                _loginInfo, parent, keepAlivePeriodHolder);
            if(session == null)
            {
                return false;
            }
            try
            {
                _replicaName = session.getReplicaName();
            }
            catch(Ice.LocalException e)
            {
                logout(true);
                JOptionPane.showMessageDialog(
                    parent,
                    "Could not retrieve replica name: " + e.toString(),
                    "Login failed",
                    JOptionPane.ERROR_MESSAGE);
                return false;
            }

            _coordinator.getStatusBar().setConnected(true);
            
            _connectedToMaster = _replicaName.equals("Master");
            if(_connectedToMaster)
            {
                _coordinator.getStatusBar().setText("Logged into Master Registry");
            }
            else
            {
                _coordinator.getStatusBar().setText("Logged into Slave Registry '" + _replicaName + "'");
            }
            
            try
            {
                _session = new Session(session, keepAlivePeriodHolder.value, parent);
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
        if(_session != null)
        {
            _session.logout(destroySession);
            _session = null;
        }
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

    boolean connectedToMaster()
    {
        return _session != null && _connectedToMaster;
    }

    String getReplicaName()
    {
        return _replicaName;
    }
   
    private LoginDialog _loginDialog;
    private LoginInfo _loginInfo;

    private final Coordinator _coordinator;
    private Preferences _loginPrefs;
  
    private Session _session;
    private boolean _connectedToMaster = false;
    private String _replicaName = "";
}
