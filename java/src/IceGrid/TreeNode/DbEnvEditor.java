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
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JTextField;

import com.jgoodies.forms.builder.DefaultFormBuilder;
import com.jgoodies.forms.layout.CellConstraints;

import IceGrid.DbEnvDescriptor;
import IceGrid.Model;
import IceGrid.ObjectDescriptor;
import IceGrid.TableDialog;
import IceGrid.Utils;

class DbEnvEditor extends ListElementEditor
{
    DbEnvEditor(JFrame parentFrame)
    {
	super(false);
	_name.getDocument().addDocumentListener(_updateListener);
	_name.setToolTipText(
	    "Identifies this Freeze database environment within an Ice communicator");
	_description.getDocument().addDocumentListener(_updateListener);
	_description.setToolTipText(
	    "An optional description for this database environment");

	JTextField dbHomeTextField = (JTextField)
	    _dbHome.getEditor().getEditorComponent();
	dbHomeTextField.getDocument().addDocumentListener(_updateListener);
	_dbHome.setToolTipText("<html><i>node data dir</i>/servers/<i>server id</i>"
			       + "/dbs/<i>db env name</i> if created by the IceGrid Node;<br>"
			       + "otherwise, IceGrid does not create this directory"
			       + "</html>");


	_properties.setEditable(false);
		
	//
	// _propertiesButton
	//
	_propertiesDialog = new TableDialog(parentFrame, 
					    "Berkeley DB Configuration Properties",
					    "Name", 
					    "Value", true);
	
	Action openPropertiesDialog = new AbstractAction("...")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    java.util.Map result = 
			_propertiesDialog.show(_propertiesMap, 
					       getProperties());
		    if(result != null)
		    {
			updated();
			_propertiesMap = result;
			setPropertiesField();
		    }
		}
	    };
	openPropertiesDialog.putValue(Action.SHORT_DESCRIPTION,
				      "Edit properties");
	_propertiesButton = new JButton(openPropertiesDialog);
    }

    void writeDescriptor()
    {
	DbEnvDescriptor descriptor = 
	    (DbEnvDescriptor)getDbEnv().getDescriptor();
	descriptor.name = _name.getText();
	descriptor.description = _description.getText();
	descriptor.dbHome = getDbHomeAsString();
	descriptor.properties = Editor.mapToProperties(_propertiesMap);
    }	    
    
    boolean isSimpleUpdate()
    {
	DbEnvDescriptor descriptor = 
	    (DbEnvDescriptor)getDbEnv().getDescriptor();
	return descriptor.name.equals(_name.getText()); 
    }

    void appendProperties(DefaultFormBuilder builder)
    {
	builder.append("Name" );
	builder.append(_name, 3);
	builder.nextLine();

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
	
	builder.append("DB Home" );
	builder.append(_dbHome, 3);
	builder.nextLine();

	builder.append("Properties");
	builder.append(_properties, _propertiesButton);
	builder.nextLine();
    }

    void show(DbEnv dbEnv)
    {
	detectUpdates(false);
	setTarget(dbEnv);

	DbEnvDescriptor descriptor = (DbEnvDescriptor)dbEnv.getDescriptor();
	
	Utils.Resolver resolver = null;
	if(dbEnv.getModel().substitute())
	{
	    resolver = dbEnv.getResolver();
	}
	boolean isEditable = dbEnv.isEditable() && resolver == null;
	
	_name.setText(
	    Utils.substitute(descriptor.name, resolver));
	_name.setEditable(isEditable);

	_description.setText(
	    Utils.substitute(descriptor.description, resolver));
	_description.setEditable(isEditable);
	_description.setOpaque(isEditable);
	
	_dbHome.setEnabled(true);
	_dbHome.setEditable(true);
	setDbHome(Utils.substitute(descriptor.dbHome, resolver));
	_dbHome.setEnabled(isEditable);
	_dbHome.setEditable(isEditable);
	
	_propertiesMap = Editor.propertiesToMap(descriptor.properties);
	setPropertiesField();
	_propertiesButton.setEnabled(isEditable);
	
	_applyButton.setEnabled(dbEnv.isEphemeral());
	_discardButton.setEnabled(dbEnv.isEphemeral());	  
	detectUpdates(true);
    }

    private DbEnv getDbEnv()
    {
	return (DbEnv)_target;
    }

    private void setDbHome(String dbHome)
    {
	if(dbHome.equals(""))
	{
	    _dbHome.setSelectedItem(NO_DB_HOME);
	}
	else 
	{
	    _dbHome.setSelectedItem(dbHome);
	}
    }
    
    private String getDbHomeAsString()
    {
	Object obj = _dbHome.getSelectedItem();
	if(obj == NO_DB_HOME)
	{
	    return "";
	}
	else
	{
	    return obj.toString();
	}
    }
    

    private void setPropertiesField()
    {
	final Utils.Resolver resolver = getDbEnv().getResolver();
	
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
	
	_properties.setText(
	    Utils.stringify(_propertiesMap.entrySet(), stringifier,
			    ", ", toolTipHolder));
	
	String toolTip = "<html>Properties used to generate a"
	    + " DB_CONFIG file in the DB home directory";
	if(toolTipHolder.value != null)
	{
	    toolTip += ":<br>" + toolTipHolder.value;
	}
	toolTip += "</html>";

	_properties.setToolTipText(toolTip);
    }

    private JTextField _name = new JTextField(20);
    private JTextArea _description = new JTextArea(3, 20);

    private JComboBox _dbHome = new JComboBox(new Object[]{NO_DB_HOME});

    private JTextField _properties = new JTextField(20);
    private java.util.Map _propertiesMap;
    private TableDialog _propertiesDialog;
    private JButton _propertiesButton = new JButton("...");
    
    static private final Object NO_DB_HOME = new Object()
	{
	    public String toString()
	    {
		return "Created by the IceGrid Node";
	    }
	};
}
