// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid.TreeNode;

import java.awt.event.ActionEvent;

import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.JButton;
import javax.swing.JComboBox;
import javax.swing.JFrame;
import javax.swing.JTextField;

import com.jgoodies.forms.builder.DefaultFormBuilder;
import com.jgoodies.forms.layout.CellConstraints;

import IceGrid.ListDialog;
import IceGrid.Model;
import IceGrid.ServerDescriptor;
import IceGrid.TableDialog;
import IceGrid.Utils;

class ServerSubEditor extends CommunicatorSubEditor
{
    ServerSubEditor(Editor mainEditor, JFrame parentFrame)
    {
	super(mainEditor, parentFrame);

	_id.getDocument().addDocumentListener(
	    _mainEditor.getUpdateListener());
	_exe.getDocument().addDocumentListener(
	    _mainEditor.getUpdateListener());
	_pwd.getDocument().addDocumentListener(
	    _mainEditor.getUpdateListener());

	_options.setEditable(false);
	_envs.setEditable(false);

	_activation = new JComboBox(new Object[]{ON_DEMAND, MANUAL});
	JTextField activationTextField = (JTextField)
	    _activation.getEditor().getEditorComponent();
	activationTextField.getDocument().addDocumentListener(
	    _mainEditor.getUpdateListener());

	_activationTimeout.getDocument().addDocumentListener(
	    _mainEditor.getUpdateListener());
	_deactivationTimeout.getDocument().addDocumentListener(
	    _mainEditor.getUpdateListener());

	
	_envDialog = new TableDialog(parentFrame, "Environment Variables",
				     "Name", "Value", true);
	Action openEnvDialog = new AbstractAction("...")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    java.util.Map result = _envDialog.show(
			_envMap, _mainEditor.getPanel());
		    if(result != null)
		    {
			_mainEditor.updated();
			_envMap = result;
			setEnvsField();
		    }
		}
	    };
	_envButton = new JButton(openEnvDialog);
	
	_optionDialog = new ListDialog(parentFrame, 
				       "Command Arguments");
	Action openOptionDialog = new AbstractAction("...")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    java.util.LinkedList result = _optionDialog.show(
			_optionList, _mainEditor.getPanel());
		    if(result != null)
		    {
			_mainEditor.updated();
			_optionList = result;
			setOptionsField();
		    }
		}
	    };
	_optionButton = new JButton(openOptionDialog);


	_distrib = new JComboBox(new Object[]{NO_DISTRIB, DEFAULT_DISTRIB});
	JTextField distribTextField = (JTextField)
	    _distrib.getEditor().getEditorComponent();
	distribTextField.getDocument().addDocumentListener(
	    _mainEditor.getUpdateListener());

	_distribDirs.setEditable(false);
	_distribDirsDialog = new ListDialog(parentFrame, 
					    "Directories");

	Action openDistribDirsDialog = new AbstractAction("...")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    java.util.LinkedList result = _distribDirsDialog.show(
			_distribDirsList, _mainEditor.getPanel());
		    if(result != null)
		    {
			_mainEditor.updated();
			_distribDirsList = result;
			setDistribDirsField();
		    }
		}
	    };
	_distribDirsButton = new JButton(openDistribDirsDialog);
    }
 
    ServerDescriptor getServerDescriptor()
    {
	return (ServerDescriptor)
	    _mainEditor.getSubDescriptor();
    }
    
    void append(DefaultFormBuilder builder)
    {    
	builder.append("Server ID");
	builder.append(_id, 3);
	builder.nextLine();
	
	//
	// Add Communicator fields
	//
	super.append(builder);

	
	builder.appendSeparator("Activation");
	builder.append("Path to Executable");
	builder.append(_exe, 3);
	builder.nextLine();
	builder.append("Working Directory");
	builder.append(_pwd, 3);
	builder.nextLine();
	builder.append("Command Arguments");
	builder.append(_options, _optionButton);
	builder.nextLine();
	builder.append("Environment Variables");
	builder.append(_envs, _envButton);
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
	builder.appendSeparator("Distribution");
	builder.append("IcePatch2 Proxy");
	builder.append(_distrib, 3);
	builder.nextLine();
	builder.append("Directories");
	builder.append(_distribDirs, _distribDirsButton);
	builder.nextLine();
    }
    
    void writeDescriptor()
    {
	ServerDescriptor descriptor = getServerDescriptor();
	descriptor.id = _id.getText();
	descriptor.exe = _exe.getText();
	descriptor.pwd = _pwd.getText();

	descriptor.options = _optionList;
	
	descriptor.envs = new java.util.LinkedList();
	java.util.Iterator p = _envMap.entrySet().iterator();
	while(p.hasNext())
	{
	    java.util.Map.Entry entry = (java.util.Map.Entry)p.next();
	    descriptor.envs.add(entry.getKey().toString() 
				 + "=" + entry.getValue().toString());
	}
     
	descriptor.activation = _activation.getSelectedItem().toString();
	descriptor.activationTimeout = _activationTimeout.getText();
	descriptor.deactivationTimeout = _deactivationTimeout.getText();

	if(_distrib.getSelectedItem() == NO_DISTRIB)
	{
	    descriptor.distrib.icepatch = "";
	}
	else
	{
	    descriptor.distrib.icepatch = _distrib.getSelectedItem().toString();
	}
	descriptor.distrib.directories = _distribDirsList;

	super.writeDescriptor(descriptor);
    }	    
    
    boolean isSimpleUpdate()
    {
	return getServerDescriptor().id.equals(_id.getText()); 
    }

    void show(boolean isEditable)
    {
	ServerDescriptor descriptor = getServerDescriptor();
	Utils.Resolver detailResolver = _mainEditor.getDetailResolver();
	
	isEditable = isEditable && (detailResolver == null);

	if(detailResolver != null)
	{
	    _id.setText(detailResolver.find("server"));
	}
	else
	{
	    _id.setText(descriptor.id);
	}
	_id.setEditable(isEditable);
	
	_exe.setText(
	    Utils.substitute(descriptor.exe, detailResolver));
	_exe.setEditable(isEditable);
	_pwd.setText(
	    Utils.substitute(descriptor.pwd, detailResolver));
	_pwd.setEditable(isEditable);

	_optionList = descriptor.options;
	setOptionsField();
	_optionButton.setEnabled(isEditable);

	_envMap = new java.util.TreeMap();
	java.util.Iterator p = descriptor.envs.iterator();
	while(p.hasNext())
	{
	    String env = (String)p.next();
	    int equal = env.indexOf('=');
	    if(equal == -1 || equal == env.length() - 1)
	    {
		_envMap.put(env, "");
	    }
	    else
	    {
		_envMap.put(env.substring(0, equal),
			    env.substring(equal + 1));
	    }
	}
	setEnvsField();
	_envButton.setEnabled(isEditable);

	String activation = Utils.substitute(descriptor.activation,
					     detailResolver);
	
	_activation.setEnabled(true);
	_activation.setEditable(true);
	if(activation.equals(ON_DEMAND))
	{
	    _activation.setSelectedItem(ON_DEMAND);
	}
	else if(activation.equals(MANUAL))
	{
	    _activation.setSelectedItem(MANUAL);
	}
	else
	{
	    _activation.setSelectedItem(activation);
	}
	_activation.setEnabled(isEditable);
	_activation.setEditable(isEditable);

	_activationTimeout.setText(
	    Utils.substitute(descriptor.activationTimeout, detailResolver));
	_activationTimeout.setEditable(isEditable);

	_deactivationTimeout.setText(
	    Utils.substitute(descriptor.deactivationTimeout, detailResolver));
	_deactivationTimeout.setEditable(isEditable);

	_distrib.setEnabled(true);
	_distrib.setEditable(true);
	String icepatch = Utils.substitute(descriptor.distrib.icepatch,
					   detailResolver);
	if(icepatch.equals(""))
	{
	    _distrib.setSelectedItem(NO_DISTRIB);
	}
	else
	{
	    _distrib.setSelectedItem(icepatch);
	}
	_distrib.setEnabled(isEditable);
	_distrib.setEditable(isEditable);

	_distribDirsList = new java.util.LinkedList(descriptor.distrib.directories);
	setDistribDirsField();
	_distribDirsButton.setEnabled(isEditable);

	show(descriptor, isEditable);
    }

    private void setEnvsField()
    {
	final Utils.Resolver resolver = _mainEditor.getDetailResolver();
	
	Ice.StringHolder toolTipHolder = new Ice.StringHolder();
	Utils.Stringifier stringifier = new Utils.Stringifier()
	    {
		public String toString(Object obj)
		{
		    java.util.Map.Entry entry = (java.util.Map.Entry)obj;
		    
		    return Utils.substitute((String)entry.getKey(), resolver) 
			+ "="
			+ Utils.substitute((String)entry.getValue(), resolver);
		}
	    };
	
	_envs.setText(
	    Utils.stringify(_envMap.entrySet(), stringifier,
			    ", ", toolTipHolder));
	_envs.setToolTipText(toolTipHolder.value);
    }

    private void setOptionsField()
    {
	final Utils.Resolver resolver = _mainEditor.getDetailResolver();

	Ice.StringHolder toolTipHolder = new Ice.StringHolder();
	Utils.Stringifier stringifier = new Utils.Stringifier()
	    {
		public String toString(Object obj)
		{
		    return Utils.substitute((String)obj, resolver);
		}
	    };
	
	_options.setText(
	    Utils.stringify(_optionList, stringifier, " ", toolTipHolder));
	_options.setToolTipText(toolTipHolder.value);
    }
    
    private void setDistribDirsField()
    {
	final Utils.Resolver resolver = _mainEditor.getDetailResolver();

	Ice.StringHolder toolTipHolder = new Ice.StringHolder();
	Utils.Stringifier stringifier = new Utils.Stringifier()
	    {
		public String toString(Object obj)
		{
		    return Utils.substitute((String)obj, resolver);
		}
	    };
	
	_distribDirs.setText(
	    Utils.stringify(_distribDirsList, stringifier, ", ", toolTipHolder));
	_distribDirs.setToolTipText(toolTipHolder.value);
    }
   
    static private final String ON_DEMAND = "on-demand";
    static private final String MANUAL = "manual";
    static private final Object NO_DISTRIB = new Object()
	{
	    public String toString()
	    {
		return "None selected";
	    }
	};

    static private final String DEFAULT_DISTRIB = "${application}.IcePatch2";

    private JTextField _id = new JTextField(20);
    private JTextField _exe = new JTextField(20);
    private JTextField _pwd = new JTextField(20);
   
    private JComboBox _activation;
    private JTextField _activationTimeout = new JTextField(20);
    private JTextField _deactivationTimeout = new JTextField(20);
    
    private JTextField _envs = new JTextField(20);
    private java.util.Map _envMap;
    private TableDialog _envDialog;
    private JButton _envButton;

    private JTextField _options = new JTextField(20);
    private java.util.LinkedList _optionList;
    private ListDialog _optionDialog;
    private JButton _optionButton;
    
    private JComboBox _distrib;

    private JTextField _distribDirs = new JTextField(20);
    private java.util.LinkedList _distribDirsList;
    private ListDialog _distribDirsDialog;
    private JButton _distribDirsButton;
}
