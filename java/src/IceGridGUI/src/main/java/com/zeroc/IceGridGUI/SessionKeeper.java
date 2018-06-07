// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceGridGUI;

import javax.swing.*;
import javax.swing.border.EmptyBorder;
import javax.swing.event.DocumentListener;
import javax.swing.event.DocumentEvent;
import javax.swing.event.ListSelectionEvent;
import javax.swing.event.ListSelectionListener;
import javax.swing.table.AbstractTableModel;
import javax.swing.filechooser.FileFilter;

import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.File;
import java.awt.Container;
import java.awt.Component;
import java.awt.Cursor;
import java.awt.Dimension;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.CardLayout;
import java.awt.BorderLayout;
import java.awt.Insets;
import java.awt.event.MouseEvent;
import java.awt.event.MouseAdapter;
import java.awt.event.ComponentAdapter;
import java.awt.event.ComponentEvent;
import com.jgoodies.forms.builder.DefaultFormBuilder;
import com.jgoodies.forms.builder.ButtonBarBuilder;
import com.jgoodies.forms.factories.Borders;
import com.jgoodies.forms.layout.FormLayout;
import com.jgoodies.forms.layout.CellConstraints;
import com.jgoodies.forms.util.LayoutStyle;

import java.util.prefs.Preferences;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Enumeration;
import java.util.Formatter;
import java.util.Collection;
import java.util.Iterator;
import java.security.Key;
import java.security.KeyStore;
import java.security.cert.Certificate;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;
import java.security.MessageDigest;

import javax.naming.ldap.LdapName;
import javax.naming.ldap.Rdn;

import java.net.NetworkInterface;
import java.net.InetAddress;

import com.zeroc.IceLocatorDiscovery.Plugin;
import com.zeroc.IceLocatorDiscovery.PluginFactory;

import com.zeroc.IceGrid.*;

//
// The SessionKeeper is responsible for establishing sessions (one at a time)
// and keeping them alive.
//

