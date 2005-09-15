// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid.TreeNode;

import java.awt.BorderLayout;
import java.awt.Component;
import java.awt.Cursor;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JComboBox;
import javax.swing.JComponent;
import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextField;
import javax.swing.JTree;

import javax.swing.event.DocumentEvent;
import javax.swing.event.DocumentListener;
import javax.swing.event.ListDataEvent;
import javax.swing.event.ListDataListener;

import javax.swing.tree.DefaultTreeCellRenderer;
import javax.swing.tree.TreePath;

import com.jgoodies.forms.builder.DefaultFormBuilder;
import com.jgoodies.forms.factories.Borders;
import com.jgoodies.forms.factories.ButtonBarFactory;
import com.jgoodies.forms.layout.FormLayout;
import com.jgoodies.forms.util.LayoutStyle;
import com.jgoodies.uif_lite.panel.SimpleInternalFrame;

import IceGrid.AdapterDescriptor;
import IceGrid.Model;
import IceGrid.ObjectDescriptor;
import IceGrid.TableDialog;
import IceGrid.Utils;

class Adapter extends Leaf
{
    static class Editor
    {
	Editor(JFrame parentFrame)
	{
	    _objects.setEditable(false);

	    //
	    // Create buttons
	    //
	    
	    //
	    // _idButton
	    //
	    AbstractAction gotoReplicatedAdapter = new AbstractAction("->")
		{
		    public void actionPerformed(ActionEvent e) 
		    {
			Object obj = _id.getSelectedItem();
			if(obj != null && _adapter != null)
			{
			    ReplicatedAdapter ra = null;
			    if(obj instanceof ReplicatedAdapter)
			    {
				ra = (ReplicatedAdapter)obj;
			    }
			    else
			    {
				ra = _adapter.getApplication().
				    findReplicatedAdapter((String)obj);
			    }
			    if(ra != null)
			    {
				_adapter.getModel().getTree().setSelectionPath
				    (ra.getPath());
			    }
			}
		    }
		};
	    gotoReplicatedAdapter.putValue(Action.SHORT_DESCRIPTION, "Goto this replicated adapter");
	    _idButton = new JButton(gotoReplicatedAdapter);

	    //
	    // _objectsButton
	    //
	    _objectsDialog = new TableDialog(parentFrame, "Registered Objects",
					     "Object Identity", "Type");

	    AbstractAction openObjectsDialog = new AbstractAction("...")
		{
		    public void actionPerformed(ActionEvent e) 
		    {
			java.util.Map result = _objectsDialog.show(_objectsMap, _panel);
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
	    // _applyButton
	    //
	    AbstractAction apply = new AbstractAction("Apply")
		{
		    public void actionPerformed(ActionEvent e) 
		    {
			if(_adapter.getModel().canUpdate())
			{
			    _adapter.getModel().disableDisplay();

			    try
			    {
				AdapterDescriptor descriptor = _adapter.getDescriptor();
				String newName = _name.getText();
				
				if(_adapter.isEphemeral())
				{
				    String newResolvedName = Utils.substitute(newName, _adapter.getResolver());
				    Adapters parent = (Adapters)_adapter.getParent();
				    
				    descriptor.name = newName;
				    writeDescriptor();
				    parent.addDescriptor(descriptor);
				    parent.removeChild(_adapter.getId(), true);
				
				    if(!_adapter.getApplication().applyUpdate())
				    {
					//
					// Restores old display
					//
					parent = (Adapters)_adapter.getModel().findNewNode(parent.getPath());
					parent.removeDescriptor(descriptor);
					try
					{
					    parent.addChild(_adapter, true);
					}
					catch(DuplicateIdException die)
					{
					    assert false;
					}
					_adapter.setParent(parent);
					_adapter.getModel().setSelectionPath(_adapter.getPath());
					return;
				    }
				    else
				    {
					//
					// No longer ephemeral
					//
					_adapter.setParent(null);
					parent = (Adapters)_adapter.getModel().findNewNode(parent.getPath());
					_adapter = (Adapter)parent.findChild(newResolvedName);
					_adapter.getModel().setSelectionPath(_adapter.getPath());
				    }
				}
				else if(descriptor.name.equals(newName))
				{
				    writeDescriptor();
				}
				else
				{
				    String newResolvedName = Utils.substitute(newName, _adapter.getResolver());

				    //
				    // Save to be able to rollback
				    //
				    AdapterDescriptor oldDescriptor = null;
				    try
				    {
					oldDescriptor = (AdapterDescriptor)descriptor.clone();
				    }
				    catch(CloneNotSupportedException ce)
				    {
					assert false; // impossible
				    }

				    TreePath oldPath = _adapter.getPath();
				    Adapters parent = (Adapters)_adapter.getParent();
				    
				    descriptor.name = newName;
				    writeDescriptor();

				    if(!_adapter.getApplication().applyUpdate())
				    {
					//
					// Restore descriptor
					// IMPORTANT: keep the same object!
					//
					descriptor.name = oldDescriptor.name;
					descriptor.id = oldDescriptor.id;
					descriptor.registerProcess = oldDescriptor.registerProcess;
					descriptor.waitForActivation = oldDescriptor.waitForActivation;
					descriptor.objects = oldDescriptor.objects;
					
					//
					// Need to find new Adapter node!
					//
					_adapter = (Adapter)_adapter.getModel().findNewNode(oldPath);
					_adapter.getModel().setSelectionPath(_adapter.getPath());
					//
					//
					// Everything was restored, user must deal with error
					//
					return;
				    }
				    else
				    {
					parent = (Adapters)_adapter.getModel().findNewNode(parent.getPath());
					_adapter = (Adapter)parent.findChild(newResolvedName);
					_adapter.getModel().setSelectionPath(_adapter.getPath());
				    }
				}
			    
				//
				// Change enclosing properties afterwards 
				//
				_adapter.setEndpoints(newName, _endpoints.getText());
				
				//
				// Mark modified
				//
				_adapter.getEditable().markModified();
			
				_applyButton.setEnabled(false);
				_discardButton.setEnabled(false);
			    }
			    finally
			    {
				_adapter.getModel().enableDisplay();
			    }
			}
			//
			// Otherwise, may wait until other user is done!
			//
		    }
		};
	    _applyButton = new JButton(apply);
	    
	    
	    //
	    // _discardButton
	    //
	    AbstractAction discard = new AbstractAction("Discard")
		{
		    public void actionPerformed(ActionEvent e) 
		    {
			//
			// Redisplay everything
			//
			_adapter.getModel().refreshDisplay();
		    }
		};
	    _discardButton = new JButton(discard);


	    //
	    // Detect updates
	    //
	    DocumentListener updateListener = new DocumentListener() 
		{
		    public void changedUpdate(DocumentEvent e)
		    {
			updated();
		    }
		    
		    public void insertUpdate(DocumentEvent e)
		    {
			updated();
		    }
		    
		    public void removeUpdate(DocumentEvent e)
		    {
			updated();
		    }
		};
	    _name.getDocument().addDocumentListener(updateListener);
	    _endpoints.getDocument().addDocumentListener(updateListener);

	    JTextField idTextField = (JTextField)_id.getEditor().getEditorComponent();
	    idTextField.getDocument().addDocumentListener(updateListener);


	    Action checkRegisterProcess = new AbstractAction("Register Process")
		{
		    public void actionPerformed(ActionEvent e)
		    {
			updated();
		    }
		};
	    _registerProcess = new JCheckBox(checkRegisterProcess);

	    Action checkWaitForActivation = new AbstractAction("Wait for Activation")
		{
		    public void actionPerformed(ActionEvent e)
		    {
			updated();
		    }
		};
	    _waitForActivation = new JCheckBox(checkWaitForActivation);
	}


	JComponent getComponent()
	{
	    if(_panel == null)
	    {
		//
		// Build everything using JGoodies's DefaultFormBuilder
		//
		FormLayout layout = new FormLayout(
		    "right:pref, 3dlu, fill:pref:grow, 3dlu, pref", "");
		DefaultFormBuilder builder = new DefaultFormBuilder(layout);
		builder.setBorder(Borders.DLU2_BORDER);
		builder.setRowGroupingEnabled(true);

		builder.setLineGapSize(LayoutStyle.getCurrent().getLinePad());

		builder.append("Name" );
		builder.append(_name, 3);
		builder.nextLine();
		
		builder.append("Id", _id );
		builder.append(_idButton);
		builder.nextLine();
		
		builder.append("Endpoints" );
		builder.append(_endpoints, 3);
		builder.nextLine();

		builder.append("Registered Objects");
		builder.append(_objects, _objectsButton);
		builder.nextLine();

		builder.append("", _registerProcess);
		builder.nextLine();
		builder.append("", _waitForActivation);
		builder.nextLine();

		JScrollPane scrollPane = new JScrollPane(builder.getPanel(),
							 JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED, 
							 JScrollPane.HORIZONTAL_SCROLLBAR_NEVER);
		scrollPane.setBorder(Borders.DIALOG_BORDER);
		
		_panel = new JPanel(new BorderLayout());
		_panel.add(scrollPane, BorderLayout.CENTER);
		
		JComponent buttonBar = ButtonBarFactory.buildRightAlignedBar(_applyButton, 
								       _discardButton);
		buttonBar.setBorder(Borders.DIALOG_BORDER);
		_panel.add(buttonBar, BorderLayout.SOUTH);
	    }
	    return _panel;
	}

	void updated()
	{
	    if(_detectUpdates)
	    {
		_applyButton.setEnabled(true);
		_discardButton.setEnabled(true);
	    }
	}
	
	//
	// Write all fields except name
	//
	void writeDescriptor()
	{
	    AdapterDescriptor descriptor = _adapter.getDescriptor();
	    descriptor.id = getIdAsString();
	    descriptor.registerProcess = _registerProcess.isSelected();
	    descriptor.waitForActivation = _waitForActivation.isSelected();
	    descriptor.objects = mapToObjectDescriptorSeq(_objectsMap);
	}	    

	void setObjectsField()
	{
	    final Utils.Resolver resolver = _adapter.getModel().substitute() ? _adapter.getResolver() : null;

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

	void setId(String adapterId)
	{
	    ReplicatedAdapters replicatedAdapters =
		_adapter.getApplication().getReplicatedAdapters();

	    ReplicatedAdapter replicatedAdapter = 
		(ReplicatedAdapter)replicatedAdapters.findChild(adapterId);
	    
	    if(replicatedAdapter != null)
	    {
		_id.setSelectedItem(replicatedAdapter);
	    }
	    else
	    {
		_id.setSelectedItem(adapterId);
	    }
	}

	String getIdAsString()
	{
	    Object obj = _id.getSelectedItem();
	    if(obj == null)
	    {
		return "";
	    }
	    else
	    {
		if(obj instanceof ReplicatedAdapter)
		{
		    ReplicatedAdapter ra = (ReplicatedAdapter)obj;
		    return ra.getId();
		}
		else
		{
		    return (String)obj;
		}
	    }
	}

	void show(Adapter adapter)
	{
	    _detectUpdates = false;

	    _adapter = adapter;
	    AdapterDescriptor descriptor = _adapter.getDescriptor();

	    Utils.Resolver resolver = null;
	    if(_adapter.getModel().substitute() && !_adapter.isEphemeral())
	    {
		resolver = _adapter.getResolver();
	    }

	    boolean isEditable = _adapter.isEditable() && resolver == null;
	    boolean inIceBox = _adapter.inIceBox();

	    _name.setText(
		Utils.substitute(descriptor.name, resolver));
	    _name.setEditable(isEditable && !inIceBox);

	    //
	    // Need to make control editable & enabled before changing it
	    //
	    _id.setEnabled(true);
	    _id.setEditable(true);

	    ReplicatedAdapters replicatedAdapters =
		adapter.getApplication().getReplicatedAdapters();
	    _id.setModel(replicatedAdapters.createComboBoxModel());
	    
	    String adapterId = Utils.substitute(descriptor.id, resolver);
	    setId(adapterId);

	    _id.setEnabled(isEditable);
	    _id.setEditable(isEditable);

	    _endpoints.setText(
		Utils.substitute(adapter.getEndpoints(), resolver));
	    _endpoints.setEditable(isEditable);

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

	    _applyButton.setEnabled(_adapter.isEphemeral());
	    _discardButton.setEnabled(_adapter.isEphemeral());	  
	    _detectUpdates = true;

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
		Ice.Identity id = Ice.Util.stringToIdentity((String)entry.getKey());
		String type = (String)entry.getValue();
		result.add(new ObjectDescriptor(id, type));
	    }
	    return result;
	}
	

	private boolean _detectUpdates;

	private JTextField _name = new JTextField(20);
	private JComboBox _id = new JComboBox();
	private JTextField _endpoints = new JTextField(20);
	private JCheckBox _registerProcess;
	private JCheckBox _waitForActivation;
	private JTextField _objects = new JTextField(20);
	private java.util.Map _objectsMap;

	private JButton _objectsButton = new JButton("...");
	private JButton _idButton;
	
	private JButton _applyButton;
	private JButton _discardButton;

	private TableDialog _objectsDialog;
	private JPanel _panel;

	private Adapter _adapter;
    }


    public Component getTreeCellRendererComponent(
	    JTree tree,
	    Object value,
	    boolean sel,
	    boolean expanded,
	    boolean leaf,
	    int row,
	    boolean hasFocus) 
    {
	if(_cellRenderer == null)
	{
	    //
	    // Initialization
	    //
	    _cellRenderer = new DefaultTreeCellRenderer();
	}
	
	_cellRenderer.setToolTipText(_toolTip);
	return _cellRenderer.getTreeCellRendererComponent(
	    tree, value, sel, expanded, leaf, row, hasFocus);
    }

    public void displayProperties()
    {
	SimpleInternalFrame propertiesFrame = _model.getPropertiesFrame();
	
	propertiesFrame.setTitle("Properties for " + _id);
	if(_editor == null)
	{
	    _editor = new Editor(_model.getMainFrame());
	}
	
	_editor.show(this);
	propertiesFrame.setContent(_editor.getComponent());
	propertiesFrame.validate();
	propertiesFrame.repaint();
    }

    public boolean destroy()
    {
	System.err.println("Destroying " + _id);

	if(_parent != null && (isEphemeral() || isEditable() && _model.canUpdate()))
	{
	    Adapters adapters = (Adapters)getParent();
	  
	    if(isEphemeral())
	    {
		adapters.removeChild(_id, true);
	    }
	    else
	    {
		adapters.removeDescriptor(_descriptor);
		getEditable().markModified();
		getApplication().applySafeUpdate();
	    }
	    setParent(null);
	    return true;
	}
	else
	{
	    return false;
	}
    }

    Adapter(String adapterName, AdapterDescriptor descriptor,
	    Utils.Resolver resolver, Application application, Model model)
    {
	super(adapterName, model);
	_descriptor = descriptor;
	_resolver = resolver;
	_ephemeral = false;
	
	if(resolver != null)
	{
	    assert application != null;
	    //
	    // In a server instance
	    //
	    _instanceId 
		= new AdapterInstanceId(_resolver.find("server"),
					_resolver.substitute(_descriptor.id));
	    
	    _proxy = application.registerAdapter(_resolver.find("node"),
						 _instanceId,
						 this);
	    createToolTip();
	}
    }

    //
    // Fresh new temporary Adapter
    // Never becomes permanent; instead a new non-ephemeral Adapter is
    // created upon a successful "apply"
    //
    Adapter(String name, AdapterDescriptor descriptor, 
	    Utils.Resolver resolver, Model model)
    {
	super("*" + name, model);
	_resolver = resolver;
	_descriptor = descriptor;
	_ephemeral = true;
    }


    public void unregister()
    {
	if(_instanceId != null)
	{
	    getApplication().unregisterAdapter(_resolver.find("node"),
					       _instanceId,
					       this);
	}
    }

    public Object copy()
    {
	try
	{
	    return _descriptor.clone();
	}
	catch(CloneNotSupportedException e)
	{
	    assert false;
	    return null;
	}
    }

    void updateProxy(Ice.ObjectPrx proxy)
    {
	assert _resolver != null;
	_proxy = proxy;
	createToolTip();
	fireNodeChangedEvent(this);
    }

    AdapterDescriptor getDescriptor()
    {
	return _descriptor;
    }

    Utils.Resolver getResolver()
    {
	return _resolver;
    }
    
    boolean isEditable()
    {
	return ((Adapters)_parent).isEditable();
    }

    boolean inIceBox()
    {
	return ((Adapters)_parent).inIceBox();
    }

    String getEndpoints()
    {
	PropertiesHolder ph = getParent().getParent().getPropertiesHolder();
	assert ph != null;
	return ph.get(_descriptor.name + ".Endpoints");
    }

    void setEndpoints(String newName, String newEndpoints)
    {
	PropertiesHolder ph = getParent().getParent().getPropertiesHolder();
	assert ph != null;
	ph.replace(_descriptor.name + ".Endpoints", newName + ".Endpoints",
		   newEndpoints);
    }

    AdapterInstanceId getInstanceId()
    {
	return _instanceId;
    }

    public boolean isEphemeral()
    {
	return _ephemeral;
    }

    private void createToolTip()
    {
	if(_proxy == null)
	{
	    _toolTip = null;
	}
	else
	{
	    _toolTip = "Proxy: " + _model.getCommunicator().proxyToString(_proxy);
	}
    }

    private boolean _ephemeral;
    private AdapterDescriptor _descriptor;
    private Utils.Resolver _resolver;

    private AdapterInstanceId _instanceId;
    private Ice.ObjectPrx _proxy;
    private String _toolTip;

    static private DefaultTreeCellRenderer _cellRenderer;
    static private Editor _editor;
}
