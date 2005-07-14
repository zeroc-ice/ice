// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid.TreeNode;

import java.awt.Component;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JComponent;
import javax.swing.JScrollPane;
import javax.swing.JTextField;
import javax.swing.JTree;
import javax.swing.tree.DefaultTreeCellRenderer;

import com.jgoodies.forms.builder.DefaultFormBuilder;
import com.jgoodies.forms.factories.Borders;
import com.jgoodies.forms.layout.FormLayout;
import com.jgoodies.forms.util.LayoutStyle;
import com.jgoodies.uif_lite.panel.SimpleInternalFrame;

import IceGrid.AdapterDescriptor;
import IceGrid.Model;
import IceGrid.ObjectDescriptor;
import IceGrid.Utils;

class Adapter extends Leaf
{
    static class Editor
    {
	JComponent getComponent()
	{
	    if(_scrollPane == null)
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
		
		builder.append("Id" );
		builder.append(_id, 3);
		builder.nextLine();
		
		builder.append("Endpoints" );
		builder.append(_endpoints, 3);
		builder.nextLine();

		builder.append("Registered Objects");
		builder.append(_objects, _objectsButton);
		builder.nextLine();

		builder.append("", _registerProcess);
		builder.nextLine();
		builder.append("", _noWaitForActivation);
		builder.nextLine();

		_scrollPane = new JScrollPane(builder.getPanel(),
					      JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED, 
					      JScrollPane.HORIZONTAL_SCROLLBAR_NEVER);
		_scrollPane.setBorder(Borders.DIALOG_BORDER);
	    }
	    return _scrollPane;
	}

	void show(Adapter adapter)
	{
	    _adapter = adapter;
	    AdapterDescriptor descriptor = adapter.getDescriptor();

	    final java.util.Map[] variables =
		_adapter.getModel().substituteVariables() ?
		adapter.getVariables() : null;

	    boolean editable = adapter.isEditable() && (variables == null);

	    _name.setText(
		Utils.substituteVariables(descriptor.name, variables));
	    _name.setEditable(editable);

	    _id.setText(
		Utils.substituteVariables(descriptor.id, variables));
	    _id.setEditable(editable);

	    _endpoints.setText(
		Utils.substituteVariables(descriptor.endpoints, variables));
	    _endpoints.setEditable(editable);

	    Ice.StringHolder toolTipHolder = new Ice.StringHolder();
	   
	    Utils.Stringifier stringifier = new Utils.Stringifier()
	    {
		public String toString(Object obj)
		{
		    ObjectDescriptor od = (ObjectDescriptor)obj;
		    Ice.Identity sid = new Ice.Identity(
			Utils.substituteVariables(od.id.name, variables),
			Utils.substituteVariables(od.id.category, variables));

		    return Ice.Util.identityToString(sid)
			+ " as '"
			+ Utils.substituteVariables(od.type, variables)
			+ "'";
		}
	    };
	    
	    _objects.setText(
		Utils.stringify(descriptor.objects, stringifier,
				", ", toolTipHolder));
	    _objects.setToolTipText(toolTipHolder.value);
	    _objects.setEditable(editable);
	    _objectsButton.setEnabled(editable);

	    _registerProcess.setSelected(descriptor.registerProcess);
	    _registerProcess.setEnabled(editable);

	    _noWaitForActivation.setSelected(descriptor.noWaitForActivation);
	    _noWaitForActivation.setEnabled(editable);
	  
	}

	private JTextField _name = new JTextField(20);
	private JTextField _id = new JTextField(20);
	private JTextField _endpoints = new JTextField(20);
	private JCheckBox _registerProcess = new JCheckBox("Register Process");
	private JCheckBox _noWaitForActivation = new JCheckBox("Don't wait for me");
	private JTextField _objects = new JTextField(20);
	private JButton _objectsButton = new JButton("...");

	private Adapter _adapter;
	private JScrollPane _scrollPane;
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
	if(_node == null)
	{
	    return null;
	}
	else
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
    }

    public void displayProperties()
    {
	SimpleInternalFrame propertiesFrame = _model.getPropertiesFrame();

	propertiesFrame.setTitle("Properties for " + _id);
	if(_editor == null)
	{
	    _editor = new Editor();
	}
	
	_editor.show(this);
	propertiesFrame.setContent(_editor.getComponent());
	propertiesFrame.validate();
	propertiesFrame.repaint();
    }


    Adapter(String adapterName, AdapterDescriptor descriptor,
	    boolean editable, java.util.Map[] variables,
	    Model model, Node node)
    {
	super(adapterName, model);
	_descriptor = descriptor;
	_editable = editable;
	_variables = variables;
	_node = node;

	if(_node != null)
	{
	    _adapterId = Utils.substituteVariables(_descriptor.id, _variables);
	    _proxy = node.registerAdapter(_adapterId, this);
	    createToolTip();
	}
	//
	// Otherwise we have an adapter in a template 
	// (possibly in a service instance within a server template)
	//
    }

    void unregister()
    {
	if(_node != null)
	{
	    _node.unregisterAdapter(_adapterId);
	    _node = null;
	}
    }

    void updateProxy(Ice.ObjectPrx proxy)
    {
	assert(_node != null);
	_proxy = proxy;
	createToolTip();
	fireNodeChangedEvent(this);
    }

    AdapterDescriptor getDescriptor()
    {
	return _descriptor;
    }

    boolean isEditable()
    {
	return _editable;
    }

    java.util.Map[] getVariables()
    {
	return _variables;
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

    private AdapterDescriptor _descriptor;
    private boolean _editable;
    private java.util.Map[] _variables;

    private String _adapterId;
    private Node _node;
    private Ice.ObjectPrx _proxy;
    private String _toolTip;

    static private DefaultTreeCellRenderer _cellRenderer;
    static private Editor _editor;
}
