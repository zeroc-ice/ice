// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGridGUI.Application;

import java.awt.event.ActionEvent;
import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;

import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.DefaultComboBoxModel;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JComboBox;
import javax.swing.JComponent;
import javax.swing.JFrame;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JTextField;
import javax.swing.event.DocumentEvent;
import javax.swing.event.DocumentListener;

import com.jgoodies.forms.builder.DefaultFormBuilder;
import com.jgoodies.forms.layout.CellConstraints;

import IceGrid.*;
import IceGridGUI.*;

class AdapterEditor extends CommunicatorChildEditor
{
    AdapterEditor(JFrame parentFrame)
    {
	_objects.setEditable(false);
	
	//
	// Create buttons
	//
	
	//
	// _replicaGroupButton
	//
	Action gotoReplicaGroup = new AbstractAction(
	    "", Utils.getIcon("/icons/16x16/goto.png"))
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    Object obj = _replicaGroupId.getSelectedItem();
		    Adapter adapter = getAdapter();

		    ReplicaGroup rg = null;
		    if(obj instanceof ReplicaGroup)
		    {
			rg = (ReplicaGroup)obj;
		    }
		    else
		    {
			String replicaGroupId = 
			    adapter.getResolver().substitute(obj.toString());

			rg = adapter.getRoot().findReplicaGroup(replicaGroupId);
		    }

		    //
		    // The button is enabled therefore rg should be != null
		    //
		    if(rg != null)
		    {
			adapter.getRoot().setSelectedNode(rg);
		    }
		}
	    };
	gotoReplicaGroup.putValue(Action.SHORT_DESCRIPTION, 
				  "Goto the definition of this replica group");
	_replicaGroupButton = new JButton(gotoReplicaGroup);
	
	//
	// _objectsButton
	//
	_objectsDialog = new TableDialog(parentFrame, 
					 "Registered Objects",
					 "Object Identity", 
					 "Type", true);
	
	Action openObjectsDialog = new AbstractAction("...")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    java.util.Map result = _objectsDialog.show(_objectsMap, 
							       getProperties());
		    if(result != null)
		    {
			updated();
			_objectsMap = result;
			setObjectsField();
		    }
		}
	    };
	openObjectsDialog.putValue(Action.SHORT_DESCRIPTION, 
				   "Edit registered objects");
	_objectsButton = new JButton(openObjectsDialog);
	
	
	Action checkRegisterProcess = new AbstractAction("Register Process")
	    {
		public void actionPerformed(ActionEvent e)
		{
		    updated();
		}
	    };
	_registerProcess = new JCheckBox(checkRegisterProcess);
	_registerProcess.setToolTipText(
	    "<html>During activation, create a Process object<br>"
	    + "in this adapter and register it with IceGrid<br>"
	    + "to enable clean shutdown; you should register<br>"
	    + "exactly one Process object per server.</html>");

	Action checkWaitForActivation = 
	    new AbstractAction("Wait for Activation")
	    {
		public void actionPerformed(ActionEvent e)
		{
		    updated();
		}
	    };
	_waitForActivation = new JCheckBox(checkWaitForActivation);
	_waitForActivation.setToolTipText(
	    "<html>When starting the enclosing server, "
	    + "does IceGrid<br>wait for this adapter to become active?</html>");
	//
	// Associate updateListener with various fields
	//
	_name.getDocument().addDocumentListener(
	    new DocumentListener() 
	    {
		public void changedUpdate(DocumentEvent e)
		{
		    update();
		}
		
		public void insertUpdate(DocumentEvent e)
		{
		    update();
		}
		
		public void removeUpdate(DocumentEvent e)
		{
		    update();
		}

		private void update()
		{
		    updated();
		    //
		    // Recompute default id
		    //
		    _defaultAdapterId = getAdapter().getDefaultAdapterId(_name.getText());
		    refreshId();
		}
	    });

	_name.setToolTipText(
	    "Identifies this object adapter within an Ice communicator");

	_endpoints.getDocument().addDocumentListener(_updateListener);
	_endpoints.setToolTipText(
	    "<html>The network interfaces on which this adapter receives requests;<br>"
	    + "for example:<br>" 
	    + " tcp (listen on all local interfaces using a random port)<br>"
	    + " tcp -h venus.foo.com (listen on just one interface)<br>"
	    + " tcp -t 10000 (sets a timeout of 10,000 milliseconds)<br>"
	    + " ssl -h venus.foo.com (accepts SSL connections instead of plain TCP)"
	    + "</html>");

	_description.getDocument().addDocumentListener(_updateListener);
	_description.setToolTipText(
	    "An optional description for this object adapter");

	JTextField idTextField = (JTextField)
	    _id.getEditor().getEditorComponent();
	idTextField.getDocument().addDocumentListener(_updateListener);
	_id.setToolTipText("If set, must be unique within this IceGrid deployment");

	JTextField replicaGroupIdTextField = (JTextField)
	    _replicaGroupId.getEditor().getEditorComponent();
	replicaGroupIdTextField.getDocument().addDocumentListener(_updateListener);
	_replicaGroupId.setToolTipText("Select a replica group");

	JTextField publishedEndpointsTextField = (JTextField)
	    _publishedEndpoints.getEditor().getEditorComponent();
	publishedEndpointsTextField.getDocument().addDocumentListener(_updateListener);
	_publishedEndpoints.setToolTipText(
	    "<html>Direct adapter: endpoints included in proxies created using this adapter.<br>"
	    + "Indirect adapter: endpoints registered with the IceGrid Registry during the activation of this adapter." 
	    + "</html>");
    }
    
   
    //
    // From CommunicatorChildEditor
    //

    void writeDescriptor()
    {
	AdapterDescriptor descriptor = 
	    (AdapterDescriptor)getAdapter().getDescriptor();
	descriptor.name = _name.getText();
	descriptor.description = _description.getText();
	descriptor.id = getIdAsString();
	descriptor.replicaGroupId = getReplicaGroupIdAsString();
	descriptor.registerProcess = _registerProcess.isSelected();
	descriptor.waitForActivation = _waitForActivation.isSelected();
	descriptor.objects = mapToObjectDescriptorSeq(_objectsMap);
    }	    
    
    boolean isSimpleUpdate()
    {
	AdapterDescriptor descriptor = 
	    (AdapterDescriptor)getAdapter().getDescriptor();

	return descriptor.name.equals(_name.getText()); 
    }

    Communicator.ChildList getChildList()
    {
	return ((Communicator)_target.getParent()).getAdapters();
    }

    protected void appendProperties(DefaultFormBuilder builder)
    {
	builder.append("Adapter Name" );
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

	builder.append("Adapter ID");
	builder.append(_id, 3);
	builder.nextLine();
	
	builder.append("Replica Group", _replicaGroupId);
	builder.append(_replicaGroupButton);
	builder.nextLine();

	builder.append("Registered Objects");
	builder.append(_objects, _objectsButton);
	builder.nextLine();
	
	builder.append("", _registerProcess);
	builder.nextLine();
	builder.append("", _waitForActivation);
	builder.nextLine();

	builder.append("Endpoints" );
	builder.append(_endpoints, 3);
	builder.nextLine();

	builder.append("Published Endpoints" );
	builder.append(_publishedEndpoints, 3);
	builder.nextLine();
    }

    protected void buildPropertiesPanel()
    {
	super.buildPropertiesPanel();
	_propertiesPanel.setName("Adapter Properties");
    }


    void postUpdate()
    {
	//
	// Change enclosing properties after successful update
	//
	String name = _name.getText();
	Adapter adapter = getAdapter();
	if(!name.equals(_oldName))
	{
	    adapter.removeProperty(_oldName, "Endpoints");
	    adapter.removeProperty(_oldName, "PublishedEndpoints");
	    _oldName = name;
	}
	
	adapter.setProperty(name, "Endpoints", _endpoints.getText());
	
	Object published = _publishedEndpoints.getSelectedItem();
	if(published == PUBLISH_ACTUAL)
	{
	    adapter.removeProperty(name, "PublishedEndpoints");
	}
	else
	{
	    adapter.setProperty(name, "PublishedEndpoints",
				published.toString());

	}
    }
    
    private void setObjectsField()
    {
	Adapter adapter = getAdapter();

	final Utils.Resolver resolver = adapter.getCoordinator().substitute() ? 
	    adapter.getResolver() : null;
	
	Ice.StringHolder toolTipHolder = new Ice.StringHolder();
	Utils.Stringifier stringifier = new Utils.Stringifier()
	    {
		public String toString(Object obj)
		{
		    java.util.Map.Entry entry = (java.util.Map.Entry)obj;
		    
		    return Utils.substitute((String)entry.getKey(), resolver) 
			+ " as '"
			+ Utils.substitute((String)entry.getValue(), resolver)
			+ "'";
		}
	    };
	
	_objects.setText(
	    Utils.stringify(_objectsMap.entrySet(), stringifier,
			    ", ", toolTipHolder));

	_objects.setToolTipText(toolTipHolder.value);
    }
    
    private void setId(String id)
    {
	if(id.equals(""))
	{
	    _id.setSelectedItem(DIRECT_ADAPTER);
	}
	else if(id.equals(_defaultAdapterId))
	{
	    _id.setSelectedItem(DEFAULT_ADAPTER_ID);
	}
	else
	{
	    _id.setSelectedItem(id);
	}
    }

    private void refreshId()
    {
	Object id = _id.getSelectedItem();
	_id.setModel(new DefaultComboBoxModel(new Object[]
	    {DIRECT_ADAPTER, DEFAULT_ADAPTER_ID}));
	_id.setSelectedItem(id);
    }

    private String getIdAsString()
    {
	Object obj = _id.getSelectedItem();
	if(obj == DIRECT_ADAPTER)
	{
	    return "";
	}
	else 
	{
	    return obj.toString();
	}
    }

    private void setReplicaGroupId(String replicaGroupId)
    {
	if(replicaGroupId.equals(""))
	{
	    _replicaGroupId.setSelectedItem(NOT_REPLICATED);
	}
	else
	{
	    ReplicaGroups replicaGroups =
		getAdapter().getRoot().getReplicaGroups();
	    
	    ReplicaGroup replicaGroup = 
		(ReplicaGroup)replicaGroups.findChild(replicaGroupId);
	    
	    if(replicaGroup != null)
	    {
		_replicaGroupId.setSelectedItem(replicaGroup);
	    }
	    else
	    {
		_replicaGroupId.setSelectedItem(replicaGroupId);
	    }
	}
    }
    
    private String getReplicaGroupIdAsString()
    {
	Object obj = _replicaGroupId.getSelectedItem();
	if(obj == NOT_REPLICATED)
	{
	    return "";
	}
	else
	{
	    return obj.toString();
	}
    }

    void show(Adapter adapter)
    {
	detectUpdates(false);
	_target = adapter;

	AdapterDescriptor descriptor = (AdapterDescriptor)adapter.getDescriptor();
	
	final Utils.Resolver resolver = adapter.getCoordinator().substitute() ?  
	    adapter.getResolver() : null;

	boolean isEditable = adapter.isEditable() && resolver == null;
	boolean inIceBox = adapter.inIceBox();
	
	_oldName = descriptor.name;

	_name.setText(Utils.substitute(descriptor.name, resolver));
	_name.setEditable(isEditable && !inIceBox);

	_description.setText(
	    Utils.substitute(descriptor.description, resolver));
	_description.setEditable(isEditable);
	_description.setOpaque(isEditable);

	//
	// Need to make control editable & enabled before changing it
	//
	_id.setEnabled(true);
	_id.setEditable(true);	
	_defaultAdapterId = adapter.getDefaultAdapterId();
	refreshId();
	if(descriptor.id == null)
	{
	    descriptor.id = _defaultAdapterId;
	}

	setId(Utils.substitute(descriptor.id, resolver));
	_id.setEnabled(isEditable);
	_id.setEditable(isEditable);

	_replicaGroupId.setEnabled(true);
	_replicaGroupId.setEditable(true);	
	
	final ReplicaGroups replicaGroups =
	    adapter.getRoot().getReplicaGroups();
	_replicaGroupId.setModel(replicaGroups.createComboBoxModel(NOT_REPLICATED));

	_replicaGroupId.addItemListener(new ItemListener()
	    {
		public void itemStateChanged(ItemEvent e)
		{
		    if(e.getStateChange() == ItemEvent.SELECTED)
		    {
			Object item = e.getItem();
			boolean enabled = (item instanceof ReplicaGroup);
			if(!enabled && item != NOT_REPLICATED)
			{
			    if(resolver != null)
			    {
				String replicaGroupId = 
				    resolver.substitute(item.toString());
				enabled = (replicaGroups.findChild(replicaGroupId) != null);
			    }
			}
			_replicaGroupButton.setEnabled(enabled);
		    }
		}
	    });

	setReplicaGroupId(Utils.substitute(descriptor.replicaGroupId, resolver));
	_replicaGroupId.setEnabled(isEditable);
	_replicaGroupId.setEditable(isEditable);

	_endpoints.setText(
	    Utils.substitute(adapter.getProperty("Endpoints"), resolver));
	_endpoints.setEditable(isEditable);
	
	_publishedEndpoints.setEnabled(true);
	_publishedEndpoints.setEditable(true);
	String published = 
	    Utils.substitute(adapter.getProperty("PublishedEndpoints"), resolver);
	if(published == null || published.equals(""))
	{
	    _publishedEndpoints.setSelectedItem(PUBLISH_ACTUAL);
	}
	else
	{
	    _publishedEndpoints.setSelectedItem(published);
	}
	_publishedEndpoints.setEnabled(isEditable);
	_publishedEndpoints.setEditable(isEditable);

	//
	// Objects
	//
	_objectsMap = objectDescriptorSeqToMap(descriptor.objects);
	setObjectsField();
	_objectsButton.setEnabled(isEditable);
	
	_registerProcess.setSelected(descriptor.registerProcess);
	_registerProcess.setEnabled(isEditable);
	
	_waitForActivation.setSelected(descriptor.waitForActivation);
	_waitForActivation.setEnabled(isEditable);

	_applyButton.setEnabled(adapter.isEphemeral());
	_discardButton.setEnabled(adapter.isEphemeral());	  
	detectUpdates(true);
    }

    Adapter getAdapter()
    {
	return (Adapter)_target;
    }

    static java.util.Map objectDescriptorSeqToMap(java.util.List objects)
    {
	java.util.Map result = new java.util.HashMap(objects.size());
	java.util.Iterator p = objects.iterator();
	while(p.hasNext())
	{
	    ObjectDescriptor od = (ObjectDescriptor)p.next();
	    result.put(Ice.Util.identityToString(od.id), od.type);
	}
	return result;
    }
    
    static java.util.LinkedList mapToObjectDescriptorSeq(java.util.Map map)
    {
	java.util.LinkedList result = new java.util.LinkedList();
	java.util.Iterator p = map.entrySet().iterator();
	while(p.hasNext())
	{
	    java.util.Map.Entry entry = (java.util.Map.Entry)p.next();
	    Ice.Identity id = 
		Ice.Util.stringToIdentity((String)entry.getKey());
	    String type = (String)entry.getValue();
	    result.add(new ObjectDescriptor(id, type));
	}
	return result;
    }

    private String _defaultAdapterId = "";

    private final Object DEFAULT_ADAPTER_ID = new Object()
	{
	    public String toString()
	    {
		return _defaultAdapterId;
	    }
	};
    
    private String _oldName;
    
    private JTextField _name = new JTextField(20);
    private JTextArea _description = new JTextArea(3, 20);

    private JComboBox _id = new JComboBox(new Object[]
	{DIRECT_ADAPTER, DEFAULT_ADAPTER_ID});
    private JComboBox _replicaGroupId = new JComboBox();
    private JButton _replicaGroupButton;

    private JTextField _endpoints = new JTextField(20);
    private JComboBox _publishedEndpoints = new JComboBox(
	new Object[]{PUBLISH_ACTUAL});

    private JTextField _currentStatus = new JTextField(20);
    private JTextField _currentEndpoints = new JTextField(20);

    private JCheckBox _registerProcess;
    private JCheckBox _waitForActivation;

    private JTextField _objects = new JTextField(20);
    private java.util.Map _objectsMap;
    private TableDialog _objectsDialog;
    private JButton _objectsButton;

    static private final Object PUBLISH_ACTUAL = new Object()
	{
	    public String toString()
	    {
		return "Actual endpoints";
	    }
	};

    static private final Object DIRECT_ADAPTER = new Object()
	{
	    public String toString()
	    {
		return "No ID (a direct adapter)";
	    }
	};
    
    static private final Object NOT_REPLICATED = new Object()
	{
	    public String toString()
	    {
		return "Does not belong to a replica group";
	    }
	};
}
