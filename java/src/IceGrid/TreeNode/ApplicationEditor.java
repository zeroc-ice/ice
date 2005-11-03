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
import javax.swing.JComponent;
import javax.swing.JComboBox;
import javax.swing.JFrame;
import javax.swing.JOptionPane;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JTextField;

import com.jgoodies.forms.builder.DefaultFormBuilder;
import com.jgoodies.forms.layout.CellConstraints;

import IceGrid.ApplicationDescriptor;
import IceGrid.ListDialog;
import IceGrid.Model;
import IceGrid.TableDialog;
import IceGrid.Utils;

class ApplicationEditor extends Editor
{
    protected void applyUpdate()
    {
	Application application = (Application)_target;
	Model model = application.getModel();

	if(model.canUpdate())
	{    
	    model.disableDisplay();

	    try
	    {
		if(application.isEphemeral())
		{
		    Root root = (Root)application.getParent();
		    writeDescriptor();
		    ApplicationDescriptor descriptor = 
			(ApplicationDescriptor)application.getDescriptor();
		    application.destroy(); // just removes the child
		    try
		    {
			root.tryAdd(descriptor);
		    }
		    catch(UpdateFailedException e)
		    {
			//
			// Add back ephemeral child
			//
			try
			{
			   root.addChild(application, true);
			}
			catch(UpdateFailedException die)
			{
			    assert false;
			}
			model.setSelectionPath(application.getPath());

			JOptionPane.showMessageDialog(
			    model.getMainFrame(),
			    e.toString(),
			    "Apply failed",
			    JOptionPane.ERROR_MESSAGE);
			return;
		    }

		    //
		    // Success
		    //
		    _target = root.findChildWithDescriptor(descriptor);
		    model.setSelectionPath(_target.getPath());
		    model.showActions(_target);
		}
		else if(isSimpleUpdate())
		{
		    writeDescriptor();
		    application.markModified();
		}
		else
		{
		    //
		    // Save to be able to rollback
		    //
		    Object savedDescriptor = application.saveDescriptor();
		    writeDescriptor();
		    try
		    {
			application.rebuild();
		    }
		    catch(UpdateFailedException e)
		    {
			application.restoreDescriptor(savedDescriptor);
			JOptionPane.showMessageDialog(
			    model.getMainFrame(),
			    e.toString(),
			    "Apply failed",
			    JOptionPane.ERROR_MESSAGE);
			return;
		    }
		    //
		    // Success
		    //
		    application.markModified();
		    model.showActions(_target);
		}
		_applyButton.setEnabled(false);
		_discardButton.setEnabled(false);
	    }
	    finally
	    {
		model.enableDisplay();
	    }
	}
    }

    Utils.Resolver getDetailResolver()
    {
	Application application = (Application)_target;
	if(application.getModel().substitute())
	{
	    return application.getResolver();
	}
	else
	{
	    return null;
	}
    }


