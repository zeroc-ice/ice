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
import java.awt.event.ActionEvent;
import javax.swing.JPopupMenu;
import javax.swing.JPanel;
import javax.swing.JTree;
import javax.swing.Icon;
import javax.swing.JMenuItem;
import javax.swing.Action;
import javax.swing.AbstractAction;
import javax.swing.JTextField;
import javax.swing.JComponent;
import javax.swing.JScrollPane;
import javax.swing.tree.DefaultTreeCellRenderer;

import javax.swing.JTextField;
import javax.swing.JComboBox;
import javax.swing.JButton;
import javax.swing.JToggleButton;

import com.jgoodies.uif_lite.panel.SimpleInternalFrame;
import com.jgoodies.forms.builder.DefaultFormBuilder;
import com.jgoodies.forms.layout.FormLayout;
import com.jgoodies.forms.factories.Borders;

import IceGrid.TreeModelI;
import IceGrid.IceBoxDescriptor;
import IceGrid.ServerInstanceDescriptor;
import IceGrid.ServerState;
import IceGrid.TemplateDescriptor;
import IceGrid.Utils;
import IceGrid.Model;
import IceGrid.ServerDynamicInfo;

class ServerInstance extends Parent
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
		FormLayout layout = new FormLayout("right:pref, 3dlu, fill:pref:grow, 3dlu, pref", "");
		DefaultFormBuilder builder = new DefaultFormBuilder(layout);
		builder.setBorder(Borders.DLU2_BORDER);
		builder.setRowGroupingEnabled(true);

		builder.appendSeparator("Server Instance Descriptor");
		
		builder.append("Node");
		builder.append(_node, 3);
		builder.nextLine();

		AbstractAction gotoTemplate = new AbstractAction("->")
		    {
			public void actionPerformed(ActionEvent e) 
			{
			    if(_server != null)
			    {
				CommonBase parent = _server.getParent(TreeModelI.APPLICATION_VIEW);
				if(parent != null)
				{
				    Application app = (Application)parent.getParent(TreeModelI.APPLICATION_VIEW);
				    if(app != null)
				    {
					ServerTemplate template = app.findServerTemplate(_template.getText());
					if(template != null)
					{
					    _server.getModel().getTreeNodeSelector().selectNode(
						template.getPath(TreeModelI.APPLICATION_VIEW), 
						TreeModelI.APPLICATION_VIEW);
					}
				    }
				}
			    }
			}
		    };
		gotoTemplate.putValue(Action.SHORT_DESCRIPTION, "Goto this template");


		builder.append("Template", _template);
		builder.append(new JButton(gotoTemplate));
		builder.nextLine();

		builder.append("Parameters", _parameterValues);
		builder.append(new JButton("..."));
		builder.nextLine();

		_myDescriptor = new JComboBox();
		_myDescriptor.addItem("");
		_myDescriptor.addItem("ServerDescriptor");
		_myDescriptor.addItem("IceBoxDescriptor");
		builder.append("My Descriptor", _myDescriptor);
		builder.append(new JToggleButton("Edit"));

		builder.append("Targets");
		builder.append(_targets, 3);
		_targets.setEditable(false);
		builder.nextLine();
		_serverDescriptorRenderer.build(builder);

		_scrollPane = new JScrollPane(builder.getPanel(),
					      JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED, 
					      JScrollPane.HORIZONTAL_SCROLLBAR_NEVER);
		_scrollPane.setBorder(Borders.DIALOG_BORDER);
	    }
	    return _scrollPane;
	}

	void show(ServerInstance server)
	{
	    _server = server;
	    ServerInstanceDescriptor descriptor = server.getDescriptor();
	    _template.setText(descriptor.template);
	    _node.setText(descriptor.node);
	    
	    Ice.StringHolder toolTipHolder = new Ice.StringHolder();
	    
	    _parameterValues.setText(Utils.stringify(descriptor.parameterValues, "=",
						    ", ", toolTipHolder));
	    _parameterValues.setToolTipText(toolTipHolder.value);

	    _targets.setText(Utils.stringify(descriptor.targets, ", ", toolTipHolder));
	    _targets.setToolTipText(toolTipHolder.value);

	    _serverDescriptorRenderer.showDescriptor(descriptor.descriptor);
	}

	private JTextField _node = new JTextField(20);
	private JTextField _template = new JTextField(20);
	private JTextField _parameterValues = new JTextField(20);
	private JComboBox _myDescriptor;
	private JTextField _targets = new JTextField(20);

	private ServerDescriptorRenderer _serverDescriptorRenderer =
	  new ServerDescriptorRenderer();

	private ServerInstance _server;

	private JScrollPane _scrollPane;
    }
  
    static class PopupMenu extends JPopupMenu
    {
	PopupMenu()
	{
	    _startAction = new AbstractAction("Start")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    _server.start();
		}
	    };

	    _stopAction = new AbstractAction("Stop")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    _server.stop();
		}
	    };

	    add(_startAction);
	    add(_stopAction);
	}
	
	void setServer(ServerInstance server)
	{
	    _server = server;
	    ServerState state = _server.getState();

	    boolean canStart = (_server.getState() == ServerState.Inactive);
	    _startAction.setEnabled(canStart);
	    _stopAction.setEnabled(!canStart);
	}

	private ServerInstance _server;
	private Action _startAction;
	private Action _stopAction;
    }
    

    public JPopupMenu getPopupMenu()
    {
	if(_popup == null)
	{
	    _popup = new PopupMenu();
	}
	_popup.setServer(this);
	return _popup;
    }

    public void displayProperties(SimpleInternalFrame frame, int view)
    {
	frame.setTitle("Properties for " + _id);
	
	if(_editor == null)
	{
	    _editor = new Editor();
	}
	_editor.show(this);
	frame.setContent(_editor.getComponent());
	frame.validate();
	frame.repaint();
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
	    _icons = new Icon[7];
	    _icons[0] = Utils.getIcon("/icons/unknown.png");
	    _icons[ServerState.Inactive.value() + 1] = Utils.getIcon("/icons/inactive.png");
	    _icons[ServerState.Activating.value() + 1] = Utils.getIcon("/icons/activating.png");
	    _icons[ServerState.Active.value() + 1] = Utils.getIcon("/icons/active.png");
	    _icons[ServerState.Deactivating.value() + 1] = Utils.getIcon("/icons/deactivating.png");
	    _icons[ServerState.Destroying.value() + 1] = Utils.getIcon("/icons/destroying.png");
	    _icons[ServerState.Destroyed.value() + 1] = Utils.getIcon("/icons/destroyed.png");
	}

	//
	// TODO: separate icons for open and close
	//
	if(expanded)
	{
	    _cellRenderer.setOpenIcon(_icons[_stateIconIndex]);
	}
	else
	{
	    _cellRenderer.setClosedIcon(_icons[_stateIconIndex]);
	}

	_cellRenderer.setToolTipText(_toolTip);
	return _cellRenderer.getTreeCellRendererComponent(
	    tree, value, sel, expanded, leaf, row, hasFocus);
    }


    //
    // Builds the server instance and all its sub-tree
    //
    ServerInstance(ServerInstanceDescriptor descriptor, 
		   Model model,
		   boolean fireNodeViewEvent)
    {
	super(descriptor.descriptor.name, model);
	
	rebuild(descriptor, fireNodeViewEvent);
    }


    void removeFromNode()
    {
	removeFromNode(true);
    }

    private void removeFromNode(boolean fireNodeViewEvent)
    {
	if(_serviceInstances != null)
	{
	    _serviceInstances.unregisterAdapters();
	}
	if(_adapters != null)
	{
	    _adapters.unregisterAll();
	}

	Node node = (Node)getParent(TreeModelI.NODE_VIEW);
	if(node != null)
	{
	    node.removeChild(_id, fireNodeViewEvent);
	}
    }

    //
    // Update the server instance and all its subtree
    //
    void rebuild(ServerInstanceDescriptor newDescriptor, 
		 boolean fireNodeViewEvent)
    {
	assert(newDescriptor != null);
	removeFromNode(fireNodeViewEvent);
	clearChildren();
	
	//
	// First check if my node changed [node view]
	// For the application view, the only way a server instance can change application 
	// is by being removed and then re-added (i.e. not with an update)
	//
	boolean newNode = false;
	
	if(_descriptor == null)
	{
	    newNode = true;
	}
	else if(_descriptor.node != newDescriptor.node)
	{
	    newNode = true;
	    //
	    // Remove from existing node
	    //
	    CommonBase parent = _parents[TreeModelI.NODE_VIEW];
	    assert(parent != null); // must be connected
	    removeParent(parent);
	    ((Parent)parent).removeChild(this);  
	}
	_descriptor = newDescriptor;
	
	Node node = _model.getNodeViewRoot().findNode(_descriptor.node);

	if(_descriptor.descriptor instanceof IceBoxDescriptor)
	{
	    _iceBoxDescriptor = (IceBoxDescriptor)_descriptor.descriptor;
	    
	    //
	    // We need to pass the node to register the adapters
	    // 
	    _serviceInstances = new ServiceInstances(_iceBoxDescriptor.services, 
						     _model, node);
	    addChild(_serviceInstances);
	    _serviceInstances.addParent(this); // no-op when newNode == true
	}
	else
	{
	    _iceBoxDescriptor = null;
	}

	_adapters = new Adapters(_descriptor.descriptor.adapters, _model, node);
	addChild(_adapters);
	_adapters.addParent(this); // no-op when newNode == true

	_dbEnvs = new DbEnvs(_descriptor.descriptor.dbEnvs, _model, false);
	addChild(_dbEnvs);
	_dbEnvs.addParent(this); // no-op when newNode == true

	if(newNode)
	{
	    updateDynamicInfo(node.getServerDynamicInfo(_id), false);
	    addParent(node); // propagates to children
	    node.addChild(this, fireNodeViewEvent);
	}
    }

    void updateDynamicInfo(ServerDynamicInfo info)
    {
	updateDynamicInfo(info, true);
    }
    
    private void updateDynamicInfo(ServerDynamicInfo info, boolean fireEvent)
    {
	if(info.state != _state || info.pid != _pid)
	{
	    _state = info.state;
	    _pid = info.pid;
	    _toolTip = toolTip(info.state, info.pid);
	    _stateIconIndex = (_state == null ? 0 : _state.value() + 1); 

	    if(fireEvent)
	    {
		//
		// Change the node representation in all views
		//
		fireNodeChangedEvent(this);
	    }
	}
    }
   
    void start()
    {
	//
	// TODO: if this can take a long time, make the invocation in a separate thread
	//

	boolean started = false;
	try
	{
	    _model.getStatusBar().setText("Starting server '" + _id + "'...");
	    started = _model.getAdmin().startServer(_id);
	}
	catch(IceGrid.ServerNotExistException e)
	{
	    _model.getStatusBar().setText("Server '" + _id + "' no longer exists.");
	}
	catch(IceGrid.NodeUnreachableException e)
	{
	    _model.getStatusBar().setText("Could not reach the node for server '" + _id + "'.");
	}
	catch(Ice.LocalException e)
	{
	    _model.getStatusBar().setText("Starting server '" + _id + "'... failed: " + e.toString());
	}
	if(started)
	{
	    _model.getStatusBar().setText("Starting server '" + _id + "'... success!");
	}
	else
	{
	    _model.getStatusBar().setText("Starting server '" + _id + "'... failed!");
	}
    }

    void stop()
    {
	try
	{
	    _model.getStatusBar().setText("Stopping server '" + _id + "'...");
	    _model.getAdmin().stopServer(_id);
	}
	catch(IceGrid.ServerNotExistException e)
	{
	    _model.getStatusBar().setText("Server '" + _id + "' no longer exists.");
	}
	catch(IceGrid.NodeUnreachableException e)
	{
	    _model.getStatusBar().setText("Could not reach the node for server '" + _id + "'.");
	}
	catch(Ice.LocalException e)
	{
	    _model.getStatusBar().setText("Stopping server '" + _id + "'... failed: " + e.toString());
	}
	_model.getStatusBar().setText("Stopping server '" + _id + "'... done.");
    }

    ServerState getState()
    {
	return _state;
    }

    ServerInstanceDescriptor getDescriptor()
    {
	return _descriptor;
    }


    public String toString()
    {
	String result = _descriptor.descriptor.name;

	if(!_descriptor.template.equals(""))
	{
	    
	    result += ": " + templateLabel(_descriptor.template, 
					   _descriptor.parameterValues.values());
	}
	return result;
    }

    private static String toolTip(ServerState state, int pid)
    {
	String result = (state == null ? "Unknown" : state.toString());

	if(pid != 0)
	{
	    result += ", pid: " + pid;
	}
	return result;
    }

    private ServerState _state = null;
    private int _stateIconIndex = 0;
    private int _pid = 0;
    private String _toolTip = toolTip(_state, _pid);

    private ServerInstanceDescriptor _descriptor;
    private TemplateDescriptor _templateDescriptor;
    private IceBoxDescriptor _iceBoxDescriptor;
   
    //
    // Children
    //
    private ServiceInstances _serviceInstances;
    private Adapters _adapters;
    private DbEnvs _dbEnvs;

    static private DefaultTreeCellRenderer _cellRenderer;
    static private Icon[] _icons;
    static private PopupMenu _popup;
    static private Editor _editor;
  
}
