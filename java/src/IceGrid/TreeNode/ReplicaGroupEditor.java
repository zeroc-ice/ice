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
import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;

import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.DefaultComboBoxModel;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JComboBox;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JTextField;

import com.jgoodies.forms.builder.DefaultFormBuilder;
import com.jgoodies.forms.layout.CellConstraints;

import IceGrid.AdaptiveLoadBalancingPolicy;
import IceGrid.RandomLoadBalancingPolicy;
import IceGrid.RoundRobinLoadBalancingPolicy;

import IceGrid.Model;
import IceGrid.ObjectDescriptor;
import IceGrid.ReplicaGroupDescriptor;
import IceGrid.TableDialog;
import IceGrid.Utils;

class ReplicaGroupEditor extends Editor
{
    protected void applyUpdate()
    {
	ReplicaGroup replicaGroup = getReplicaGroup();
	Model model = replicaGroup.getModel();

	if(model.canUpdate())
	{    
	    model.disableDisplay();

	    try
	    {
		if(replicaGroup.isEphemeral())
		{
		    ReplicaGroups replicaGroups = (ReplicaGroups)replicaGroup.getParent();
		    writeDescriptor();
		    ReplicaGroupDescriptor descriptor = 
			(ReplicaGroupDescriptor)replicaGroup.getDescriptor();
		    replicaGroup.destroy(); // just removes the child
		    
		    try
		    {
			replicaGroups.tryAdd(descriptor, true);
		    }
		    catch(UpdateFailedException e)
		    {
			//
			// Add back ephemeral child
			//
			try
			{
			    replicaGroups.addChild(replicaGroup, true);
			}
			catch(UpdateFailedException die)
			{
			    assert false;
			}
			model.setSelectionPath(replicaGroup.getPath());

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
		    _target = replicaGroups.findChildWithDescriptor(descriptor);
		}
		else if(isSimpleUpdate())
		{
		    writeDescriptor();
		    _target.getEditable().markModified();
		}
		else
		{
		    //
		    // Save to be able to rollback
		    //
		    Object savedDescriptor = replicaGroup.saveDescriptor();
		    ReplicaGroups replicaGroups = (ReplicaGroups)replicaGroup.getParent();
		    writeDescriptor();
		    ReplicaGroupDescriptor descriptor = 
			(ReplicaGroupDescriptor)replicaGroup.getDescriptor();

		    replicaGroups.removeChild(replicaGroup, true);	    
		    try
		    {
			replicaGroups.tryAdd(descriptor, false);
		    }
		    catch(UpdateFailedException e)
		    {
			//
			// Restore all
			//
			try
			{
			    replicaGroups.addChild(replicaGroup, true);
			}
			catch(UpdateFailedException die)
			{
			    assert false;
			}
			replicaGroup.restoreDescriptor(savedDescriptor);
			model.setSelectionPath(replicaGroup.getPath());

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
		    replicaGroups.removeElement(replicaGroup.getId()); // replaced by brand new ReplicaGroup

		    _target = replicaGroups.findChildWithDescriptor(descriptor);
		    model.setSelectionPath(_target.getPath());
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
	Application application = _target.getApplication();
	if(application.getModel().substitute())
	{
	    return application.getResolver();
	}
	else
	{
	    return null;
	}
    }

    ReplicaGroupEditor(JFrame parentFrame)
    {
	super(false, true);

	_objects.setEditable(false);
	
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
	_objectsButton = new JButton(openObjectsDialog);
	
	//
	// load balancing
	//
	_loadBalancing.addItemListener(new ItemListener()
	    {
		public void itemStateChanged(ItemEvent e)
		{
		    if(e.getStateChange() == ItemEvent.SELECTED)
		    {
			updated();

			Object item = e.getItem();
			_nReplicasLabel.setVisible(item != RETURN_ALL);
			_nReplicas.setVisible(item != RETURN_ALL);
			
			_loadSampleLabel.setVisible(item == ADAPTIVE);
			_loadSample.setVisible(item == ADAPTIVE);
		    }
		}
	    });

	//
	// Associate updateListener with various fields
	//
	_id.getDocument().addDocumentListener(_updateListener);
	_description.getDocument().addDocumentListener(_updateListener);
	_nReplicas.getDocument().addDocumentListener(_updateListener);

	_loadSample.setEditable(true);
	JTextField loadSampleTextField = (JTextField)
	    _loadSample.getEditor().getEditorComponent();
	loadSampleTextField.getDocument().addDocumentListener(_updateListener);

    }
    
    void writeDescriptor()
    {
	ReplicaGroupDescriptor descriptor = 
	    (ReplicaGroupDescriptor)getReplicaGroup().getDescriptor();

	descriptor.id = _id.getText();
	descriptor.description = _description.getText();
	descriptor.objects = AdapterEditor.mapToObjectDescriptorSeq(_objectsMap);
	
	Object loadBalancing = _loadBalancing.getSelectedItem();
	if(loadBalancing == RETURN_ALL)
	{
	    descriptor.loadBalancing = null;
	}
	else if(loadBalancing == RANDOM)
	{
	    descriptor.loadBalancing = new RandomLoadBalancingPolicy(
		_nReplicas.getText());
	}
	else if(loadBalancing == ROUND_ROBIN)
	{
	    descriptor.loadBalancing = new RoundRobinLoadBalancingPolicy(
		_nReplicas.getText());
	}
	else if(loadBalancing == ADAPTIVE)
	{
	    descriptor.loadBalancing = new AdaptiveLoadBalancingPolicy(
		_nReplicas.getText(), 
		_loadSample.getSelectedItem().toString());
	}
	else
	{
	    assert false;
	}
    }	    
    
    boolean isSimpleUpdate()
    {
	ReplicaGroupDescriptor descriptor = 
	    (ReplicaGroupDescriptor)getReplicaGroup().getDescriptor();
	return descriptor.id.equals(_id.getText());
    }

    void appendProperties(DefaultFormBuilder builder)
    {
	builder.append("Replica Group ID" );
	builder.append(_id, 3);
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

	builder.append("Registered Objects");
	builder.append(_objects, _objectsButton);
	builder.nextLine();

	builder.append("Load Balancing Policy");
	builder.append(_loadBalancing, 3);
	builder.nextLine();
	_nReplicasLabel = builder.append("How many Adapters?");
	builder.append(_nReplicas, 3);
	builder.nextLine();
	_loadSampleLabel = builder.append("Load Sample");
	builder.append(_loadSample, 3);
	builder.nextLine();
    }

    void setObjectsField()
    {
	final Utils.Resolver resolver = getDetailResolver();
	
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
    
    
    void show(ReplicaGroup replicaGroup)
    {
	//
	// Make sure everything is built
	//
	getProperties();


	detectUpdates(false);
	setTarget(replicaGroup);

	Utils.Resolver resolver = getDetailResolver();
	boolean isEditable = (resolver == null);
	
	ReplicaGroupDescriptor descriptor = 
	    (ReplicaGroupDescriptor)replicaGroup.getDescriptor();
	
	_id.setText(descriptor.id);
	_id.setEditable(isEditable);
	
	_description.setText(
	    Utils.substitute(descriptor.description, resolver));
	_description.setEditable(isEditable);
	_description.setOpaque(isEditable);

	_objectsMap = AdapterEditor.objectDescriptorSeqToMap(descriptor.objects);
	setObjectsField();
	_objectsButton.setEnabled(isEditable);

	_loadBalancing.setEnabled(true);
	_loadBalancing.setEditable(true);

	if(descriptor.loadBalancing == null)
	{
	    _loadBalancing.setSelectedItem(RETURN_ALL);
	    _nReplicas.setText("1");
	    _loadSample.setSelectedItem("1");
	}
	else if(descriptor.loadBalancing instanceof RandomLoadBalancingPolicy)
	{
	    _loadBalancing.setSelectedItem(RANDOM);
	    _nReplicas.setText(
		Utils.substitute(descriptor.loadBalancing.nReplicas, resolver));
	    _loadSample.setSelectedItem("1");
	}
	else if(descriptor.loadBalancing instanceof RoundRobinLoadBalancingPolicy)
	{
	    _loadBalancing.setSelectedItem(ROUND_ROBIN);
	    _nReplicas.setText(
		Utils.substitute(descriptor.loadBalancing.nReplicas, resolver));
	    _loadSample.setSelectedItem("1");
	}
	else if(descriptor.loadBalancing instanceof AdaptiveLoadBalancingPolicy)
	{
	    _loadBalancing.setSelectedItem(ADAPTIVE);
	    _nReplicas.setText(
		Utils.substitute(descriptor.loadBalancing.nReplicas, resolver));

	    _loadSample.setSelectedItem(
		Utils.substitute(
		    ((AdaptiveLoadBalancingPolicy)descriptor.loadBalancing).loadSample,
		    resolver));
	}
	else
	{
	    assert false;
	}
	_nReplicas.setEditable(isEditable);
	_loadSample.setEditable(isEditable);
	_loadBalancing.setEnabled(isEditable);
	_loadBalancing.setEditable(isEditable);

	_applyButton.setEnabled(replicaGroup.isEphemeral());
	_discardButton.setEnabled(replicaGroup.isEphemeral());	  
	detectUpdates(true);
    }

    private ReplicaGroup getReplicaGroup()
    {
	return (ReplicaGroup)_target;
    }

    static private String RETURN_ALL = "Return all";
    static private String RANDOM = "Random";
    static private String ROUND_ROBIN = "Round-robin";
    static private String ADAPTIVE = "Adaptive";

    private JTextField _id = new JTextField(20);
    private JTextArea _description = new JTextArea(3, 20);

    private JComboBox _loadBalancing = new JComboBox(new Object[]
	{ADAPTIVE, RANDOM, RETURN_ALL, ROUND_ROBIN}); 
    
    private JLabel _nReplicasLabel;
    private JTextField _nReplicas = new JTextField(20);

    private JLabel _loadSampleLabel;
    private JComboBox _loadSample = new JComboBox(new Object[]
	{"1", "5", "15"});
    
    private JTextField _objects = new JTextField(20);
    private java.util.Map _objectsMap;
    private TableDialog _objectsDialog;
    private JButton _objectsButton;
}