    ApplicationEditor(JFrame parentFrame)
    {
	super(false, true);
	_name.getDocument().addDocumentListener(_updateListener);
	_description.getDocument().addDocumentListener(_updateListener);
	_variables.setEditable(false);
	
	//
	// Variables
	//
	_variablesDialog = new TableDialog(parentFrame, 
					   "Variables", "Name", "Value", true);
	
	Action openVariablesDialog = new AbstractAction("...")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    java.util.TreeMap result = _variablesDialog.show(_variablesMap, 
								     getProperties());
		    if(result != null)
		    {
			updated();
			_variablesMap = result;
			setVariablesField();
		    }
		}
	    };
	openVariablesDialog.putValue(Action.SHORT_DESCRIPTION, 
				     "Edit variables");
	_variablesButton = new JButton(openVariablesDialog);

	//
	// Distrib
	//
	_distrib = new JComboBox(new Object[]{NO_DISTRIB, DEFAULT_DISTRIB});
	_distrib.setEditable(true);
	_distrib.setToolTipText("The proxy to the IcePatch2 server holding your files");

	JTextField distribTextField = (JTextField)
	    _distrib.getEditor().getEditorComponent();
	distribTextField.getDocument().addDocumentListener(
	    _updateListener);

	_distribDirs.setEditable(false);
	_distribDirsDialog = new ListDialog(parentFrame, 
					    "Directories", true);

	Action openDistribDirsDialog = new AbstractAction("...")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    java.util.LinkedList result = _distribDirsDialog.show(
			_distribDirsList, getProperties());
		    if(result != null)
		    {
			updated();
			_distribDirsList = result;
			setDistribDirsField();
		    }
		}
	    };
	openDistribDirsDialog.putValue(Action.SHORT_DESCRIPTION, 
				       "Edit directory list");
	_distribDirsButton = new JButton(openDistribDirsDialog);

    }
 
    void appendProperties(DefaultFormBuilder builder)
    {    
	builder.append("Name");
	builder.append(_name, 3);

	builder.append("Description");
	builder.nextLine();
	builder.append("");
	builder.nextRow(-2);
	CellConstraints cc = new CellConstraints();
	JScrollPane scrollPane = new JScrollPane(_description);
	builder.add(scrollPane, 
		    cc.xywh(builder.getColumn(), builder.getRow(), 3, 3));
	builder.nextRow(2);
	builder.nextLine();

	builder.append("Variables", _variables);
	builder.append(_variablesButton);
	builder.nextLine();

	JComponent c = builder.appendSeparator("Distribution");
	c.setToolTipText("Files shared by all servers in your application");
	builder.append("IcePatch2 Proxy");
	builder.append(_distrib, 3);
	builder.nextLine();
	builder.append("Directories");
	builder.append(_distribDirs, _distribDirsButton);
	builder.nextLine();
    }
    
    boolean isSimpleUpdate()
    {
	ApplicationDescriptor descriptor = (ApplicationDescriptor)_target.getDescriptor();
	return (_variablesMap.equals(descriptor.variables));
    }

    void writeDescriptor()
    {
	ApplicationDescriptor descriptor = (ApplicationDescriptor)_target.getDescriptor();
	descriptor.name = _name.getText();
	descriptor.variables = _variablesMap;
	descriptor.description = _description.getText();
	
	if(_distrib.getSelectedItem() == NO_DISTRIB)
	{
	    descriptor.distrib.icepatch = "";
	}
	else
	{
	    descriptor.distrib.icepatch = _distrib.getSelectedItem().toString();
	}
	descriptor.distrib.directories = _distribDirsList;
    }	    
    

    void show(Application application)
    {
	detectUpdates(false);
	setTarget(application);
	
	Utils.Resolver resolver = getDetailResolver();
	boolean isEditable = (resolver == null);
	
	_name.setText(application.getId());
	_name.setEditable(application.isEphemeral());

	ApplicationDescriptor descriptor = 
	    (ApplicationDescriptor)application.getDescriptor();

	_description.setText(
	    Utils.substitute(descriptor.description, resolver));
	_description.setEditable(isEditable);
	_description.setOpaque(isEditable);
	_description.setToolTipText("An optional description for this application");
	
	_variablesMap = descriptor.variables;
	setVariablesField();
	_variablesButton.setEnabled(isEditable);

	_distrib.setEnabled(true);
	_distrib.setEditable(true);
	String icepatch = 
	    Utils.substitute(descriptor.distrib.icepatch, resolver);
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

	_applyButton.setEnabled(application.isEphemeral());
	_discardButton.setEnabled(application.isEphemeral());	  
	detectUpdates(true);
    }
    
    private void setDistribDirsField()
    {
	final Utils.Resolver resolver = getDetailResolver();

	Ice.StringHolder toolTipHolder = new Ice.StringHolder();
	Utils.Stringifier stringifier = new Utils.Stringifier()
	    {
		public String toString(Object obj)
		{
		    return Utils.substitute((String)obj, resolver);
		}
	    };
	
	_distribDirs.setText(
	    Utils.stringify(_distribDirsList, stringifier, ", ", 
			    toolTipHolder));

	String toolTip = "<html>Include only these directories";

	if(toolTipHolder.value != null)
	{
	    toolTip += ":<br>" + toolTipHolder.value;
	}
	toolTip += "</html>";
	_distribDirs.setToolTipText(toolTip);
    }

    private void setVariablesField()
    {
	final Utils.Resolver resolver = getDetailResolver();

	Utils.Stringifier stringifier = new Utils.Stringifier()
	    {
		public String toString(Object obj)
		{
		    java.util.Map.Entry entry = (java.util.Map.Entry)obj;
		    
		    return (String)entry.getKey() + "="
			+ Utils.substitute((String)entry.getValue(), resolver);
		}
	    };

	Ice.StringHolder toolTipHolder = new Ice.StringHolder();
	_variables.setText(
	    Utils.stringify(_variablesMap.entrySet(), stringifier, 
			    ", ", toolTipHolder));
	_variables.setToolTipText(toolTipHolder.value);
    }
    
    static private final Object NO_DISTRIB = new Object()
	{
	    public String toString()
	    {
		return "None selected";
	    }
	};
    static private final String DEFAULT_DISTRIB = "${application}.IcePatch2";

    private JTextField _name = new JTextField(20);
    private JTextArea _description = new JTextArea(3, 20);

    private JTextField _variables = new JTextField(20);
    private JButton _variablesButton;
    private TableDialog _variablesDialog;
    private java.util.TreeMap _variablesMap;
    
    private JComboBox _distrib;
    private JTextField _distribDirs = new JTextField(20);
    private java.util.LinkedList _distribDirsList;
    private ListDialog _distribDirsDialog;
    private JButton _distribDirsButton;
}
