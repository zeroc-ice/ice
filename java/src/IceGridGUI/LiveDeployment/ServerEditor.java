// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGridGUI.LiveDeployment;

import java.awt.event.ActionEvent;

import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JComponent;
import javax.swing.JScrollPane;
import javax.swing.JTextField;
import javax.swing.JToolBar;

import com.jgoodies.forms.builder.DefaultFormBuilder;
import com.jgoodies.forms.layout.CellConstraints;

import com.jgoodies.looks.Options;
import com.jgoodies.looks.HeaderStyle;
import com.jgoodies.looks.BorderStyle;
import com.jgoodies.looks.plastic.PlasticLookAndFeel;

import IceGrid.*;
import IceGridGUI.*;

class ServerEditor extends CommunicatorEditor
{
    public JToolBar getToolBar()
    {
        if(_toolBar == null)
        {
            _toolBar = new ToolBar();
        }
        return _toolBar;
    }


    ServerEditor(Coordinator c)
    {
        _coordinator = c;
        _currentState.setEditable(false);
        _currentPid.setEditable(false);
        _enabled.setEnabled(false);
        
        _application.setEditable(false);
        _exe.setEditable(false);
        _pwd.setEditable(false);
   
        _activation.setEditable(false);
        _activationTimeout.setEditable(false);
        _deactivationTimeout.setEditable(false);

        _options.setEditable(false);
        _user.setEditable(false);
        
        _allocatable.setEnabled(false);
        _applicationDistrib.setEnabled(false);
        _icepatch.setEditable(false);
        _directories.setEditable(false);

        Action gotoApplication = new AbstractAction(
            "", Utils.getIcon("/icons/16x16/goto.png"))
            {
                public void actionPerformed(ActionEvent e) 
                {
                    ApplicationPane app = _coordinator.openLiveApplication(_application.getText());
                    if(app != null)
                    {
                        app.getRoot().
                            selectServer(((Node)_target.getParent()).getId(), _target.getId());
                    }
                }
            };
        gotoApplication.putValue(Action.SHORT_DESCRIPTION, 
                              "View/Edit this application");
        _gotoApplication = new JButton(gotoApplication);
    }


    void show(Server server)
    {
        _target = server;

        ServerState state = server.getState();
        if(state == null)
        {
            _currentState.setText("Unknown");
            _currentPid.setText("");
            _enabled.setSelected(false);
        }
        else
        {
            _currentState.setText(state.toString());
            int pid = server.getPid();
            if(pid == 0)
            {
                _currentPid.setText("");
            }
            else
            {
                _currentPid.setText(Integer.toString(pid));
            }
            _enabled.setSelected(server.isEnabled());
        }

        ServerDescriptor descriptor = server.getServerDescriptor();
        final Utils.Resolver resolver = server.getResolver();

        _application.setText(resolver.find("application"));

        super.show(descriptor, server.getProperties(), resolver);
        
        _exe.setText(resolver.substitute(descriptor.exe));
        _pwd.setText(resolver.substitute(descriptor.pwd));

        Ice.StringHolder toolTipHolder = new Ice.StringHolder();
        Utils.Stringifier stringifier =  new Utils.Stringifier()
            {
                public String toString(Object obj)
                {
                    return resolver.substitute((String)obj);
                }
            };
        
        _options.setText(
            Utils.stringify(descriptor.options, stringifier, " ", toolTipHolder));
        _options.setToolTipText(toolTipHolder.value);

        _envs.setEnvs(descriptor.envs, resolver);

        _user.setText(resolver.substitute(descriptor.user));

        _activation.setText(resolver.substitute(descriptor.activation));
        _activationTimeout.setText(resolver.substitute(descriptor.activationTimeout));
        _deactivationTimeout.setText(resolver.substitute(descriptor.deactivationTimeout));
        
        _allocatable.setSelected(descriptor.allocatable);

        _applicationDistrib.setSelected(descriptor.applicationDistrib);
        _icepatch.setText(resolver.substitute(resolver.substitute(descriptor.distrib.icepatch)));

        toolTipHolder = new Ice.StringHolder();
        
        _directories.setText(
            Utils.stringify(descriptor.distrib.directories, stringifier, ", ", 
                            toolTipHolder));

        String toolTip = "<html>Include only these directories";

        if(toolTipHolder.value != null)
        {
            toolTip += ":<br>" + toolTipHolder.value;
        }
        toolTip += "</html>";
        _directories.setToolTipText(toolTip);
    }


