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

import javax.swing.JFrame;
import javax.swing.JTextField;

import com.jgoodies.forms.builder.DefaultFormBuilder;

import IceGrid.DbEnvDescriptor;
import IceGrid.Model;
import IceGrid.ObjectDescriptor;
import IceGrid.TableDialog;
import IceGrid.Utils;

class DbEnvEditor extends ListElementEditor
{
    DbEnvEditor(JFrame parentFrame)
    {
	_name.getDocument().addDocumentListener(_updateListener);
	_dbHome.getDocument().addDocumentListener(_updateListener);

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
					       getPanel());
		    if(result != null)
		    {
			updated();
			_propertiesMap = result;
			setPropertiesField();
		    }
		}
	    };
	_propertiesButton = new JButton(openPropertiesDialog);
    }
    
   
    //
    // From Editor:
    //

    void writeDescriptor()
    {
	DbEnvDescriptor descriptor = 
	    (DbEnvDescriptor)getDbEnv().getDescriptor();
	descriptor.name = _name.getText();
	descriptor.dbHome = _dbHome.getText();
	descriptor.properties = Editor.mapToProperties(_propertiesMap);
    }	    
    
    boolean isSimpleUpdate()
    {
	DbEnvDescriptor descriptor = 
	    (DbEnvDescriptor)getDbEnv().getDescriptor();
	return descriptor.name.equals(_name.getText()); 
    }

    void append(DefaultFormBuilder builder)
    {
	builder.append("Name" );
	builder.append(_name, 3);
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

	_dbHome.setText(
	    Utils.substitute(descriptor.dbHome, resolver));
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
	_properties.setToolTipText(toolTipHolder.value);
    }

    private JTextField _name = new JTextField(20);
    private JTextField _dbHome = new JTextField(20);

    private JTextField _properties = new JTextField(20);
    private java.util.Map _propertiesMap;
    private TableDialog _propertiesDialog;
    private JButton _propertiesButton = new JButton("...");
}
