//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.IceGridGUI;

import java.lang.reflect.Constructor;
import java.net.URI;
import java.util.concurrent.Executors;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.ScheduledThreadPoolExecutor;
import java.util.prefs.Preferences;
import java.util.prefs.BackingStoreException;
import java.util.Enumeration;
import java.util.Collection;
import java.awt.*;
import java.awt.event.*;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;

import java.nio.file.Paths;
import java.nio.file.Files;

import javax.swing.*;
import javax.swing.border.EmptyBorder;
import javax.swing.filechooser.FileFilter;
import javax.swing.text.JTextComponent;

import com.jgoodies.looks.Options;
import com.jgoodies.looks.HeaderStyle;
import com.jgoodies.looks.BorderStyle;
import com.jgoodies.looks.plastic.PlasticLookAndFeel;
import com.jgoodies.forms.builder.DefaultFormBuilder;
import com.jgoodies.forms.builder.ButtonBarBuilder;
import com.jgoodies.forms.factories.Borders;
import com.jgoodies.forms.layout.FormLayout;
import com.jgoodies.forms.util.LayoutStyle;

import java.security.KeyStore;
import java.security.cert.Certificate;
import java.security.cert.X509Certificate;

import com.zeroc.Ice.LocatorFinderPrx;
import com.zeroc.IceGrid.*;

import javax.naming.ldap.LdapName;
import javax.naming.ldap.Rdn;

import com.zeroc.IceGridGUI.LiveDeployment.MetricsViewEditor.MetricsViewTransferableData;

//
// This class coordinates the communications between the various objects
// that make up the IceGrid GUI.
// It also provides both the menu bar and tool bar.
//
public class Coordinator
{
    public interface IGraphView
    {
        public void close();
        public void addSeries(final MetricsViewTransferableData data);
        public String getTitle();
    }

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

        @Override
        public void setText(String text)
        {
            _text.setText(text);
        }

        @Override
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

    private class ActionWrapper extends AbstractAction implements java.beans.PropertyChangeListener
    {
        public ActionWrapper(String name)
        {
            super(name);
            setEnabled(false);
        }

        @Override
        public void actionPerformed(ActionEvent e)
        {
            if(_target != null)
            {
                _target.actionPerformed(e);
            }
        }

        @Override
        public void propertyChange(java.beans.PropertyChangeEvent e)
        {
            //
            // The only property we're interested in is isEnabled
            //

            for(java.beans.PropertyChangeListener l : getPropertyChangeListeners())
            {
                l.propertyChange(e);
            }
        }

        void setTarget(Action t)
        {
            if(_target != null)
            {
                _target.removePropertyChangeListener(this);
            }
            _target = t;
            if(_target != null)
            {
                _target.addPropertyChangeListener(this);
                setEnabled(_target.isEnabled());
            }
            else
            {
                setEnabled(false);
            }
        }

        Action getTarget()
        {
            return _target;
        }

        private Action _target;
    }

    private class DeleteTextAction extends AbstractAction
    {
        public DeleteTextAction(String name)
        {
            super(name);
        }

        @Override
        public void actionPerformed(ActionEvent e)
        {
            if(_target != null)
            {
                Action a = _target.getActionMap().get("delete-next");
                if(a != null)
                {
                    a.actionPerformed(new ActionEvent(_target, ActionEvent.ACTION_PERFORMED, null));
                }
            }
        }

        void setTarget(JTextComponent t)
        {
            _target = t;
        }

        private JTextComponent _target;
    }

    private class FocusListener implements java.beans.PropertyChangeListener
    {
        @Override
        public void propertyChange(java.beans.PropertyChangeEvent e)
        {
            Object o = e.getNewValue();
            if(o == null)
            {
                unknownTarget();
            }
            else if(o instanceof JTextComponent)
            {
                enableTextEditActions((JTextComponent)o);
            }
            else if(o instanceof JTree)
            {
                JTree tree = (JTree)o;
                if(tree.getModel().getRoot() instanceof com.zeroc.IceGridGUI.Application.Root)
                {
                    enableTreeEditActions();
                }
                else
                {
                    disableAllEditMenusAndButtons();
                }
            }
            else
            {
                unknownTarget();
            }
        }

        private void unknownTarget()
        {
            if(_copy.getTarget() == _copyText)
            {
                disableAllEditMenusAndButtons();
            }
            //
            // Otherwise (for good tree), do nothing.
            //
        }

        private void enableTextEditActions(JTextComponent target)
        {
            boolean editable = target.isEditable() && target.isEnabled();

            _cutText.setEnabled(editable);
            _copyText.setEnabled(true);
            _pasteText.setEnabled(editable);
            _deleteText.setEnabled(editable);
            _deleteText.setTarget(target);

            if(_copy.getTarget() != _copyText)
            {
                _cut.setTarget(_cutText);
                _copy.setTarget(_copyText);
                _paste.setTarget(_pasteText);
                _delete.setTarget(_deleteText);
                _moveUp.setTarget(null);
                _moveDown.setTarget(null);
            }
        }
    }