    protected void appendProperties(DefaultFormBuilder builder)
    {
        builder.appendSeparator("Runtime Status");

        builder.append("State");
        builder.append(_currentState, 3);
        builder.nextLine();

        builder.append("Process ID");
        builder.append(_currentPid, 3);
        builder.nextLine();
        
        builder.append("", _enabled);
        builder.nextLine();
        
        builder.appendSeparator("Configuration");

        builder.append("Application");
        builder.append(_application);
        builder.append(_gotoApplication);
        builder.nextLine();

        //
        // Add Communicator fields
        //
        super.appendProperties(builder);
        
        builder.appendSeparator("Activation");
        builder.append("Path to Executable");
        builder.append(_exe, 3);
        builder.nextLine();
        builder.append("Working Directory");
        builder.append(_pwd, 3);
        builder.nextLine();
        builder.append("Command Arguments");
        builder.append(_options, 3);
        builder.nextLine();
        builder.append("Run as");
        builder.append(_user, 3);
        builder.nextLine();
        builder.append("Environment Variables");
        builder.nextLine();
        builder.append("");
        builder.nextLine();
        builder.append("");
        builder.nextLine();
        builder.append("");
        builder.nextRow(-6);
        CellConstraints cc = new CellConstraints();
        JScrollPane scrollPane = new JScrollPane(_envs);
        builder.add(scrollPane, 
                    cc.xywh(builder.getColumn(), builder.getRow(), 3, 7));
        builder.nextRow(6);
        builder.nextLine();

        builder.append("Activation Mode");
        builder.append(_activation, 3);
        builder.nextLine();
        builder.append("Activation Timeout");
        builder.append(_activationTimeout, 3);
        builder.nextLine();
        builder.append("Deactivation Timeout");
        builder.append(_deactivationTimeout, 3);
        builder.nextLine();
        builder.append("", _allocatable);
        builder.nextLine();

        JComponent c = builder.appendSeparator("Distribution");
        c.setToolTipText("Files specific to this server");

        builder.append("", _applicationDistrib);
        builder.nextLine();
        builder.append("IcePatch2 Proxy");
        builder.append(_icepatch, 3);
        builder.nextLine();
        builder.append("Directories");
        builder.append(_directories, 3);
        builder.nextLine();
    }

    protected void buildPropertiesPanel()
    {
        super.buildPropertiesPanel();
        _propertiesPanel.setName("Server Properties");
    }


    private class ToolBar extends JToolBar
    {
        private ToolBar()
        {
            putClientProperty(Options.HEADER_STYLE_KEY, HeaderStyle.SINGLE);
            putClientProperty(PlasticLookAndFeel.BORDER_STYLE_KEY, BorderStyle.SEPARATOR);
            setFloatable(false);
            putClientProperty("JToolBar.isRollover", Boolean.TRUE);
            
            LiveActions la = _coordinator.getLiveActionsForMenu();

            add(la.get(TreeNode.START));
            add(la.get(TreeNode.STOP));
            addSeparator();
            add(la.get(TreeNode.ENABLE));
            add(la.get(TreeNode.DISABLE));
        }
    }


    private Coordinator _coordinator;
    private Server _target;

    private JTextField _currentState = new JTextField(20);
    private JTextField _currentPid = new JTextField(20);
    private JCheckBox _enabled = new JCheckBox("Enabled");

    private JTextField _application = new JTextField(20);
    private JButton _gotoApplication;

    private JTextField _exe = new JTextField(20);
    private JTextField _pwd = new JTextField(20);
    private JTextField _user = new JTextField(20);
   
    private JTextField _activation = new JTextField(20);
    private JTextField _activationTimeout = new JTextField(20);
    private JTextField _deactivationTimeout = new JTextField(20);
    private JCheckBox _allocatable = new JCheckBox("Allocatable");


    private TableField _envs = new TableField("Name", "Value");  
   
    private JTextField _options = new JTextField(20);
    private JCheckBox _applicationDistrib = new JCheckBox("Depends on the application distribution");

    private JTextField _icepatch = new JTextField(20);
    private JTextField _directories = new JTextField(20);

    private JToolBar _toolBar;
}