@SuppressWarnings("unchecked")
public class SessionKeeper
{
    //
    // An AdminSessionPrx and various objects associated with that session
    //
    private class Session
    {
        Session(AdminSessionPrx session, long sessionTimeout, int acmTimeout, boolean routed, final Component parent)
            throws java.lang.Throwable
        {
            _session = session;
            _routed = routed;
            try
            {
                _admin = _session.getAdmin();
            }
            catch(final com.zeroc.Ice.LocalException e)
            {
                while(true)
                {
                    try
                    {
                        SwingUtilities.invokeAndWait(() ->
                            {
                                logout(true);
                                JOptionPane.showMessageDialog(
                                    parent,
                                    "Could not retrieve Admin proxy: " + e.toString(),
                                    "Login failed",
                                    JOptionPane.ERROR_MESSAGE);
                            });
                        break;
                    }
                    catch(java.lang.InterruptedException ex)
                    {
                        // Ignore and retry
                    }
                    catch(java.lang.reflect.InvocationTargetException ex)
                    {
                        break;
                    }
                }
                throw e;
            }

            try
            {
                if(!routed)
                {
                    com.zeroc.Ice.ObjectPrx adminCallbackTemplate = _session.getAdminCallbackTemplate();

                    if(adminCallbackTemplate != null)
                    {
                        _adminCallbackCategory = adminCallbackTemplate.ice_getIdentity().category;

                        String publishedEndpoints = null;
                        for(com.zeroc.Ice.Endpoint endpoint : adminCallbackTemplate.ice_getEndpoints())
                        {
                            String endpointString = endpoint.toString();
                            if(publishedEndpoints == null)
                            {
                                publishedEndpoints = endpointString;
                            }
                            else
                            {
                                publishedEndpoints += ":" + endpointString;
                            }
                        }
                        _coordinator.getCommunicator().getProperties().setProperty(
                            "CallbackAdapter.PublishedEndpoints", publishedEndpoints);
                    }
                }
                _serverAdminCategory = _admin.getServerAdminCategory();
            }
            catch(final com.zeroc.Ice.OperationNotExistException e)
            {
                while(true)
                {
                    try
                    {
                        SwingUtilities.invokeAndWait(() ->
                            {
                                logout(true);
                                JOptionPane.showMessageDialog(
                                    parent,
                                    "This version of IceGrid GUI requires a newer IceGrid Registry",
                                    "Login failed: Version Mismatch",
                                    JOptionPane.ERROR_MESSAGE);
                            });
                        break;
                    }
                    catch(java.lang.InterruptedException ex)
                    {
                        // Ignore and retry
                    }
                    catch(java.lang.reflect.InvocationTargetException ex)
                    {
                        break;
                    }
                }
                throw e;
            }
            catch(final com.zeroc.Ice.LocalException e)
            {
                while(true)
                {
                    try
                    {
                        SwingUtilities.invokeAndWait(() ->
                            {
                                logout(true);
                                JOptionPane.showMessageDialog(
                                    parent,
                                    "Could not retrieve admin callback template or server admin category: " +
                                    e.toString(),
                                    "Login failed",
                                    JOptionPane.ERROR_MESSAGE);
                            });
                        break;
                    }
                    catch(java.lang.InterruptedException ex)
                    {
                        // Ignore and retry
                    }
                    catch(java.lang.reflect.InvocationTargetException ex)
                    {
                        break;
                    }
                }
                throw e;
            }

            if(acmTimeout > 0)
            {
                _session.ice_getConnection().setACM(
                    java.util.OptionalInt.of(acmTimeout),
                    null,
                    java.util.Optional.of(com.zeroc.Ice.ACMHeartbeat.HeartbeatAlways));

                _session.ice_getConnection().setCloseCallback(con ->
                    {
                        try
                        {
                            con.getInfo(); // This throws when the connection is closed.
                            assert(false);
                        }
                        catch(final com.zeroc.Ice.LocalException ex)
                        {
                            SwingUtilities.invokeLater(() ->
                                {
                                    sessionLost("Failed to contact the IceGrid registry: " + ex.toString());
                                });
                        }
                    });
            }
            else
            {
                _keepAliveFuture = _coordinator.getScheduledExecutor().scheduleAtFixedRate(() ->
                    {
                        _session.keepAliveAsync().whenComplete((result, ex) ->
                            {
                                if(ex != null)
                                {
                                    SwingUtilities.invokeLater(() ->
                                        {
                                            sessionLost("Failed to contact the IceGrid registry: " + ex.toString());
                                        });
                                }
                            });
                    }, sessionTimeout / 2, sessionTimeout / 2, java.util.concurrent.TimeUnit.SECONDS);
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

        String getServerAdminCategory()
        {
            return _serverAdminCategory;
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
                com.zeroc.Ice.ObjectAdapter adminRouterAdapter = _coordinator.getCommunicator().
                    createObjectAdapterWithEndpoints("IceGrid.AdminRouter", "tcp -h localhost");

                _routedAdmin = AdminPrx.uncheckedCast(adminRouterAdapter.addWithUUID(new AdminRouter(_admin)));

                adminRouterAdapter.activate();
            }
            return _routedAdmin;
        }

        com.zeroc.Ice.ObjectPrx addCallback(com.zeroc.Ice.Object servant, String name, String facet)
        {
            if(_adminCallbackCategory == null)
            {
                return null;
            }
            else
            {
                return _adapter.addFacet(servant, new com.zeroc.Ice.Identity(name, _adminCallbackCategory), facet);
            }
        }

        com.zeroc.Ice.ObjectPrx retrieveCallback(String name, String facet)
        {
            if(_adminCallbackCategory == null)
            {
                return null;
            }
            else
            {
                com.zeroc.Ice.Identity ident = new com.zeroc.Ice.Identity(name, _adminCallbackCategory);
                if(_adapter.findFacet(ident, facet) == null)
                {
                    return null;
                }
                else
                {
                    return _adapter.createProxy(ident).ice_facet(facet);
                }
            }
        }

        com.zeroc.Ice.Object removeCallback(String name, String facet)
        {
            if(_adminCallbackCategory == null || _adapter == null)
            {
                return null;
            }
            else
            {
                return _adapter.removeFacet(new com.zeroc.Ice.Identity(name, _adminCallbackCategory), facet);
            }
        }

        void close(boolean destroySession)
        {
            if(_keepAliveFuture != null)
            {
                _keepAliveFuture.cancel(false);
                _keepAliveFuture = null;
            }

            if(_adapter != null)
            {
                _adapter.destroy();
                _adapter = null;
            }

            _session.ice_getConnection().setCloseCallback(null);

            if(destroySession)
            {
                _coordinator.destroySession(_session, _routed);
            }
            _coordinator.setConnected(false);
        }

        public void registerObservers() throws java.lang.Throwable
        {
            //
            // Create the object adapter for the observers
            //
            String category;

            if(!_routed)
            {
                category = "observer";

                String adapterName = _adminCallbackCategory == null ? "" : "CallbackAdapter";

                _adapter = _coordinator.getCommunicator().createObjectAdapter(adapterName);
                _adapter.activate();
                _session.ice_getConnection().setAdapter(_adapter);
            }
            else
            {
                com.zeroc.Glacier2.RouterPrx router = com.zeroc.Glacier2.RouterPrx.uncheckedCast(
                    _coordinator.getCommunicator().getDefaultRouter());
                category = router.getCategoryForClient();
                _adminCallbackCategory = category;

                _adapter = _coordinator.getCommunicator().createObjectAdapterWithRouter("RoutedAdapter", router);
                _adapter.activate();
            }

            //
            // Create servants and proxies
            //
            _applicationObserverIdentity.name = "application-" + java.util.UUID.randomUUID().toString();
            _applicationObserverIdentity.category = category;
            _adapterObserverIdentity.name = "adapter-" + java.util.UUID.randomUUID().toString();
            _adapterObserverIdentity.category = category;
            _objectObserverIdentity.name = "object-" + java.util.UUID.randomUUID().toString();
            _objectObserverIdentity.category = category;
            _registryObserverIdentity.name = "registry-" + java.util.UUID.randomUUID().toString();
            _registryObserverIdentity.category = category;
            _nodeObserverIdentity.name = "node-" + java.util.UUID.randomUUID().toString();
            _nodeObserverIdentity.category = category;

            while(true)
            {
                try
                {
                    SwingUtilities.invokeAndWait(() ->
                        {
                            ApplicationObserverI applicationObserverServant = new ApplicationObserverI(
                                _admin.ice_getIdentity().category, _coordinator);

                            ApplicationObserverPrx applicationObserver =
                                ApplicationObserverPrx.uncheckedCast(
                                    _adapter.add(applicationObserverServant, _applicationObserverIdentity));

                            AdapterObserverPrx adapterObserver =
                                AdapterObserverPrx.uncheckedCast(
                                    _adapter.add(new AdapterObserverI(_coordinator), _adapterObserverIdentity));

                            ObjectObserverPrx objectObserver =
                                ObjectObserverPrx.uncheckedCast(
                                    _adapter.add(new ObjectObserverI(_coordinator), _objectObserverIdentity));

                            RegistryObserverPrx registryObserver =
                                RegistryObserverPrx.uncheckedCast(
                                    _adapter.add(new RegistryObserverI(_coordinator), _registryObserverIdentity));

                            NodeObserverPrx nodeObserver =
                                NodeObserverPrx.uncheckedCast(
                                    _adapter.add(new NodeObserverI(_coordinator), _nodeObserverIdentity));

                            try
                            {
                                if(_routed)
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
                        });
                    break;
                }
                catch(java.lang.InterruptedException ex)
                {
                    // Ignore and retry
                }
                catch(java.lang.reflect.InvocationTargetException ex)
                {
                    throw ex.getCause();
                }
            }
        }

        private final AdminSessionPrx _session;
        private final boolean _routed;

        private java.util.concurrent.Future<?> _keepAliveFuture;

        private com.zeroc.Ice.ObjectAdapter _adapter;
        private AdminPrx _admin;
        private String _serverAdminCategory;
        private String _adminCallbackCategory;
        private AdminPrx _routedAdmin;
        private com.zeroc.Ice.Identity _applicationObserverIdentity = new com.zeroc.Ice.Identity();
        private com.zeroc.Ice.Identity _adapterObserverIdentity = new com.zeroc.Ice.Identity();
        private com.zeroc.Ice.Identity _objectObserverIdentity = new com.zeroc.Ice.Identity();
        private com.zeroc.Ice.Identity _registryObserverIdentity = new com.zeroc.Ice.Identity();
        private com.zeroc.Ice.Identity _nodeObserverIdentity = new com.zeroc.Ice.Identity();
    }

    private static JScrollPane createStrippedScrollPane(Component component)
    {
        JScrollPane scrollPane = new JScrollPane(component);
        scrollPane.setHorizontalScrollBarPolicy(ScrollPaneConstants.HORIZONTAL_SCROLLBAR_NEVER);
        return scrollPane;
    }

    public class ConnectionInfo implements Comparable<ConnectionInfo>
    {
        public ConnectionInfo()
        {
        }

        public ConnectionInfo(String uuid, Preferences prefs)
        {
            _uuid = uuid;
            _prefs = prefs;
            load();
        }

        @Override
        public synchronized String toString()
        {

            String name = getInstanceName();
            name += ":";
            if(getDefaultEndpoint())
            {
                if(getSSL())
                {
                    name += "ssl";
                }
                else
                {
                    name += "tcp";
                }
                String host = getHost();
                if(host.indexOf('"') == -1)
                {
                    host = "\"" + host + "\"";
                }

                name += " -h " + host + " -p " + Integer.toString(getPort());
            }
            else
            {
                name += getEndpoint();
            }
            return name;
        }

        @Override
        public int compareTo(ConnectionInfo other)
        {
            return _instanceName.compareTo(other._instanceName);
        }

        public synchronized void save() throws java.util.prefs.BackingStoreException
        {
            if(_prefs == null)
            {
                Preferences prefs = Coordinator.getPreferences().node("Configurations");
                if(_uuid == null)
                {
                    _uuid = java.util.UUID.randomUUID().toString();
                }
                _prefs = prefs.node(_uuid);
            }

            //
            // Set the first stored connection as default.
            //
            if(Coordinator.getPreferences().node("Configurations").childrenNames().length == 1)
            {
                setIsDefault(true);
            }

            _prefs.clear();

            _prefs.put("instanceName", _instanceName);
            if(_defaultEndpoint)
            {
                _prefs.putBoolean("defaultEndpoint", true);
                _prefs.putBoolean("defaultPort", _defaultPort);
                _prefs.put("host", _host);
                if(!_defaultPort)
                {
                    _prefs.putInt("port", _port);
                }
                _prefs.putBoolean("SSL", _ssl);
            }
            else
            {
                _prefs.putBoolean("customEndpoint", true);
                _prefs.put("endpoint", _endpoint);
            }

            _prefs.put("auth", _auth.toString());
            if(_auth == AuthType.UsernamePasswordAuthType)
            {
                _prefs.put("username", _username);
                _prefs.putBoolean("storePassword", _storePassword);
                if(_storePassword)
                {
                    _prefs.put("password", getPassword() != null ? new String(getPassword()) : null);
                }
                if(_useX509Certificate)
                {
                    _prefs.putBoolean("useX509Certificate", true);
                }
            }

            if(_auth == AuthType.X509CertificateAuthType || _useX509Certificate)
            {
                _prefs.put("alias", _alias);
                _prefs.putBoolean("storeKeyPassword", _storeKeyPassword);
                if(_storeKeyPassword)
                {
                    _prefs.put("keyPassword", getKeyPassword() != null ? new String(getKeyPassword()) : null);
                }
                _prefs.putBoolean("useX509Certificate", true);
            }

            _prefs.putBoolean("direct", _direct);
            if(_direct)
            {
                _prefs.putBoolean("connectToMaster", _connectToMaster);
            }

            _prefs.putBoolean("isDefault", _isDefault);

            _prefs.flush();
        }

        public synchronized void load()
        {
            if(_prefs == null)
            {
                return;
            }
            setDirect(_prefs.getBoolean("direct", false));
            if(_direct)
            {
                setConnectToMaster(_prefs.getBoolean("connectToMaster", false));
            }
            setInstanceName(_prefs.get("instanceName", ""));
            if(_prefs.getBoolean("defaultEndpoint", false))
            {
                setDefaultPort(_prefs.getBoolean("defaultPort", true));
                setHost(_prefs.get("host", ""));
                setSSL(_prefs.getBoolean("SSL", true));
                int port = 0;
                if(getDefaultPort())
                {
                    if(getDirect())
                    {
                        if(getSSL())
                        {
                            port = ICEGRID_SSL_PORT;
                        }
                        else
                        {
                            port = ICEGRID_TCP_PORT;
                        }
                    }
                    else
                    {
                        if(getSSL())
                        {
                            port = GLACIER2_SSL_PORT;
                        }
                        else
                        {
                            port = GLACIER2_TCP_PORT;
                        }
                    }
                }
                else
                {
                    if(getDirect())
                    {
                        if(getSSL())
                        {
                            port = _prefs.getInt("port", ICEGRID_SSL_PORT);
                        }
                        else
                        {
                            port = _prefs.getInt("port", ICEGRID_SSL_PORT);
                        }
                    }
                    else
                    {
                        if(getSSL())
                        {
                            port = _prefs.getInt("port", GLACIER2_SSL_PORT);
                        }
                        else
                        {
                            port = _prefs.getInt("port", GLACIER2_TCP_PORT);
                        }
                    }
                }
                setPort(port);
            }
            else
            {
                setEndpoint(_prefs.get("endpoint", ""));
            }

            if(_prefs.get("auth", AuthType.X509CertificateAuthType.toString()).equals(
                                                AuthType.UsernamePasswordAuthType.toString()))
            {
                setAuth(AuthType.UsernamePasswordAuthType);
                setUsername(_prefs.get("username", ""));
                setStorePassword(_prefs.getBoolean("storePassword", false));
                if(_storePassword)
                {
                    String tmp = _prefs.get("password", "");
                    char[] password = new char[tmp.length()];
                    tmp.getChars(0, tmp.length(), password, 0);
                    setPassword(password);
                    tmp = null;
                }
                setUseX509Certificate(_prefs.getBoolean("useX509Certificate", false));
            }
            else
            {
                setAuth(AuthType.X509CertificateAuthType);
            }

            if(_useX509Certificate || _auth == AuthType.X509CertificateAuthType)
            {
                setAlias(_prefs.get("alias", ""));
                setStoreKeyPassword(_prefs.getBoolean("storeKeyPassword", false));
                setUseX509Certificate(_prefs.getBoolean("useX509Certificate", false));
                if(_storeKeyPassword)
                {
                    String tmp = _prefs.get("keyPassword", "");
                    char[] keyPassword = new char[tmp.length()];
                    tmp.getChars(0, tmp.length(), keyPassword, 0);
                    setKeyPassword(keyPassword);
                    tmp = null;
                }
            }
            setIsDefault(_prefs.getBoolean("isDefault", false));
        }

        public synchronized String getUUID()
        {
            return _uuid;
        }

        public synchronized void setUUID(String uuid)
        {
            this._uuid = uuid;
        }

        public synchronized String getInstanceName()
        {
            return _instanceName;
        }

        public synchronized void setInstanceName(String instanceName)
        {
            _instanceName = instanceName == null ? null : instanceName.trim();
        }

        public synchronized boolean getDefaultEndpoint()
        {
            return _defaultEndpoint;
        }

        public synchronized void setDefaultEndpoint(boolean value)
        {
            _defaultEndpoint = value;
            if(!_defaultEndpoint)
            {
                _host = null;
            }
        }

        public synchronized boolean getCustomEndpoint()
        {
            return _customEndpoint;
        }

        public synchronized void setCustomEndpoint(boolean value)
        {
            _customEndpoint = value;
            if(!_customEndpoint)
            {
                _endpoint = null;
            }
        }

        public synchronized void setHost(String host)
        {
            _host = host == null ? null : host.trim();
            setDefaultEndpoint(true);
            setCustomEndpoint(false);
        }

        public String getHost()
        {
            return _host;
        }

        public synchronized void setDefaultPort(boolean value)
        {
            _defaultPort = value;
        }

        public synchronized boolean getDefaultPort()
        {
            return _defaultPort;
        }

        public synchronized void setPort(int port)
        {
            _port = port;
            setDefaultEndpoint(true);
            setCustomEndpoint(false);
        }

        public synchronized int getPort()
        {
            if(_defaultPort)
            {
                if(getDirect())
                {
                    if(getSSL())
                    {
                        return ICEGRID_SSL_PORT;
                    }
                    else
                    {
                        return ICEGRID_TCP_PORT;
                    }
                }
                else
                {
                    if(getSSL())
                    {
                        return GLACIER2_SSL_PORT;
                    }
                    else
                    {
                        return GLACIER2_TCP_PORT;
                    }
                }
            }
            return _port;
        }

        public synchronized void setSSL(boolean ssl)
        {
            _ssl = ssl;
            setDefaultEndpoint(true);
            setCustomEndpoint(false);
        }

        public synchronized boolean getSSL()
        {
            return _ssl;
        }

        public synchronized void setEndpoint(String endpoint)
        {
            _endpoint = endpoint == null ? null : endpoint.trim();
            setDefaultEndpoint(false);
            setCustomEndpoint(true);
        }

        public String getEndpoint()
        {
            return _endpoint;
        }

        public synchronized void setAuth(AuthType value)
        {
            _auth = value;
        }

        public synchronized AuthType getAuth()
        {
            return _auth;
        }

        public synchronized boolean getDirect()
        {
            return _direct;
        }

        public synchronized void setDirect(boolean direct)
        {
            _direct = direct;
        }

        public synchronized void setConnectToMaster(boolean connectToMaster)
        {
            _connectToMaster = connectToMaster;
        }

        public synchronized boolean getConnectToMaster()
        {
            return _connectToMaster;
        }

        public synchronized void setUseX509Certificate(boolean useX509Certificate)
        {
            _useX509Certificate = useX509Certificate;
        }

        public boolean getUseX509Certificate()
        {
            return _useX509Certificate;
        }

        public synchronized String getUsername()
        {
            return _username;
        }

        public synchronized void setUsername(String username)
        {
            _username = username == null ? null : username.trim();
        }

        public synchronized char[] getPassword()
        {
            return _password;
        }

        public synchronized void setPassword(char[] password)
        {
            _password = password;
        }

        private synchronized boolean getStorePassword()
        {
            return _storePassword;
        }

        private synchronized void setStorePassword(boolean value)
        {
            _storePassword = value;
        }

        public synchronized String getAlias()
        {
            return _alias;
        }

        public synchronized void setAlias(String alias)
        {
            _alias = alias;
        }

        private synchronized boolean getStoreKeyPassword()
        {
            return _storeKeyPassword;
        }

        private synchronized void setStoreKeyPassword(boolean value)
        {
            _storeKeyPassword = value;
        }

        public synchronized void setKeyPassword(char[] password)
        {
            _keyPassword = password;
        }

        public synchronized char[] getKeyPassword()
        {
            return _keyPassword;
        }

        public synchronized boolean isDefault()
        {
            return _isDefault;
        }

        public synchronized void setIsDefault(boolean isDefault)
        {
            _isDefault = isDefault;
        }

        private Preferences _prefs; // The preferences node associated to this configuration, when stored.
        private String _uuid; // The unique id used as the node name in the preferences object
        private String _instanceName = "";
        private boolean _defaultEndpoint;
        private boolean _customEndpoint;
        private String _host = "";
        private boolean _defaultPort;
        private int _port;
        private boolean _ssl;
        private String _endpoint = "";
        private AuthType _auth = AuthType.UsernamePasswordAuthType;
        private boolean _direct;
        private boolean _connectToMaster;
        private String _username = "";
        private char[] _password;
        private boolean _storePassword;
        private boolean _useX509Certificate;
        private String _alias = "";
        private char[] _keyPassword;
        private boolean _storeKeyPassword;
        private boolean _isDefault;
    }

    //
    // FocusListener implementation that unselect the text
    // of a text component after focus gained.
    //
    public class FocusListener implements java.awt.event.FocusListener
    {
        public FocusListener(javax.swing.text.JTextComponent field)
        {
            _field = field;
        }

        @Override
        public void focusGained(java.awt.event.FocusEvent fe)
        {
            _field.setCaretPosition(_field.getDocument().getLength());
        }

        @Override
        public void focusLost(java.awt.event.FocusEvent fe)
        {

        }
        private javax.swing.text.JTextComponent _field;
    }

    private class ConnectionWizardDialog extends JDialog
    {
        ConnectionWizardDialog(JDialog parent)
        {
            super(parent, true);
            _x509CertificateDefault = true;
            initialize("New Connection - IceGrid GUI", parent);
            _connectNow = true;
        }

        ConnectionWizardDialog(ConnectionInfo inf, JDialog parent)
        {
            super(parent, true);
            _conf = inf;
            _x509CertificateDefault = false;
            initialize("Edit Connection - IceGrid GUI", parent);
            _connectNow = false;
            _nextButton.requestFocusInWindow();
        }

        public void refreshDiscoveryLocators()
        {
            final com.zeroc.Ice.Communicator communicator = _coordinator.getCommunicator();
            _discoveryStatus.setText("Searching for registries...");
            try
            {
                _coordinator.getExecutor().submit(() ->
                    {
                        synchronized(SessionKeeper.this)
                        {
                            try
                            {
                                if(_discoveryPlugin == null)
                                {
                                    PluginFactory f = new PluginFactory();
                                    _discoveryPlugin = (Plugin)f.create(communicator, "IceGridAdmin.Discovery", null);
                                    _discoveryPlugin.initialize();
                                }

                                final List<com.zeroc.Ice.LocatorPrx> locators = _discoveryPlugin.getLocators("", 1000);
                                SwingUtilities.invokeLater(() ->
                                {
                                    _directDiscoveryLocatorModel.clear();
                                    for(com.zeroc.Ice.LocatorPrx locator : locators)
                                    {
                                        _directDiscoveryLocatorModel.addElement(locator);
                                    }
                                    if(_directDiscoveryLocatorModel.size() > 0 &&
                                       _directDiscoveryLocatorList.getSelectedIndex() == -1)
                                    {
                                        _directDiscoveryLocatorList.setSelectedIndex(0);
                                    }

                                    if(_directDiscoveryLocatorModel.size() > 0)
                                    {
                                        _discoveryStatus.setText("");
                                    }
                                    else
                                    {
                                        _discoveryStatus.setText("No registries found");
                                    }
                                });
                            }
                            catch(final com.zeroc.Ice.LocalException ex)
                            {
                                SwingUtilities.invokeLater(() ->
                                    {
                                        _discoveryStatus.setText("No registries found");
                                        JOptionPane.showMessageDialog(ConnectionWizardDialog.this,
                                                                      ex.toString(),
                                                                      "Error while looking up registries",
                                                                      JOptionPane.ERROR_MESSAGE);
                                    });
                            }
                        }
                    });
            }
            catch(com.zeroc.Ice.LocalException ex)
            {
                JOptionPane.showMessageDialog(ConnectionWizardDialog.this, ex.toString(),
                                              "Error while looking up registries", JOptionPane.ERROR_MESSAGE);
            }
        }

        private void initialize(String title, final JDialog parent)
        {
            setTitle(title);
            getContentPane().setLayout(new BorderLayout());
            _cardPanel = new JPanel();
            _cardPanel.setBorder(new EmptyBorder(new Insets(5, 10, 5, 10)));

            _cardLayout = new CardLayout();
            _cardPanel.setLayout(_cardLayout);

            _wizardSteps.push(WizardStep.ConnectionTypeStep);

            _directInstanceName = new JLabel();
            _routedInstanceName = new JLabel();

            // Connection type panel
            {
                FormLayout layout = new FormLayout("pref", "pref");
                DefaultFormBuilder builder = new DefaultFormBuilder(layout);
                builder.border(Borders.DIALOG);
                builder.rowGroupingEnabled(true);

                ButtonGroup group = new ButtonGroup();

                _directConnection = new JRadioButton(new AbstractAction("Direct Connection")
                    {
                        @Override
                        public void actionPerformed(ActionEvent e)
                        {
                            validatePanel();
                        }
                    });
                _directConnection.setSelected(true);
                group.add(_directConnection);
                _routedConnection = new JRadioButton(new AbstractAction("Routed Connection")
                    {
                        @Override
                        public void actionPerformed(ActionEvent e)
                        {
                            validatePanel();
                        }
                    });
                group.add(_routedConnection);

                builder.append(new JLabel("<html><b>Connection Type</b></html>"));
                builder.append(_directConnection);
                builder.append(new JLabel("Connect directly to an IceGrid registry."));
                builder.append(_routedConnection);
                builder.append(new JLabel("Connect to an IceGrid registry through a Glacier2 router."));
                _cardPanel.add(builder.getPanel(), WizardStep.ConnectionTypeStep.toString());
            }

            // Direct Discovery Locator List
            {
                _directDiscoveryLocatorModel = new DefaultListModel<>();
                _directDiscoveryLocatorList = new JList(_directDiscoveryLocatorModel)
                    {
                        @Override
                        public String getToolTipText(MouseEvent evt)
                        {
                            int index = locationToIndex(evt.getPoint());
                            if(index < 0)
                            {
                                return null;
                            }
                            Object obj = getModel().getElementAt(index);
                            if(obj != null && obj instanceof com.zeroc.Ice.LocatorPrx)
                            {
                                return obj.toString();
                            }
                            return null;
                        }
                    };
                _directDiscoveryLocatorList.setVisibleRowCount(7);
                _directDiscoveryLocatorList.setFixedCellWidth(500);
                _directDiscoveryLocatorList.addMouseListener(
                    new MouseAdapter()
                        {
                            @Override
                            public void mouseClicked(MouseEvent e)
                            {
                                if(e.getClickCount() == 2 && e.getButton() == MouseEvent.BUTTON1)
                                {
                                    int index = _directDiscoveryLocatorList.locationToIndex(e.getPoint());
                                    if(index != -1)
                                    {
                                        Object obj = _directDiscoveryLocatorModel.getElementAt(index);
                                        if(obj != null && obj instanceof com.zeroc.Ice.LocatorPrx)
                                        {
                                            _nextButton.doClick(0);
                                        }
                                    }
                                }
                            }
                        });

                _directDiscoveryLocatorList.addListSelectionListener(new ListSelectionListener()
                {
                    @Override
                    public void valueChanged(ListSelectionEvent event)
                    {
                        validatePanel();
                    }
                });

                ButtonGroup group = new ButtonGroup();
                _directDiscoveryDiscoveredLocators = new JRadioButton(new AbstractAction("Discovered Registries")
                {
                    @Override
                    public void actionPerformed(ActionEvent e)
                    {
                        _directDiscoveryLocatorList.setEnabled(true);
                        _discoveryStatus.setEnabled(true);
                        _discoveryRefresh.setEnabled(true);
                        validatePanel();
                        refreshDiscoveryLocators();
                    }
                });
                _directDiscoveryDiscoveredLocators.setSelected(true);
                group.add(_directDiscoveryDiscoveredLocators);

                JPanel discoveryStatus;
                {
                    FormLayout layout = new FormLayout("pref:grow, 2dlu, pref", "");
                    DefaultFormBuilder builder = new DefaultFormBuilder(layout);

                    _discoveryStatus = new JLabel();
                    _discoveryRefresh = new JButton(new AbstractAction("Refresh")
                    {
                        @Override
                        public void actionPerformed(ActionEvent e)
                        {
                            refreshDiscoveryLocators();
                        }
                    });

                    builder.rowGroupingEnabled(true);
                    builder.append(_discoveryStatus, _discoveryRefresh);
                    discoveryStatus = builder.getPanel();
                }

                _directDiscoveryManualEndpoint = new JRadioButton(new AbstractAction("Manual Endpoint")
                {
                    @Override
                    public void actionPerformed(ActionEvent e)
                    {
                        _directDiscoveryLocatorList.setEnabled(false);
                        _discoveryStatus.setEnabled(false);
                        _discoveryRefresh.setEnabled(false);
                        validatePanel();
                    }
                });
                group.add(_directDiscoveryManualEndpoint);

                {
                    FormLayout layout = new FormLayout("pref:grow", "pref");
                    DefaultFormBuilder builder = new DefaultFormBuilder(layout);
                    builder.border(Borders.DIALOG);
                    builder.rowGroupingEnabled(false);
                    builder.append(_directDiscoveryDiscoveredLocators);
                    builder.append(createStrippedScrollPane(_directDiscoveryLocatorList));
                    builder.append(discoveryStatus);
                    builder.append(_directDiscoveryManualEndpoint);
                    _cardPanel.add(builder.getPanel(), WizardStep.DirectDiscoveryChooseStep.toString());
                }
            }

            // Direct Instance Panel
            {
                FormLayout layout = new FormLayout("pref:grow", "pref");
                DefaultFormBuilder builder = new DefaultFormBuilder(layout);
                builder.border(Borders.DIALOG);
                builder.rowGroupingEnabled(true);

                _directConnectToMaster = new JCheckBox("Connect to a Master Registry.");
                _directConnectToMaster.setSelected(true); // on by default
                builder.append(_directConnectToMaster);
                builder.append(new JLabel("You need to connect to a Master Registry to change definitions."));
                _cardPanel.add(builder.getPanel(), WizardStep.DirectMasterStep.toString());
            }

            // Direct Endpoint panel
            {
                FormLayout layout = new FormLayout("pref:grow", "pref");
                DefaultFormBuilder builder = new DefaultFormBuilder(layout);
                builder.border(Borders.DIALOG);
                builder.rowGroupingEnabled(true);

                ButtonGroup group = new ButtonGroup();
                _directDefaultEndpoints = new JRadioButton(
                    new AbstractAction("A hostname and a port number?")
                        {
                            @Override
                            public void actionPerformed(ActionEvent e)
                            {
                                validatePanel();
                            }
                        });
                _directDefaultEndpoints.setSelected(true);
                group.add(_directDefaultEndpoints);
                _directCustomEndpoints = new JRadioButton(new AbstractAction("An endpoint string?")
                    {
                        @Override
                        public void actionPerformed(ActionEvent e)
                        {
                            validatePanel();
                        }
                    });
                group.add(_directCustomEndpoints);

                builder.append(new JLabel("<html><b>Addressing Information</b></html>"));
                builder.append(new JLabel("Do you want to provide addressing information for the IceGrid registry as:"));
                builder.append(_directDefaultEndpoints);
                builder.append(_directCustomEndpoints);
                _cardPanel.add(builder.getPanel(), WizardStep.DirectEndpointStep.toString());
            }

            // Routed Endpoint panel
            {
                FormLayout layout = new FormLayout("pref:grow", "pref");
                DefaultFormBuilder builder = new DefaultFormBuilder(layout);
                builder.border(Borders.DIALOG);
                builder.rowGroupingEnabled(true);

                ButtonGroup group = new ButtonGroup();

                _routedDefaultEndpoints = new JRadioButton(
                    new AbstractAction("A hostname and a port number?")
                        {
                            @Override
                            public void actionPerformed(ActionEvent e)
                            {
                                validatePanel();
                            }
                        });
                _routedDefaultEndpoints.setSelected(true);
                group.add(_routedDefaultEndpoints);
                _routedCustomEndpoints = new JRadioButton(new AbstractAction("An endpoint string?")
                    {
                        @Override
                        public void actionPerformed(ActionEvent e)
                        {
                            validatePanel();
                        }
                    });
                group.add(_routedCustomEndpoints);

                builder.append(new JLabel("<html><b>Addressing Information</b></html>"));
                builder.append(new JLabel("Do you want to provide addressing information for the Glacier2 router as:"));
                builder.append(_routedDefaultEndpoints);

                builder.append(_routedCustomEndpoints);

                _cardPanel.add(builder.getPanel(), WizardStep.RoutedEndpointStep.toString());
            }

            // Direct Default Endpoint panel
            {
                FormLayout layout = new FormLayout("pref, 2dlu, pref:grow", "pref");
                DefaultFormBuilder builder = new DefaultFormBuilder(layout);
                builder.border(Borders.DIALOG);
                builder.rowGroupingEnabled(true);

                _directDefaultEndpointHost = new JTextField(20);
                _directDefaultEndpointHost.getDocument().addDocumentListener(new DocumentListener()
                    {
                        @Override
                        public void changedUpdate(DocumentEvent e)
                        {
                            validatePanel();
                            _directDefaultEndpointHost.requestFocusInWindow();
                        }
                        @Override
                        public void removeUpdate(DocumentEvent e)
                        {
                            validatePanel();
                            _directDefaultEndpointHost.requestFocusInWindow();
                        }
                        @Override
                        public void insertUpdate(DocumentEvent e)
                        {
                            validatePanel();
                            _directDefaultEndpointHost.requestFocusInWindow();
                        }
                    });

                builder.append("<html><b>Hostname:</b></html>", _directDefaultEndpointHost);
                builder.append("", new JLabel("The hostname or IP address of the IceGrid registry."));
                builder.nextLine();
                _directDefaultEndpointPort = new JTextField(5);
                _directDefaultEndpointPort.addFocusListener(new FocusListener(_directDefaultEndpointPort));
                _directDefaultEndpointPort.getDocument().addDocumentListener(new DocumentListener()
                    {
                        @Override
                        public void changedUpdate(DocumentEvent e)
                        {
                            validatePanel();
                            _directDefaultEndpointPort.requestFocusInWindow();
                        }
                        @Override
                        public void removeUpdate(DocumentEvent e)
                        {
                            validatePanel();
                            _directDefaultEndpointPort.requestFocusInWindow();
                        }
                        @Override
                        public void insertUpdate(DocumentEvent e)
                        {
                            validatePanel();
                            _directDefaultEndpointPort.requestFocusInWindow();
                        }
                    });
                builder.append("<html><b>Port number:</b></html>", _directDefaultEndpointPort);
                builder.append("", new JLabel("<html>The port number the IceGrid registry listens on; " +
                               "leave empty to use the default <br/>IceGrid registry port number.</html>"));
                builder.nextLine();
                ButtonGroup group = new ButtonGroup();
                _directDefaultEndpointTCP = new JRadioButton(new AbstractAction("TCP")
                {
                    @Override
                    public void actionPerformed(ActionEvent e)
                    {
                    }
                });
                group.add(_directDefaultEndpointTCP);

                _directDefaultEndpointSSL = new JRadioButton(new AbstractAction("SSL")
                {
                    @Override
                    public void actionPerformed(ActionEvent e)
                    {
                    }
                });
                group.add(_directDefaultEndpointSSL);
                _directDefaultEndpointTCP.setSelected(true);
                JPanel protocolOptionPane;
                {
                    DefaultFormBuilder protocolBuilder =
                        new DefaultFormBuilder(new FormLayout("pref, 2dlu, pref", "pref"));
                    protocolBuilder.append(_directDefaultEndpointTCP,_directDefaultEndpointSSL);
                    protocolOptionPane = protocolBuilder.getPanel();
                }
                builder.append("<html><b>Protocol:</b></html>", protocolOptionPane);
                _cardPanel.add(builder.getPanel(), WizardStep.DirectDefaultEndpointStep.toString());
            }

            // Routed Default Endpoint panel
            {
                FormLayout layout = new FormLayout("pref, 2dlu, pref:grow", "pref");
                DefaultFormBuilder builder = new DefaultFormBuilder(layout);
                builder.border(Borders.DIALOG);
                builder.rowGroupingEnabled(true);

                _routedDefaultEndpointHost = new JTextField(20);
                _routedDefaultEndpointHost.addFocusListener(new FocusListener(_routedDefaultEndpointHost));
                _routedDefaultEndpointHost.getDocument().addDocumentListener(new DocumentListener()
                    {
                        @Override
                        public void changedUpdate(DocumentEvent e)
                        {
                            validatePanel();
                            _routedDefaultEndpointHost.requestFocusInWindow();
                        }

                        @Override
                        public void removeUpdate(DocumentEvent e)
                        {
                            validatePanel();
                            _routedDefaultEndpointHost.requestFocusInWindow();
                        }

                        @Override
                        public void insertUpdate(DocumentEvent e)
                        {
                            validatePanel();
                            _routedDefaultEndpointHost.requestFocusInWindow();
                        }
                    });

                builder.append("<html><b>Hostname:</b></html>", _routedDefaultEndpointHost);
                builder.append("", new JLabel("The hostname or IP address of the Glacier2 router."));
                builder.nextLine();
                _routedDefaultEndpointPort = new JTextField(5);
                _routedDefaultEndpointPort.addFocusListener(new FocusListener(_routedDefaultEndpointPort));
                _routedDefaultEndpointPort.getDocument().addDocumentListener(new DocumentListener()
                    {
                        @Override
                        public void changedUpdate(DocumentEvent e)
                        {
                            validatePanel();
                            _routedDefaultEndpointPort.requestFocusInWindow();
                        }

                        @Override
                        public void removeUpdate(DocumentEvent e)
                        {
                            validatePanel();
                            _routedDefaultEndpointPort.requestFocusInWindow();
                        }

                        @Override
                        public void insertUpdate(DocumentEvent e)
                        {
                            validatePanel();
                            _routedDefaultEndpointPort.requestFocusInWindow();
                        }
                    });
                builder.append("<html><b>Port:</b></html>", _routedDefaultEndpointPort);
                builder.append("", new JLabel("<html>The port number the Glacier2 router listens on; " +
                               "leave empty to use the default <br/>Glacier2 router port number.</html>"));

                builder.nextLine();
                ButtonGroup group = new ButtonGroup();
                _routedDefaultEndpointTCP = new JRadioButton(new AbstractAction("TCP")
                {
                    @Override
                    public void actionPerformed(ActionEvent e)
                    {
                        validatePanel();
                    }
                });
                group.add(_routedDefaultEndpointTCP);

                _routedDefaultEndpointSSL = new JRadioButton(new AbstractAction("SSL")
                {
                    @Override
                    public void actionPerformed(ActionEvent e)
                    {
                    }
                });
                group.add(_routedDefaultEndpointSSL);
                _routedDefaultEndpointTCP.setSelected(true);
                JPanel protocolOptionPane;
                {
                    DefaultFormBuilder protocolBuilder =
                        new DefaultFormBuilder(new FormLayout("pref, 2dlu, pref", "pref"));
                    protocolBuilder.append(_routedDefaultEndpointTCP,_routedDefaultEndpointSSL);
                    protocolOptionPane = protocolBuilder.getPanel();
                }
                builder.append("<html><b>Protocol:</b></html>", protocolOptionPane);
                _cardPanel.add(builder.getPanel(), WizardStep.RoutedDefaultEndpointStep.toString());
            }

            // Direct Custom Endpoint panel
            {
                FormLayout layout = new FormLayout("pref:grow", "pref");
                DefaultFormBuilder builder = new DefaultFormBuilder(layout);
                builder.border(Borders.DIALOG);
                builder.rowGroupingEnabled(true);

                _directCustomEndpointValue = new JTextField(20);
                _directCustomEndpointValue.addFocusListener(new FocusListener(_directCustomEndpointValue));
                _directCustomEndpointValue.getDocument().addDocumentListener(new DocumentListener()
                    {
                        @Override
                        public void changedUpdate(DocumentEvent e)
                        {
                            validatePanel();
                        }

                        @Override
                        public void removeUpdate(DocumentEvent e)
                        {
                            validatePanel();
                        }

                        @Override
                        public void insertUpdate(DocumentEvent e)
                        {
                            validatePanel();
                        }
                    });

                builder.append(new JLabel("<html><b>IceGrid Registry Endpoint(s)</b></html>"));
                builder.append(_directCustomEndpointValue);
                builder.append(new JLabel("<html>Corresponds to the client endpoints of the IceGrid registry.<br/>" +
                                          "For example: tcp -h registry.domain.com -p 4061</html>"));
                _cardPanel.add(builder.getPanel(), WizardStep.DirectCustomEnpointStep.toString());
            }

            // Routed Custom Endpoint panel
            {
                FormLayout layout = new FormLayout("pref:grow", "pref");
                DefaultFormBuilder builder = new DefaultFormBuilder(layout);
                builder.border(Borders.DIALOG);
                builder.rowGroupingEnabled(true);

                _routedCustomEndpointValue = new JTextField(20);
                _routedCustomEndpointValue.addFocusListener(new FocusListener(_routedCustomEndpointValue));
                _routedCustomEndpointValue.getDocument().addDocumentListener(new DocumentListener()
                    {
                        @Override
                        public void changedUpdate(DocumentEvent e)
                        {
                            validatePanel();
                        }

                        @Override
                        public void removeUpdate(DocumentEvent e)
                        {
                            validatePanel();
                        }

                        @Override
                        public void insertUpdate(DocumentEvent e)
                        {
                            validatePanel();
                        }
                    });

                builder.append(new JLabel("<html><b>Glacier2 Router Endpoint(s)</b></html>"));
                builder.append(_routedCustomEndpointValue);
                builder.append(new JLabel("<html>Corresponds to the client endpoints of the Glacier2 router.<br/>" +
                                          "For example: tcp -h router.domain.com -p 4063</html>"));
                _cardPanel.add(builder.getPanel(), WizardStep.RoutedCustomEnpointStep.toString());
            }

            // X509Certificate panel
            {
                FormLayout layout = new FormLayout("pref:grow", "pref");
                DefaultFormBuilder builder = new DefaultFormBuilder(layout);
                builder.border(Borders.DIALOG);
                builder.rowGroupingEnabled(true);

                ButtonGroup group = new ButtonGroup();

                _x509CertificateNoButton = new JRadioButton(new AbstractAction("No")
                    {
                        @Override
                        public void actionPerformed(ActionEvent e)
                        {
                            _x509CertificateDefault = false;
                            _usernamePasswordAuthButton.setSelected(true);
                            validatePanel();
                        }
                    });
                _x509CertificateNoButton.setSelected(true);
                group.add(_x509CertificateNoButton);
                _x509CertificateYesButton = new JRadioButton(new AbstractAction("Yes")
                    {
                        @Override
                        public void actionPerformed(ActionEvent e)
                        {
                            _x509CertificateDefault = false;
                            validatePanel();
                        }
                    });
                group.add(_x509CertificateYesButton);

                builder.append(
                    new JLabel(
                        "<html><b>Do you want to provide an X.509 certificate for SSL authentication?</b></html>"));
                builder.append(_x509CertificateNoButton);
                builder.append(_x509CertificateYesButton);

                _cardPanel.add(builder.getPanel(), WizardStep.X509CertificateStep.toString());
            }

            // Direct X509 credentials panel
            {
                _directCertificateAliases = new JComboBox();
                _directCertificateAliases.addActionListener(new ActionListener ()
                    {
                        @Override
                        public void actionPerformed(ActionEvent e)
                        {
                            validatePanel();
                        }
                    });

                _directImportCertificate = new JButton(new AbstractAction("Import...")
                    {
                        @Override
                        public void actionPerformed(ActionEvent e)
                        {
                            CertificateManagerDialog d = certificateManager(ConnectionWizardDialog.this);
                            if(d != null)
                            {
                                d.load();
                                d.setActiveTab(0); // Select My Certificates tab
                                d.showDialog();
                                loadCertificateAliases(_directCertificateAliases);
                                validatePanel();
                            }
                        }
                    });

                JPanel alias;
                {
                    FormLayout layout = new FormLayout("pref:grow, 2dlu, pref", "");
                    DefaultFormBuilder builder = new DefaultFormBuilder(layout);
                    builder.rowGroupingEnabled(true);
                    builder.append(_directCertificateAliases, _directImportCertificate);
                    alias = builder.getPanel();
                }

                JPanel panel;
                {
                    FormLayout layout = new FormLayout("pref, 2dlu, pref:grow", "");
                    DefaultFormBuilder builder = new DefaultFormBuilder(layout);
                    builder.rowGroupingEnabled(true);

                    builder.append("<html><b>Alias:</b></html>", alias);
                    builder.append("", new JLabel(
                        "<html><p>Your X.509 certificate for SSL authentication.</p></html>"));

                    _directCertificatePassword = new JPasswordField();
                    builder.append("<html><b>Password:</b></html>", _directCertificatePassword);
                    builder.append("", new JLabel("<html>Enter your certificate password above to save it " +
                                                  "with this connection; otherwise<br>you will need to enter " +
                                                  "this password each time you connect.</p></html>"));

                    panel = builder.getPanel();
                }

                FormLayout layout = new FormLayout("pref:grow", "pref");
                DefaultFormBuilder builder = new DefaultFormBuilder(layout);
                builder.border(Borders.DIALOG);
                builder.rowGroupingEnabled(true);
                builder.append(new JLabel("<html><b>X.509 Certificate</b></html>"));
                builder.append(panel);
                _cardPanel.add(builder.getPanel(), WizardStep.DirectX509CredentialsStep.toString());
            }

            // Routed X509 credentials panel
            {
                _routedCertificateAliases = new JComboBox();
                _routedCertificateAliases.addActionListener (new ActionListener ()
                    {
                        @Override
                        public void actionPerformed(ActionEvent e)
                        {
                            validatePanel();
                        }
                    });

                _routedImportCertificate = new JButton(new AbstractAction("Import...")
                    {
                        @Override
                        public void actionPerformed(ActionEvent e)
                        {
                            CertificateManagerDialog d = certificateManager(ConnectionWizardDialog.this);
                            if(d != null)
                            {
                                d.load();
                                d.setActiveTab(0); // Select My Certificates tab
                                d.showDialog();
                                loadCertificateAliases(_routedCertificateAliases);
                                validatePanel();
                            }
                        }
                    });

                JPanel alias;
                {
                    FormLayout layout = new FormLayout("pref:grow, 2dlu, pref", "");
                    DefaultFormBuilder builder = new DefaultFormBuilder(layout);
                    builder.rowGroupingEnabled(true);
                    builder.append(_routedCertificateAliases, _routedImportCertificate);
                    alias = builder.getPanel();
                }

                JPanel panel;
                {
                    FormLayout layout = new FormLayout("pref, 2dlu, pref:grow", "");
                    DefaultFormBuilder builder = new DefaultFormBuilder(layout);
                    builder.rowGroupingEnabled(true);

                    builder.append("<html><b>Alias:</b></html>", alias);
                    builder.append("", new JLabel(
                        "<html><p>Your X.509 certificate for SSL authentication.</p></html>"));

                    _routedCertificatePassword = new JPasswordField();
                    builder.append("<html><b>Password:</b></html>", _routedCertificatePassword);
                    builder.append("", new JLabel("<html>Enter your certificate password above to save it " +
                                                  "with this connection; otherwise<br>you will need to enter " +
                                                  "this password each time you connect.</p></html>"));

                    panel = builder.getPanel();
                }

                FormLayout layout = new FormLayout("pref:grow", "pref");
                DefaultFormBuilder builder = new DefaultFormBuilder(layout);
                builder.border(Borders.DIALOG);
                builder.rowGroupingEnabled(true);
                builder.append(new JLabel("<html><b>X.509 Certificate</b></html>"));
                builder.append(panel);
                _cardPanel.add(builder.getPanel(), WizardStep.RoutedX509CredentialsStep.toString());
            }

            // Authentication panel
            {
                FormLayout layout = new FormLayout("pref:grow", "pref");
                DefaultFormBuilder builder = new DefaultFormBuilder(layout);
                builder.border(Borders.DIALOG);
                builder.rowGroupingEnabled(true);

                ButtonGroup group = new ButtonGroup();

                _usernamePasswordAuthButton = new JRadioButton(
                    new AbstractAction("Log in with a username and password")
                        {
                            @Override
                            public void actionPerformed(ActionEvent e)
                            {
                                validatePanel();
                            }
                        });
                _usernamePasswordAuthButton.setSelected(true);
                group.add(_usernamePasswordAuthButton);
                _certificateAuthButton = new JRadioButton(new AbstractAction("Log in with my X.509 certificate")
                    {
                        @Override
                        public void actionPerformed(ActionEvent e)
                        {
                            validatePanel();
                        }
                    });
                group.add(_certificateAuthButton);

                builder.append(new JLabel("<html><b>Authentication Type</b></html>"));
                builder.append(_usernamePasswordAuthButton);
                builder.append(_certificateAuthButton);

                _cardPanel.add(builder.getPanel(), WizardStep.AuthStep.toString());
            }

            // Direct Username password credentials panel
            {
                FormLayout layout = new FormLayout("pref, 2dlu, pref:grow", "pref");
                DefaultFormBuilder builder = new DefaultFormBuilder(layout);
                builder.border(Borders.DIALOG);
                builder.rowGroupingEnabled(true);

                _directUsername = new JTextField();
                _directUsername.getDocument().addDocumentListener(new DocumentListener()
                    {
                        @Override
                        public void changedUpdate(DocumentEvent e)
                        {
                            validatePanel();
                            _directUsername.requestFocusInWindow();
                        }

                        @Override
                        public void removeUpdate(DocumentEvent e)
                        {
                            validatePanel();
                            _directUsername.requestFocusInWindow();
                        }

                        @Override
                        public void insertUpdate(DocumentEvent e)
                        {
                            validatePanel();
                            _directUsername.requestFocusInWindow();
                        }
                    });

                builder.append("<html><b>Username:</b></html>", _directUsername);
                _directPassword = new JPasswordField();
                builder.append("<html><b>Password:</b></html>", _directPassword);
                builder.append("", new JLabel("<html>Enter your password above to save it with this connection; " +
                                              "otherwise you will<br>need to enter your password each time " +
                                              "you connect.</p></html>"));

                _cardPanel.add(builder.getPanel(), WizardStep.DirectUsernamePasswordCredentialsStep.toString());
            }

            // Routed Username password credentials panel
            {
                FormLayout layout = new FormLayout("pref, 2dlu, pref:grow", "pref");
                DefaultFormBuilder builder = new DefaultFormBuilder(layout);
                builder.border(Borders.DIALOG);
                builder.rowGroupingEnabled(true);

                _routedUsername = new JTextField();
                _routedUsername.getDocument().addDocumentListener(new DocumentListener()
                    {
                        @Override
                        public void changedUpdate(DocumentEvent e)
                        {
                            validatePanel();
                            _routedUsername.requestFocusInWindow();
                        }

                        @Override
                        public void removeUpdate(DocumentEvent e)
                        {
                            validatePanel();
                            _routedUsername.requestFocusInWindow();
                        }

                        @Override
                        public void insertUpdate(DocumentEvent e)
                        {
                            validatePanel();
                            _routedUsername.requestFocusInWindow();
                        }
                    });

                builder.append("<html><b>Username:</b></html>", _routedUsername);
                _routedPassword = new JPasswordField();
                builder.append("<html><b>Password:</b></html>", _routedPassword);
                builder.append("", new JLabel("<html>Enter your Glacier2 password above to save it with this " +
                                              "connection; otherwise<br>you will need to enter your password " +
                                              "each time you connect.</p></html>"));

                _cardPanel.add(builder.getPanel(), WizardStep.RoutedUsernamePasswordCredentialsStep.toString());
            }

            _backButton = new JButton();
            AbstractAction backAction = new AbstractAction("< Back")
                {
                    @Override
                    public void actionPerformed(ActionEvent e)
                    {
                        if(_wizardSteps.size() <= 1)
                        {
                            _backButton.setEnabled(false);
                            return;
                        }
                        _wizardSteps.pop();

                        _cardLayout.show(_cardPanel, _wizardSteps.elementAt(_wizardSteps.size() - 1).toString());

                        if(_wizardSteps.size() <= 1)
                        {
                            _backButton.setEnabled(false);
                        }
                        validatePanel();
                    }
                };
            _backButton.setAction(backAction);
            _backButton.setEnabled(false);

            _nextButton = new JButton();
            AbstractAction nextAction = new AbstractAction("Next >")
                {
                    @Override
                    public void actionPerformed(ActionEvent e)
                    {
                        _nextButton.setEnabled(false);
                        WizardStep step = _wizardSteps.elementAt(_wizardSteps.size() - 1);
                        switch(step)
                        {
                            case ConnectionTypeStep:
                            {
                                if(_directConnection.isSelected())
                                {
                                    _cardLayout.show(_cardPanel, WizardStep.DirectMasterStep.toString());
                                    _wizardSteps.push(WizardStep.DirectMasterStep);
                                }
                                else
                                {
                                    _cardLayout.show(_cardPanel, WizardStep.RoutedEndpointStep.toString());
                                    _wizardSteps.push(WizardStep.RoutedEndpointStep);
                                }
                                break;
                            }

                            case DirectMasterStep:
                            {
                                _cardLayout.show(_cardPanel, WizardStep.DirectDiscoveryChooseStep.toString());
                                _wizardSteps.push(WizardStep.DirectDiscoveryChooseStep);
                                if(_directDiscoveryDiscoveredLocators.isSelected())
                                {
                                    refreshDiscoveryLocators();
                                }
                                break;
                            }

                            case DirectDiscoveryChooseStep:
                            {
                                if(_directDiscoveryManualEndpoint.isSelected())
                                {
                                    _cardLayout.show(_cardPanel, WizardStep.DirectEndpointStep.toString());
                                    _wizardSteps.push(WizardStep.DirectEndpointStep);
                                }
                                else
                                {
                                    com.zeroc.Ice.LocatorPrx locator = _directDiscoveryLocatorList.getSelectedValue();
                                    _directInstanceName.setText(locator.ice_getIdentity().category);

                                    String endpoints = null;
                                    for(com.zeroc.Ice.Endpoint endpoint : locator.ice_getEndpoints())
                                    {
                                        if(endpoints == null)
                                        {
                                            endpoints = endpoint.toString();
                                        }
                                        else
                                        {
                                            endpoints += ":" + endpoint.toString();
                                        }
                                    }
                                    _directCustomEndpointValue.setText(endpoints);
                                    _directCustomEndpoints.setSelected(true);

                                    _cardLayout.show(_cardPanel, WizardStep.DirectCustomEnpointStep.toString());
                                    _wizardSteps.push(WizardStep.DirectCustomEnpointStep);
                                }
                                break;
                            }

                            case DirectEndpointStep:
                            {
                                if(_directDefaultEndpoints.isSelected())
                                {
                                    _cardLayout.show(_cardPanel, WizardStep.DirectDefaultEndpointStep.toString());
                                    _wizardSteps.push(WizardStep.DirectDefaultEndpointStep);
                                }
                                else
                                {
                                    _cardLayout.show(_cardPanel, WizardStep.DirectCustomEnpointStep.toString());
                                    _wizardSteps.push(WizardStep.DirectCustomEnpointStep);
                                }
                                break;
                            }

                            case RoutedEndpointStep:
                            {
                                if(_routedDefaultEndpoints.isSelected())
                                {
                                    _cardLayout.show(_cardPanel, WizardStep.RoutedDefaultEndpointStep.toString());
                                    _wizardSteps.push(WizardStep.RoutedDefaultEndpointStep);
                                }
                                else
                                {
                                    _cardLayout.show(_cardPanel, WizardStep.RoutedCustomEnpointStep.toString());
                                    _wizardSteps.push(WizardStep.RoutedCustomEnpointStep);
                                }
                                break;
                            }

                            case DirectDefaultEndpointStep:
                            {
                                if(_directDefaultEndpointSSL.isSelected())
                                {
                                    _cardLayout.show(_cardPanel, WizardStep.X509CertificateStep.toString());
                                    _wizardSteps.push(WizardStep.X509CertificateStep);
                                }
                                else
                                {
                                    _cardLayout.show(_cardPanel,
                                    WizardStep.DirectUsernamePasswordCredentialsStep.toString());
                                    _wizardSteps.push(WizardStep.DirectUsernamePasswordCredentialsStep);
                                }
                                if(_x509CertificateDefault)
                                {
                                    if(_directDefaultEndpointSSL.isSelected())
                                    {
                                        _x509CertificateYesButton.setSelected(true);
                                        _certificateAuthButton.setSelected(true);
                                    }
                                    else
                                    {
                                        _x509CertificateNoButton.setSelected(true);
                                        _usernamePasswordAuthButton.setSelected(true);
                                    }
                                }
                                break;
                            }

                            case RoutedDefaultEndpointStep:
                            {
                                if(_routedDefaultEndpointSSL.isSelected())
                                {
                                    _cardLayout.show(_cardPanel, WizardStep.X509CertificateStep.toString());
                                    _wizardSteps.push(WizardStep.X509CertificateStep);
                                }
                                else
                                {
                                    _cardLayout.show(_cardPanel,
                                    WizardStep.RoutedUsernamePasswordCredentialsStep.toString());
                                    _wizardSteps.push(WizardStep.RoutedUsernamePasswordCredentialsStep);
                                }
                                if(_x509CertificateDefault)
                                {
                                    if(_routedDefaultEndpointSSL.isSelected())
                                    {
                                        _x509CertificateYesButton.setSelected(true);
                                        _certificateAuthButton.setSelected(true);
                                    }
                                    else
                                    {
                                        _x509CertificateNoButton.setSelected(true);
                                        _usernamePasswordAuthButton.setSelected(true);
                                    }
                                }
                                break;
                            }

                            case DirectCustomEnpointStep:
                            {
                                try
                                {
                                    com.zeroc.Ice.Identity id = new com.zeroc.Ice.Identity();
                                    id.name = "Locator";
                                    id.category = _directInstanceName.getText();
                                    StringBuilder endpoint = new StringBuilder();
                                    endpoint.append(_coordinator.getCommunicator().identityToString(id));
                                    endpoint.append(":");
                                    endpoint.append(_directCustomEndpointValue.getText());
                                    _coordinator.getCommunicator().stringToProxy(endpoint.toString());
                                    if(containsSecureEndpoints(endpoint.toString()))
                                    {
                                        _cardLayout.show(_cardPanel, WizardStep.X509CertificateStep.toString());
                                        _wizardSteps.push(WizardStep.X509CertificateStep);
                                    }
                                    else
                                    {
                                        _cardLayout.show(_cardPanel,
                                                         WizardStep.DirectUsernamePasswordCredentialsStep.toString());
                                        _wizardSteps.push(WizardStep.DirectUsernamePasswordCredentialsStep);
                                    }
                                }
                                catch(com.zeroc.Ice.EndpointParseException ex)
                                {
                                    JOptionPane.showMessageDialog(
                                        ConnectionWizardDialog.this,
                                        ex.str,
                                        "Error parsing endpoint",
                                        JOptionPane.ERROR_MESSAGE);
                                    return;
                                }
                                catch(com.zeroc.Ice.ProxyParseException ex)
                                {
                                    JOptionPane.showMessageDialog(
                                        ConnectionWizardDialog.this,
                                        ex.str,
                                        "Error parsing endpoint",
                                        JOptionPane.ERROR_MESSAGE);
                                    return;
                                }
                                if(_x509CertificateDefault)
                                {
                                    if(containsSecureEndpoints(_directCustomEndpointValue.getText()))
                                    {
                                        _x509CertificateYesButton.setSelected(true);
                                        _certificateAuthButton.setSelected(true);
                                    }
                                    else
                                    {
                                        _x509CertificateNoButton.setSelected(true);
                                        _usernamePasswordAuthButton.setSelected(true);
                                    }
                                }
                                break;
                            }
                            case RoutedCustomEnpointStep:
                            {
                                try
                                {
                                    com.zeroc.Ice.Identity id = new com.zeroc.Ice.Identity();
                                    id.name = "router";
                                    id.category = _routedInstanceName.getText();
                                    StringBuilder endpoint = new StringBuilder();
                                    endpoint.append(_coordinator.getCommunicator().identityToString(id));
                                    endpoint.append(":");
                                    endpoint.append(_routedCustomEndpointValue.getText());
                                    _coordinator.getCommunicator().stringToProxy(endpoint.toString());
                                    if(containsSecureEndpoints(endpoint.toString()))
                                    {
                                        _cardLayout.show(_cardPanel, WizardStep.X509CertificateStep.toString());
                                        _wizardSteps.push(WizardStep.X509CertificateStep);
                                    }
                                    else
                                    {
                                        _cardLayout.show(_cardPanel,
                                        WizardStep.RoutedUsernamePasswordCredentialsStep.toString());
                                        _wizardSteps.push(WizardStep.RoutedUsernamePasswordCredentialsStep);
                                    }
                                }
                                catch(com.zeroc.Ice.EndpointParseException ex)
                                {
                                    JOptionPane.showMessageDialog(
                                        ConnectionWizardDialog.this,
                                        ex.str,
                                        "Error parsing endpoint",
                                        JOptionPane.ERROR_MESSAGE);
                                    return;
                                }
                                catch(com.zeroc.Ice.ProxyParseException ex)
                                {
                                    JOptionPane.showMessageDialog(
                                        ConnectionWizardDialog.this,
                                        ex.str,
                                        "Error parsing endpoint",
                                        JOptionPane.ERROR_MESSAGE);
                                    return;
                                }
                                if(_x509CertificateDefault)
                                {
                                    if(containsSecureEndpoints(_routedCustomEndpointValue.getText()))
                                    {
                                        _x509CertificateYesButton.setSelected(true);
                                        _certificateAuthButton.setSelected(true);
                                    }
                                    else
                                    {
                                        _x509CertificateNoButton.setSelected(true);
                                        _usernamePasswordAuthButton.setSelected(true);
                                    }
                                }
                                break;
                            }

                            case X509CertificateStep:
                            {
                                if(_x509CertificateYesButton.isSelected())
                                {
                                    if(_directConnection.isSelected())
                                    {
                                        loadCertificateAliases(_directCertificateAliases);
                                        _cardLayout.show(_cardPanel, WizardStep.DirectX509CredentialsStep.toString());
                                        _wizardSteps.push(WizardStep.DirectX509CredentialsStep);
                                    }
                                    else
                                    {
                                        loadCertificateAliases(_routedCertificateAliases);
                                        _cardLayout.show(_cardPanel, WizardStep.RoutedX509CredentialsStep.toString());
                                        _wizardSteps.push(WizardStep.RoutedX509CredentialsStep);
                                    }
                                }
                                else
                                {
                                    if(_directConnection.isSelected())
                                    {
                                        _cardLayout.show(_cardPanel,
                                                         WizardStep.DirectUsernamePasswordCredentialsStep.toString());
                                        _wizardSteps.push(WizardStep.DirectUsernamePasswordCredentialsStep);
                                    }
                                    else
                                    {
                                        _cardLayout.show(_cardPanel,
                                                         WizardStep.RoutedUsernamePasswordCredentialsStep.toString());
                                        _wizardSteps.push(WizardStep.RoutedUsernamePasswordCredentialsStep);
                                    }
                                }
                                break;
                            }
                            case RoutedX509CredentialsStep:
                            case DirectX509CredentialsStep:
                            {
                                _cardLayout.show(_cardPanel, WizardStep.AuthStep.toString());
                                _wizardSteps.push(WizardStep.AuthStep);
                                break;
                            }
                            case AuthStep:
                            {
                                if(_usernamePasswordAuthButton.isSelected())
                                {
                                    if(_directConnection.isSelected())
                                    {
                                        _cardLayout.show(_cardPanel,
                                                         WizardStep.DirectUsernamePasswordCredentialsStep.toString());
                                        _wizardSteps.push(WizardStep.DirectUsernamePasswordCredentialsStep);
                                    }
                                    else
                                    {
                                        _cardLayout.show(_cardPanel,
                                                         WizardStep.RoutedUsernamePasswordCredentialsStep.toString());
                                        _wizardSteps.push(WizardStep.RoutedUsernamePasswordCredentialsStep);
                                    }
                                }
                                break;
                            }

                            default:
                            {
                                break;
                            }
                        }
                        if(_wizardSteps.size() > 0)
                        {
                            _backButton.setEnabled(true);
                        }
                        // Validate the new selected panel
                        validatePanel();
                    }
                };
            _nextButton.setAction(nextAction);
            _nextButton.setEnabled(false);

            _finishButton = new JButton();
            AbstractAction finishAction = new AbstractAction("Finish")
                {
                    @Override
                    public void actionPerformed(ActionEvent e)
                    {
                        synchronized(SessionKeeper.this)
                        {
                            if(_discoveryPlugin != null)
                            {
                                _discoveryPlugin.destroy();
                                _discoveryPlugin = null;
                            }
                        }

                        ConnectionInfo inf = getConfiguration();
                        if(inf == null)
                        {
                            inf = new ConnectionInfo();
                        }

                        final boolean direct = _directConnection.isSelected();
                        inf.setDirect(direct);
                        if(direct)
                        {
                            inf.setInstanceName(_directInstanceName.getText());
                            inf.setConnectToMaster(_directConnectToMaster.isSelected());
                            if(_usernamePasswordAuthButton.isSelected())
                            {
                                inf.setUsername(_directUsername.getText());
                                if(_directPassword.getPassword() != null && _directPassword.getPassword().length > 0)
                                {
                                    inf.setPassword(_directPassword.getPassword());
                                    inf.setStorePassword(true);
                                }
                                else
                                {
                                    inf.setPassword(null);
                                    inf.setStorePassword(false);
                                }
                            }

                            if(_x509CertificateYesButton.isSelected())
                            {
                                inf.setAlias((String)_directCertificateAliases.getSelectedItem());
                                if(_directCertificatePassword.getPassword() != null &&
                                   _directCertificatePassword.getPassword().length > 0)
                                {
                                    inf.setKeyPassword(_directCertificatePassword.getPassword());
                                    inf.setStoreKeyPassword(true);
                                }
                                else
                                {
                                    inf.setKeyPassword(null);
                                    inf.setStoreKeyPassword(false);
                                }
                            }

                            if(_directDefaultEndpoints.isSelected())
                            {
                                inf.setHost(_directDefaultEndpointHost.getText());
                                inf.setSSL(_directDefaultEndpointSSL.isSelected());
                                String port = _directDefaultEndpointPort.getText();
                                if(port != null && !port.isEmpty())
                                {
                                    try
                                    {
                                        inf.setPort(Integer.parseInt(port));
                                        inf.setDefaultPort(false);
                                    }
                                    catch(NumberFormatException ex)
                                    {
                                    }
                                }
                                else
                                {
                                    inf.setDefaultPort(true);
                                }
                            }
                            else
                            {
                                inf.setEndpoint(_directCustomEndpointValue.getText());
                            }
                        }
                        else
                        {
                            inf.setInstanceName(_routedInstanceName.getText());

                            if(_usernamePasswordAuthButton.isSelected())
                            {
                                inf.setUsername(_routedUsername.getText());
                                if(_routedPassword.getPassword() != null && _routedPassword.getPassword().length > 0)
                                {
                                    inf.setPassword(_routedPassword.getPassword());
                                    inf.setStorePassword(true);
                                }
                                else
                                {
                                    inf.setPassword(null);
                                    inf.setStorePassword(false);
                                }
                            }

                            if(_x509CertificateYesButton.isSelected())
                            {
                                inf.setAlias((String)_routedCertificateAliases.getSelectedItem());
                                if(_routedCertificatePassword.getPassword() != null &&
                                   _routedCertificatePassword.getPassword().length > 0)
                                {
                                    inf.setKeyPassword(_routedCertificatePassword.getPassword());
                                    inf.setStoreKeyPassword(true);
                                }
                                else
                                {
                                    inf.setKeyPassword(null);
                                    inf.setStoreKeyPassword(false);
                                }
                            }

                            if(_routedDefaultEndpoints.isSelected())
                            {
                                inf.setHost(_routedDefaultEndpointHost.getText());
                                inf.setSSL(_routedDefaultEndpointSSL.isSelected());
                                String port = _routedDefaultEndpointPort.getText();
                                if(port != null && !port.isEmpty())
                                {
                                    try
                                    {
                                        inf.setPort(Integer.parseInt(port));
                                        inf.setDefaultPort(false);
                                    }
                                    catch(NumberFormatException ex)
                                    {
                                    }
                                }
                                else
                                {
                                    inf.setDefaultPort(true);
                                }
                            }
                            else
                            {
                                inf.setEndpoint(_routedCustomEndpointValue.getText());
                            }
                        }

                        if(_usernamePasswordAuthButton.isSelected())
                        {
                            inf.setAuth(AuthType.UsernamePasswordAuthType);
                            inf.setUseX509Certificate(_x509CertificateYesButton.isSelected());
                        }
                        else
                        {
                            inf.setAuth(AuthType.X509CertificateAuthType);
                            inf.setUseX509Certificate(true);
                        }

                        if(_connectNow)
                        {
                            login(parent, inf);
                        }
                        else
                        {
                            ConnectionWizardDialog.this.dispose();
                        }
                    }
                };
            _finishButton.setAction(finishAction);
            _finishButton.setEnabled(false);

            _cancelButton = new JButton();
            AbstractAction cancelAction = new AbstractAction("Cancel")
                {
                    @Override
                    public void actionPerformed(ActionEvent e)
                    {
                        synchronized(SessionKeeper.this)
                        {
                            if(_discoveryPlugin != null)
                            {
                                _discoveryPlugin.destroy();
                                _discoveryPlugin = null;
                            }
                        }
                        dispose();
                    }
                };
            _cancelButton.setAction(cancelAction);

            JComponent buttonBar = new ButtonBarBuilder().addGlue().addButton(_backButton, _nextButton).
                addUnrelatedGap().addButton(_finishButton, _cancelButton).build();
            buttonBar.setBorder(Borders.DIALOG);
            getContentPane().add(buttonBar, java.awt.BorderLayout.SOUTH);

            getContentPane().add(_cardPanel, java.awt.BorderLayout.CENTER);

            pack();
            setResizable(false);
            load();
            validatePanel();
        }

        //
        // Validate current step, and enabled/disable buttons
        //
        boolean validatePanel()
        {
            WizardStep step = _wizardSteps.elementAt(_wizardSteps.size() - 1);

            boolean lastStep = false; // No next step
            switch(step)
            {
                case DirectDiscoveryChooseStep:
                {
                    if(_directDiscoveryManualEndpoint.isSelected())
                    {
                        _directDiscoveryManualEndpoint.requestFocusInWindow();
                    }
                    else
                    {
                        _directDiscoveryLocatorList.requestFocusInWindow();
                    }
                    break;
                }

                case DirectEndpointStep:
                {
                    if(_directDefaultEndpoints.isSelected())
                    {
                        _directDefaultEndpoints.requestFocusInWindow();
                    }
                    else
                    {
                        _directCustomEndpoints.requestFocusInWindow();
                    }
                    break;
                }
                case DirectDefaultEndpointStep:
                {
                    _directDefaultEndpointHost.requestFocusInWindow();
                    break;
                }
                case DirectCustomEnpointStep:
                {
                    _directCustomEndpointValue.requestFocusInWindow();
                    break;
                }

                case RoutedEndpointStep:
                {
                    if(_routedDefaultEndpoints.isSelected())
                    {
                        _routedDefaultEndpoints.requestFocusInWindow();
                    }
                    else
                    {
                        _routedCustomEndpoints.requestFocusInWindow();
                    }
                    break;
                }
                case RoutedDefaultEndpointStep:
                {
                    _routedDefaultEndpointHost.requestFocusInWindow();
                    break;
                }
                case RoutedCustomEnpointStep:
                {
                    _routedCustomEndpointValue.requestFocusInWindow();
                    break;
                }
                case X509CertificateStep:
                {
                    if(_x509CertificateYesButton.isSelected())
                    {
                        _x509CertificateYesButton.requestFocusInWindow();
                    }
                    else
                    {
                        _x509CertificateNoButton.requestFocusInWindow();
                    }
                }
                case DirectX509CredentialsStep:
                {
                    _directCertificateAliases.requestFocusInWindow();
                    break;
                }
                case RoutedX509CredentialsStep:
                {
                    _routedCertificateAliases.requestFocusInWindow();
                    break;
                }
                case AuthStep:
                {
                    if(_usernamePasswordAuthButton.isSelected())
                    {
                        _usernamePasswordAuthButton.requestFocusInWindow();
                    }
                    else
                    {
                        lastStep = true;
                        _certificateAuthButton.requestFocusInWindow();
                    }
                    break;
                }
                case DirectUsernamePasswordCredentialsStep:
                {
                    lastStep = true;
                    _directUsername.requestFocusInWindow();
                    break;
                }
                case RoutedUsernamePasswordCredentialsStep:
                {
                    lastStep = true;
                    _routedUsername.requestFocusInWindow();
                    break;
                }

                default:
                {
                    break;
                }
            }

            boolean validated = validateWizardStep(step);

            _nextButton.setEnabled(validated && !lastStep);

            if(!validated)
            {
                return false;
            }

            _finishButton.setEnabled(validateConfiguration());

            if(lastStep)
            {
                getRootPane().setDefaultButton(_finishButton);
            }
            else
            {
                getRootPane().setDefaultButton(_nextButton);
            }
            return validated;
        }

        boolean validateWizardStep(WizardStep step)
        {
            boolean validated = false;
            switch(step)
            {
                case ConnectionTypeStep:
                {
                    validated = true;
                    break;
                }

                case DirectDiscoveryChooseStep:
                {
                    if(_directDiscoveryManualEndpoint.isSelected())
                    {
                        validated = true;
                    }
                    else
                    {
                        validated = _directDiscoveryLocatorList.getSelectedValue() != null;
                    }
                    break;
                }

                case DirectDefaultEndpointStep:
                {
                    validated = _directDefaultEndpointHost.getText() != null &&
                                _directDefaultEndpointHost.getText().length() > 0;
                    String port = _directDefaultEndpointPort.getText();
                    if(port != null && port.length() > 0)
                    {
                        try
                        {
                            Integer.parseInt(port);
                        }
                        catch(NumberFormatException ex)
                        {
                            validated = false;
                            JOptionPane.showMessageDialog(
                                        ConnectionWizardDialog.this,
                                        "Invalid port number `" + port + "'",
                                        "Invalid port Number",
                                        JOptionPane.ERROR_MESSAGE);
                        }
                    }
                    break;
                }
                case DirectCustomEnpointStep:
                {
                    validated = _directCustomEndpointValue.getText() != null &&
                                _directCustomEndpointValue.getText().length() > 0;
                    break;
                }

                case RoutedDefaultEndpointStep:
                {
                    validated = _routedDefaultEndpointHost.getText() != null &&
                                _routedDefaultEndpointHost.getText().length() > 0;
                    String port = _routedDefaultEndpointPort.getText();
                    if(port != null && port.length() > 0)
                    {
                        try
                        {
                            Integer.parseInt(port);
                        }
                        catch(NumberFormatException ex)
                        {
                            validated = false;
                            JOptionPane.showMessageDialog(
                                        ConnectionWizardDialog.this,
                                        "Invalid port number `" + port + "'",
                                        "Invalid port Number",
                                        JOptionPane.ERROR_MESSAGE);
                        }
                    }
                    break;
                }
                case RoutedCustomEnpointStep:
                {
                    validated = _routedCustomEndpointValue.getText() != null &&
                                _routedCustomEndpointValue.getText().length() > 0;
                    break;
                }
                case DirectX509CredentialsStep:
                {
                    validated = _directCertificateAliases.getSelectedItem() != null;
                    break;
                }
                case RoutedX509CredentialsStep:
                {
                    validated = _routedCertificateAliases.getSelectedItem() != null;
                    break;
                }
                case DirectUsernamePasswordCredentialsStep:
                {
                    validated = _directUsername.getText() != null && _directUsername.getText().length() > 0;
                    break;
                }
                case RoutedUsernamePasswordCredentialsStep:
                {
                    validated = _routedUsername.getText() != null && _routedUsername.getText().length() > 0;
                    break;
                }

                case DirectMasterStep:
                case RoutedEndpointStep:
                case DirectEndpointStep:
                case AuthStep:
                case X509CertificateStep:
                {
                    validated = true;
                    break;
                }
                default:
                {
                    break;
                }
            }
            return validated;
        }

        boolean validateConfiguration()
        {
            //
            // If there isn't secure endpoints, we must set auth type to username password
            // and use X509 certificate to false.
            //
            if(!hasSecureEndpoints())
            {
                _x509CertificateNoButton.setSelected(true);
                _usernamePasswordAuthButton.setSelected(true);
            }

            if(!validateWizardStep(WizardStep.ConnectionTypeStep))
            {
                return false;
            }
            if(_directConnection.isSelected())
            {
                if(_directDefaultEndpoints.isSelected())
                {
                    if(!validateWizardStep(WizardStep.DirectDefaultEndpointStep))
                    {
                        return false;
                    }
                }
                else
                {
                    if(!validateWizardStep(WizardStep.DirectCustomEnpointStep))
                    {
                        return false;
                    }
                }
            }
            else // Routed
            {
                if(_routedDefaultEndpoints.isSelected())
                {
                    if(!validateWizardStep(WizardStep.RoutedDefaultEndpointStep))
                    {
                        return false;
                    }
                }
                else
                {
                    if(!validateWizardStep(WizardStep.RoutedCustomEnpointStep))
                    {
                        return false;
                    }
                }
            }

            if(_x509CertificateYesButton.isSelected())
            {
                if(_directConnection.isSelected())
                {
                    if(!validateWizardStep(WizardStep.DirectX509CredentialsStep))
                    {
                        return false;
                    }
                }
                else
                {
                    if(!validateWizardStep(WizardStep.RoutedX509CredentialsStep))
                    {
                        return false;
                    }
                }
            }

            if(_usernamePasswordAuthButton.isSelected())
            {
                if(_directConnection.isSelected())
                {
                    if(!validateWizardStep(WizardStep.DirectUsernamePasswordCredentialsStep))
                    {
                        return false;
                    }
                }
                else
                {
                    if(!validateWizardStep(WizardStep.RoutedUsernamePasswordCredentialsStep))
                    {
                        return false;
                    }
                }
            }
            return true;
        }

        public void loadCertificateAliases(JComboBox component)
        {
            CertificateManagerDialog certificateManager = certificateManager(this);
            if(certificateManager != null)
            {
                String item = (String)component.getSelectedItem();
                component.setModel(new DefaultComboBoxModel(certificateManager.myCertsAliases()));
                if(item != null)
                {
                    component.setSelectedItem(item);
                }
            }
        }

        private void load()
        {
            if(_conf != null)
            {
                if(_conf.getDirect())
                {
                    _directDiscoveryManualEndpoint.setSelected(true);
                    _directConnection.setSelected(true);
                    _directConnectToMaster.setSelected(_conf.getConnectToMaster());
                    _directInstanceName.setText(_conf.getInstanceName());

                    if(_conf.getAuth() == AuthType.UsernamePasswordAuthType)
                    {
                        _directUsername.setText(_conf.getUsername());
                        if(_conf.getPassword() != null && _conf.getStorePassword())
                        {
                            _directPassword.setText(new String(_conf.getPassword()));
                        }
                    }

                    if(_conf.getUseX509Certificate())
                    {
                        _directCertificateAliases.setSelectedItem(_conf.getAlias());
                        if(_conf.getKeyPassword() != null && _conf.getStoreKeyPassword())
                        {
                            _directCertificatePassword.setText(new String(_conf.getKeyPassword()));
                        }
                    }

                    if(_conf.getDefaultEndpoint())
                    {
                        _directDefaultEndpoints.setSelected(true);
                        _directDefaultEndpointHost.setText(_conf.getHost());

                        if(_conf.getSSL())
                        {
                            _directDefaultEndpointSSL.setSelected(true);
                            if(!_conf.getDefaultPort())
                            {
                                _directDefaultEndpointPort.setText(Integer.toString(_conf.getPort()));
                            }
                        }
                        else
                        {
                            _directDefaultEndpointTCP.setSelected(true);
                            if(!_conf.getDefaultPort())
                            {
                                _directDefaultEndpointPort.setText(Integer.toString(_conf.getPort()));
                            }
                        }
                    }
                    else
                    {
                        _directCustomEndpoints.setSelected(true);
                        _directCustomEndpointValue.setText(_conf.getEndpoint());
                    }
                }
                else
                {
                    _routedConnection.setSelected(true);
                    _routedInstanceName.setText(_conf.getInstanceName());

                    if(_conf.getAuth() == AuthType.UsernamePasswordAuthType)
                    {
                        _routedUsername.setText(_conf.getUsername());
                        if(_conf.getPassword() != null && _conf.getStorePassword())
                        {
                            _routedPassword.setText(new String(_conf.getPassword()));
                        }
                    }

                    if(_conf.getUseX509Certificate())
                    {
                        _routedCertificateAliases.setSelectedItem(_conf.getAlias());
                        if(_conf.getKeyPassword() != null && _conf.getStoreKeyPassword())
                        {
                            _routedCertificatePassword.setText(new String(_conf.getKeyPassword()));
                        }
                    }

                    if(_conf.getDefaultEndpoint())
                    {
                        _routedDefaultEndpoints.setSelected(true);
                        _routedDefaultEndpointHost.setText(_conf.getHost());

                        if(_conf.getSSL())
                        {
                            _routedDefaultEndpointSSL.setSelected(true);
                            if(!_conf.getDefaultPort())
                            {
                                _routedDefaultEndpointPort.setText(Integer.toString(_conf.getPort()));
                            }
                        }
                        else
                        {
                            _routedDefaultEndpointTCP.setSelected(true);
                            if(!_conf.getDefaultPort())
                            {
                                _routedDefaultEndpointPort.setText(Integer.toString(_conf.getPort()));
                            }
                        }
                    }
                    else
                    {
                        _routedCustomEndpoints.setSelected(true);
                        _routedCustomEndpointValue.setText(_conf.getEndpoint());
                    }
                }

                if(_conf.getAuth() == AuthType.UsernamePasswordAuthType)
                {
                    _usernamePasswordAuthButton.setSelected(true);
                    if(_conf.getUseX509Certificate())
                    {
                        _x509CertificateYesButton.setSelected(true);
                    }
                    else
                    {
                        _x509CertificateNoButton.setSelected(true);
                    }
                }
                else
                {
                    _x509CertificateYesButton.setSelected(true);
                    _certificateAuthButton.setSelected(true);
                }
                validateConfiguration();
            }
        }

        private ConnectionInfo getConfiguration()
        {
            return _conf;
        }

        public boolean hasSecureEndpoints()
        {
            if(_directConnection.isSelected())
            {
                if(_directDefaultEndpoints.isSelected())
                {
                    return _directDefaultEndpointSSL.isSelected();
                }
                else
                {
                    com.zeroc.Ice.Identity id = new com.zeroc.Ice.Identity();
                    id.name = "Locator";
                    id.category = _directInstanceName.getText();
                    StringBuilder endpoint = new StringBuilder();
                    endpoint.append(_coordinator.getCommunicator().identityToString(id));
                    endpoint.append(":");
                    endpoint.append(_directCustomEndpointValue.getText());
                    return containsSecureEndpoints(endpoint.toString());
                }
            }
            else
            {
                if(_routedDefaultEndpoints.isSelected())
                {
                    return _routedDefaultEndpointSSL.isSelected();
                }
                else
                {
                    com.zeroc.Ice.Identity id = new com.zeroc.Ice.Identity();
                    id.name = "router";
                    id.category = _routedInstanceName.getText();
                    StringBuilder endpoint = new StringBuilder();
                    endpoint.append(_coordinator.getCommunicator().identityToString(id));
                    endpoint.append(":");
                    endpoint.append(_routedCustomEndpointValue.getText());
                    return containsSecureEndpoints(endpoint.toString());
                }
            }
        }

        private JPanel _cardPanel;
        private CardLayout _cardLayout;
        private JButton _backButton;
        private JButton _nextButton;
        private JButton _finishButton;
        private JButton _cancelButton;

        // Connection type components
        private JRadioButton _directConnection;
        private JRadioButton _routedConnection;

        // Direct Instance panel components
        private JLabel _directInstanceName;
        private JCheckBox _directConnectToMaster;

        // Direct Discovery Endpoints
        private JList<com.zeroc.Ice.LocatorPrx> _directDiscoveryLocatorList;
        private DefaultListModel<com.zeroc.Ice.LocatorPrx> _directDiscoveryLocatorModel;
        private JRadioButton _directDiscoveryDiscoveredLocators;
        private JLabel _discoveryStatus;
        private JButton _discoveryRefresh;

        private JRadioButton _directDiscoveryManualEndpoint;

        // Direct Endpoints panel components
        private JRadioButton _directDefaultEndpoints;

        // Direct Default endpoints panel components
        private JRadioButton _directCustomEndpoints;
        private JTextField _directDefaultEndpointHost;
        private JTextField _directDefaultEndpointPort;
        private JRadioButton _directDefaultEndpointTCP;
        private JRadioButton _directDefaultEndpointSSL;

        // Direct Custom endpoints panel components
        private JTextField _directCustomEndpointValue;

        // Routed Instance panel components
        private JLabel _routedInstanceName;

        // Routed Endpoints panel components
        private JRadioButton _routedDefaultEndpoints;
        private JRadioButton _routedCustomEndpoints;

        // Routed Default endpoints panel components
        private JTextField _routedDefaultEndpointHost;
        private JTextField _routedDefaultEndpointPort;
        private JRadioButton _routedDefaultEndpointTCP;
        private JRadioButton _routedDefaultEndpointSSL;

        // Routed Custom endpoints panel components
        private JTextField _routedCustomEndpointValue;

        // X509 Certificate panel components
        private JRadioButton _x509CertificateNoButton;
        private JRadioButton _x509CertificateYesButton;

        // Direct X509 Credentials
        private JComboBox _directCertificateAliases;
        private JButton _directImportCertificate;
        private JPasswordField _directCertificatePassword;

        // Routed X509 Credentials
        private JComboBox _routedCertificateAliases;
        private JButton _routedImportCertificate;
        private JPasswordField _routedCertificatePassword;

        // Auth panel components
        private JRadioButton _usernamePasswordAuthButton;
        private JRadioButton _certificateAuthButton;

        // Direct username password credentials components.
        private JTextField _directUsername;
        private JPasswordField _directPassword;

        // Routed username password credentials components.
        private JTextField _routedUsername;
        private JPasswordField _routedPassword;

        // Finish configuration panel components
        private boolean _connectNow;

        //
        // The wizard steps the user has walked throw.
        //
        java.util.Stack<WizardStep> _wizardSteps = new java.util.Stack<>();

        ConnectionInfo _conf;
        private boolean _x509CertificateDefault;
    }

    private boolean containsSecureEndpoints(String str)
    {
        try
        {
            for(com.zeroc.Ice.Endpoint endpoint : _coordinator.getCommunicator().stringToProxy(str).ice_getEndpoints())
            {
                if(endpoint.getInfo().secure())
                {
                    return true;
                }
            }
        }
        catch(com.zeroc.Ice.EndpointParseException ex)
        {
        }
        catch(com.zeroc.Ice.ProxyParseException ex)
        {
        }
        return false;
    }

    enum WizardStep {ConnectionTypeStep,
                     DirectMasterStep, DirectDiscoveryChooseStep, DirectEndpointStep, DirectDefaultEndpointStep,
                        DirectCustomEnpointStep,
                     RoutedEndpointStep, RoutedDefaultEndpointStep, RoutedCustomEnpointStep,
                     X509CertificateStep,
                     DirectX509CredentialsStep, RoutedX509CredentialsStep,
                     AuthStep,
                     DirectUsernamePasswordCredentialsStep, RoutedUsernamePasswordCredentialsStep};

    enum AuthType {UsernamePasswordAuthType, X509CertificateAuthType};

    private class ConnectionDetailDialog extends JDialog
    {
        ConnectionDetailDialog(ConnectionInfo inf)
        {
            super(_coordinator.getMainFrame(), "Connection Details - IceGrid GUI", true);
            setDefaultCloseOperation(WindowConstants.DISPOSE_ON_CLOSE);
            JPanel detailsPane = null;
            {
                FormLayout layout = new FormLayout("right:pref, 2dlu, left:pref:grow", "pref");
                DefaultFormBuilder builder = new DefaultFormBuilder(layout);
                builder.border(Borders.DIALOG);
                builder.rowGroupingEnabled(true);
                builder.lineGapSize(LayoutStyle.getCurrent().getLinePad());

                builder.addSeparator("Connection Details");
                builder.nextLine();
                if(inf.getDirect())
                {
                    builder.append(new JLabel("<html><b>IceGrid instance name:</b></html>"),
                                   new JLabel(inf.getInstanceName()));
                }
                else
                {
                    builder.append(new JLabel("<html><b>IceGrid instance name:</b></html>"),
                                   new JLabel(inf.getInstanceName()));
                }

                boolean ssl = false;
                if(inf.getDefaultEndpoint())
                {
                    builder.append(new JLabel("<html><b>Hostname:</b></html>"),
                                new JLabel(inf.getHost()));

                    builder.append(new JLabel("<html><b>Port:</b></html>"),
                                   new JLabel(Integer.toString(inf.getPort())));

                    if(inf.getSSL())
                    {
                        builder.append(new JLabel("<html><b>Protocol:</b></html>"),
                                    new JLabel("SSL"));
                    }
                    else
                    {
                        builder.append(new JLabel("<html><b>Protocol:</b></html>"),
                                    new JLabel("TCP"));
                    }
                    ssl = inf.getSSL();
                }
                else
                {
                    builder.append(new JLabel("<html><b>Endpoints:</b></html>"),
                                   new JLabel(inf.getEndpoint()));

                    com.zeroc.Ice.Identity id = new com.zeroc.Ice.Identity();
                    id.name = inf.getDirect() ? "Locator" : "router";
                    id.category = inf.getInstanceName();
                    StringBuilder endpoint = new StringBuilder();
                    endpoint.append(_coordinator.getCommunicator().identityToString(id));
                    endpoint.append(":");
                    endpoint.append(inf.getEndpoint());
                    ssl = containsSecureEndpoints(endpoint.toString());
                }

                if(inf.getAuth() == AuthType.UsernamePasswordAuthType)
                {
                    builder.append(new JLabel("<html><b>Authentication mode:</b></html>"),
                                   new JLabel("Username and password"));
                }
                else
                {
                    builder.append(new JLabel("<html><b>Authentication mode:</b></html>"),
                                   new JLabel("SSL Certificate"));
                }

                if(ssl)
                {
                    if(inf.getUseX509Certificate())
                    {
                        builder.append(new JLabel("<html><b>Use SSL Client Certificate:</b></html>"),
                                       new JLabel("Yes"));
                    }
                    else
                    {
                        builder.append(new JLabel("<html><b>Use SSL Client Certificate:</b></html>"),
                                       new JLabel("No"));
                    }
                }
                detailsPane = builder.getPanel();
            }

            Container contentPane = getContentPane();
            contentPane.setLayout(new BoxLayout(contentPane, BoxLayout.Y_AXIS));
            contentPane.add(detailsPane);

            JButton closeButton = new JButton("Close");
            closeButton.addActionListener(new ActionListener()
                {
                    @Override
                    public void actionPerformed(ActionEvent e)
                    {
                        dispose();
                    }
                });
            JComponent buttonBar = new ButtonBarBuilder().addGlue().addButton(closeButton).build();
            buttonBar.setBorder(Borders.DIALOG);
            contentPane.add(buttonBar);
            getRootPane().setDefaultButton(closeButton);
            pack();
            setResizable(false);
        }
    }

    private class ConnectionManagerDialog extends JDialog
    {
        ConnectionManagerDialog()
        {
            super(_coordinator.getMainFrame(), "Saved Connections - IceGrid GUI", true);
            setDefaultCloseOperation(WindowConstants.HIDE_ON_CLOSE);

            JPanel connectionActionPanel = null;
            {
                FormLayout layout = new FormLayout("pref, 2dlu, pref, 2dlu, pref, 2dlu, pref");

                DefaultFormBuilder builder = new DefaultFormBuilder(layout);

                _newConnectionButton  = new JButton("New Connection");
                _newConnectionButton.setToolTipText("Configure a new connection with IceGrid registry");
                _newConnectionButton.addActionListener(new ActionListener()
                    {
                        @Override
                        public void actionPerformed(ActionEvent e)
                        {
                            JDialog dialog = new ConnectionWizardDialog(ConnectionManagerDialog.this);
                            setConnectionWizard(dialog);
                            Utils.addEscapeListener(dialog);
                            dialog.setLocationRelativeTo(ConnectionManagerDialog.this);
                            dialog.setVisible(true);
                        }
                    });
                builder.append(_newConnectionButton);
                builder.nextLine();

                _viewConnectionButton = new JButton("View Connection");
                _viewConnectionButton.setToolTipText("View connection details.");
                _viewConnectionButton.setEnabled(false);
                _viewConnectionButton.addActionListener(new ActionListener()
                    {
                        @Override
                        public void actionPerformed(ActionEvent e)
                        {
                            Object obj = _connectionList.getSelectedValue();
                            if(obj != null && obj instanceof ConnectionInfo)
                            {
                                ConnectionInfo inf = (ConnectionInfo)obj;

                                JDialog dialog = new ConnectionDetailDialog(inf);
                                Utils.addEscapeListener(dialog);
                                dialog.setLocationRelativeTo(ConnectionManagerDialog.this);
                                dialog.setVisible(true);
                            }
                        }
                    });
                builder.append(_viewConnectionButton);
                builder.nextLine();

                _editConnectionButton = new JButton("Edit Connection");
                _editConnectionButton.setToolTipText("Edit connection configuration");
                _editConnectionButton.setEnabled(false);
                _editConnectionButton.addActionListener(new ActionListener()
                    {
                        @Override
                        public void actionPerformed(ActionEvent e)
                        {
                            Object obj = _connectionList.getSelectedValue();
                            if(obj != null && obj instanceof ConnectionInfo)
                            {
                                ConnectionInfo inf = (ConnectionInfo)obj;

                                JDialog dialog = new ConnectionWizardDialog(inf, ConnectionManagerDialog.this);
                                Utils.addEscapeListener(dialog);
                                dialog.setLocationRelativeTo(ConnectionManagerDialog.this);
                                dialog.setVisible(true);
                            }
                        }
                    });
                builder.append(_editConnectionButton);
                builder.nextLine();

                _setDefaultConnectionButton = new JButton("Set As Default");
                _setDefaultConnectionButton.setToolTipText("Set the connection to use by default");
                _setDefaultConnectionButton.setEnabled(false);
                _setDefaultConnectionButton.addActionListener(new ActionListener()
                    {
                        @Override
                        public void actionPerformed(ActionEvent e)
                        {
                            _connectionListModel.setDefault();
                        }
                    });
                builder.append(_setDefaultConnectionButton);
                builder.nextLine();

                _removeConnectionButton = new JButton("Remove Connection");
                _removeConnectionButton.setToolTipText("Remove connection configuration");
                _removeConnectionButton.setEnabled(false);
                _removeConnectionButton.addActionListener(new ActionListener()
                    {
                        @Override
                        public void actionPerformed(ActionEvent e)
                        {
                            int index = _connectionList.getSelectedIndex();
                            Object obj = _connectionList.getSelectedValue();
                            if(obj != null && obj instanceof ConnectionInfo)
                            {
                                ConnectionInfo inf = (ConnectionInfo)obj;

                                if(JOptionPane.showConfirmDialog(ConnectionManagerDialog.this,
                                        "Do you want to remove the selected configuration?",
                                        "Remove Configuration - IceGrid GUI",
                                        JOptionPane.YES_NO_OPTION) == JOptionPane.YES_OPTION)
                                {
                                    try
                                    {
                                        String uuid = inf.getUUID();
                                        Preferences prefs = Coordinator.getPreferences().node("Configurations");
                                        prefs = prefs == null ? null : prefs.node(uuid);
                                        if(prefs != null)
                                        {
                                            prefs.removeNode();
                                            load();
                                            if(index > 0)
                                            {
                                                _connectionList.setSelectedIndex(index - 1);
                                            }
                                            if(_connectionListModel.size() > 0)
                                            {
                                                _connectionList.setSelectedIndex(0);
                                            }

                                            if(_connectionList.getSelectedIndex() == -1)
                                            {
                                                _viewConnectionButton.setEnabled(false);
                                                _editConnectionButton.setEnabled(false);
                                                _setDefaultConnectionButton.setEnabled(false);
                                                _removeConnectionButton.setEnabled(false);
                                                _connectButton.setEnabled(false);
                                            }
                                        }
                                    }
                                    catch(java.util.prefs.BackingStoreException ex)
                                    {
                                        JOptionPane.showMessageDialog(ConnectionManagerDialog.this, ex.toString(),
                                                                      "Error removing saved connection",
                                                                      JOptionPane.ERROR_MESSAGE);
                                    }
                                }
                            }
                        }
                    });
                builder.append(_removeConnectionButton);
                builder.nextLine();

                connectionActionPanel = builder.getPanel();
                connectionActionPanel.setBorder(Borders.DIALOG);
            }

            JPanel savedConfigurationsPanel = null;
            {
                FormLayout layout = new FormLayout("left:pref", "pref");
                DefaultFormBuilder builder = new DefaultFormBuilder(layout);
                builder.border(Borders.DIALOG);
                builder.rowGroupingEnabled(true);
                _connectionListModel = new ConnectionListModel();

                class ConnectionListRenderer extends DefaultListCellRenderer
                {
                    @Override
                    public Component
                    getListCellRendererComponent(JList list, Object value, int index, boolean selected,
                                                 boolean hasFocus)
                    {
                        JLabel label = (JLabel)super.getListCellRendererComponent(list, value, index, selected,
                                                                                  hasFocus);
                        if(value instanceof ConnectionInfo)
                        {
                            ConnectionInfo conn = (ConnectionInfo)value;
                            ImageIcon icon = Utils.getIcon(conn.isDefault() ? "/icons/16x16/default.png" :
                                                                              "/icons/16x16/transparent.png");

                            label.setIcon(icon);
                        }
                        return label;
                    }
                }
                _connectionList = new JList(_connectionListModel)
                    {
                        @Override
                        public String getToolTipText(MouseEvent evt)
                        {
                            int index = locationToIndex(evt.getPoint());
                            if(index < 0)
                            {
                                return null;
                            }
                            Object obj = getModel().getElementAt(index);
                            if(obj != null && obj instanceof ConnectionInfo)
                            {
                                ConnectionInfo inf = (ConnectionInfo) obj;
                                return inf.toString();
                            }
                            return null;
                        }
                    };
                _connectionList.setCellRenderer(new ConnectionListRenderer());
                _connectionList.setVisibleRowCount(7);
                _connectionList.addListSelectionListener(new ListSelectionListener()
                    {
                        @Override
                        public void valueChanged(ListSelectionEvent event)
                        {
                            if(!event.getValueIsAdjusting())
                            {
                                _connectButton.setEnabled(true);
                                _viewConnectionButton.setEnabled(true);
                                _editConnectionButton.setEnabled(true);
                                _removeConnectionButton.setEnabled(true);

                                Object selected = _connectionList.getSelectedValue();
                                if(selected != null && ((ConnectionInfo)selected).isDefault())
                                {
                                    _setDefaultConnectionButton.setEnabled(false);
                                }
                                else
                                {
                                    _setDefaultConnectionButton.setEnabled(true);
                                }
                            }
                        }
                    });

                _connectionList.addMouseListener(
                    new MouseAdapter()
                        {
                            @Override
                            public void mouseClicked(MouseEvent e)
                            {
                                if(e.getClickCount() == 2 && e.getButton() == MouseEvent.BUTTON1)
                                {
                                    int index = _connectionList.locationToIndex(e.getPoint());
                                    if(index != -1)
                                    {
                                        Object obj = _connectionListModel.getElementAt(index);
                                        if(obj != null && obj instanceof ConnectionInfo)
                                        {
                                            ConnectionInfo inf = (ConnectionInfo)obj;
                                            login(ConnectionManagerDialog.this, inf);
                                        }
                                    }
                                }
                            }
                        });

                _connectionList.setFixedCellWidth(500);
                builder.append(createStrippedScrollPane(_connectionList));

                savedConfigurationsPanel = builder.getPanel();
            }

            JPanel mainPanel = null;
            {
                FormLayout layout = new FormLayout("pref:grow, 2dlu, pref", "pref");
                CellConstraints cc = new CellConstraints();
                mainPanel = new JPanel(layout);
                mainPanel.add(savedConfigurationsPanel, cc.xy(1, 1));
                mainPanel.add(connectionActionPanel, cc.xy(3, 1));
            }

            Container contentPane = getContentPane();
            contentPane.setLayout(new BoxLayout(contentPane, BoxLayout.Y_AXIS));
            contentPane.add(mainPanel);

            _connectButton = new JButton("Connect");
            _connectButton.addActionListener(new ActionListener()
                {
                    @Override
                    public void actionPerformed(ActionEvent e)
                    {
                        Object obj = _connectionList.getSelectedValue();
                        if(obj != null && obj instanceof ConnectionInfo)
                        {
                            ConnectionInfo inf = (ConnectionInfo)obj;
                            login(ConnectionManagerDialog.this, inf);
                        }
                    }
                });
            _connectButton.setEnabled(false);
            getRootPane().setDefaultButton(_connectButton);

            JButton closeButton = new JButton("Close");
            closeButton.addActionListener(new ActionListener()
                {
                    @Override
                    public void actionPerformed(ActionEvent e)
                    {
                        setVisible(false);
                    }
                });
            JComponent buttonBar = new ButtonBarBuilder().addGlue().addButton(_connectButton, closeButton).build();
            buttonBar.setBorder(Borders.DIALOG);
            contentPane.add(buttonBar);

            pack();
            setResizable(false);
            load();
        }

        void load()
        {
            int selected = _connectionList.getSelectedIndex();
            int defaultIndex = -1;
            Preferences prefs = Coordinator.getPreferences().node("Configurations");
            _connectionListModel.clear();
            try
            {
                String[] childrenNames = prefs.childrenNames();
                for(int i = 0; i < childrenNames.length; ++i)
                {
                    ConnectionInfo info = new ConnectionInfo(childrenNames[i], prefs.node(childrenNames[i]));
                    if(info.isDefault())
                    {
                        defaultIndex = i;
                    }
                    _connectionListModel.addElement(info);
                }

                if(selected >= 0 && selected < _connectionListModel.size())
                {
                    _connectionList.setSelectedIndex(selected);
                }
                else if(defaultIndex >= 0 && defaultIndex < _connectionListModel.size())
                {
                    _connectionList.setSelectedIndex(defaultIndex);
                    _setDefaultConnectionButton.setEnabled(false);
                }
                else if(selected == -1)
                {
                    _connectionList.setSelectedIndex(0);
                }
                else
                {
                    _editConnectionButton.setEnabled(false);
                    _removeConnectionButton.setEnabled(false);
                    _setDefaultConnectionButton.setEnabled(false);
                    _connectButton.setEnabled(false);
                }
            }
            catch(java.util.prefs.BackingStoreException ex)
            {
                JOptionPane.showMessageDialog(this, ex.toString(), "Failed to load saved connections",
                                              JOptionPane.ERROR_MESSAGE);
            }
        }

        void showDialog()
        {
            if(isVisible() == false)
            {
                setLocationRelativeTo(_coordinator.getMainFrame());
                setVisible(true);
            }
        }

        public void setConnectionWizard(JDialog dialog)
        {
            if(_connectionWizard != null)
            {
                _connectionWizard.dispose();
            }
            _connectionWizard = dialog;
        }

        class ConnectionListModel extends DefaultListModel
        {
            public void setDefault()
            {
                ConnectionInfo info = (ConnectionInfo)_connectionList.getSelectedValue();
                if(info == null)
                {
                    return;
                }
                info.setIsDefault(true);
                try
                {
                    info.save();
                }
                catch(java.util.prefs.BackingStoreException ex)
                {
                    JOptionPane.showMessageDialog(_coordinator.getMainFrame(), ex.toString(), "Error saving connection",
                                                  JOptionPane.ERROR_MESSAGE);
                }
                for(Object obj : toArray())
                {
                    if(obj != info)
                    {
                        ((ConnectionInfo)obj).setIsDefault(false);
                        try
                        {
                            ((ConnectionInfo)obj).save();
                        }
                        catch(java.util.prefs.BackingStoreException ex)
                        {
                            JOptionPane.showMessageDialog(_coordinator.getMainFrame(), ex.toString(),
                                                         "Error saving connection", JOptionPane.ERROR_MESSAGE);
                        }
                    }
                }
                _setDefaultConnectionButton.setEnabled(false);
                fireContentsChanged(this, 0, size() -1);
            }
        }

        private JList _connectionList;
        private ConnectionListModel _connectionListModel;

        private JButton _newConnectionButton;
        private JButton _viewConnectionButton;
        private JButton _editConnectionButton;
        private JButton _setDefaultConnectionButton;
        private JButton _removeConnectionButton;

        private JDialog _connectionWizard;

        private JButton _connectButton;
    }

    //
    // Check if the certificate with the given alias requires a password.
    //
    public boolean checkCertificateRequirePassword(String alias)
    {
        try
        {
            java.security.KeyStore keyStore = java.security.KeyStore.getInstance("JKS");
            keyStore.load(new FileInputStream(_coordinator.getDataDirectory() + File.separator + "MyCerts.jks"), null);
            if(keyStore.isKeyEntry(alias))
            {
                keyStore.getKey(alias, new char[]{});
            }
        }
        catch(java.security.UnrecoverableKeyException ex)
        {
            return true;
        }
        catch(Exception ex)
        {
        }
        return false;
    }

    public boolean checkCertificatePassword(String alias, char[] password)
    {
        try
        {
            java.security.KeyStore keyStore = java.security.KeyStore.getInstance("JKS");
            keyStore.load(new FileInputStream(_coordinator.getDataDirectory() + File.separator + "MyCerts.jks"), null);
            if(keyStore.isKeyEntry(alias))
            {
                keyStore.getKey(alias, password);
            }
        }
        catch(Exception ex)
        {
            return false;
        }
        return true;
    }

    public class KeyStorePanel extends JPanel
    {
        public class RequestPasswordResult
        {
            public char[] password;
            public boolean accepted;
        }

        public RequestPasswordResult requestPassword(String title, String label)
        {
            RequestPasswordResult r = new RequestPasswordResult();
            final JPasswordField passwordField = new JPasswordField();
            JOptionPane optionPane = new JOptionPane(new JComponent[]{new JLabel(label), passwordField},
                                                     JOptionPane.QUESTION_MESSAGE, JOptionPane.OK_CANCEL_OPTION);
            JDialog dialog = optionPane.createDialog(KeyStorePanel.this, title);
            Utils.addEscapeListener(dialog);
            dialog.addComponentListener(new ComponentAdapter()
                {
                    @Override
                    public void componentShown(ComponentEvent e)
                    {
                        SwingUtilities.invokeLater(() -> passwordField.requestFocusInWindow());
                    }
                });
            dialog.setLocationRelativeTo(KeyStorePanel.this);
            dialog.setVisible(true);
            Object result = optionPane.getValue();
            dialog.dispose();

            if(result != null && result instanceof Integer && ((Integer)result).intValue() == JOptionPane.OK_OPTION)
            {
                r.password = passwordField.getPassword();
                r.accepted = true;
            }
            else
            {
                r.accepted = false;
            }
            return r;
        }

        public KeyStorePanel() throws java.security.KeyStoreException
        {
            _keyStore = KeyStore.getInstance("JKS");

            FormLayout layout = new FormLayout("pref:grow", "pref:grow");
            DefaultFormBuilder builder = new DefaultFormBuilder(layout);
            final String columnNames[] = new String[]{"Alias", "Subject", "Issuer"};
            _tableModel = new AbstractTableModel()
                {
                    @Override
                    public String getColumnName(int col)
                    {
                        return columnNames[col].toString();
                    }

                    @Override
                    public int getRowCount()
                    {
                        if(_aliases == null)
                        {
                            return 0;
                        }
                        return _aliases.size();
                    }

                    @Override
                    public int getColumnCount()
                    {
                        return columnNames.length;
                    }

                    @Override
                    public Object getValueAt(int row, int col)
                    {
                        if(_aliases == null)
                        {
                            return "";
                        }
                        if(col == 0)
                        {
                            return _aliases.get(row);
                        }

                        try
                        {
                            Certificate cert = _keyStore.getCertificate(_aliases.get(row));
                            if(cert instanceof X509Certificate)
                            {
                                X509Certificate x509Cert = (X509Certificate)cert;
                                if(col == 1)
                                {
                                    return x509Cert.getSubjectX500Principal().toString();
                                }
                                else if(col == 2)
                                {
                                    return x509Cert.getIssuerX500Principal().toString();
                                }
                            }
                        }
                        catch(java.security.KeyStoreException ex)
                        {
                            // Ignored
                        }
                        return "";
                    }

                    @Override
                    public boolean isCellEditable(int row, int col)
                    {
                        return false;
                    }

                    @Override
                    public void setValueAt(Object value, int row, int col)
                    {
                    }
                };

            _certificatesTable = new JTable(_tableModel);
            JScrollPane certificatesScroll = createStrippedScrollPane(_certificatesTable);
            certificatesScroll.setMinimumSize(new Dimension(500, 200));
            certificatesScroll.setPreferredSize(new Dimension(500, 200));
            builder.add(certificatesScroll);

            // Buttons
            {
                _importButton = new JButton();
                AbstractAction importAction = new AbstractAction("Import")
                {
                    @Override
                    public void actionPerformed(ActionEvent e)
                    {
                        String defaultPath = Coordinator.getPreferences().node("Configurations").get("importDirectory", "");
                        JFileChooser chooser = new JFileChooser(defaultPath.equals("") ? null : new File(defaultPath));
                        chooser.setFileFilter(new FileFilter()
                            {
                                //Accept all directories and *.pfx, *.p12 files.
                                @Override
                                public boolean accept(File f)
                                {
                                    if(f.isDirectory())
                                    {
                                        return true;
                                    }

                                    if(f != null && (f.getName().toUpperCase().endsWith(".PFX") ||
                                                     f.getName().toUpperCase().endsWith(".P12")))
                                    {
                                        return true;
                                    }
                                    return false;
                                }

                                @Override
                                public String getDescription()
                                {
                                    return "PKCS12 Files (*.pfx, *.p12)";
                                }
                            });

                        chooser.setFileFilter(new FileFilter()
                            {
                                // Accept all directories and *.pem, *.crt files.
                                @Override
                                public boolean accept(File f)
                                {
                                    if(f.isDirectory())
                                    {
                                        return true;
                                    }

                                    if(f != null && (f.getName().toUpperCase().endsWith(".PEM") ||
                                                     f.getName().toUpperCase().endsWith(".CRT")))
                                    {
                                        return true;
                                    }
                                    return false;
                                }

                                @Override
                                public String getDescription()
                                {
                                    return "PEM Files (*.pem, *.crt)";
                                }
                            });

                        chooser.setFileFilter(new FileFilter()
                            {
                                //Accept all directories and *.jks files.
                                @Override
                                public boolean accept(File f)
                                {
                                    if(f.isDirectory())
                                    {
                                        return true;
                                    }

                                    if(f != null && f.getName().toUpperCase().endsWith(".JKS"))
                                    {
                                        return true;
                                    }
                                    return false;
                                }

                                @Override
                                public String getDescription()
                                {
                                    return "Java Key Store Files (*.jks)";
                                }
                            });
                        File keyFile = null;
                        if(chooser.showOpenDialog(KeyStorePanel.this) == JFileChooser.APPROVE_OPTION )
                        {
                            final String filePath = chooser.getSelectedFile().getAbsolutePath();
                            final boolean pkcs12 = filePath.toUpperCase().endsWith(".PFX") ||
                                                   filePath.toUpperCase().endsWith(".P12");
                            final boolean pem = filePath.toUpperCase().endsWith(".PEM") ||
                                                filePath.toUpperCase().endsWith(".CRT");
                            keyFile = new File(filePath);
                            if(pkcs12)
                            {
                                KeyStore keyStore = null;
                                boolean loaded = false;
                                while(true)
                                {
                                    try
                                    {
                                        keyStore = KeyStore.getInstance("pkcs12");
                                        RequestPasswordResult r = requestPassword("KeyStore Password - IceGrid GUI",
                                                                                  "KeyStore password:");
                                        if(r.accepted)
                                        {
                                            keyStore.load(new FileInputStream(keyFile), r.password);
                                            loaded = true;
                                        }
                                        break;

                                    }
                                    catch(java.io.IOException ex)
                                    {
                                        JOptionPane.showMessageDialog(KeyStorePanel.this,
                                                                      "Invalid certificate password",
                                                                      "Invalid certificate password",
                                                                      JOptionPane.ERROR_MESSAGE);
                                    }
                                    catch(java.lang.Exception ex)
                                    {
                                        JOptionPane.showMessageDialog(KeyStorePanel.this, ex.toString(),
                                                                      "Error importing certificate",
                                                                      JOptionPane.ERROR_MESSAGE);
                                        return;
                                    }
                                }

                                if(loaded)
                                {
                                    try
                                    {
                                        importKeyStore(keyStore);
                                    }
                                    catch(java.lang.Exception ex)
                                    {
                                        JOptionPane.showMessageDialog(KeyStorePanel.this, ex.toString(),
                                                                      "Error importing certificate",
                                                                      JOptionPane.ERROR_MESSAGE);
                                        return;
                                    }
                                }
                            }
                            // PEM File
                            else if(pem)
                            {
                                try
                                {
                                    FileInputStream fis = new FileInputStream(keyFile);
                                    CertificateFactory cf = CertificateFactory.getInstance("X.509");
                                    Collection c = cf.generateCertificates(fis);
                                    Iterator i = c.iterator();
                                    while(i.hasNext())
                                    {
                                        Certificate certificate = (Certificate)i.next();
                                        final String newAlias = JOptionPane.showInputDialog(KeyStorePanel.this,
                                                                                    "Certificate Alias",
                                                                                    "Certificate Alias",
                                                                                    JOptionPane.INFORMATION_MESSAGE);
                                        if(newAlias == null || newAlias.length() == 0)
                                        {
                                            continue;
                                        }
                                        if(_keyStore.containsAlias(newAlias))
                                        {
                                            if(JOptionPane.showConfirmDialog(
                                                    KeyStorePanel.this,
                                                    "<html>Your KeyStore already contains a certificate with alias `" +
                                                    newAlias + "'<br/>" +
                                                    "Do you want to update the certificate?</html>",
                                                    "Confirm Certificate Update - IceGrid GUI",
                                                    JOptionPane.YES_NO_OPTION) == JOptionPane.NO_OPTION)
                                            {
                                                continue;
                                            }
                                        }
                                        _keyStore.setCertificateEntry(newAlias, certificate);
                                    }
                                    _keyStore.store(new FileOutputStream(_keyStorePath), new char[]{});
                                    load(_keyStorePath);
                                }
                                catch(java.lang.Exception ex)
                                {
                                    JOptionPane.showMessageDialog(KeyStorePanel.this, ex.toString(),
                                                                  "Error importing certificate",
                                                                  JOptionPane.ERROR_MESSAGE);
                                    return;
                                }
                            }
                            else
                            {
                                try
                                {
                                    java.security.KeyStore keyStore = java.security.KeyStore.getInstance("JKS");
                                    keyStore.load(new FileInputStream(keyFile), null);
                                    importKeyStore(keyStore);
                                }
                                catch(java.lang.Exception ex)
                                {
                                    JOptionPane.showMessageDialog(KeyStorePanel.this, ex.toString(),
                                                                  "Error importing certificate",
                                                                  JOptionPane.ERROR_MESSAGE);
                                    return;
                                }
                            }
                        }
                        if(keyFile != null)
                        {
                            Coordinator.getPreferences().node("Configurations").put("importDirectory", keyFile.getParent());
                        }
                    }
                };
                _importButton.setAction(importAction);

                _viewButton = new JButton();
                AbstractAction viewAction = new AbstractAction("View")
                {
                    @Override
                    public void actionPerformed(ActionEvent e)
                    {
                        int index = _certificatesTable.getSelectionModel().getMinSelectionIndex();
                        if(index != -1)
                        {
                            try
                            {
                                Certificate cert = _keyStore.getCertificate(_aliases.get(index));
                                if(cert instanceof X509Certificate)
                                {
                                    X509Certificate x509Cert = (X509Certificate)cert;
                                    CertificateDetailDialog d = new CertificateDetailDialog(x509Cert);
                                    d.showDialog();
                                }
                            }
                            catch(java.lang.Exception ex)
                            {
                                JOptionPane.showMessageDialog(KeyStorePanel.this, ex.toString(),
                                                              "Error loading certificate",
                                                              JOptionPane.ERROR_MESSAGE);
                                return;
                            }
                        }
                    }
                };
                _viewButton.setAction(viewAction);

                _removeButton = new JButton();
                AbstractAction removeAction = new AbstractAction("Remove")
                {
                    @Override
                    public void actionPerformed(ActionEvent e)
                    {
                        int index = _certificatesTable.getSelectionModel().getMinSelectionIndex();
                        if(index != -1)
                        {
                            if(JOptionPane.showConfirmDialog(KeyStorePanel.this,
                                    "Do you want to remove the certficate with alias `" + _aliases.get(index) + "'?",
                                    "Remove Certificate - IceGrid GUI",
                                    JOptionPane.YES_NO_OPTION) == JOptionPane.YES_OPTION)
                            try
                            {
                                _keyStore.deleteEntry(_aliases.get(index));
                                _keyStore.store(new FileOutputStream(_keyStorePath), new char[]{});
                                load(_keyStorePath);
                                _tableModel.fireTableStructureChanged();
                            }
                            catch(java.lang.Exception ex)
                            {
                                JOptionPane.showMessageDialog(KeyStorePanel.this, ex.toString(),
                                                              "Error removing certificate",
                                                              JOptionPane.ERROR_MESSAGE);
                                return;
                            }
                        }
                    }
                };
                _removeButton.setAction(removeAction);
            }

            setLayout(new BoxLayout(this, BoxLayout.Y_AXIS));
            add(builder.getPanel());
            JComponent buttonBar = new ButtonBarBuilder().addButton(_importButton, _viewButton, _removeButton).
                                                                                                      addGlue().build();
            buttonBar.setBorder(Borders.DIALOG);
            add(buttonBar);
        }

        public void importKeyStore(java.security.KeyStore keyStore) throws java.security.KeyStoreException,
                                                                           java.security.NoSuchAlgorithmException,
                                                                           java.security.UnrecoverableKeyException,
                                                                           java.security.cert.CertificateException,
                                                                           java.security.GeneralSecurityException,
                                                                           java.io.FileNotFoundException,
                                                                           java.io.IOException
        {
            for(Enumeration<String> aliases = keyStore.aliases(); aliases.hasMoreElements(); )
            {
                String alias = aliases.nextElement();
                if(keyStore.isKeyEntry(alias))
                {
                    RequestPasswordResult r = null;
                    Key key = null;
                    while(true)
                    {
                        try
                        {
                            r = requestPassword("Certificate Password For <" + alias + "> - IceGrid GUI",
                                                "Certificate password for <" + alias + ">:");
                            if(r.accepted)
                            {
                                key = keyStore.getKey(alias, r.password);
                            }
                            break;
                        }
                        catch(java.security.UnrecoverableKeyException ex)
                        {
                            JOptionPane.showMessageDialog(this, "Invalid certificate password",
                                                          "Invalid certificate password",
                                                          JOptionPane.ERROR_MESSAGE);
                        }
                    }
                    if(key == null)
                    {
                        continue;
                    }
                    Certificate[] chain = keyStore.getCertificateChain(alias);

                    String newAlias = alias;
                    if(newAlias == null || newAlias.length() == 0 || _keyStore.containsAlias(newAlias))
                    {
                        newAlias = JOptionPane.showInputDialog(KeyStorePanel.this,
                                                        "Certificate Alias For <" + alias + ">",
                                                        "certificate alias for <" + alias + ">:",
                                                        JOptionPane.INFORMATION_MESSAGE);
                    }

                    if(newAlias == null)
                    {
                        continue;
                    }

                    if(_keyStore.containsAlias(newAlias))
                    {
                        if(JOptionPane.showConfirmDialog(
                            KeyStorePanel.this,
                            "<html>Your KeyStore already contains a certificate with alias `" + newAlias + "'<br/>" +
                            "Do you want to update the certificate?</html>",
                            "Confirm Certificate Update - IceGrid GUI",
                            JOptionPane.YES_NO_OPTION) == JOptionPane.NO_OPTION)
                        {
                            continue;
                        }
                    }
                    _keyStore.setKeyEntry(newAlias, key, r.password, chain);
                }
                else if(keyStore.isCertificateEntry(alias))
                {
                    String newAlias = alias;
                    if(newAlias == null || newAlias.length() == 0 || _keyStore.containsAlias(newAlias))
                    {
                        newAlias = JOptionPane.showInputDialog(KeyStorePanel.this,
                                                        "Certificate Alias For <" + alias + ">",
                                                        "certificate alias for <" + alias + ">:",
                                                        JOptionPane.INFORMATION_MESSAGE);
                    }

                    if(_keyStore.containsAlias(newAlias))
                    {
                        if(JOptionPane.showConfirmDialog(
                            KeyStorePanel.this,
                            "<html>Your KeyStore already contains a certificate with alias `" + newAlias + "'<br/>" +
                            "Do you want to update the certificate?</html>",
                            "Confirm Certificate Update - IceGrid GUI",
                            JOptionPane.YES_NO_OPTION) == JOptionPane.NO_OPTION)
                        {
                            continue;
                        }
                    }

                    _keyStore.setCertificateEntry(newAlias, keyStore.getCertificate(alias));
                }
            }
            _keyStore.store(new FileOutputStream(_keyStorePath), new char[]{});
            load(_keyStorePath);
        }

        public void load(String path) throws java.io.IOException, java.security.GeneralSecurityException
        {
            _keyStorePath = path;
            FileInputStream is = null;
            if(new File(path).isFile())
            {
                is = new FileInputStream(new File(path));
            }

            _keyStore.load(is, null);
            //
            // Store the new created KeyStore
            //
            if(is == null)
            {
                _keyStore.store(new FileOutputStream(_keyStorePath), new char[]{});
            }

            _aliases = new java.util.Vector<>();
            for(Enumeration<String> e = _keyStore.aliases(); e.hasMoreElements() ;)
            {
                _aliases.add(e.nextElement());
            }
            _tableModel.fireTableStructureChanged();
        }

        public java.util.Vector<String> aliases()
        {
            return _aliases;
        }

        private JTable _certificatesTable;
        private JButton _importButton;
        private JButton _viewButton;
        private JButton _removeButton;
        private KeyStore _keyStore;
        private String _keyStorePath;
        private java.util.Vector<String> _aliases;
        private AbstractTableModel _tableModel;
    }

    //
    // X509 Certificate panel factories
    //
    public static JPanel getSubjectPanel(X509Certificate cert) throws javax.naming.InvalidNameException
    {
        java.util.HashMap< String, Object> details = new java.util.HashMap<>();
        LdapName dn = new LdapName(cert.getSubjectX500Principal().getName());
        for(Rdn rdn: dn.getRdns())
        {
            details.put(rdn.getType().toUpperCase(), rdn.getValue());
        }

        FormLayout layout = new FormLayout("right:pref, 2dlu, left:pref:grow", "pref");
        DefaultFormBuilder builder = new DefaultFormBuilder(layout);
        builder.border(Borders.DIALOG);
        builder.rowGroupingEnabled(true);
        builder.lineGapSize(LayoutStyle.getCurrent().getLinePad());
        builder.addSeparator("Subject");
        builder.nextLine();

        if(details.get("CN") != null)
        {
            builder.append(new JLabel("<html><b>Common Name (CN):</b></html>"),
                           new JLabel(details.get("CN").toString()));
        }
        else
        {
            builder.append(new JLabel("<html><b>Common Name (CN):</b></html>"));
        }

        if(details.get("O") != null)
        {
            builder.append(new JLabel("<html><b>Organization (O):</b></html>"),
                           new JLabel(details.get("O").toString()));
        }
        else
        {
            builder.append(new JLabel("<html><b>Organization (O):</b></html>"));
        }

        if(details.get("OU") != null)
        {
            builder.append(new JLabel("<html><b>Organization Unit (OU):</b></html>"),
                           new JLabel(details.get("OU").toString()));
        }
        else
        {
            builder.append(new JLabel("<html><b>Organization Unit (OU):</b></html>"));
        }
        builder.append(new JLabel("<html><b>Serial Number:</b></html>"),
                       new JLabel(cert.getSerialNumber().toString()));

        return builder.getPanel();
    }

    public static JPanel getIssuerPanel(X509Certificate cert) throws javax.naming.InvalidNameException
    {
        java.util.HashMap< String, Object> details = new java.util.HashMap<>();

        LdapName dn = new LdapName(cert.getIssuerX500Principal().getName());
        for(Rdn rdn: dn.getRdns())
        {
            details.put(rdn.getType().toUpperCase(), rdn.getValue());
        }

        FormLayout layout = new FormLayout("right:pref, 2dlu, left:pref:grow", "pref");
        DefaultFormBuilder builder = new DefaultFormBuilder(layout);
        builder.border(Borders.DIALOG);
        builder.rowGroupingEnabled(true);
        builder.lineGapSize(LayoutStyle.getCurrent().getLinePad());
        builder.addSeparator("Issuer");
        builder.nextLine();
        if(details.get("CN") != null)
        {
            builder.append(new JLabel("<html><b>Common Name (CN):</b></html>"),
                           new JLabel(details.get("CN").toString()));
        }
        else
        {
            builder.append(new JLabel("<html><b>Common Name (CN):</b></html>"));
        }

        if(details.get("O") != null)
        {
            builder.append(new JLabel("<html><b>Organization (O):</b></html>"),
                           new JLabel(details.get("O").toString()));
        }
        else
        {
            builder.append(new JLabel("<html><b>Organization (O):</b></html>"));
        }

        if(details.get("OU") != null)
        {
            builder.append(new JLabel("<html><b>Organization Unit (OU):</b></html>"),
                           new JLabel(details.get("OU").toString()));
        }
        else
        {
            builder.append(new JLabel("<html><b>Organization Unit (OU):</b></html>"));
        }

        return builder.getPanel();
    }

    public static JPanel getValidityPanel(X509Certificate cert)
    {
        FormLayout layout = new FormLayout("right:pref, 2dlu, left:pref:grow", "pref");
        DefaultFormBuilder builder = new DefaultFormBuilder(layout);
        builder.border(Borders.DIALOG);
        builder.rowGroupingEnabled(true);
        builder.lineGapSize(LayoutStyle.getCurrent().getLinePad());
        builder.addSeparator("Validity");
        builder.nextLine();
        builder.append(new JLabel("<html><b>Issued On:</b></html>"),
                        new JLabel(cert.getNotBefore().toString()));
        builder.append(new JLabel("<html><b>Expires On:</b></html>"),
                        new JLabel(cert.getNotAfter().toString()));
        builder.nextLine();

        return builder.getPanel();
    }

    public static JPanel getFingerprintPanel(X509Certificate cert) throws java.security.GeneralSecurityException
    {
        FormLayout layout = new FormLayout("right:pref, 2dlu, left:pref:grow", "pref");
        DefaultFormBuilder builder = new DefaultFormBuilder(layout);
        builder.border(Borders.DIALOG);
        builder.rowGroupingEnabled(true);
        builder.lineGapSize(LayoutStyle.getCurrent().getLinePad());
        builder.addSeparator("Fingerprints");
        builder.nextLine();

        String sha1Fingerprint = "";
        {
            MessageDigest md = MessageDigest.getInstance("SHA-1");
            byte[] der = cert.getEncoded();
            md.update(der);
            byte[] digest = md.digest();

            StringBuilder sb = new StringBuilder(digest.length * 2);

            Formatter formatter = new Formatter(sb);
            try
            {
                for(int i = 0; i < digest.length;)
                {
                    formatter.format("%02x", digest[i]);
                    i++;
                    if(i < digest.length)
                    {
                        sb.append(":");
                    }
                }
            }
            finally
            {
                formatter.close();
            }
            sha1Fingerprint = sb.toString().toUpperCase();
        }

        builder.append(new JLabel("<html><b>SHA-1 Fingerprint:</b></html>"),
                        new JLabel(sha1Fingerprint));
        builder.nextLine();

        String md5Fingerprint = "";
        {
            MessageDigest md = MessageDigest.getInstance("MD5");
            byte[] der = cert.getEncoded();
            md.update(der);
            byte[] digest = md.digest();

            StringBuilder sb = new StringBuilder(digest.length * 2);

            Formatter formatter = new Formatter(sb);
            try
            {
                for(int i = 0; i < digest.length;)
                {
                    formatter.format("%02x", digest[i]);
                    i++;
                    if(i < digest.length)
                    {
                        sb.append(":");
                    }
                }
            }
            finally
            {
                formatter.close();
            }
            md5Fingerprint = sb.toString().toUpperCase();
        }
        builder.append(new JLabel("<html><b>MD5 Fingerprint:</b></html>"), new JLabel(md5Fingerprint));
        builder.nextLine();

        return builder.getPanel();
    }

    public static JPanel getSubjectAlternativeNamesPanel(X509Certificate cert)
        throws java.security.cert.CertificateParsingException
    {
        FormLayout layout = new FormLayout("right:pref, 2dlu, left:pref:grow", "pref");
        DefaultFormBuilder builder = new DefaultFormBuilder(layout);
        builder.border(Borders.DIALOG);
        builder.rowGroupingEnabled(true);
        builder.lineGapSize(LayoutStyle.getCurrent().getLinePad());
        builder.addSeparator("Subject Alternate Names");
        builder.nextLine();

        Collection<List<?>> altNames = cert.getSubjectAlternativeNames();
        if(altNames != null)
        {
            for(List<?> l : altNames)
            {
                Integer kind = (Integer)l.get(0);
                String value = l.get(1).toString();
                if(kind == 2)
                {
                    builder.append(new JLabel("<html><b>DNS Name:</b></html>"), new JLabel(value));
                    builder.nextLine();
                }
                else if(kind == 7)
                {
                    builder.append(new JLabel("<html><b>IP Address:</b></html>"), new JLabel(value));
                    builder.nextLine();
                }
            }
        }
        return builder.getPanel();
    }

    public class CertificateDetailDialog extends JDialog
    {
        CertificateDetailDialog(X509Certificate cert)
            throws java.security.GeneralSecurityException, java.io.IOException, javax.naming.InvalidNameException
        {
            super(_coordinator.getMainFrame(), "Certificate Details - IceGrid GUI", true);
            setDefaultCloseOperation(WindowConstants.DISPOSE_ON_CLOSE);

            Container contentPane = getContentPane();
            contentPane.setLayout(new BoxLayout(contentPane, BoxLayout.Y_AXIS));
            contentPane.add(getSubjectPanel(cert));
            contentPane.add(getSubjectAlternativeNamesPanel(cert));
            contentPane.add(getIssuerPanel(cert));
            contentPane.add(getValidityPanel(cert));
            contentPane.add(getFingerprintPanel(cert));

            JButton closeButton = new JButton("Close");
            closeButton.addActionListener(new ActionListener()
                {
                    @Override
                    public void actionPerformed(ActionEvent e)
                    {
                        dispose();
                    }
                });
            JComponent buttonBar = new ButtonBarBuilder().addGlue().addButton(closeButton).build();
            buttonBar.setBorder(Borders.DIALOG);
            contentPane.add(buttonBar);

            pack();
            setResizable(false);
        }

        void showDialog()
        {
            if(isVisible() == false)
            {
                setLocationRelativeTo(_coordinator.getMainFrame());
                setVisible(true);
            }
        }
    }

    public class CertificateManagerDialog extends JDialog
    {
        CertificateManagerDialog() throws java.security.KeyStoreException
        {
            super(_coordinator.getMainFrame(), "Certificate Manager - IceGrid GUI", true);
            _tabs = new JTabbedPane();

            _identityCertificatesPanel = new KeyStorePanel();
            _tabs.addTab("My Certificates", _identityCertificatesPanel);

            _serverCertificatesPanel = new KeyStorePanel();
            _tabs.addTab("Server Certificates", _serverCertificatesPanel);

            _authorityCertificatesPanel = new KeyStorePanel();
            _tabs.addTab("Trusted CAs", _authorityCertificatesPanel);

            Container contentPane = getContentPane();
            contentPane.setLayout(new BoxLayout(contentPane, BoxLayout.Y_AXIS));
            contentPane.add(_tabs);

            JButton closeButton = new JButton("Close");
            closeButton.addActionListener(new ActionListener()
                {
                    @Override
                    public void actionPerformed(ActionEvent e)
                    {
                        setVisible(false);
                    }
                });
            getRootPane().setDefaultButton(closeButton);

            JComponent buttonBar = new ButtonBarBuilder().addGlue().addButton(closeButton).build();
            buttonBar.setBorder(Borders.DIALOG);
            contentPane.add(buttonBar);
            setResizable(false);
            pack();
        }

        boolean load()
        {
            try
            {
                _identityCertificatesPanel.load(_coordinator.getDataDirectory() + File.separator + "MyCerts.jks");
                _serverCertificatesPanel.load(_coordinator.getDataDirectory() + File.separator + "ServerCerts.jks");
                _authorityCertificatesPanel.load(_coordinator.getDataDirectory() + File.separator +
                                                 "AuthorityCerts.jks");
            }
            catch(java.lang.Exception ex)
            {
                JOptionPane.showMessageDialog(this, ex.toString(), "Error loading keystore", JOptionPane.ERROR_MESSAGE);
                return false;
            }
            return true;
        }

        java.util.Vector<String> myCertsAliases()
        {
            return _identityCertificatesPanel.aliases();
        }

        void setActiveTab(int index)
        {
            _tabs.setSelectedIndex(index);
        }

        void showDialog()
        {
            if(isVisible() == false)
            {
                setLocationRelativeTo(_coordinator.getMainFrame());
                setVisible(true);
            }
        }

        private JTabbedPane _tabs;
        private KeyStorePanel _identityCertificatesPanel;
        private KeyStorePanel _serverCertificatesPanel;
        private KeyStorePanel _authorityCertificatesPanel;
    }

    SessionKeeper(Coordinator coordinator)
    {
        _coordinator = coordinator;
    }

    void connectionManager()
    {
        if(_connectionManagerDialog == null)
        {
            _connectionManagerDialog = new ConnectionManagerDialog();
            Utils.addEscapeListener(_connectionManagerDialog);
        }
        _connectionManagerDialog.showDialog();
    }

    public CertificateManagerDialog certificateManager(Component parent)
    {
        if(_certificateManagerDialog == null)
        {
            try
            {
                _certificateManagerDialog = new CertificateManagerDialog();
                Utils.addEscapeListener(_certificateManagerDialog);
            }
            catch(java.security.KeyStoreException ex)
            {
                JOptionPane.showMessageDialog(parent, ex.toString(),
                                              "Failed to load certificate manager dialog",
                                              JOptionPane.ERROR_MESSAGE);
                return null;
            }

            if(!_certificateManagerDialog.load())
            {
                _certificateManagerDialog.dispose();
                _certificateManagerDialog = null;
            }
        }
        return _certificateManagerDialog;
    }

    class AuthDialog extends JDialog
    {
        public AuthDialog(JDialog parent, String title)
        {
            super(parent, title, true);
            setDefaultCloseOperation(WindowConstants.DISPOSE_ON_CLOSE);
        }

        public void showDialog()
        {
            setLocationRelativeTo(getOwner());
            setVisible(true);
        }
    }

    class StoredPasswordAuthDialog extends AuthDialog
    {
        StoredPasswordAuthDialog(JDialog parent)
        {
            super(parent, "Login - IceGrid GUI");

            Container contentPane = getContentPane();
            contentPane.setLayout(new BoxLayout(contentPane, BoxLayout.Y_AXIS));

            FormLayout layout = new FormLayout("pref:grow", "");
            DefaultFormBuilder builder = new DefaultFormBuilder(layout);
            builder.border(Borders.DIALOG);
            builder.append(new JLabel("Connecting please wait"));
            contentPane.add(builder.getPanel());
            pack();
            setResizable(false);
        }
    }

    private void login(final JDialog parent, final ConnectionInfo info)
    {
        if(_authDialog != null)
        {
            _authDialog.dispose();
            _authDialog = null;
        }

        if(info.getAuth() == SessionKeeper.AuthType.UsernamePasswordAuthType)
        {
            class UsernamePasswordAuthDialog extends AuthDialog
            {
                UsernamePasswordAuthDialog()
                {
                    super(parent, "Login - IceGrid GUI");

                    Container contentPane = getContentPane();
                    contentPane.setLayout(new BoxLayout(contentPane, BoxLayout.Y_AXIS));

                    {
                        // Build the basic login panel.
                        FormLayout layout = new FormLayout("pref, 2dlu, pref:grow, 2dlu, pref", "");
                        DefaultFormBuilder builder = new DefaultFormBuilder(layout);
                        builder.border(Borders.DIALOG);

                        if(info.getPassword() == null || info.getPassword().length == 0)
                        {
                            _username = new JTextField(20);
                            _username.setText(info.getUsername());
                            _username.setEditable(false);
                            builder.append(new JLabel("Username"), _username);
                            builder.nextLine();
                            _password = new JPasswordField(20);
                            builder.append(new JLabel("Password"), _password);
                            builder.nextLine();
                            _storePassword = new JCheckBox("Save Password.");
                            _storePassword.setEnabled(_password.getPassword() != null &&
                                                      _password.getPassword().length > 0);
                            _password.getDocument().addDocumentListener(new DocumentListener()
                                {
                                    @Override
                                    public void changedUpdate(DocumentEvent e)
                                    {
                                        _storePassword.setEnabled(_password.getPassword() != null &&
                                                                  _password.getPassword().length > 0);
                                    }

                                    @Override
                                    public void removeUpdate(DocumentEvent e)
                                    {
                                        _storePassword.setEnabled(_password.getPassword() != null &&
                                                                  _password.getPassword().length > 0);
                                    }

                                    @Override
                                    public void insertUpdate(DocumentEvent e)
                                    {
                                        _storePassword.setEnabled(_password.getPassword() != null &&
                                                                  _password.getPassword().length > 0);
                                    }
                                });
                            builder.append("", _storePassword);
                            builder.nextLine();
                        }

                        if(info.getUseX509Certificate() && (info.getKeyPassword() == null ||
                                                            info.getKeyPassword().length == 0))
                        {
                            _keyAlias = new JTextField(20);
                            _keyAlias.setText(info.getAlias());
                            _keyAlias.setEditable(false);
                            builder.append(new JLabel("Key Alias"), _keyAlias);
                            builder.nextLine();
                            _keyPassword = new JPasswordField(20);
                            builder.append(new JLabel("Key Password"), _keyPassword);
                            builder.nextLine();
                            _storeKeyPassword = new JCheckBox("Save Key Password.");
                            _storeKeyPassword.setEnabled(_keyPassword.getPassword() != null &&
                                                         _keyPassword.getPassword().length > 0);
                            _keyPassword.getDocument().addDocumentListener(new DocumentListener()
                                {
                                    @Override
                                    public void changedUpdate(DocumentEvent e)
                                    {
                                        _storeKeyPassword.setEnabled(_keyPassword.getPassword() != null &&
                                                                     _keyPassword.getPassword().length > 0);
                                    }

                                    @Override
                                    public void removeUpdate(DocumentEvent e)
                                    {
                                        _storeKeyPassword.setEnabled(_keyPassword.getPassword() != null &&
                                                                     _keyPassword.getPassword().length > 0);
                                    }

                                    @Override
                                    public void insertUpdate(DocumentEvent e)
                                    {
                                        _storeKeyPassword.setEnabled(_keyPassword.getPassword() != null &&
                                                                     _keyPassword.getPassword().length > 0);
                                    }
                                });
                            builder.append("", _storeKeyPassword);
                            builder.nextLine();
                        }

                        contentPane.add(builder.getPanel());
                    }

                    JButton okButton = new JButton();
                    JButton cancelButton = new JButton();

                    AbstractAction okAction = new AbstractAction("OK")
                        {
                            @Override
                            public void actionPerformed(ActionEvent e)
                            {
                                if(_session != null)
                                {
                                    logout(true);
                                }
                                assert _session == null;
                                if(_password != null)
                                {
                                    info.setPassword(_password.getPassword());
                                    info.setStorePassword(_storePassword.isSelected());
                                }
                                if(_keyPassword != null)
                                {
                                    info.setKeyPassword(_keyPassword.getPassword());
                                    info.setStoreKeyPassword(_storeKeyPassword.isSelected());
                                }

                                if(checkCertificateRequirePassword(info.getAlias()) &&
                                   !checkCertificatePassword(info.getAlias(), info.getKeyPassword()))
                                {
                                    dispose();
                                    permissionDenied(parent, info, "Invalid certificate password");
                                }
                                else
                                {
                                    _authDialog.setCursor(Cursor.getPredefinedCursor(Cursor.WAIT_CURSOR));
                                    _authDialog.setDefaultCloseOperation(WindowConstants.DO_NOTHING_ON_CLOSE);
                                    Utils.removeEscapeListener(_authDialog);
                                    okButton.setEnabled(false);
                                    cancelButton.setEnabled(false);
                                    _coordinator.login(SessionKeeper.this, info,parent);
                                }
                            }
                        };
                    okButton.setAction(okAction);

                    AbstractAction cancelAction = new AbstractAction("Cancel")
                        {
                            @Override
                            public void actionPerformed(ActionEvent e)
                            {
                                dispose();
                            }
                        };
                    cancelButton.setAction(cancelAction);

                    JComponent buttonBar = new ButtonBarBuilder().addGlue().addButton(okButton, cancelButton).
                                                                                                      addGlue().build();
                    buttonBar.setBorder(Borders.DIALOG);
                    contentPane.add(buttonBar);

                    getRootPane().setDefaultButton(okButton);
                    pack();
                    setResizable(false);
                }

                private JTextField _username;
                private JPasswordField _password;
                private JCheckBox _storePassword;
                private JTextField _keyAlias;
                private JPasswordField _keyPassword;
                private JCheckBox _storeKeyPassword;
            }

            //
            // If there isn't a store password or the certificate requires a password
            // and the password isn't provided, we show the login dialog.
            //
            if((info.getPassword() == null || info.getPassword().length == 0) ||
               (info.getUseX509Certificate() && checkCertificateRequirePassword(info.getAlias()) &&
                (info.getKeyPassword() == null || info.getKeyPassword().length == 0)))
            {
                _authDialog = new UsernamePasswordAuthDialog();
                Utils.addEscapeListener(_authDialog);
                _authDialog.showDialog();
            }
            else
            {
                if(_session != null)
                {
                    logout(true);
                }
                assert _session == null;

                if(info.getUseX509Certificate() && checkCertificateRequirePassword(info.getAlias()) &&
                   !checkCertificatePassword(info.getAlias(), info.getKeyPassword()))
                {
                    permissionDenied(parent, info, "Invalid certificate password");
                }
                else
                {
                    _authDialog = new StoredPasswordAuthDialog(parent);
                    _authDialog.setCursor(Cursor.getPredefinedCursor(Cursor.WAIT_CURSOR));
                    _authDialog.setDefaultCloseOperation(WindowConstants.DO_NOTHING_ON_CLOSE);
                    _coordinator.login(SessionKeeper.this, info, parent);
                    _authDialog.showDialog();
                }
            }
        }
        else // X509CertificateAuthDialog dialog
        {
            class X509CertificateAuthDialog extends AuthDialog
            {
                X509CertificateAuthDialog()
                {
                    super(parent, "Login - IceGrid GUI");

                    Container contentPane = getContentPane();
                    contentPane.setLayout(new BoxLayout(contentPane, BoxLayout.Y_AXIS));

                    {
                        // Build the basic login panel.
                        FormLayout layout = new FormLayout("pref, 2dlu, pref:grow, 2dlu, pref", "");
                        DefaultFormBuilder builder = new DefaultFormBuilder(layout);
                        builder.border(Borders.DIALOG);

                        builder.append(new JLabel("Key Password"), _keyPassword);
                        builder.nextLine();
                        _storeKeyPassword = new JCheckBox("Save Key Password.");
                        _storeKeyPassword.setEnabled(false);
                        _keyPassword.getDocument().addDocumentListener(new DocumentListener()
                            {
                                @Override
                                public void changedUpdate(DocumentEvent e)
                                {
                                    _storeKeyPassword.setEnabled(_keyPassword.getPassword() != null &&
                                                                 _keyPassword.getPassword().length > 0);
                                }

                                @Override
                                public void removeUpdate(DocumentEvent e)
                                {
                                    _storeKeyPassword.setEnabled(_keyPassword.getPassword() != null &&
                                                                 _keyPassword.getPassword().length > 0);
                                }

                                @Override
                                public void insertUpdate(DocumentEvent e)
                                {
                                    _storeKeyPassword.setEnabled(_keyPassword.getPassword() != null &&
                                                                 _keyPassword.getPassword().length > 0);
                                }
                            });
                        builder.append("", _storeKeyPassword);
                        builder.nextLine();
                        contentPane.add(builder.getPanel());
                    }

                    JButton okButton = new JButton();
                    JButton cancelButton = new JButton();

                    AbstractAction okAction = new AbstractAction("OK")
                        {
                            @Override
                            public void actionPerformed(ActionEvent e)
                            {
                                if(_session != null)
                                {
                                    logout(true);
                                }
                                assert _session == null;

                                info.setKeyPassword(_keyPassword.getPassword());

                                if(checkCertificateRequirePassword(info.getAlias()) &&
                                   !checkCertificatePassword(info.getAlias(), info.getKeyPassword()))
                                {
                                    dispose();
                                    permissionDenied(parent, info, "Invalid certificate password");
                                }
                                else
                                {
                                    _authDialog.setCursor(Cursor.getPredefinedCursor(Cursor.WAIT_CURSOR));
                                    _authDialog.setDefaultCloseOperation(WindowConstants.DO_NOTHING_ON_CLOSE);
                                    Utils.removeEscapeListener(_authDialog);
                                    okButton.setEnabled(false);
                                    cancelButton.setEnabled(false);
                                    _coordinator.login(SessionKeeper.this, info, parent);
                                }
                            }
                        };
                    okButton.setAction(okAction);

                    AbstractAction cancelAction = new AbstractAction("Cancel")
                        {
                            @Override
                            public void actionPerformed(ActionEvent e)
                            {
                                dispose();
                            }
                        };
                    cancelButton.setAction(cancelAction);

                    JComponent buttonBar = new ButtonBarBuilder().addGlue().addButton(okButton, cancelButton).
                        addGlue().build();
                    buttonBar.setBorder(Borders.DIALOG);
                    contentPane.add(buttonBar);

                    getRootPane().setDefaultButton(okButton);
                    pack();
                    setResizable(false);
                }

                private JPasswordField _keyPassword = new JPasswordField(20);
                private JCheckBox _storeKeyPassword;
            }

            //
            // If the certificate requires a password and the password isn't provided, we
            // show the login dialog.
            //
            if((info.getKeyPassword() == null || info.getKeyPassword().length == 0) &&
                checkCertificateRequirePassword(info.getAlias()))
            {
                _authDialog = new X509CertificateAuthDialog();
                Utils.addEscapeListener(_authDialog);
                _authDialog.showDialog();
            }
            else
            {
                if(_session != null)
                {
                    logout(true);
                }
                assert _session == null;

                if(checkCertificateRequirePassword(info.getAlias()) &&
                   !checkCertificatePassword(info.getAlias(), info.getKeyPassword()))
                {
                    permissionDenied(parent, info, "Invalid certificate password");
                }
                else
                {
                    _authDialog = new StoredPasswordAuthDialog(parent);
                    _authDialog.setCursor(Cursor.getPredefinedCursor(Cursor.WAIT_CURSOR));
                    _authDialog.setDefaultCloseOperation(WindowConstants.DO_NOTHING_ON_CLOSE);
                    _coordinator.login(SessionKeeper.this, info, parent);
                    _authDialog.showDialog();
                }
            }
        }
    }

    public void loginSuccess(final JDialog parent, final long sessionTimeout, final int acmTimeout,
                             final AdminSessionPrx adminSession, final ConnectionInfo info)
    {
        try
        {
            if(!info.getStorePassword())
            {
                info.setPassword(null);
            }
            if(!info.getStoreKeyPassword())
            {
                info.setKeyPassword(null);
            }
            info.save();
            _connectionManagerDialog.load();
        }
        catch(java.util.prefs.BackingStoreException ex)
        {
            JOptionPane.showMessageDialog(
                    parent,
                    ex.toString(),
                    "Error saving connection",
                    JOptionPane.ERROR_MESSAGE);
        }

        assert adminSession != null;
        try
        {
            _replicaName = adminSession.getReplicaName();
        }
        catch(com.zeroc.Ice.LocalException e)
        {
            logout(true);
            JOptionPane.showMessageDialog(
                parent,
                "Could not retrieve replica name: " + e.toString(),
                "Login failed",
                JOptionPane.ERROR_MESSAGE);
            return;
        }

        _coordinator.setConnected(true);

        _connectedToMaster = _replicaName.equals("Master");
        if(_connectedToMaster)
        {
            _coordinator.getStatusBar().setText("Logged into Master Registry");
        }
        else
        {
            _coordinator.getStatusBar().setText("Logged into Slave Registry '" + _replicaName + "'");
        }

        //
        // Create the session in its own thread as it made remote calls
        //
        _coordinator.getExecutor().submit(() ->
            {
                try
                {
                    final Session session = new Session(adminSession, sessionTimeout, acmTimeout, !info.getDirect(), parent);
                    SwingUtilities.invokeAndWait(() ->
                                                 {
                                                     _session = session;
                                                 });
                    try
                    {
                        session.registerObservers();
                    }
                    catch(final com.zeroc.Ice.LocalException e)
                    {
                        while(true)
                        {
                            try
                            {
                                SwingUtilities.invokeAndWait(() ->
                                                             {
                                                                 logout(true);
                                                                 JOptionPane.showMessageDialog(parent, "Could not register observers: " + e.toString(),
                                                                                               "Login failed", JOptionPane.ERROR_MESSAGE);
                                                             });
                                break;
                            }
                            catch(java.lang.InterruptedException ex)
                            {
                                // Ignore and retry
                            }
                            catch(java.lang.reflect.InvocationTargetException ex)
                            {
                                break;
                            }
                        }
                    }
                }
                catch(java.lang.Throwable e)
                {
                    return;
                }

                SwingUtilities.invokeLater(() ->
                    {
                        if(_authDialog != null)
                        {
                            _authDialog.dispose();
                            _authDialog = null;
                        }
                        _connectionManagerDialog.setConnectionWizard(null);
                        _connectionManagerDialog.setVisible(false);
                    });
            });
    }

    public void loginFailed()
    {
        if(_authDialog != null)
        {
            _authDialog.dispose();
            _authDialog = null;
        }
    }

    public void permissionDenied(final JDialog parent, final ConnectionInfo info, final String msg)
    {
        if(_authDialog != null)
        {
            _authDialog.dispose();
            _authDialog = null;
        }

        class PermissionDeniedAuthDialog extends AuthDialog
        {
            PermissionDeniedAuthDialog()
            {
                super(parent, "Login - IceGrid GUI");

                Container contentPane = getContentPane();
                contentPane.setLayout(new BoxLayout(contentPane, BoxLayout.Y_AXIS));
                {
                    // Build the basic login panel.
                    FormLayout layout = new FormLayout("pref, 2dlu, pref:grow, 2dlu, pref", "");
                    DefaultFormBuilder builder = new DefaultFormBuilder(layout);
                    builder.border(Borders.DIALOG);

                    if(info.getAuth() == AuthType.UsernamePasswordAuthType)
                    {
                        _username = new JTextField(20);
                        _username.setText(info.getUsername());
                        _username.setEditable(false);
                        builder.append(new JLabel("Username"), _username);
                        builder.nextLine();
                        _password = new JPasswordField(20);
                        if(info.getPassword() != null)
                        {
                            _password.setText(new String(info.getPassword()));
                        }
                        builder.append(new JLabel("Password"), _password);
                        builder.nextLine();
                        _storePassword = new JCheckBox("Save Password.");
                        _storePassword.setSelected(info.getStorePassword());
                        _storePassword.setEnabled(_password.getPassword() != null &&
                                                  _password.getPassword().length > 0);
                        _password.getDocument().addDocumentListener(new DocumentListener()
                            {
                                @Override
                                public void changedUpdate(DocumentEvent e)
                                {
                                    _storePassword.setEnabled(_password.getPassword() != null &&
                                                              _password.getPassword().length > 0);
                                }

                                @Override
                                public void removeUpdate(DocumentEvent e)
                                {
                                    _storePassword.setEnabled(_password.getPassword() != null &&
                                                              _password.getPassword().length > 0);
                                }

                                @Override
                                public void insertUpdate(DocumentEvent e)
                                {
                                    _storePassword.setEnabled(_password.getPassword() != null &&
                                                              _password.getPassword().length > 0);
                                }
                            });
                        builder.append("", _storePassword);
                        builder.nextLine();
                    }

                    if(info.getUseX509Certificate())
                    {
                        _keyAlias = new JTextField(20);
                        _keyAlias.setText(info.getAlias());
                        _keyAlias.setEditable(false);
                        builder.append(new JLabel("Key Alias"), _keyAlias);
                        builder.nextLine();
                        _keyPassword = new JPasswordField(20);
                        if(info.getKeyPassword() != null)
                        {
                            _keyPassword.setText(new String(info.getKeyPassword()));
                        }
                        builder.append(new JLabel("Key Password"), _keyPassword);
                        builder.nextLine();
                        _storeKeyPassword = new JCheckBox("Save Key Password.");
                        _storeKeyPassword.setSelected(info.getStoreKeyPassword());
                        _storeKeyPassword.setEnabled(_keyPassword.getPassword() != null &&
                                                     _keyPassword.getPassword().length > 0);
                        _keyPassword.getDocument().addDocumentListener(new DocumentListener()
                            {
                                @Override
                                public void changedUpdate(DocumentEvent e)
                                {
                                    _storeKeyPassword.setEnabled(_keyPassword.getPassword() != null &&
                                                                 _keyPassword.getPassword().length > 0);
                                }

                                @Override
                                public void removeUpdate(DocumentEvent e)
                                {
                                    _storeKeyPassword.setEnabled(_keyPassword.getPassword() != null &&
                                                                 _keyPassword.getPassword().length > 0);
                                }

                                @Override
                                public void insertUpdate(DocumentEvent e)
                                {
                                    _storeKeyPassword.setEnabled(_keyPassword.getPassword() != null &&
                                                                 _keyPassword.getPassword().length > 0);
                                }
                            });
                        builder.append("", _storeKeyPassword);
                        builder.nextLine();
                    }

                    contentPane.add(builder.getPanel());
                }

                JButton okButton = new JButton();
                JButton editConnectionButton = new JButton();
                JButton cancelButton = new JButton();

                AbstractAction okAction = new AbstractAction("OK")
                    {
                        @Override
                        public void actionPerformed(ActionEvent e)
                        {
                            if(_session != null)
                            {
                                logout(true);
                            }
                            assert _session == null;
                            if(_password != null)
                            {
                                info.setPassword(_password.getPassword());
                                info.setStorePassword(_storePassword.isSelected());
                            }

                            boolean certificatePasswordMatch = true;
                            if(_keyPassword != null)
                            {
                                info.setKeyPassword(_keyPassword.getPassword());
                                info.setStoreKeyPassword(_storeKeyPassword.isSelected());
                                certificatePasswordMatch = checkCertificatePassword(info.getAlias(),
                                                                                    info.getKeyPassword());
                            }

                            if(!certificatePasswordMatch)
                            {
                                dispose();
                                permissionDenied(parent, info, "Invalid certificate password");
                            }
                            else
                            {
                                _authDialog.setCursor(Cursor.getPredefinedCursor(Cursor.WAIT_CURSOR));
                                okButton.setEnabled(false);
                                editConnectionButton.setEnabled(false);
                                cancelButton.setEnabled(false);
                                Utils.removeEscapeListener(_authDialog);
                                _authDialog.setDefaultCloseOperation(WindowConstants.DO_NOTHING_ON_CLOSE);
                                _coordinator.login(SessionKeeper.this, info, parent);
                            }
                        }
                    };
                okButton.setAction(okAction);

                AbstractAction editConnectionAction = new AbstractAction("Edit Connection")
                    {
                        @Override
                        public void actionPerformed(ActionEvent e)
                        {
                            info.load();
                            dispose();
                            _authDialog = null;
                            JDialog dialog = new ConnectionWizardDialog(info, parent);
                            Utils.addEscapeListener(dialog);
                            dialog.setLocationRelativeTo(parent);
                            dialog.setVisible(true);
                        }
                    };
                editConnectionButton.setAction(editConnectionAction);

                AbstractAction cancelAction = new AbstractAction("Cancel")
                    {
                        @Override
                        public void actionPerformed(ActionEvent e)
                        {
                            info.load();
                            dispose();
                            _authDialog = null;
                        }
                    };
                cancelButton.setAction(cancelAction);

                JComponent buttonBar = new ButtonBarBuilder().addGlue().addButton(okButton, editConnectionButton,
                    cancelButton).addGlue().build();
                buttonBar.setBorder(Borders.DIALOG);
                contentPane.add(buttonBar);

                getRootPane().setDefaultButton(okButton);
                pack();
                setResizable(false);
            }

            private JTextField _username;
            private JPasswordField _password;
            private JCheckBox _storePassword;
            private JTextField _keyAlias;
            private JPasswordField _keyPassword;
            private JCheckBox _storeKeyPassword;
        }

        JOptionPane.showMessageDialog(parent, "Permission denied: " + msg, "Login failed",
                                      JOptionPane.ERROR_MESSAGE);

        _authDialog = new PermissionDeniedAuthDialog();
        Utils.addEscapeListener(_authDialog);
        _authDialog.showDialog();
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

    String getServerAdminCategory()
    {
        return _session == null ? null : _session.getServerAdminCategory();
    }

    com.zeroc.Ice.ObjectPrx addCallback(com.zeroc.Ice.Object servant, String name, String facet)
    {
        return _session == null ? null : _session.addCallback(servant, name, facet);
    }

    com.zeroc.Ice.ObjectPrx retrieveCallback(String name, String facet)
    {
        return _session == null ? null : _session.retrieveCallback(name, facet);
    }

    com.zeroc.Ice.Object removeCallback(String name, String facet)
    {
        return _session == null ? null : _session.removeCallback(name, facet);
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

    private ConnectionManagerDialog _connectionManagerDialog;
    private CertificateManagerDialog _certificateManagerDialog;
    private static AuthDialog _authDialog;

    private final Coordinator _coordinator;
    private Plugin _discoveryPlugin;

    private Session _session;
    private boolean _connectedToMaster = false;
    private String _replicaName = "";

    private static final int ICEGRID_TCP_PORT = 4061;
    private static final int ICEGRID_SSL_PORT = 4062;
    private static final int GLACIER2_TCP_PORT = 4063;
    private static final int GLACIER2_SSL_PORT = 4064;
}