    private void enableTreeEditActions()
    {
        _cut.setTarget(null);
        _copy.setTarget(_appActionsForMenu.get(com.zeroc.IceGridGUI.Application.TreeNode.COPY));
        _paste.setTarget(_appActionsForMenu.get(com.zeroc.IceGridGUI.Application.TreeNode.PASTE));
        _delete.setTarget(_appActionsForMenu.get(com.zeroc.IceGridGUI.Application.TreeNode.DELETE));
        _moveUp.setTarget(_appActionsForMenu.get(com.zeroc.IceGridGUI.Application.TreeNode.MOVE_UP));
        _moveDown.setTarget(_appActionsForMenu.get(com.zeroc.IceGridGUI.Application.TreeNode.MOVE_DOWN));
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

            _newMenu.add(_appActionsForMenu.get(com.zeroc.IceGridGUI.Application.TreeNode.NEW_ADAPTER));
            _newMenu.add(_appActionsForMenu.get(com.zeroc.IceGridGUI.Application.TreeNode.NEW_NODE));
            _newMenu.add(_appActionsForMenu.get(com.zeroc.IceGridGUI.Application.TreeNode.NEW_PROPERTY_SET));
            _newMenu.add(_appActionsForMenu.get(com.zeroc.IceGridGUI.Application.TreeNode.NEW_REPLICA_GROUP));

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
            _newServerMenu.add(_appActionsForMenu.get(com.zeroc.IceGridGUI.Application.TreeNode.NEW_SERVER));
            _newServerMenu.add(_appActionsForMenu.get(com.zeroc.IceGridGUI.Application.TreeNode.NEW_SERVER_ICEBOX));
            _newServerMenu.add(
                _appActionsForMenu.get(com.zeroc.IceGridGUI.Application.TreeNode.NEW_SERVER_FROM_TEMPLATE));

            //
            // New service sub-sub-menu
            //
            _newServiceMenu = new JMenu("Service");
            _newServiceMenu.setEnabled(false);
            _newMenu.add(_newServiceMenu);
            _newServiceMenu.add(_appActionsForMenu.get(com.zeroc.IceGridGUI.Application.TreeNode.NEW_SERVICE));
            _newServiceMenu.add(
                _appActionsForMenu.get(com.zeroc.IceGridGUI.Application.TreeNode.NEW_SERVICE_FROM_TEMPLATE));

            //
            // New template sub-sub-menu
            //
            _newTemplateMenu = new JMenu("Template");
            _newTemplateMenu.setEnabled(false);
            _newMenu.add(_newTemplateMenu);
            _newTemplateMenu.add(_appActionsForMenu.get(com.zeroc.IceGridGUI.Application.TreeNode.NEW_TEMPLATE_SERVER));
            _newTemplateMenu.add(
                _appActionsForMenu.get(com.zeroc.IceGridGUI.Application.TreeNode.NEW_TEMPLATE_SERVER_ICEBOX));
            _newTemplateMenu.add(
                _appActionsForMenu.get(com.zeroc.IceGridGUI.Application.TreeNode.NEW_TEMPLATE_SERVICE));

            //
            // New Graph sub-menu
            //
            _newMenu.addSeparator();
            _newMenu.add(_newGraph);

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
            fileMenu.add(_saveToRegistryWithoutRestart);
            fileMenu.addSeparator();
            fileMenu.add(_discardUpdates);
            fileMenu.addSeparator();
            fileMenu.add(_certificateManager);
            if(!System.getProperty("os.name").startsWith("Mac OS"))
            {
                fileMenu.addSeparator();
                fileMenu.add(_exit);
            }

            //
            // Edit menu
            //
            JMenu editMenu = new JMenu("Edit");
            editMenu.setMnemonic(java.awt.event.KeyEvent.VK_E);
            add(editMenu);

            editMenu.add(_cut);
            editMenu.add(_copy);
            editMenu.add(_paste);
            editMenu.add(_delete);
            editMenu.addSeparator();
            editMenu.add(_moveUp);
            editMenu.add(_moveDown);

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
            _appMenu.add(_showApplicationDetails);
            _appMenu.add(_showLiveDeploymentFilters);
            _appMenu.addSeparator();
            _appMenu.add(_removeApplicationFromRegistry);
            _appMenu.setEnabled(false);

            //
            // Metrics View sub-menu
            //
            _metricsViewMenu = new JMenu("Metrics View");
            _metricsViewMenu.setEnabled(false);
            toolsMenu.add(_metricsViewMenu);
            _metricsViewMenu.add(
                _liveActionsForMenu.get(com.zeroc.IceGridGUI.LiveDeployment.TreeNode.ENABLE_METRICS_VIEW));
            _metricsViewMenu.add(
                _liveActionsForMenu.get(com.zeroc.IceGridGUI.LiveDeployment.TreeNode.DISABLE_METRICS_VIEW));

            //
            // Node sub-menu
            //
            _nodeMenu = new JMenu("Node");
            _nodeMenu.setEnabled(false);
            toolsMenu.add(_nodeMenu);
            _nodeMenu.add(_liveActionsForMenu.get(com.zeroc.IceGridGUI.LiveDeployment.TreeNode.RETRIEVE_ICE_LOG));
            _nodeMenu.add(_liveActionsForMenu.get(com.zeroc.IceGridGUI.LiveDeployment.TreeNode.RETRIEVE_STDOUT));
            _nodeMenu.add(_liveActionsForMenu.get(com.zeroc.IceGridGUI.LiveDeployment.TreeNode.RETRIEVE_STDERR));
            _nodeMenu.addSeparator();
            _nodeMenu.add(_liveActionsForMenu.get(com.zeroc.IceGridGUI.LiveDeployment.TreeNode.START_ALL_SERVERS));
            _nodeMenu.add(_liveActionsForMenu.get(com.zeroc.IceGridGUI.LiveDeployment.TreeNode.STOP_ALL_SERVERS));
            _nodeMenu.addSeparator();
            _nodeMenu.add(_liveActionsForMenu.get(com.zeroc.IceGridGUI.LiveDeployment.TreeNode.SHUTDOWN_NODE));

            //
            // Registry sub-menu
            //
            _registryMenu = new JMenu("Registry");
            _registryMenu.setEnabled(false);
            toolsMenu.add(_registryMenu);
            _registryMenu.add(_liveActionsForMenu.get(com.zeroc.IceGridGUI.LiveDeployment.TreeNode.ADD_OBJECT));
            _registryMenu.addSeparator();
            _registryMenu.add(_liveActionsForMenu.get(com.zeroc.IceGridGUI.LiveDeployment.TreeNode.RETRIEVE_ICE_LOG));
            _registryMenu.add(_liveActionsForMenu.get(com.zeroc.IceGridGUI.LiveDeployment.TreeNode.RETRIEVE_STDOUT));
            _registryMenu.add(_liveActionsForMenu.get(com.zeroc.IceGridGUI.LiveDeployment.TreeNode.RETRIEVE_STDERR));
            _registryMenu.addSeparator();
            _registryMenu.add(_liveActionsForMenu.get(com.zeroc.IceGridGUI.LiveDeployment.TreeNode.SHUTDOWN_REGISTRY));

            //
            // Server sub-menu
            //
            _serverMenu = new JMenu("Server");
            _serverMenu.setEnabled(false);
            toolsMenu.add(_serverMenu);
            _serverMenu.add(_liveActionsForMenu.get(com.zeroc.IceGridGUI.LiveDeployment.TreeNode.START));
            _serverMenu.add(_liveActionsForMenu.get(com.zeroc.IceGridGUI.LiveDeployment.TreeNode.STOP));
            _serverMenu.addSeparator();
            _serverMenu.add(_liveActionsForMenu.get(com.zeroc.IceGridGUI.LiveDeployment.TreeNode.ENABLE));
            _serverMenu.add(_liveActionsForMenu.get(com.zeroc.IceGridGUI.LiveDeployment.TreeNode.DISABLE));
            _serverMenu.addSeparator();
            _serverMenu.add(_liveActionsForMenu.get(com.zeroc.IceGridGUI.LiveDeployment.TreeNode.WRITE_MESSAGE));
            _serverMenu.add(_liveActionsForMenu.get(com.zeroc.IceGridGUI.LiveDeployment.TreeNode.RETRIEVE_ICE_LOG));
            _serverMenu.add(_liveActionsForMenu.get(com.zeroc.IceGridGUI.LiveDeployment.TreeNode.RETRIEVE_STDOUT));
            _serverMenu.add(_liveActionsForMenu.get(com.zeroc.IceGridGUI.LiveDeployment.TreeNode.RETRIEVE_STDERR));
            _serverMenu.add(_liveActionsForMenu.get(com.zeroc.IceGridGUI.LiveDeployment.TreeNode.RETRIEVE_LOG_FILE));
            _serverMenu.addSeparator();
            _signalMenu = new JMenu("Send Signal");
            _serverMenu.add(_signalMenu);
            _signalMenu.add(_liveActionsForMenu.get(com.zeroc.IceGridGUI.LiveDeployment.TreeNode.SIGHUP));
            _signalMenu.add(_liveActionsForMenu.get(com.zeroc.IceGridGUI.LiveDeployment.TreeNode.SIGINT));
            _signalMenu.add(_liveActionsForMenu.get(com.zeroc.IceGridGUI.LiveDeployment.TreeNode.SIGQUIT));
            _signalMenu.add(_liveActionsForMenu.get(com.zeroc.IceGridGUI.LiveDeployment.TreeNode.SIGKILL));
            _signalMenu.add(_liveActionsForMenu.get(com.zeroc.IceGridGUI.LiveDeployment.TreeNode.SIGUSR1));
            _signalMenu.add(_liveActionsForMenu.get(com.zeroc.IceGridGUI.LiveDeployment.TreeNode.SIGUSR2));
            _signalMenu.add(_liveActionsForMenu.get(com.zeroc.IceGridGUI.LiveDeployment.TreeNode.SIGTERM));
            _serverMenu.addSeparator();
            _serverMenu.add(_liveActionsForMenu.get(com.zeroc.IceGridGUI.LiveDeployment.TreeNode.OPEN_DEFINITION));

            //
            // Service sub-menu
            //
            _serviceMenu = new JMenu("Service");
            _serviceMenu.setEnabled(false);
            toolsMenu.add(_serviceMenu);
            _serviceMenu.add(_liveActionsForMenu.get(com.zeroc.IceGridGUI.LiveDeployment.TreeNode.START));
            _serviceMenu.add(_liveActionsForMenu.get(com.zeroc.IceGridGUI.LiveDeployment.TreeNode.STOP));
            _serviceMenu.addSeparator();
            _serviceMenu.add(_liveActionsForMenu.get(com.zeroc.IceGridGUI.LiveDeployment.TreeNode.RETRIEVE_ICE_LOG));
            _serviceMenu.add(_liveActionsForMenu.get(com.zeroc.IceGridGUI.LiveDeployment.TreeNode.RETRIEVE_LOG_FILE));

            //
            // Help menu
            //
            JMenu helpMenu = new JMenu("Help");
            helpMenu.setMnemonic(java.awt.event.KeyEvent.VK_H);
            add(helpMenu);

            helpMenu.add(_helpContents);

            if(!System.getProperty("os.name").startsWith("Mac OS"))
            {
                helpMenu.addSeparator();
                helpMenu.add(_about);
            }
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

            button = new JButton(_showLiveDeploymentFilters);
            button.setText(null);
            button.setIcon(Utils.getIcon("/icons/24x24/view_filter.png"));
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
            button = new JButton(_saveToRegistryWithoutRestart);
            button.setText(null);
            button.setIcon(Utils.getIcon("/icons/24x24/save_without_restart.png"));
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

            button = new JButton(_copy);
            button.setText(null);
            button.setIcon(Utils.getIcon("/icons/24x24/copy.png"));
            add(button);
            button = new JButton(_paste);
            button.setText(null);
            button.setIcon(Utils.getIcon("/icons/24x24/paste.png"));
            add(button);
            button = new JButton(_delete);
            button.setText(null);
            button.setIcon(Utils.getIcon("/icons/24x24/delete.png"));
            add(button);

            addSeparator();

            add(_showVarsTool);
            add(_substituteTool);
        }
    }

    static private class ReuseConnectionRouter implements com.zeroc.Ice.Router
    {
        public
        ReuseConnectionRouter(com.zeroc.Ice.ObjectPrx proxy)
        {
            _clientProxy = proxy;
        }

        @Override
        public com.zeroc.Ice.Router.GetClientProxyResult
        getClientProxy(com.zeroc.Ice.Current current)
        {
            return new com.zeroc.Ice.Router.GetClientProxyResult(_clientProxy, java.util.Optional.of(false));
        }

        @Override
        public com.zeroc.Ice.ObjectPrx
        getServerProxy(com.zeroc.Ice.Current current)
        {
            return null;
        }

        @Override
        public com.zeroc.Ice.ObjectPrx[]
        addProxies(com.zeroc.Ice.ObjectPrx[] proxies, com.zeroc.Ice.Current current)
        {
            return new com.zeroc.Ice.ObjectPrx[0];
        }

        private final com.zeroc.Ice.ObjectPrx _clientProxy;
    }

    public com.zeroc.Ice.Communicator getCommunicator()
    {
        if(_communicator == null)
        {
            _communicator = com.zeroc.Ice.Util.initialize(_initData);
        }
        return _communicator;
    }

    public com.zeroc.Ice.Properties getProperties()
    {
        return _initData.properties;
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

    public Action getShowLiveDeploymentFiltersAction()
    {
        return _showLiveDeploymentFilters;
    }

    public Action getSaveAction()
    {
        return _save;
    }

    public Action getSaveToRegistryAction()
    {
        return _saveToRegistry;
    }

    public Action getSaveToRegistryWithoutRestartAction()
    {
        return _saveToRegistryWithoutRestart;
    }

    public Action getSaveToFileAction()
    {
        return _saveToFile;
    }

    public Action getDiscardUpdatesAction()
    {
        return _discardUpdates;
    }

    public void disableAllEditMenusAndButtons()
    {
        _cut.setTarget(null);
        _copy.setTarget(null);
        _paste.setTarget(null);
        _delete.setTarget(null);
        _moveUp.setTarget(null);
        _moveDown.setTarget(null);
    }

    //
    // Open live application and select application tab
    //
    public ApplicationPane openLiveApplication(String applicationName)
    {
        ApplicationPane app = _liveApplications.get(applicationName);
        if(app == null)
        {
            ApplicationDescriptor desc = _liveDeploymentRoot.getApplicationDescriptor(applicationName);
            if(desc == null)
            {
                JOptionPane.showMessageDialog(
                    _mainFrame,
                    "The application '" + applicationName + "' was not found in the registry.",
                    "No such application",
                    JOptionPane.ERROR_MESSAGE);
                return null;
            }
            //
            // Essential: deep-copy desc!
            //
            desc = com.zeroc.IceGridGUI.Application.Root.copyDescriptor(desc);
            com.zeroc.IceGridGUI.Application.Root root;
            try
            {
                root = new com.zeroc.IceGridGUI.Application.Root(this, desc, true, null);
            }
            catch(com.zeroc.IceGridGUI.Application.UpdateFailedException e)
            {
                JOptionPane.showMessageDialog(
                    _mainFrame,
                    e.toString(),
                    "Bad Application Descriptor: Unable to load from Registry",
                    JOptionPane.ERROR_MESSAGE);
                return null;
            }

            app = new ApplicationPane(root);
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

    public void addLiveApplication(com.zeroc.IceGridGUI.Application.Root root)
    {
        ApplicationPane app = _mainPane.findApplication(root);
        assert app != null;
        _liveApplications.put(app.getRoot().getId(), app);
    }

    public ApplicationPane getLiveApplication(String name)
    {
        return _liveApplications.get(name);
    }

    //
    // From the Application observer:
    //
    void applicationInit(String instanceName, int serial, java.util.List<ApplicationInfo> applications)
    {
        assert _latestSerial == -1;
        _latestSerial = serial;

        _liveDeploymentRoot.applicationInit(instanceName, _sessionKeeper.getReplicaName(), applications);
        //
        // When we get this init, we can't have any live Application yet.
        //
    }

    void applicationAdded(int serial, ApplicationInfo info)
    {
        _liveDeploymentRoot.applicationAdded(info);
        _liveDeploymentPane.refresh();
        _statusBar.setText("Last update: new application '" + info.descriptor.name + "'");
        updateSerial(serial);
    }

    void applicationRemoved(int serial, String name)
    {
        _liveDeploymentRoot.applicationRemoved(name);
        _liveDeploymentPane.refresh();
        _statusBar.setText("Last update: application '" + name + "' was removed");

        ApplicationPane app = _liveApplications.get(name);

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

    void applicationUpdated(int serial, ApplicationUpdateInfo update)
    {
        _liveDeploymentRoot.applicationUpdated(update);
        _liveDeploymentPane.refresh();

        _statusBar.setText("Last update: application  '" + update.descriptor.name + "' was updated");

        ApplicationPane app = _liveApplications.get(update.descriptor.name);

        if(app != null)
        {
            if(app.getRoot().update(update.descriptor))
            {
                app.refresh();
            }
        }
        updateSerial(serial);
    }

    //
    // From the Adapter observer:
    //
    void adapterInit(AdapterInfo[] adapters)
    {
        _liveDeploymentRoot.adapterInit(adapters);
        _liveDeploymentPane.refresh();
    }

    void adapterAdded(AdapterInfo info)
    {
        _liveDeploymentRoot.adapterAdded(info);
        _liveDeploymentPane.refresh();
    }

    void adapterUpdated(AdapterInfo info)
    {
        _liveDeploymentRoot.adapterUpdated(info);
        _liveDeploymentPane.refresh();
    }

    void adapterRemoved(String id)
    {
        _liveDeploymentRoot.adapterRemoved(id);
        _liveDeploymentPane.refresh();
    }

    //
    // From the Object observer:
    //
    void objectInit(ObjectInfo[] objects)
    {
        _liveDeploymentRoot.objectInit(objects);
        _liveDeploymentPane.refresh();
    }

    void objectAdded(ObjectInfo info)
    {
        _liveDeploymentRoot.objectAdded(info);
        _liveDeploymentPane.refresh();
    }

    void objectUpdated(ObjectInfo info)
    {
        _liveDeploymentRoot.objectUpdated(info);
        _liveDeploymentPane.refresh();
    }

    void objectRemoved(com.zeroc.Ice.Identity id)
    {
        _liveDeploymentRoot.objectRemoved(id);
        _liveDeploymentPane.refresh();
    }

    public void accessDenied(AccessDeniedException e)
    {
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
            com.zeroc.IceGridGUI.Application.Root.copyDescriptor((ApplicationDescriptor)descriptor);

        com.zeroc.IceGridGUI.Application.Root root = new com.zeroc.IceGridGUI.Application.Root(this, desc);
        ApplicationPane app = new ApplicationPane(root);
        _mainPane.addApplication(app);
        _mainPane.setSelectedComponent(app);
        root.setSelectedNode(root);
    }

    public void removeApplicationFromRegistry(final String name)
    {
        _mainFrame.setCursor(Cursor.getPredefinedCursor(Cursor.WAIT_CURSOR));

        try
        {
            Runnable runnable = new Runnable()
                {
                    private void release()
                    {
                        releaseExclusiveWriteAccess();
                        getMainFrame().setCursor(Cursor.getPredefinedCursor(Cursor.DEFAULT_CURSOR));
                    }

                    private void handleFailure(String prefix, String title, String message)
                    {
                        release();
                        getStatusBar().setText(prefix + "failed!");

                        JOptionPane.showMessageDialog(
                            getMainFrame(),
                            message,
                            title,
                            JOptionPane.ERROR_MESSAGE);
                    }

                    @Override
                    public void run()
                    {
                        getMainFrame().setCursor(Cursor.getPredefinedCursor(Cursor.WAIT_CURSOR));
                        boolean asyncRelease = false;

                        final String prefix = "Deleting application '" + name + "'...";

                        if(_traceSaveToRegistry)
                        {
                            traceSaveToRegistry("sending removeApplication for application " + name);
                        }

                        try
                        {
                            _sessionKeeper.getAdmin().removeApplicationAsync(name).whenComplete(
                                (result, ex) ->
                                {
                                    if(_traceSaveToRegistry)
                                    {
                                        traceSaveToRegistry("removeApplication for application " + name +
                                                            (ex == null ? ": success" : ": failed"));
                                    }

                                    if(ex == null)
                                    {
                                        SwingUtilities.invokeLater(() ->
                                                                   {
                                                                       release();
                                                                       getStatusBar().setText(prefix + "done.");
                                                                   });
                                    }
                                    else
                                    {
                                        if(ex instanceof com.zeroc.Ice.UserException)
                                        {
                                            SwingUtilities.invokeLater(() ->
                                                                       {
                                                                           handleFailure(prefix, "Delete failed",
                                                                                         "IceGrid exception: " + ex.toString());
                                                                       });
                                        }
                                        else
                                        {
                                            SwingUtilities.invokeLater(() ->
                                                                       {
                                                                           handleFailure(prefix, "Delete failed",
                                                                                         "Communication exception: " + ex.toString());
                                                                       });
                                        }
                                    }
                                });
                            asyncRelease = true;
                        }
                        catch(com.zeroc.Ice.LocalException e)
                        {
                            if(_traceSaveToRegistry)
                            {
                                traceSaveToRegistry("Ice communications exception while removing application " + name);
                            }

                            JOptionPane.showMessageDialog(
                                getMainFrame(),
                                e.toString(),
                                "Communication Exception",
                                JOptionPane.ERROR_MESSAGE);
                        }
                        finally
                        {
                            if(!asyncRelease)
                            {
                                releaseExclusiveWriteAccess();
                                getMainFrame().setCursor(Cursor.getPredefinedCursor(Cursor.DEFAULT_CURSOR));
                            }
                        }
                    }
                };

            acquireExclusiveWriteAccess(runnable);
        }
        catch(AccessDeniedException e)
        {
            accessDenied(e);
        }
        catch(com.zeroc.Ice.LocalException e)
        {
            JOptionPane.showMessageDialog(
                _mainFrame,
                "Could not remove application '" + name +
                "' from IceGrid registry:\n" + e.toString(),
                "Trouble with IceGrid registry",
                JOptionPane.ERROR_MESSAGE);
        }
    }

    public void acquireExclusiveWriteAccess(Runnable runnable)
        throws AccessDeniedException
    {
        if(_writeSerial == -1)
        {
            _mainFrame.setCursor(Cursor.getPredefinedCursor(Cursor.WAIT_CURSOR));
            try
            {
                _writeSerial = _sessionKeeper.getSession().startUpdate();

                if(_traceSaveToRegistry)
                {
                    traceSaveToRegistry("startUpdate returned serial " + _writeSerial);
                }
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

        if(_traceSaveToRegistry)
        {
            traceSaveToRegistry("acquireExclusiveWriteAccess: writeAccessCount is " + _writeAccessCount);
        }

        if(runnable != null)
        {
            if(_traceSaveToRegistry)
            {
                traceSaveToRegistry("latestSerial is " + _latestSerial);
            }

            if(_writeSerial <= _latestSerial)
            {
                if(_traceSaveToRegistry)
                {
                    traceSaveToRegistry("run update immediately");
                }
                runnable.run();
            }
            else
            {
                if(_traceSaveToRegistry)
                {
                    traceSaveToRegistry("wait for updates from ApplicationObserver");
                }
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
                if(_traceSaveToRegistry)
                {
                    traceSaveToRegistry("finishUpdate done");
                }
            }
            catch(AccessDeniedException e)
            {
                accessDenied(e);
            }
            catch(com.zeroc.Ice.ObjectNotExistException e)
            {
                //
                // Ignored, the session is gone, and so is the exclusive access.
                //
            }
            catch(com.zeroc.Ice.LocalException e)
            {
                JOptionPane.showMessageDialog(
                    _mainFrame,
                    "Could not release exclusive write access on the IceGrid registry:\n"
                    + e.toString(),
                    "Trouble with IceGrid registry",
                    JOptionPane.ERROR_MESSAGE);
            }
        }

        if(_traceSaveToRegistry)
        {
            traceSaveToRegistry("releaseExclusiveWriteAccess: writeAccessCount is " + _writeAccessCount);
        }
    }

    private void updateSerial(int serial)
    {
        assert serial == _latestSerial + 1;
        _latestSerial = serial;

        if(_writeAccessCount > 0 &&
           _writeSerial <= _latestSerial &&
           _onExclusiveWrite != null)
        {
            Runnable runnable = _onExclusiveWrite;
            _onExclusiveWrite = null;

            if(_traceSaveToRegistry)
            {
                traceSaveToRegistry("writeSerial <= latestSerial, running update");
            }
            runnable.run();
            _mainFrame.setCursor(Cursor.getPredefinedCursor(Cursor.DEFAULT_CURSOR));
        }
    }

    //
    // From the Registry observer:
    //
    void registryUp(RegistryInfo info)
    {
        _liveDeploymentRoot.registryUp(info);
        _liveDeploymentPane.refresh();
    }

    void registryDown(String registry)
    {
        _liveDeploymentRoot.registryDown(registry);
        _liveDeploymentPane.refresh();
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
        for(ApplicationPane p : _liveApplications.values())
        {
            if(p.getRoot().kill())
            {
                _mainPane.remove(p);
            }
        }
        _liveApplications.clear();

        //
        // Close al graphs
        //
        java.util.List<IGraphView> views = new java.util.ArrayList<>(_graphViews);
        for(IGraphView v : views)
        {
            v.close();
        }

        _logout.setEnabled(false);
        _showLiveDeploymentFilters.setEnabled(false);
        _openApplicationFromRegistry.setEnabled(false);
        _showApplicationDetails.setEnabled(false);
        _removeApplicationFromRegistry.setEnabled(false);
        _appMenu.setEnabled(false);
        _newApplicationWithDefaultTemplates.setEnabled(false);
        _acquireExclusiveWriteAccess.setEnabled(false);
        _releaseExclusiveWriteAccess.setEnabled(false);
        _saveToRegistry.setEnabled(false);
        _saveToRegistryWithoutRestart.setEnabled(false);
    }

    enum TrustDecision { YesAlways, YesThisTime, No };

    void
    login(final SessionKeeper sessionKeeper,
          final SessionKeeper.ConnectionInfo info,
          final JDialog parent)
    {
        //
        // Keep certificates arround for connection retry
        //
        _transientCert = null;
        _liveDeploymentRoot.clear();

        destroyCommunicator();

        com.zeroc.Ice.InitializationData initData = _initData.clone();
        initData.properties = initData.properties._clone();
        initData.properties.setProperty("Ice.Plugin.IceSSL", "com.zeroc.IceSSL.PluginFactory");
        initData.properties.setProperty("IceSSL.VerifyPeer", "0");

        if(info.getAuth() == SessionKeeper.AuthType.X509CertificateAuthType | info.getUseX509Certificate())
        {
            try
            {
                initData.properties.setProperty("IceSSL.Keystore", getDataDirectory() + "/MyCerts.jks");
            }
            catch(java.lang.Exception e)
            {
                JOptionPane.showMessageDialog(parent, e.toString(), "Failed to access data directory",
                                              JOptionPane.ERROR_MESSAGE);
                _sessionKeeper.loginFailed();
                return;
            }
            if(info.getKeyPassword() != null)
            {
                initData.properties.setProperty("IceSSL.Password", new String(info.getKeyPassword()));
            }
            initData.properties.setProperty("IceSSL.Alias", info.getAlias());
        }

        //
        // Clear Ice.Default.Router
        // (when info.routed, we don't want to route the router)
        //
        initData.properties.setProperty("Ice.Default.Router", "");

        try
        {
            _communicator = com.zeroc.Ice.Util.initialize(initData);
        }
        catch(com.zeroc.Ice.LocalException e)
        {
            JOptionPane.showMessageDialog(parent,
                                          e.toString(),
                                          "Communicator initialization failed",
                                          JOptionPane.ERROR_MESSAGE);
            _sessionKeeper.loginFailed();
            return;
        }

        class CertificateVerifier implements com.zeroc.IceSSL.CertificateVerifier
        {
            public CertificateVerifier()
                throws java.io.IOException, java.security.GeneralSecurityException, java.lang.Exception
            {
                {
                    _trustedCaKeyStore = KeyStore.getInstance("JKS");

                    FileInputStream is = null;
                    final String path = getDataDirectory() + "/AuthorityCerts.jks";
                    if(new File(path).isFile())
                    {
                        is = new FileInputStream(new File(path));
                    }

                    _trustedCaKeyStore.load(is, null);
                }

                {
                    _trustedServerKeyStore = KeyStore.getInstance("JKS");

                    FileInputStream is = null;
                    final String path = getDataDirectory() + "/ServerCerts.jks";
                    if(new File(path).isFile())
                    {
                        is = new FileInputStream(new File(path));
                    }

                    _trustedServerKeyStore.load(is, null);
                }
            }

            class AcceptInvalidCertDialog implements Runnable
            {
                public TrustDecision show(com.zeroc.IceSSL.ConnectionInfo info, boolean validDate,
                                          boolean validAlternateName, boolean trustedCA)
                {
                    _info = info;
                    _validDate = validDate;
                    _validAlternateName = validAlternateName;
                    _trustedCA = trustedCA;

                    while(true)
                    {
                        try
                        {
                            SwingUtilities.invokeAndWait(this);
                            break;
                        }
                        catch(java.lang.InterruptedException e)
                        {
                            // Ignore and retry
                        }
                        catch(java.lang.reflect.InvocationTargetException e)
                        {
                            break;
                        }
                    }
                    return _decision;
                }

                @Override
                public void
                run()
                {
                    try
                    {
                        UntrustedCertificateDialog dialog = new UntrustedCertificateDialog(parent, _info, _validDate,
                                                                                           _validAlternateName,
                                                                                           _trustedCA);
                        Utils.addEscapeListener(dialog);
                        _decision = dialog.showDialog();
                    }
                    catch(java.lang.Exception ex)
                    {
                        JOptionPane.showMessageDialog(parent, ex.toString(), "Failed to inspect certificate details",
                                                      JOptionPane.ERROR_MESSAGE);
                    }
                }

                private com.zeroc.IceSSL.ConnectionInfo _info;
                private boolean _validDate;
                private boolean _validAlternateName;
                private boolean _trustedCA;
                private TrustDecision _decision = TrustDecision.No;
            }

            @Override
            public boolean verify(com.zeroc.IceSSL.ConnectionInfo info)
            {
                if(!(info.certs[0] instanceof X509Certificate))
                {
                    return false;
                }

                X509Certificate cert = (X509Certificate) info.certs[0];
                byte[] encoded;
                try
                {
                    encoded = cert.getEncoded();
                }
                catch(java.security.GeneralSecurityException ex)
                {
                    return false;
                }

                //
                // Compare the server certificate with a previous accepted certificate if
                // any, the transient certificate is reset by Coordinator.login, and is only
                // useful in case the connection is retry, because a timeout or ACM closed
                // it while the certificate verifier was waiting for the user decision.
                //
                // This avoids to show the dialog again if the user already granted the cert for
                // this login operation.
                //
                if(_transientCert != null && _transientCert.equals(cert))
                {
                    return true;
                }

                //
                // Compare the server with the user trusted server certificates.
                //
                try
                {
                    for(Enumeration<String> e = _trustedServerKeyStore.aliases(); e.hasMoreElements() ;)
                    {
                        String alias = e.nextElement();
                        if(!_trustedServerKeyStore.isCertificateEntry(alias))
                        {
                            continue;
                        }

                        Certificate c = _trustedServerKeyStore.getCertificate(alias);
                        try
                        {
                            if(java.util.Arrays.equals(encoded, c.getEncoded()))
                            {
                                return true;
                            }
                        }
                        catch(java.security.GeneralSecurityException ex)
                        {
                            // Skip to next certificate
                            continue;
                        }
                    }
                }
                catch(final java.security.KeyStoreException ex)
                {
                    while(true)
                    {
                        try
                        {
                            SwingUtilities.invokeAndWait(() ->
                                                         {
                                                             JOptionPane.showMessageDialog(parent, ex.toString(), "Error loading keystore",
                                                                                           JOptionPane.ERROR_MESSAGE);
                                                         });
                            break;
                        }
                        catch(java.lang.InterruptedException e)
                        {
                        }
                        catch(java.lang.reflect.InvocationTargetException e)
                        {
                            break;
                        }
                    }
                    return false;
                }

                boolean validDate = true;
                boolean trustedCA = false;
                boolean validAlternateName = false;

                //
                // Check the certificate date is valid.
                //

                java.util.Date now = new java.util.Date();
                if(now.getTime() > cert.getNotAfter().getTime() || now.getTime() < cert.getNotBefore().getTime())
                {
                    validDate = false;
                }

                String remoteAddress = null;
                for(com.zeroc.Ice.ConnectionInfo p = info.underlying; p != null; p = p.underlying)
                {
                    if(p instanceof com.zeroc.Ice.IPConnectionInfo)
                    {
                        remoteAddress = ((com.zeroc.Ice.IPConnectionInfo)p).remoteAddress;
                        break;
                    }
                }

                //
                // Check server alternate names match the connection remote address
                //
                if(remoteAddress != null)
                {
                    try
                    {
                        Collection<java.util.List<?>> altNames = cert.getSubjectAlternativeNames();
                        if(altNames != null)
                        {
                            for(java.util.List<?> l : altNames)
                            {
                                Integer kind = (Integer)l.get(0);
                                if(kind != 2 && kind != 7)
                                {
                                    continue;
                                }
                                if(remoteAddress.equalsIgnoreCase(l.get(1).toString()))
                                {
                                    validAlternateName = true;
                                    break;
                                }
                            }
                        }
                    }
                    catch(java.security.cert.CertificateParsingException ex)
                    {
                        validAlternateName = false;
                    }
                }

                //
                // Check if the certificate has been signed by any of the trusted certificate
                // authorities.
                //
                try
                {
                    for(Enumeration<String> e = _trustedCaKeyStore.aliases(); e.hasMoreElements() ;)
                    {
                        String alias = e.nextElement();
                        if(!_trustedCaKeyStore.isCertificateEntry(alias))
                        {
                            continue;
                        }
                        Certificate c = _trustedCaKeyStore.getCertificate(alias);
                        try
                        {
                            cert.verify(c.getPublicKey());
                            trustedCA = true;
                            break;
                        }
                        catch(java.security.GeneralSecurityException ex)
                        {
                            // Skip to next certificate
                            continue;
                        }
                    }
                }
                catch(final java.security.KeyStoreException ex)
                {
                    while(true)
                    {
                        try
                        {
                            SwingUtilities.invokeAndWait(() ->
                                                         {
                                                             JOptionPane.showMessageDialog(parent, ex.toString(), "Error loading keystore",
                                                                                           JOptionPane.ERROR_MESSAGE);
                                                         });
                            break;
                        }
                        catch(java.lang.InterruptedException e)
                        {
                        }
                        catch(java.lang.reflect.InvocationTargetException e)
                        {
                            break;
                        }
                    }
                    return false;
                }

                if(validDate && validAlternateName && trustedCA)
                {
                    return true;
                }

                TrustDecision decision = new AcceptInvalidCertDialog().show(info, validDate, validAlternateName,
                                                                            trustedCA);

                if(decision == TrustDecision.YesThisTime)
                {
                    _transientCert = (X509Certificate) info.certs[0];
                    return true;
                }
                else if(decision == TrustDecision.YesAlways)
                {
                    try
                    {
                        String CN = "";
                        LdapName dn = new LdapName(cert.getSubjectX500Principal().getName());
                        for(Rdn rdn: dn.getRdns())
                        {
                            if(rdn.getType().toUpperCase().equals("CN"))
                            {
                                CN = rdn.getValue().toString();
                                break;
                            }
                        }
                        _trustedServerKeyStore.setCertificateEntry(CN, info.certs[0]);
                        _trustedServerKeyStore.store(new FileOutputStream(getDataDirectory() + "/ServerCerts.jks"),
                                                     new char[]{});
                        sessionKeeper.certificateManager(parent).load();
                        return true;
                    }
                    catch(final java.lang.Exception ex)
                    {
                        while(true)
                        {
                            try
                            {
                                SwingUtilities.invokeAndWait(() ->
                                                             {
                                                                 JOptionPane.showMessageDialog(parent, ex.toString(),
                                                                                               "Error saving certificate",
                                                                                               JOptionPane.ERROR_MESSAGE);
                                                             });
                                break;
                            }
                            catch(java.lang.InterruptedException e)
                            {
                            }
                            catch(java.lang.reflect.InvocationTargetException e)
                            {
                                break;
                            }
                        }
                    }
                }
                return false;
            }

            private KeyStore _trustedCaKeyStore;
            private KeyStore _trustedServerKeyStore;
        }

        com.zeroc.IceSSL.Plugin plugin = (com.zeroc.IceSSL.Plugin)_communicator.getPluginManager().getPlugin("IceSSL");
        try
        {
            plugin.setCertificateVerifier(new CertificateVerifier());
        }
        catch(final java.lang.Exception ex)
        {
            while(true)
            {
                try
                {
                    SwingUtilities.invokeAndWait(() ->
                                                 {
                                                     JOptionPane.showMessageDialog(parent, ex.toString(),
                                                                                   "Error creating certificate verifier",
                                                                                   JOptionPane.ERROR_MESSAGE);
                                                     _sessionKeeper.loginFailed();
                                                 });
                    break;
                }
                catch(java.lang.InterruptedException e)
                {
                }
                catch(java.lang.reflect.InvocationTargetException e)
                {
                    break;
                }
            }
            return;
        }

        final String finderStr = "Ice/" + (info.getDirect() ? "LocatorFinder" : "RouterFinder") + ":" +
            (info.getDefaultEndpoint() ?
             ((info.getSSL() ? "ssl" : "tcp") + " -h " + info.getHost() + " -p " + info.getPort()) :
             info.getEndpoint());

        class ConnectionCallback
        {
            synchronized public void setSession(AdminSessionPrx session)
            {
                _session = session;
            }

            synchronized public AdminSessionPrx getSession()
            {
                return _session;
            }

            synchronized public void setACMTimeout(int acmTimeout)
            {
                _acmTimeout = acmTimeout;
            }

            synchronized public void setReplicaName(String replicaName)
            {
                _replicaName = replicaName;
            }

            synchronized public void loginSuccess()
            {
                _logout.setEnabled(true);
                _showLiveDeploymentFilters.setEnabled(true);
                _openApplicationFromRegistry.setEnabled(true);
                _showApplicationDetails.setEnabled(true);
                _removeApplicationFromRegistry.setEnabled(true);
                _appMenu.setEnabled(true);
                _newApplicationWithDefaultTemplates.setEnabled(true);
                _acquireExclusiveWriteAccess.setEnabled(true);
                _mainPane.setSelectedComponent(_liveDeploymentPane);
                _sessionKeeper.loginSuccess(parent, _acmTimeout, _session, _replicaName, info);
            }

            synchronized public void loginFailed()
            {
                _sessionKeeper.loginFailed();
                _failed = true;
            }

            synchronized public void permissionDenied(String msg)
            {
                _failed = true;
                _sessionKeeper.permissionDenied(parent, info, msg);
            }

            synchronized public boolean failed()
            {
                return _failed;
            }

            private AdminSessionPrx _session;
            private int _acmTimeout = 0;
            private String _replicaName;
            private boolean _failed = false;
        }

        if(!info.getDirect())
        {
            final ConnectionCallback cb = new ConnectionCallback();
            getExecutor().submit(() ->
                       {
                           try
                           {
                               com.zeroc.Ice.RouterFinderPrx finder = com.zeroc.Ice.RouterFinderPrx.uncheckedCast(
                                   _communicator.stringToProxy(finderStr));
                               info.setInstanceName(finder.getRouter().ice_getIdentity().category);
                               info.save();
                               com.zeroc.Glacier2.RouterPrx router = com.zeroc.Glacier2.RouterPrx.uncheckedCast(
                                   finder.ice_identity(new com.zeroc.Ice.Identity("router", info.getInstanceName())));

                               //
                               // The session must be routed through this router
                               //
                               _communicator.setDefaultRouter(router);

                               com.zeroc.Glacier2.SessionPrx s;
                               if(info.getAuth() == SessionKeeper.AuthType.X509CertificateAuthType)
                               {
                                   router = com.zeroc.Glacier2.RouterPrx.uncheckedCast(router.ice_secure(true));

                                   s = router.createSessionFromSecureConnection();

                                   if(s == null)
                                   {
                                       SwingUtilities.invokeLater(() ->
                                                                  {
                                                                      JOptionPane.showMessageDialog(
                                                                          parent,
                                                                          "createSessionFromSecureConnection returned a null session: \n"
                                                                          + "verify that Glacier2.SSLSessionManager is set to "
                                                                          + "<IceGridInstanceName>/AdminSSLSessionManager in your Glacier2 "
                                                                          + "router configuration",
                                                                          "Login failed",
                                                                          JOptionPane.ERROR_MESSAGE);
                                                                      cb.loginFailed();
                                                                  });
                                       return;
                                   }
                               }
                               else
                               {
                                   router = com.zeroc.Glacier2.RouterPrx.uncheckedCast(router.ice_preferSecure(true));

                                   s = router.createSession(info.getUsername(), info.getPassword() != null ?
                                                            new String(info.getPassword()) : "");

                                   if(s == null)
                                   {
                                       SwingUtilities.invokeLater(() ->
                                                                  {
                                                                      JOptionPane.showMessageDialog(
                                                                          parent,
                                                                          "createSession returned a null session: \n"
                                                                          + "verify that Glacier2.SessionManager is set to "
                                                                          + "<IceGridInstanceName>/AdminSessionManager in your Glacier2 "
                                                                          + "router configuration",
                                                                          "Login failed",
                                                                          JOptionPane.ERROR_MESSAGE);
                                                                      cb.loginFailed();
                                                                  });
                                       return;
                                   }
                               }
                               cb.setSession(AdminSessionPrx.uncheckedCast(s));
                               cb.setACMTimeout(router.getACMTimeout());
                               cb.setReplicaName(cb.getSession().getReplicaName());
                               SwingUtilities.invokeLater(() -> cb.loginSuccess());
                           }
                           catch(final com.zeroc.Glacier2.PermissionDeniedException e)
                           {
                               SwingUtilities.invokeLater(() ->
                                                          {
                                                              String msg = e.reason;
                                                              if(msg.length() == 0)
                                                              {
                                                                  msg = info.getAuth() == SessionKeeper.AuthType.X509CertificateAuthType ?
                                                                      "Invalid credentials" : "Invalid username/password";
                                                              }
                                                              if(info.getAuth() == SessionKeeper.AuthType.X509CertificateAuthType)
                                                              {
                                                                  JOptionPane.showMessageDialog(parent, "Permission denied: " + msg,
                                                                                                "Login failed", JOptionPane.ERROR_MESSAGE);
                                                                  cb.loginFailed();
                                                              }
                                                              else
                                                              {
                                                                  cb.permissionDenied(msg);
                                                              }
                                                          });
                               return;
                           }
                           catch(final com.zeroc.Glacier2.CannotCreateSessionException e)
                           {
                               SwingUtilities.invokeLater(() ->
                                                          {
                                                              JOptionPane.showMessageDialog(parent, "Could not create session: " + e.reason,
                                                                                            "Login failed", JOptionPane.ERROR_MESSAGE);
                                                              cb.loginFailed();
                                                          });
                               return;
                           }
                           catch(final java.util.prefs.BackingStoreException ex)
                           {
                               SwingUtilities.invokeLater(() ->
                                                          {
                                                              JOptionPane.showMessageDialog(getMainFrame(), ex.toString(),
                                                                                            "Error saving connection", JOptionPane.ERROR_MESSAGE);
                                                          });
                               return;
                           }
                           catch(final com.zeroc.Ice.LocalException e)
                           {
                               SwingUtilities.invokeLater(() ->
                                                          {
                                                              JOptionPane.showMessageDialog(parent, "Could not create session: " + e.toString(),
                                                                                            "Login failed", JOptionPane.ERROR_MESSAGE);
                                                              cb.loginFailed();
                                                          });
                               return;
                           }
                       });
        }
        else
        {
            class RegistryCallback extends ConnectionCallback
            {
                synchronized public void setRegistry(RegistryPrx registry)
                {
                    _registry = registry;
                }

                synchronized public RegistryPrx getRegistry()
                {
                    return _registry;
                }

                synchronized public void setCurrentRegistry(RegistryPrx value)
                {
                    _currentRegistry = value;
                }

                synchronized public RegistryPrx getCurrentRegistry()
                {
                    return _currentRegistry;
                }

                synchronized public void setLocator(com.zeroc.IceGrid.LocatorPrx locator)
                {
                    _locator = locator;
                }

                synchronized public com.zeroc.IceGrid.LocatorPrx getLocator()
                {
                    return _locator;
                }

                private com.zeroc.IceGrid.LocatorPrx _locator;
                private RegistryPrx _registry;
                private RegistryPrx _currentRegistry;
            }

            final RegistryCallback cb = new RegistryCallback();

            if(info.getCustomEndpoint() && info.getEndpoint().equals(""))
            {
                JOptionPane.showMessageDialog(
                    parent,
                    "You need to provide one or more endpoints for the Registry",
                    "Login failed",
                    JOptionPane.ERROR_MESSAGE);
                cb.loginFailed();
                return;
            }

            getExecutor().submit(() ->
                       {
                           synchronized(Coordinator.this)
                           {
                               try
                               {
                                   LocatorFinderPrx finder = LocatorFinderPrx.uncheckedCast(
                                       _communicator.stringToProxy(finderStr));

                                   info.setInstanceName(finder.getLocator().ice_getIdentity().category);
                                   info.save();

                                   //
                                   // The client uses the locator only without routing
                                   //
                                   cb.setLocator(com.zeroc.IceGrid.LocatorPrx.checkedCast(
                                                     finder.ice_identity(
                                                         new com.zeroc.Ice.Identity("Locator", info.getInstanceName()))));

                                   if(cb.getLocator() == null)
                                   {
                                       SwingUtilities.invokeLater(() ->
                                                                  {
                                                                      JOptionPane.showMessageDialog(
                                                                          parent,
                                                                          "This version of IceGrid GUI requires an IceGrid Registry "
                                                                          + "version 3.3 or higher",
                                                                          "Version Mismatch",
                                                                          JOptionPane.ERROR_MESSAGE);
                                                                      cb.loginFailed();
                                                                  });
                                       return;
                                   }
                                   cb.setCurrentRegistry(cb.getLocator().getLocalRegistry());
                                   _communicator.setDefaultLocator(cb.getLocator());
                               }
                               catch(final java.util.prefs.BackingStoreException ex)
                               {
                                   SwingUtilities.invokeLater(() ->
                                                              {
                                                                  JOptionPane.showMessageDialog(
                                                                      getMainFrame(),
                                                                      ex.toString(),
                                                                      "Error saving connection",
                                                                      JOptionPane.ERROR_MESSAGE);
                                                              });
                                   return;
                               }
                               catch(final com.zeroc.Ice.LocalException e)
                               {
                                   SwingUtilities.invokeLater(() ->
                                                              {
                                                                  JOptionPane.showMessageDialog(
                                                                      parent,
                                                                      "Could not create session: " + e.toString(),
                                                                      "Login failed",
                                                                      JOptionPane.ERROR_MESSAGE);
                                                                  cb.loginFailed();
                                                              });
                                   return;
                               }

                               cb.setRegistry(cb.getCurrentRegistry());
                               if(info.getConnectToMaster() &&
                                  !cb.getCurrentRegistry().ice_getIdentity().name.equals("Registry"))
                               {
                                   com.zeroc.Ice.Identity masterRegistryId = new com.zeroc.Ice.Identity();
                                   masterRegistryId.category = info.getInstanceName();
                                   masterRegistryId.name = "Registry";

                                   cb.setRegistry(RegistryPrx.uncheckedCast(_communicator.stringToProxy(
                                                                                "\"" + _communicator.identityToString(masterRegistryId) +
                                                                                "\"")));
                               }

                               //
                               // If the registry to use is the locator local registry, we install a default router
                               // to ensure we'll use a single connection regardless of the endpoints returned in the
                               // proxies of the various session/admin methods (useful if used over a ssh tunnel).
                               //
                               if(cb.getRegistry().ice_getIdentity().equals(cb.getCurrentRegistry().ice_getIdentity()))
                               {
                                   try
                                   {
                                       com.zeroc.Ice.ObjectAdapter colloc = _communicator.createObjectAdapter("");
                                       com.zeroc.Ice.ObjectPrx router =
                                           colloc.addWithUUID(new ReuseConnectionRouter(cb.getLocator()));
                                       _communicator.setDefaultRouter(com.zeroc.Ice.RouterPrx.uncheckedCast(router));
                                       cb.setRegistry(cb.getRegistry().ice_router(_communicator.getDefaultRouter()));
                                   }
                                   catch(final com.zeroc.Ice.LocalException e)
                                   {
                                       SwingUtilities.invokeLater(() ->
                                                                  {
                                                                      JOptionPane.showMessageDialog(parent, "Could not create session: " +
                                                                                                    e.toString(), "Login failed",
                                                                                                    JOptionPane.ERROR_MESSAGE);
                                                                      cb.loginFailed();
                                                                  });
                                       return;
                                   }
                               }
                               do
                               {
                                   try
                                   {
                                       if(info.getAuth() == SessionKeeper.AuthType.X509CertificateAuthType)
                                       {
                                           cb.setRegistry(cb.getRegistry().ice_secure(true));
                                           cb.setSession(cb.getRegistry().createAdminSessionFromSecureConnection());
                                           assert cb.getSession() != null;
                                       }
                                       else
                                       {
                                           cb.setRegistry(cb.getRegistry().ice_preferSecure(true));

                                           cb.setSession(cb.getRegistry().createAdminSession(info.getUsername(),
                                                                                             info.getPassword() != null ? new String(info.getPassword()) : ""));
                                           assert cb.getSession() != null;
                                       }
                                       cb.setACMTimeout(cb.getRegistry().getACMTimeout());
                                   }
                                   catch(final com.zeroc.IceGrid.PermissionDeniedException e)
                                   {
                                       SwingUtilities.invokeLater(() ->
                                                                  {
                                                                      String msg = e.reason;
                                                                      if(msg.length() == 0)
                                                                      {
                                                                          msg = info.getAuth() == SessionKeeper.AuthType.X509CertificateAuthType ?
                                                                              "Invalid credentials" : "Invalid username/password";
                                                                      }

                                                                      if(info.getAuth() == SessionKeeper.AuthType.X509CertificateAuthType)
                                                                      {
                                                                          JOptionPane.showMessageDialog(parent, "Permission denied: " + e.reason,
                                                                                                        "Login failed",
                                                                                                        JOptionPane.ERROR_MESSAGE);
                                                                          cb.loginFailed();
                                                                      }
                                                                      else
                                                                      {
                                                                          cb.permissionDenied(msg);
                                                                      }
                                                                  });
                                       return;
                                   }
                                   catch(final com.zeroc.Ice.LocalException e)
                                   {
                                       if(cb.getRegistry().ice_getIdentity().equals(
                                              cb.getCurrentRegistry().ice_getIdentity()))
                                       {
                                           SwingUtilities.invokeLater(() ->
                                                                      {
                                                                          JOptionPane.showMessageDialog(parent, "Could not create session: " +
                                                                                                        e.toString(), "Login failed",
                                                                                                        JOptionPane.ERROR_MESSAGE);
                                                                          cb.loginFailed();
                                                                      });
                                           return;
                                       }
                                       else
                                       {
                                           while(true)
                                           {
                                               try
                                               {
                                                   SwingUtilities.invokeAndWait(() ->
                                                                                {
                                                                                    if(JOptionPane.showConfirmDialog(
                                                                                           parent,
                                                                                           "Unable to connect to the Master Registry:\n " +
                                                                                           e.toString() +
                                                                                           "\n\nDo you want to connect to a Slave Registry?",
                                                                                           "Cannot connect to Master Registry",
                                                                                           JOptionPane.YES_NO_OPTION,
                                                                                           JOptionPane.QUESTION_MESSAGE) == JOptionPane.YES_OPTION)
                                                                                    {
                                                                                        cb.setRegistry(cb.getCurrentRegistry());
                                                                                    }
                                                                                    else
                                                                                    {
                                                                                        cb.loginFailed();
                                                                                    }
                                                                                });
                                                   break;
                                               }
                                               catch(java.lang.InterruptedException ex)
                                               {
                                                   // Ignore and retry
                                               }
                                               catch(java.lang.reflect.InvocationTargetException ex)
                                               {
                                                   cb.loginFailed();
                                                   break;
                                               }
                                           }
                                           if(cb.failed())
                                           {
                                               return;
                                           }
                                       }
                                   }
                               } while(cb.getSession() == null);

                               cb.setReplicaName(cb.getSession().getReplicaName());
                               SwingUtilities.invokeLater(() -> cb.loginSuccess());
                           }
                       });
        }
    }

    void destroySession(AdminSessionPrx session, boolean routed)
    {
        _liveDeploymentRoot.closeAllShowLogDialogs();

        try
        {
            if(!routed)
            {
                session.destroyAsync();
            }
            else
            {
                com.zeroc.Glacier2.RouterPrx.uncheckedCast(_communicator.getDefaultRouter()).destroySessionAsync();
            }
        }
        catch(com.zeroc.Ice.LocalException e)
        {
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

            boolean refresh = true;
            if(getCurrentTab() instanceof ApplicationPane)
            {
                ApplicationPane app = (ApplicationPane)getCurrentTab();
                if(!app.applyUpdates(true))
                {
                    _substitute = !_substitute;
                    refresh = false;
                }
            }

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

            if(refresh)
            {
                getCurrentTab().refresh();
            }
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

    public String getServerAdminCategory()
    {
        return _sessionKeeper.getServerAdminCategory();
    }

    public com.zeroc.Ice.ObjectPrx addCallback(com.zeroc.Ice.Object servant, String name, String facet)
    {
        return _sessionKeeper.addCallback(servant, name, facet);
    }

    public com.zeroc.Ice.ObjectPrx retrieveCallback(String name, String facet)
    {
        return _sessionKeeper.retrieveCallback(name, facet);
    }

    public com.zeroc.Ice.Object removeCallback(String name, String facet)
    {
        return _sessionKeeper.removeCallback(name, facet);
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
            FileParserPrx fileParser = FileParserPrx.checkedCast(
                getCommunicator().stringToProxy(_fileParser).ice_router(null));
            return fileParser.parse(file.getAbsolutePath(), _sessionKeeper.getRoutedAdmin());
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
        catch(com.zeroc.Ice.LocalException e)
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
            {
            }
            _icegridadminProcess = null;
            _fileParser = null;
        }
    }

    public File saveToFile(boolean ask, com.zeroc.IceGridGUI.Application.Root root, File file)
    {
        if(ask || file == null)
        {
            if(file != null)
            {
                _saveXMLChooser.setSelectedFile(file);
            }
            else
            {
                _saveXMLChooser.setCurrentDirectory(_openChooser.getCurrentDirectory());
            }

            int result = _saveXMLChooser.showSaveDialog(_mainFrame);

            if(file == null || result == JFileChooser.APPROVE_OPTION)
            {
                _openChooser.setCurrentDirectory(_saveXMLChooser.getCurrentDirectory());
            }

            if(result == JFileChooser.APPROVE_OPTION)
            {
                file = _saveXMLChooser.getSelectedFile();
            }
            else
            {
                file = null;
            }
        }
        if(file != null)
        {
            if(!file.exists() && file.getName().indexOf('.') == -1)
            {
                file = new File(file.getAbsolutePath() + ".xml");
            }

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

    public JFileChooser getSaveLogFileChooser()
    {
        return _saveLogFileChooser;
    }

    public JFileChooser getSaveIceLogChooser()
    {
        return _saveIceLogChooser;
    }

    static private com.zeroc.Ice.Properties createProperties(String[] args, java.util.List<String> rArgs)
    {
        com.zeroc.Ice.Properties properties = com.zeroc.Ice.Util.createProperties();

        //
        // Set various default values
        //
        properties.setProperty("Ice.Override.ConnectTimeout", "5000");

        //
        // Disable retries
        //
        properties.setProperty("Ice.RetryIntervals", "-1");

        return com.zeroc.Ice.Util.createProperties(args, properties, rArgs);
    }

    Coordinator(JFrame mainFrame, String[] args, Preferences prefs)
    {
        _connected = false;
        _mainFrame = mainFrame;
        _prefs = prefs;
        _initData = new com.zeroc.Ice.InitializationData();

        _initData.logger = new Logger(mainFrame);
        java.util.List<String> rArgs = new java.util.ArrayList<>();
        _initData.properties = createProperties(args, rArgs);
        //
        // We enable IceSSL so the communicator knows how to parse ssl endpoints.
        //
        _initData.properties.setProperty("Ice.Plugin.IceSSL", "com.zeroc.IceSSL.PluginFactory");

        if(!rArgs.isEmpty())
        {
            String msg = "Extra command-line arguments: ";
            for(String arg : rArgs)
            {
                msg += arg + " ";
            }
            _initData.logger.warning(msg);
        }

        _traceObservers = _initData.properties.getPropertyAsInt("IceGridAdmin.Trace.Observers") > 0;
        _traceSaveToRegistry = _initData.properties.getPropertyAsInt("IceGridAdmin.Trace.SaveToRegistry") > 0;

        _liveDeploymentRoot = new com.zeroc.IceGridGUI.LiveDeployment.Root(this);

        _sessionKeeper = new SessionKeeper(this);

        _shutdownHook = new Thread("Shutdown hook")
            {
                @Override
                public void run()
                {
                    destroyIceGridAdmin();
                    destroyCommunicator();
                }
            };

        try
        {
            Runtime.getRuntime().addShutdownHook(_shutdownHook);
        }
        catch(IllegalStateException e)
        {
            //
            // Shutdown in progress, ignored
            //
        }

        _saveXMLChooser = new JFileChooser(_prefs.get("current directory", null));
        _saveXMLChooser.addChoosableFileFilter(new FileFilter()
            {
                @Override
                public boolean accept(File f)
                {
                    return f.isDirectory() || f.getName().endsWith(".xml");
                }

                @Override
                public String getDescription()
                {
                    return ".xml files";
                }
            });

        _openChooser = new JFileChooser(_saveXMLChooser.getCurrentDirectory());
        _openChooser.addChoosableFileFilter(_saveXMLChooser.getChoosableFileFilters()[1]);

        _saveLogFileChooser = new JFileChooser(_prefs.get("current directory", null));
        _saveLogFileChooser.addChoosableFileFilter(new FileFilter()
            {
                @Override
                public boolean accept(File f)
                {
                    return f.isDirectory() ||
                        f.getName().endsWith(".out") ||
                        f.getName().endsWith(".err") ||
                        f.getName().endsWith(".log") ||
                        f.getName().endsWith(".txt");
                }

                @Override
                public String getDescription()
                {
                    return ".out .err .log .txt files";
                }
            });

        _saveIceLogChooser = new JFileChooser(_prefs.get("current directory", null));
        _saveIceLogChooser.addChoosableFileFilter(new FileFilter()
            {
                @Override
                public boolean accept(File f)
                {
                    return f.isDirectory() || f.getName().endsWith(".csv");
                }

                @Override
                public String getDescription()
                {
                    return ".cvs files";
                }
            });

        javax.swing.UIManager.put("FileChooser.readOnly", Boolean.TRUE);

        final int MENU_MASK = Toolkit.getDefaultToolkit().getMenuShortcutKeyMask();

        //
        // Common actions (nodes not involved)
        //
        _newApplication = new AbstractAction("Application")
            {
                @Override
                public void actionPerformed(ActionEvent e)
                {
                    newApplication();
                }
            };

        _newApplicationWithDefaultTemplates =
            new AbstractAction("Application with Default Templates from Registry")
            {
                @Override
                public void actionPerformed(ActionEvent e)
                {
                    newApplicationWithDefaultTemplates();
                }
            };
        _newApplicationWithDefaultTemplates.setEnabled(false);

        _login = new AbstractAction("Login...")
            {
                @Override
                public void actionPerformed(ActionEvent e)
                {
                    _sessionKeeper.connectionManager();
                }
            };
        _login.putValue(Action.SHORT_DESCRIPTION, "Log into an IceGrid Registry");

        _logout = new AbstractAction("Logout")
            {
                @Override
                public void actionPerformed(ActionEvent e)
                {
                    if(_graphViews.size() > 0)
                    {
                        if(JOptionPane.YES_OPTION != JOptionPane.showConfirmDialog(
                               getMainFrame(), "Close all open Metrics Graph Views and logout?", "Confirm logout",
                               JOptionPane.YES_NO_OPTION))
                        {
                            return;
                        }

                        java.util.List<IGraphView> views = new java.util.ArrayList<>(_graphViews);
                        for(IGraphView v : views)
                        {
                            v.close();
                        }
                    }
                    _sessionKeeper.logout(true);
                    _statusBar.setText("Logged out");
                }
            };
        _logout.putValue(Action.SHORT_DESCRIPTION, "Logout");
        _logout.setEnabled(false);

        _acquireExclusiveWriteAccess = new AbstractAction("Acquire Exclusive Write Access")
            {
                @Override
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

        _releaseExclusiveWriteAccess = new AbstractAction("Release Exclusive Write Access")
            {
                @Override
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

        _newGraph = new AbstractAction("Metrics Graph")
            {
                @Override
                public void actionPerformed(ActionEvent e)
                {
                    createGraphView();
                }
            };
        _newGraph.setEnabled(false);

        _showLiveDeploymentFilters = new AbstractAction("Filter live deployment")
            {
                @Override
                public void actionPerformed(ActionEvent e)
                {

                    Object[] applicationNames = _liveDeploymentRoot.getApplicationNames();

                    if(applicationNames.length == 0)
                    {
                        JOptionPane.showMessageDialog(
                            _mainFrame,
                            "The registry does not contain any applications",
                            "Empty registry",
                            JOptionPane.INFORMATION_MESSAGE);
                    }
                    else
                    {
                        java.util.List<Object> names = new java.util.ArrayList<>();
                        names.add("<All>");
                        names.addAll(java.util.Arrays.asList(applicationNames));
                        String appName = (String)JOptionPane.showInputDialog(
                            _mainFrame, "Which Application do you want to see in the live deployment?",
                            "Filter live deployment",
                            JOptionPane.QUESTION_MESSAGE, null,
                            names.toArray(), names.get(0));

                        if(appName != null)
                        {
                            if(appName.equals("<All>"))
                            {
                                _liveDeploymentRoot.setApplicationNameFilter(null);
                            }
                            else
                            {
                                _liveDeploymentRoot.setApplicationNameFilter(appName);
                            }
                        }
                    }
                }
            };
        _showLiveDeploymentFilters.putValue(Action.SHORT_DESCRIPTION, "Filter live deployment");
        _showLiveDeploymentFilters.setEnabled(false);

        _openApplicationFromFile = new AbstractAction("Application from File")
            {
                @Override
                public void actionPerformed(ActionEvent e)
                {
                    int result = _openChooser.showOpenDialog(_mainFrame);
                    if(result == JFileChooser.APPROVE_OPTION)
                    {
                        File file = _openChooser.getSelectedFile();

                        ApplicationDescriptor desc = parseFile(file);

                        if(desc != null)
                        {
                            com.zeroc.IceGridGUI.Application.Root root;
                            try
                            {
                                root = new com.zeroc.IceGridGUI.Application.Root(Coordinator.this, desc, false, file);
                            }
                            catch(com.zeroc.IceGridGUI.Application.UpdateFailedException ex)
                            {
                                JOptionPane.showMessageDialog(
                                    _mainFrame,
                                    ex.toString(),
                                    "Bad Application Descriptor: Unable to load from file",
                                    JOptionPane.ERROR_MESSAGE);
                                return;
                            }

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

        _openApplicationFromRegistry = new AbstractAction("Application from Registry")
            {
                @Override
                public void actionPerformed(ActionEvent e)
                {
                    Object[] applicationNames = _liveDeploymentRoot.getApplicationNames();

                    if(applicationNames.length == 0)
                    {
                        JOptionPane.showMessageDialog(
                            _mainFrame,
                            "The registry does not contain any applications",
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
                            ApplicationPane app = openLiveApplication(appName);
                            if(app != null)
                            {
                                com.zeroc.IceGridGUI.Application.Root root = app.getRoot();
                                if(root.getSelectedNode() == null)
                                {
                                    root.setSelectedNode(root);
                                }
                            }
                        }
                    }
                }
            };
        _openApplicationFromRegistry.putValue(Action.SHORT_DESCRIPTION, "Open application from registry");
        _openApplicationFromRegistry.setEnabled(false);

        _closeApplication = new AbstractAction("Close Application")
            {
                @Override
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

        if(System.getProperty("os.name").startsWith("Mac OS"))
        {
            _closeApplication.putValue(Action.ACCELERATOR_KEY,
                                       KeyStroke.getKeyStroke(KeyEvent.VK_W, MENU_MASK));
        }
        else
        {
            _closeApplication.putValue(Action.ACCELERATOR_KEY,
                                       KeyStroke.getKeyStroke(KeyEvent.VK_F4, MENU_MASK));
        }
        _closeApplication.setEnabled(false);

        _save = new AbstractAction("Save")
            {
                @Override
                public void actionPerformed(ActionEvent e)
                {
                    try
                    {
                        getCurrentTab().save();
                    }
                    catch(Exception ex)
                    {
                        JOptionPane.showMessageDialog(_mainFrame, ex.toString(), "Error saving application",
                                                      JOptionPane.ERROR_MESSAGE);
                    }
                }
            };
        _save.setEnabled(false);
        _save.putValue(Action.ACCELERATOR_KEY,
                       KeyStroke.getKeyStroke(KeyEvent.VK_S, MENU_MASK));
        _save.putValue(Action.SHORT_DESCRIPTION, "Save");

        _saveToRegistry = new AbstractAction("Save to Registry (Servers may restart)")
            {
                @Override
                public void actionPerformed(ActionEvent e)
                {
                    getCurrentTab().saveToRegistry(true);
                }
            };
        _saveToRegistry.setEnabled(false);
        _saveToRegistry.putValue(Action.SHORT_DESCRIPTION, "Save to registry (servers may restart)");

        _saveToRegistryWithoutRestart = new AbstractAction("Save to Registry (No server restart)")
            {
                @Override
                public void actionPerformed(ActionEvent e)
                {
                    getCurrentTab().saveToRegistry(false);
                }
            };
        _saveToRegistryWithoutRestart.setEnabled(false);
        _saveToRegistryWithoutRestart.putValue(Action.SHORT_DESCRIPTION, "Save to registry (no server restart)");

        _saveToFile = new AbstractAction("Save to File")
            {
                @Override
                public void actionPerformed(ActionEvent e)
                {
                    try
                    {
                        getCurrentTab().saveToFile();
                    }
                    catch(Exception ex)
                    {
                        JOptionPane.showMessageDialog(_mainFrame, ex.toString(), "Error saving application",
                                                      JOptionPane.ERROR_MESSAGE);
                    }
                }
            };
        _saveToFile.setEnabled(false);
        _saveToFile.putValue(Action.SHORT_DESCRIPTION, "Save to file");

        _discardUpdates = new AbstractAction("Discard Updates")
            {
                @Override
                public void actionPerformed(ActionEvent e)
                {
                    getCurrentTab().discardUpdates();
                }
            };
        _discardUpdates.setEnabled(false);
        _discardUpdates.putValue(Action.SHORT_DESCRIPTION, "Discard updates and reload application");

        _certificateManager = new AbstractAction("Certificate Manager...")
            {
                @Override
                public void actionPerformed(ActionEvent e)
                {
                    SessionKeeper.CertificateManagerDialog d = _sessionKeeper.certificateManager(getMainFrame());
                    if(d != null)
                    {
                        d.showDialog();
                    }
                }
            };
        _certificateManager.putValue(Action.SHORT_DESCRIPTION, "Manage SSL Certificates");

        _exit = new AbstractAction("Exit")
            {
                @Override
                public void actionPerformed(ActionEvent e)
                {
                    exit(0);
                }
            };
        _exit.putValue(Action.ACCELERATOR_KEY, KeyStroke.getKeyStroke("alt F4"));

        _back = new AbstractAction("Go Back to the Previous Node")
            {
                @Override
                public void actionPerformed(ActionEvent e)
                {
                    getCurrentTab().back();
                }
            };
        _back.setEnabled(false);
        _back.putValue(Action.SHORT_DESCRIPTION, "Go back to the previous node");

        _forward =  new AbstractAction("Go to the Next Node")
            {
                @Override
                public void actionPerformed(ActionEvent e)
                {
                    getCurrentTab().forward();
                }
            };
        _forward.setEnabled(false);
        _forward.putValue(Action.SHORT_DESCRIPTION, "Go to the next node");

        _helpContents = new AbstractAction("Contents")
            {
                @Override
                public void actionPerformed(ActionEvent e)
                {
                    helpContents();
                }
            };

        _about = new AbstractAction("About")
            {
                @Override
                public void actionPerformed(ActionEvent e)
                {
                    about();
                }
            };

        _showApplicationDetails = new AbstractAction("Show details")
            {
                @Override
                public void actionPerformed(ActionEvent e)
                {
                    Object[] applicationNames = _liveDeploymentRoot.getApplicationNames();

                    if(applicationNames.length == 0)
                    {
                        JOptionPane.showMessageDialog(
                            _mainFrame,
                            "There is no application deployed in this IceGrid registry",
                            "No application",
                            JOptionPane.INFORMATION_MESSAGE);
                    }
                    else
                    {
                        String appName = _liveDeploymentRoot.getApplicationNameFilter();
                        if(appName == null)
                        {
                            appName = (String)JOptionPane.showInputDialog(
                                _mainFrame, "Which application do you to display",
                                "Show details",
                                JOptionPane.QUESTION_MESSAGE, null,
                                applicationNames, applicationNames[0]);
                        }

                        if(appName != null)
                        {
                            _liveDeploymentRoot.showApplicationDetails(appName);
                        }
                    }
                }
            };
        _showApplicationDetails.setEnabled(false);

        _removeApplicationFromRegistry = new AbstractAction("Remove from Registry")
            {
                @Override
                public void actionPerformed(ActionEvent e)
                {
                    Object[] applicationNames = _liveDeploymentRoot.getApplicationNames();

                    if(applicationNames.length == 0)
                    {
                        JOptionPane.showMessageDialog(
                            _mainFrame,
                            "There is no application deployed in this IceGrid registry",
                            "No application",
                            JOptionPane.INFORMATION_MESSAGE);
                    }
                    else
                    {
                        String appName = (String)JOptionPane.showInputDialog(
                            _mainFrame, "Which application do you want to remove?",
                            "Remove application",
                            JOptionPane.QUESTION_MESSAGE, null,
                            applicationNames, applicationNames[0]);

                        if(appName != null)
                        {
                            removeApplicationFromRegistry(appName);
                        }
                    }
                }
            };
        _removeApplicationFromRegistry.setEnabled(false);

        _cut = new ActionWrapper("Cut");
        _cut.putValue(Action.ACCELERATOR_KEY, KeyStroke.getKeyStroke(KeyEvent.VK_X, MENU_MASK));
        _cut.putValue(Action.SHORT_DESCRIPTION, "Cut");

        _copy = new ActionWrapper("Copy");
        _copy.putValue(Action.ACCELERATOR_KEY, KeyStroke.getKeyStroke(KeyEvent.VK_C, MENU_MASK));
        _copy.putValue(Action.SHORT_DESCRIPTION, "Copy");

        _paste = new ActionWrapper("Paste");
        _paste.putValue(Action.ACCELERATOR_KEY, KeyStroke.getKeyStroke(KeyEvent.VK_V, MENU_MASK));
        _paste.putValue(Action.SHORT_DESCRIPTION, "Paste");

        _delete = new ActionWrapper("Delete");
        _delete.putValue(Action.ACCELERATOR_KEY, KeyStroke.getKeyStroke("DELETE"));
        _delete.putValue(Action.SHORT_DESCRIPTION, "Delete");

        _moveUp = new ActionWrapper("Move Up");
        _moveDown = new ActionWrapper("Move Down");

        _showVarsMenuItem =
            new JCheckBoxMenuItem(_appActionsForMenu.get(com.zeroc.IceGridGUI.Application.TreeNode.SHOW_VARS));
        _showVarsTool = new JToggleButton(_appActionsForMenu.get(com.zeroc.IceGridGUI.Application.TreeNode.SHOW_VARS));
        _showVarsTool.setIcon(Utils.getIcon("/icons/24x24/show_vars.png"));
        _showVarsTool.setText("");

        _substituteMenuItem = new
            JCheckBoxMenuItem(_appActionsForMenu.get(com.zeroc.IceGridGUI.Application.TreeNode.SUBSTITUTE_VARS));
        _substituteTool =
            new JToggleButton(_appActionsForMenu.get(com.zeroc.IceGridGUI.Application.TreeNode.SUBSTITUTE_VARS));
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

        _mainFrame.getContentPane().add(new ToolBar(), BorderLayout.PAGE_START);

        _mainFrame.getContentPane().add(_statusBar, BorderLayout.PAGE_END);

        java.awt.KeyboardFocusManager kbm = java.awt.KeyboardFocusManager.getCurrentKeyboardFocusManager();
        kbm.addPropertyChangeListener("permanentFocusOwner", new FocusListener());

        _liveDeploymentPane = new LiveDeploymentPane(_liveDeploymentRoot);
        _mainPane = new MainPane(this);
        _mainFrame.getContentPane().add(_mainPane, BorderLayout.CENTER);

        ScheduledThreadPoolExecutor executor =
            new ScheduledThreadPoolExecutor(1, (Runnable r) -> new Thread(r, "Pinger"));
        executor.setExecuteExistingDelayedTasksAfterShutdownPolicy(false);
        _scheduledExecutor = executor;
        _executor = Executors.newSingleThreadExecutor((Runnable r) -> new Thread(r));
    }

    public IGraphView createGraphView()
    {
        IGraphView view = null;
        Class<?> c1 = com.zeroc.IceInternal.Util.findClass("com.zeroc.IceGridGUI.LiveDeployment.GraphView", null);
        if(c1 == null)
        {
            JOptionPane.showMessageDialog(_mainFrame,
                                          "IceGrid GUI was built without Metrics Graph Support",
                                          "IceGrid GUI Info",
                                          JOptionPane.INFORMATION_MESSAGE);
        }
        else if(com.zeroc.IceInternal.Util.findClass("javafx.embed.swing.JFXPanel", null) == null)
        {
            JOptionPane.showMessageDialog(_mainFrame,
                                          "The Metrics Graph view requires JavaFX",
                                          "IceGrid GUI Info",
                                          JOptionPane.INFORMATION_MESSAGE);
        }
        else
        {
            StringBuilder title = new StringBuilder();
            title.append("Metrics Graph");
            if(_graphViews.size() > 0)
            {
                title.append(" - ");
                title.append(Integer.toString(_graphViews.size()));
            }

            try
            {
                Constructor<?> ctor = c1.getConstructor(new Class[] { Coordinator.class, String.class });
                view = (IGraphView)ctor.newInstance(new Object[] { Coordinator.this, title.toString() });
                _graphViews.add(view);
            }
            catch(NoSuchMethodException ex)
            {
            }
            catch(InstantiationException ex)
            {
            }
            catch(IllegalAccessException ex)
            {
            }
            catch(java.lang.reflect.InvocationTargetException ex)
            {
            }
        }
        return view;
    }

    public LiveDeploymentPane getLiveDeploymentPane()
    {
        return _liveDeploymentPane;
    }

    public com.zeroc.IceGridGUI.LiveDeployment.Root getLiveDeploymentRoot()
    {
        return _liveDeploymentRoot;
    }

    private void newApplication()
    {
        ApplicationDescriptor desc =  new ApplicationDescriptor("NewApplication",
                                                                new java.util.TreeMap<String, String>(),
                                                                new java.util.LinkedList<ReplicaGroupDescriptor>(),
                                                                new java.util.HashMap<String, TemplateDescriptor>(),
                                                                new java.util.HashMap<String, TemplateDescriptor>(),
                                                                new java.util.HashMap<String, NodeDescriptor>(),
                                                                "",
                                                                new java.util.HashMap<String, PropertySetDescriptor>());
        com.zeroc.IceGridGUI.Application.Root root = new com.zeroc.IceGridGUI.Application.Root(this, desc);
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
            com.zeroc.IceGridGUI.Application.Root root = new com.zeroc.IceGridGUI.Application.Root(this, descriptor);
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
        catch(com.zeroc.Ice.LocalException e)
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
        String version = com.zeroc.Ice.Util.stringVersion();

        int pos = version.indexOf('a');
        if(pos == -1)
        {
            pos = version.indexOf('b');
        }

        if(pos != -1)
        {
            // 3.7a3 or 3.7b1 becomes simply 3.7
            version = version.substring(0, pos);
        }

        String[] tokens = version.split(".");
        if(tokens.length > 2)
        {
            version = tokens[0] + "." + tokens[1];
        }

        Desktop desktop = Desktop.isDesktopSupported() ? Desktop.getDesktop() : null;
        if(desktop != null && desktop.isSupported(Desktop.Action.BROWSE))
        {
            try
            {
                desktop.browse(new URI("https://doc.zeroc.com/ice/" + version + "/ice-services/icegrid/icegrid-gui-tool"));
            }
            catch(Exception e)
            {
                JOptionPane.showMessageDialog(null,
                                              "Error attempting to launch web browser" + ":\n" + e.getLocalizedMessage());
            }
        }
        else
        {
            JOptionPane.showMessageDialog(null,"Launching a browser is not supported on your platform.");
        }
    }

    private void about()
    {
        String text = "IceGrid GUI version "
            + com.zeroc.Ice.Util.stringVersion() + "\n"
            + "Copyright \u00A9 ZeroC, Inc. All rights reserved.\n";

        JOptionPane.showMessageDialog(
            _mainFrame,
            text,
            "About - IceGrid GUI",
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
        _mainFrame.pack();
        Utils.restoreWindowBounds(_mainFrame, _prefs, "Window", null);
        _mainFrame.setVisible(true);
    }

    void exit(int status)
    {
        if(_graphViews.size() > 0)
        {
            if(JOptionPane.YES_OPTION != JOptionPane.showConfirmDialog(getMainFrame(),
                                                                       "Close all open windows and exit?",
                                                                       "Confirm exit",
                                                                       JOptionPane.YES_NO_OPTION))
            {
                return;
            }

            java.util.List<IGraphView> views = new java.util.ArrayList<>(_graphViews);
            for(IGraphView v : views)
            {
                v.close();
            }
        }

        if(_openChooser != null)
        {
            File dir = _openChooser.getCurrentDirectory();
            if(dir != null)
            {
                _prefs.put("current directory", dir.getAbsolutePath());
            }
        }

        Utils.storeWindowBounds(_mainFrame, _prefs.node("Window"));

        _sessionKeeper.logout(true);

        destroyIceGridAdmin();
        destroyCommunicator();

        _scheduledExecutor.shutdown();
        _scheduledExecutor = null;

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
            _communicator.destroy();
            _communicator = null;
        }
    }

    public AdminSessionPrx getSession()
    {
        return _sessionKeeper.getSession();
    }

    public boolean connectedToMaster()
    {
        return _sessionKeeper.connectedToMaster();
    }

    public SessionKeeper getSessionKeeper()
    {
        return _sessionKeeper;
    }

    public static Preferences getPreferences()
    {
        return Preferences.userRoot().node("IceGridGUI");
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

    public void showActions(com.zeroc.IceGridGUI.LiveDeployment.TreeNode node)
    {
        boolean[] availableActions = _liveActionsForMenu.setTarget(node);
        _appActionsForMenu.setTarget(null);

        _newServerMenu.setEnabled(false);
        _newServiceMenu.setEnabled(false);
        _newTemplateMenu.setEnabled(false);

        _appMenu.setEnabled(true);

        _metricsViewMenu.setEnabled(availableActions[com.zeroc.IceGridGUI.LiveDeployment.TreeNode.ENABLE_METRICS_VIEW] ||
                                    availableActions[com.zeroc.IceGridGUI.LiveDeployment.TreeNode.DISABLE_METRICS_VIEW]);

        _nodeMenu.setEnabled(availableActions[com.zeroc.IceGridGUI.LiveDeployment.TreeNode.SHUTDOWN_NODE]);

        _registryMenu.setEnabled(availableActions[com.zeroc.IceGridGUI.LiveDeployment.TreeNode.SHUTDOWN_REGISTRY]);

        _signalMenu.setEnabled(availableActions[com.zeroc.IceGridGUI.LiveDeployment.TreeNode.SIGHUP]);

        _serverMenu.setEnabled(availableActions[com.zeroc.IceGridGUI.LiveDeployment.TreeNode.OPEN_DEFINITION]);

        _serviceMenu.setEnabled(node instanceof com.zeroc.IceGridGUI.LiveDeployment.Service &&
                                (availableActions[com.zeroc.IceGridGUI.LiveDeployment.TreeNode.RETRIEVE_ICE_LOG] ||
                                 availableActions[com.zeroc.IceGridGUI.LiveDeployment.TreeNode.RETRIEVE_LOG_FILE] ||
                                 availableActions[com.zeroc.IceGridGUI.LiveDeployment.TreeNode.START] ||
                                 availableActions[com.zeroc.IceGridGUI.LiveDeployment.TreeNode.STOP]));
    }

    public void showActions(com.zeroc.IceGridGUI.Application.TreeNode node)
    {
        boolean[] availableActions = _appActionsForMenu.setTarget(node);
        _liveActionsForMenu.setTarget(null);

        _newServerMenu.setEnabled(
            availableActions[com.zeroc.IceGridGUI.Application.TreeNode.NEW_SERVER] ||
            availableActions[com.zeroc.IceGridGUI.Application.TreeNode.NEW_SERVER_ICEBOX] ||
            availableActions[com.zeroc.IceGridGUI.Application.TreeNode.NEW_SERVER_FROM_TEMPLATE]);

        _newServiceMenu.setEnabled(
            availableActions[com.zeroc.IceGridGUI.Application.TreeNode.NEW_SERVICE] ||
            availableActions[com.zeroc.IceGridGUI.Application.TreeNode.NEW_SERVICE_FROM_TEMPLATE]);

        _newTemplateMenu.setEnabled(
            availableActions[com.zeroc.IceGridGUI.Application.TreeNode.NEW_TEMPLATE_SERVER] ||
            availableActions[com.zeroc.IceGridGUI.Application.TreeNode.NEW_TEMPLATE_SERVER_ICEBOX] ||
            availableActions[com.zeroc.IceGridGUI.Application.TreeNode.NEW_TEMPLATE_SERVICE]);

        _appMenu.setEnabled(false);
        _metricsViewMenu.setEnabled(false);
        _nodeMenu.setEnabled(false);
        _registryMenu.setEnabled(false);
        _signalMenu.setEnabled(false);
        _serverMenu.setEnabled(false);
        _serviceMenu.setEnabled(false);

        enableTreeEditActions();
    }

    public void removeGraphView(IGraphView view)
    {
        _graphViews.remove(view);
    }

    public boolean traceObservers()
    {
        return _traceObservers;
    }

    public void traceObserver(String message)
    {
        trace("Observers", message);
    }

    public boolean traceSaveToRegistry()
    {
        return _traceSaveToRegistry;
    }

    public void traceSaveToRegistry(String message)
    {
        trace("SaveToRegistry", message);
    }

    public void setConnected(boolean connected)
    {
        _connected = connected;
        _statusBar.setConnected(connected);
        _newGraph.setEnabled(connected);
        if(!connected)
        {
            destroyCommunicator();
        }
    }

    public boolean connected()
    {
        return _connected;
    }

    public void tryMigrateDataDirectory()
    {
        String oldDataDir = null;
        if(System.getProperty("os.name").startsWith("Windows"))
        {
            String regKey = "\"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders\"";
            String regQuery = "reg query " + regKey + " /v Personal";
            try
            {
                java.lang.Process process = Runtime.getRuntime().exec(regQuery);
                process.waitFor();
                if(process.exitValue() != 0)
                {
                    JOptionPane.showMessageDialog(getMainFrame(),
                                                  "Could not read Windows registry key `" + regKey + "'",
                                                  "Initialization Exception",
                                                  JOptionPane.ERROR_MESSAGE);
                    return;
                }

                java.io.InputStream is = process.getInputStream();
                java.io.StringWriter sw = new java.io.StringWriter();
                int c;
                while((c = is.read()) != -1)
                {
                    sw.write(c);
                }
                String[] result = sw.toString().split("\n");
                for(String line : result)
                {
                    int i = line.indexOf("REG_SZ");
                    if(i == -1)
                    {
                        continue;
                    }
                    oldDataDir = line.substring(i + "REG_SZ".length(), line.length()).trim();
                    break;
                }

                if(oldDataDir == null)
                {
                    JOptionPane.showMessageDialog(getMainFrame(),
                                              "Could not get Documents dir from Windows registry key `" + regKey + "'",
                                              "Initialization Exception",
                                              JOptionPane.ERROR_MESSAGE);
                    return;
                }
                oldDataDir = Paths.get(oldDataDir, "ZeroC",  "IceGrid Admin", "KeyStore").toString();
            }
            catch(java.io.IOException ex)
            {
                JOptionPane.showMessageDialog(getMainFrame(),
                                              "Could not read Windows registry key `" + regKey + "'\n" + ex.toString(),
                                              "Initialization Exception",
                                              JOptionPane.ERROR_MESSAGE);
                return;
            }
            catch(java.lang.InterruptedException ex)
            {
                JOptionPane.showMessageDialog(getMainFrame(),
                                              "Could not read Windows registry key `" + regKey + "'\n" + ex.toString(),
                                              "Initialization Exception",
                                              JOptionPane.ERROR_MESSAGE);
                return;
            }
        }
        else if(System.getProperty("os.name").startsWith("Mac OS"))
        {
            oldDataDir = Paths.get(System.getProperty("user.home"), ".ZeroC", "IceGrid Admin", "KeyStore").toString();
        }

        if(oldDataDir != null)
        {
            String dataDir = getDataDirectory();
            if(new File(dataDir).isDirectory() && new File(dataDir).list().length == 0 &&
               new File(oldDataDir).isDirectory() && new File(oldDataDir).list().length > 0)
            {
                for(File f : new File(oldDataDir).listFiles())
                {
                    try
                    {
                        Files.copy(Paths.get(oldDataDir, f.getName()),
                                   Paths.get(dataDir, f.getName()));
                        new File(Paths.get(oldDataDir, f.getName()).toString()).delete();
                    }
                    catch(java.io.IOException ex)
                    {
                        JOptionPane.showMessageDialog(getMainFrame(),
                                                      "Could not move `" +
                                                      Paths.get(oldDataDir, f.getName()).toString() + "' to " +
                                                      "`" + Paths.get(oldDataDir, f.getName()).toString() + "'",
                                                      "Initialization Exception",
                                                      JOptionPane.ERROR_MESSAGE);
                        return;
                    }
                }
            }
        }
    }

    public String getDataDirectory()
    {
        String dataDir = null;
        if(System.getProperty("os.name").startsWith("Windows"))
        {
            dataDir = Paths.get(System.getenv("APPDATA"), "..", "Local", "ZeroC",
                                "IceGrid Admin", "KeyStore").toString();
        }
        else if(System.getProperty("os.name").startsWith("Mac OS"))
        {
            dataDir = Paths.get(System.getProperty("user.home"), "Library", "Application Support", "ZeroC",
                                "IceGrid Admin", "KeyStore").toString();
        }
        else
        {
            dataDir = Paths.get(System.getProperty("user.home"), ".ZeroC", "IceGrid Admin", "KeyStore").toString();
        }

        if(!new File(dataDir).isDirectory())
        {
            new File(dataDir).mkdirs();
        }

        return dataDir;
    }

    public IGraphView[] getGraphViews()
    {
        return _graphViews.toArray(new IGraphView[_graphViews.size()]);
    }

    public ScheduledExecutorService getScheduledExecutor()
    {
        return _scheduledExecutor;
    }

    public ExecutorService getExecutor()
    {
        return _executor;
    }

    public boolean needsSaving()
    {
        boolean v = false;
        for(int i = 0, length = _mainPane.getTabCount(); i < length; ++i)
        {
            Component component = _mainPane.getComponentAt(i);
            if(component instanceof ApplicationPane)
            {
                if(((ApplicationPane)component).getRoot().needsSaving())
                {
                    v = true;
                    break;
                }
            }
        }
        return v;
    }

    //
    // May run in any thread
    //
    private void trace(String category, String message)
    {
        //
        // It would be nicer to use the communicator's logger,
        // but accessing _communicator is not thread-safe.
        //
        _initData.logger.trace(category, message);
    }

    static class UntrustedCertificateDialog extends JDialog
    {
        public UntrustedCertificateDialog(java.awt.Window owner, com.zeroc.IceSSL.ConnectionInfo info,
                                          boolean validDate, boolean validAlternateName, boolean trustedCA)
            throws java.security.GeneralSecurityException, java.io.IOException,
            javax.naming.InvalidNameException
        {
            super(owner, "Connection Security Warning - IceGrid GUI");
            setDefaultCloseOperation(WindowConstants.DISPOSE_ON_CLOSE);

            Container contentPane = getContentPane();
            contentPane.setLayout(new BoxLayout(contentPane, BoxLayout.Y_AXIS));

            X509Certificate cert = (X509Certificate)info.certs[0];
            {
                DefaultFormBuilder builder = new DefaultFormBuilder(new FormLayout("pref", "pref"));
                builder.border(Borders.DIALOG);
                builder.rowGroupingEnabled(true);
                builder.lineGapSize(LayoutStyle.getCurrent().getLinePad());

                builder.append(new JLabel("The validation of the SSL Certificate provided by the server has failed"));

                if(validDate)
                {
                    builder.append(new JLabel("The certificate date is valid.", _infoIcon, SwingConstants.LEADING));
                }
                else
                {
                    builder.append(new JLabel("The certificate date is invalid.", _warnIcon, SwingConstants.LEADING));
                }

                if(validAlternateName)
                {
                    builder.append(new JLabel("The subject alternate name matches the connection remote address.",
                                              _infoIcon, SwingConstants.LEADING));
                }
                else
                {
                    builder.append(new JLabel("The subject alternate name doesn't match the connection remote address.",
                                              _warnIcon, SwingConstants.LEADING));
                }

                if(trustedCA)
                {
                    builder.append(new JLabel("The server certificate is signed by a trusted CA.", _infoIcon,
                                              SwingConstants.LEADING));
                }
                else
                {
                    builder.append(new JLabel("The server certificate is not signed by a trusted CA.", _warnIcon,
                                              SwingConstants.LEADING));
                }
                contentPane.add(builder.getPanel());
            }

            contentPane.add(SessionKeeper.getSubjectPanel(cert));
            contentPane.add(SessionKeeper.getSubjectAlternativeNamesPanel(cert));
            contentPane.add(SessionKeeper.getIssuerPanel(cert));
            contentPane.add(SessionKeeper.getValidityPanel(cert));
            contentPane.add(SessionKeeper.getFingerprintPanel(cert));

            JButton yesAlwaysButton = new JButton(new AbstractAction("Yes, Always Trust")
                {
                    @Override
                    public void actionPerformed(ActionEvent e)
                    {
                        _decision = TrustDecision.YesAlways;
                        dispose();
                    }
                });

            JButton yesButton = new JButton(new AbstractAction("Yes, Just This Time")
                {
                    @Override
                    public void actionPerformed(ActionEvent e)
                    {
                        _decision = TrustDecision.YesThisTime;
                        dispose();
                    }
                });

            JButton noButton = new JButton(new AbstractAction("No")
                {
                    @Override
                    public void actionPerformed(ActionEvent e)
                    {
                        _decision = TrustDecision.No;
                        dispose();
                    }
                });
            getRootPane().setDefaultButton(noButton);
            JComponent buttonBar = new ButtonBarBuilder().addGlue().addButton(yesAlwaysButton, yesButton,
                                                                              noButton).build();
            buttonBar.setBorder(Borders.DIALOG);
            contentPane.add(buttonBar);
            pack();
            setResizable(false);
        }

        TrustDecision showDialog()
        {
            setLocationRelativeTo(getOwner());
            setModal(true);
            setVisible(true);
            return _decision;
        }

        private TrustDecision _decision = TrustDecision.No;
        private static Icon _infoIcon = new ImageIcon(
            Utils.iconToImage(UIManager.getIcon("OptionPane.informationIcon")).
            getScaledInstance(16, 16, java.awt.Image.SCALE_SMOOTH ));

        private static Icon _warnIcon = new ImageIcon(
            Utils.iconToImage(UIManager.getIcon("OptionPane.warningIcon")).
            getScaledInstance(16, 16, java.awt.Image.SCALE_SMOOTH ));
    }

    private final com.zeroc.Ice.InitializationData _initData;
    private com.zeroc.Ice.Communicator _communicator;

    private boolean _traceObservers;
    private boolean _traceSaveToRegistry;

    private Preferences _prefs;
    private StatusBarI _statusBar = new StatusBarI();

    private com.zeroc.IceGridGUI.LiveDeployment.Root _liveDeploymentRoot;
    private LiveDeploymentPane _liveDeploymentPane;

    //
    // Maps application-name to ApplicationPane (only for 'live' applications)
    //
    private java.util.Map<String, ApplicationPane> _liveApplications = new java.util.HashMap<>();

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
    private final SessionKeeper _sessionKeeper;

    private Object _clipboard;

    //
    // Actions
    //
    private Action _newApplication;
    private Action _newApplicationWithDefaultTemplates;
    private Action _login;
    private Action _logout;
    private Action _acquireExclusiveWriteAccess;
    private Action _releaseExclusiveWriteAccess;

    private Action _newGraph;

    private Action _showLiveDeploymentFilters;
    private Action _openApplicationFromFile;
    private Action _openApplicationFromRegistry;
    private Action _closeApplication;
    private Action _save;
    private Action _saveToRegistry;
    private Action _saveToRegistryWithoutRestart;
    private Action _saveToFile;
    private Action _discardUpdates;
    private Action _certificateManager;
    private Action _exit;
    private Action _back;
    private Action _forward;
    private Action _helpContents;
    private Action _about;
    private Action _showApplicationDetails;
    private Action _removeApplicationFromRegistry;

    private Action _cutText = new javax.swing.text.DefaultEditorKit.CutAction();
    private Action _copyText = new javax.swing.text.DefaultEditorKit.CopyAction();
    private Action _pasteText = new javax.swing.text.DefaultEditorKit.PasteAction();
    private DeleteTextAction _deleteText = new DeleteTextAction("Delete");

    //
    // These actions delegate to the "active" action
    //
    private ActionWrapper _cut;
    private ActionWrapper _copy;
    private ActionWrapper _paste;
    private ActionWrapper _delete;
    private ActionWrapper _moveUp;
    private ActionWrapper _moveDown;

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
    private JMenu _metricsViewMenu;
    private JMenu _nodeMenu;
    private JMenu _registryMenu;
    private JMenu _serverMenu;
    private JMenu _serviceMenu;
    private JMenu _signalMenu;

    private final Thread _shutdownHook;

    private JFileChooser _openChooser;
    private JFileChooser _saveXMLChooser;
    private JFileChooser _saveLogFileChooser;
    private JFileChooser _saveIceLogChooser;

    private Process _icegridadminProcess;
    private String _fileParser;
    private boolean _connected;

    private X509Certificate _transientCert;

    private java.util.List<IGraphView> _graphViews = new java.util.ArrayList<>();

    private ScheduledExecutorService _scheduledExecutor;
    private ExecutorService _executor;
}
