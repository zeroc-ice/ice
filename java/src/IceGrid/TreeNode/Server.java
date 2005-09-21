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

import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.DefaultComboBoxModel;
import javax.swing.Icon;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JComboBox;
import javax.swing.JComponent;
import javax.swing.JLabel;
import javax.swing.JMenuItem;
import javax.swing.JPanel;
import javax.swing.JPopupMenu;
import javax.swing.JRadioButton;
import javax.swing.JScrollPane;
import javax.swing.JTextField;
import javax.swing.JToggleButton;
import javax.swing.JTree;
import javax.swing.UIManager;
import javax.swing.tree.DefaultTreeCellRenderer;

import com.jgoodies.forms.builder.DefaultFormBuilder;
import com.jgoodies.forms.factories.Borders;
import com.jgoodies.forms.layout.FormLayout;
import com.jgoodies.forms.layout.Sizes;
import com.jgoodies.forms.util.LayoutStyle;
import com.jgoodies.uif_lite.panel.SimpleInternalFrame;

import IceGrid.IceBoxDescriptor;
import IceGrid.Model;
import IceGrid.PropertyDescriptor;
import IceGrid.TemplateDescriptor;
import IceGrid.TreeModelI;
import IceGrid.ServerDescriptor;
import IceGrid.ServerDynamicInfo;
import IceGrid.ServerInstanceDescriptor;
import IceGrid.ServerState;
import IceGrid.Utils;

//
// We can have 4 kinds of servers:
// - Plain server (no template)
// - Icebox server (no template)
// - Server instance
// - Icebox instance
//
class Server extends EditableParent
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

		//
		// Type combox box
		//
		builder.append("Type");
		builder.append(_type, 3);
		builder.nextLine();
		
		//
		// Template combox box
		//
		builder.append(_templateLabel, _template, _templateButton);
		builder.nextLine();

		//
		// Parameters
		//
		builder.append(_parameterValuesLabel, _parameterValues, _parameterValuesButton);
		builder.nextLine();

		builder.setLineGapSize(LayoutStyle.getCurrent().getParagraphPad());
		
		_serverDescriptorEditor.build(builder);

		_scrollPane = new JScrollPane(builder.getPanel(),
					      JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED, 
					      JScrollPane.HORIZONTAL_SCROLLBAR_NEVER);
		_scrollPane.setBorder(Borders.DIALOG_BORDER);
	    }
	    return _scrollPane;
	}

	void show(Server server)
	{
	    /*
	    _server = server;
	    ServerInstanceDescriptor descriptor = server.getDescriptor();

	    ServerDescriptor serverDescriptor = null;
	    Ice.StringHolder toolTipHolder = new Ice.StringHolder();
	   
	    java.util.Map[] variables = null;

	    if(descriptor.template.length() == 0)
	    {
		serverDescriptor = null;
		if(_server.getModel().substituteVariables())
		{
		    variables = new java.util.Map[] {
			server.getNodeVariables(), 
			server.getApplication().getVariables()};
		}

		if(serverDescriptor instanceof IceBoxDescriptor)
		{
		    _type.setSelectedIndex(ICEBOX);
		}
		else
		{
		    _type.setSelectedIndex(PLAIN_SERVER);
		}

		_templateLabel.setEnabled(false);
		_template.setEnabled(false);
		_template.setModel(new DefaultComboBoxModel());
		_template.setSelectedItem(null);
		_templateButton.setEnabled(false);

		_parameterValuesLabel.setEnabled(false);
		_parameterValues.setEnabled(false);
		_parameterValues.setText("");
		_parameterValues.setToolTipText(null);
		_parameterValuesButton.setEnabled(false);
	    }
	    else
	    {
		_type.setSelectedIndex(TEMPLATE_INSTANCE);

		_templateLabel.setEnabled(true);
		_template.setEnabled(true);
		_templateButton.setEnabled(true);
		
		Application application = _server.getApplication();

		TemplateDescriptor templateDescriptor =
		    application.findServerTemplateDescriptor(descriptor.template);

		_template.setModel(application.getServerTemplates().getComboBoxModel());
		_template.setSelectedItem(templateDescriptor);
		serverDescriptor = (ServerDescriptor)templateDescriptor.descriptor;

		_parameterValuesLabel.setEnabled(true);
		_parameterValues.setEnabled(true);
		_parameterValuesButton.setEnabled(true);
		
		String stringifiedParameterValues;
		if(_server.getModel().substituteVariables())
		{
		    java.util.Map substitutedParameters = Utils.substituteVariables(
			descriptor.parameterValues, _server.getNodeVariables(), 
			application.getVariables());
		    
		    stringifiedParameterValues = Utils.stringify(substitutedParameters, "=",
								 ", ", toolTipHolder);
		    _parameterValues.setEditable(false);

		    variables = new java.util.Map[]{
			substitutedParameters, 
			server.getNodeVariables(),
			application.getVariables()};
		}
		else
		{
		    stringifiedParameterValues = Utils.stringify(descriptor.parameterValues, "=",
								 ", ", toolTipHolder);
		    _parameterValues.setEditable(true);
		}
		_parameterValues.setText(stringifiedParameterValues);
		_parameterValues.setToolTipText(toolTipHolder.value);
	    }

	    _targets.setText(Utils.stringify(descriptor.targets, ", ", toolTipHolder));
	    _targets.setToolTipText(toolTipHolder.value);

	    _serverDescriptorEditor.show(serverDescriptor, variables);
	    */
	}
	
	Editor(boolean editDetails)
	{
	    _serverDescriptorEditor = new ServerDescriptorEditor(editDetails);
	    //
	    // gotoTemplate action
	    //
	    AbstractAction gotoTemplate = new AbstractAction("->")
		{
		    public void actionPerformed(ActionEvent e) 
		    {
			ServerTemplate template = (ServerTemplate)
			    _template.getSelectedItem();
			if(template != null && _server != null)
			{
			    _server.getModel().getTree().setSelectionPath
				(template.getPath());
			}
		    }
		};
	    gotoTemplate.putValue(Action.SHORT_DESCRIPTION, "Goto this template");
	    _templateButton = new JButton(gotoTemplate);
	    _parameterValuesButton = new JButton("...");
	}


	private final int PLAIN_SERVER = 0;
	private final int ICEBOX = 1;
	private final int TEMPLATE_INSTANCE = 2;
	private String[] _typeList = {"Plain Server", "IceBox Server", "Template Instance"};
	private JComboBox _type = new JComboBox(_typeList);

	private JLabel _templateLabel = new JLabel("Template");
	private JComboBox _template = new JComboBox();
	private JButton _templateButton;
	private JLabel  _parameterValuesLabel = new JLabel("Parameters");
	private JTextField _parameterValues = new JTextField(20);
	private JButton _parameterValuesButton;

	private ServerDescriptorEditor _serverDescriptorEditor;

	private Server _server;
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
	
	void setServer(Server server)
	{
	    _server = server;
	    ServerState state = _server.getState();

	    boolean canStart = (_server.getState() == ServerState.Inactive);
	    _startAction.setEnabled(canStart);
	    _stopAction.setEnabled(!canStart);
	}

	private Server _server;
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

    public void displayProperties()
    {
	SimpleInternalFrame propertiesFrame = _model.getPropertiesFrame();

	propertiesFrame.setTitle("Properties for " + _id);
	
	//
	// Pick the appropriate editor
	//
	Editor editor = null;
	if(_instanceDescriptor != null || _model.substitute())
	{
	    if(_editor == null)
	    {
		_editor = new Editor(false);
	    }

	    editor = _editor;
	}
	else
	{
	    if(_fullEditor == null)
	    {
		_fullEditor = new Editor(true);
	    }
	    editor = _fullEditor;
	}

	editor.show(this);
	propertiesFrame.setContent(editor.getComponent());
	propertiesFrame.validate();
	propertiesFrame.repaint();
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
	    _icons[ServerState.Activating.value() + 1] = 
		Utils.getIcon("/icons/activating.png");
	    _icons[ServerState.Active.value() + 1] = 
		Utils.getIcon("/icons/active.png");
	    _icons[ServerState.Deactivating.value() + 1] = 
		Utils.getIcon("/icons/deactivating.png");
	    _icons[ServerState.Destroying.value() + 1] = 
		Utils.getIcon("/icons/destroying.png");
	    _icons[ServerState.Destroyed.value() + 1] = 
		Utils.getIcon("/icons/destroyed.png");
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


    public PropertiesHolder getPropertiesHolder()
    {
	return _propertiesHolder;
    }

    static public ServerInstanceDescriptor
    copyDescriptor(ServerInstanceDescriptor sid)
    {
	return (ServerInstanceDescriptor)sid.clone();
    }

    static public ServerDescriptor
    copyDescriptor(ServerDescriptor sd)
    {
	ServerDescriptor copy = null;
	copy = (ServerDescriptor)sd.clone();
	copy.adapters = Adapters.copyDescriptors(copy.adapters);
	copy.dbEnvs = DbEnvs.copyDescriptors(copy.dbEnvs);
	// TODO: copy.patchs = Patchs.copyDescriptor(copy.patchs);

	if(copy instanceof IceBoxDescriptor)
	{
	    IceBoxDescriptor ib = (IceBoxDescriptor)copy;
	    ib.services = Services.copyDescriptors(ib.services);
	}
	return copy;
    }
    

    public Object getDescriptor()
    {
	if(_instanceDescriptor != null)
	{
	    return _instanceDescriptor;
	}
	else
	{
	    return _serverDescriptor;
	}
    }

    public Object copy()
    {
	if(_instanceDescriptor != null)
	{
	    return copyDescriptor(_instanceDescriptor);
	}
	else
	{
	    return copyDescriptor(_serverDescriptor);
	}
    }

    //
    // Builds the server and all its sub-tree
    //
    Server(boolean brandNew, String serverId, 
	   Utils.Resolver resolver, ServerInstanceDescriptor instanceDescriptor,
	   ServerDescriptor serverDescriptor,
	   Application application) throws DuplicateIdException
    {
	super(brandNew, serverId, application.getModel());
	Ice.IntHolder pid = new Ice.IntHolder();
	_state = application.registerServer(resolver.find("node"),
					    _id,
					    this,
					    pid);
	_pid = pid.value;
	rebuild(resolver, instanceDescriptor, serverDescriptor, application);
    }

    Server(Server o)
    {
	super(o, true);

	_state = o._state;
	_stateIconIndex = o._stateIconIndex;
	_pid = o._pid;
	_toolTip = o._toolTip;

	_instanceDescriptor = o._instanceDescriptor;
	_serverDescriptor = o._serverDescriptor;

	_resolver = o._resolver;

	_services = o._services;
	_adapters = o._adapters;
	_dbEnvs = o._dbEnvs;
    }

    //
    // Update the server and all its subtree
    //
    void rebuild(Utils.Resolver resolver,  
		 ServerInstanceDescriptor instanceDescriptor,
		 ServerDescriptor serverDescriptor,
		 Application application) throws DuplicateIdException
    {
	assert serverDescriptor != null;
	_resolver = resolver;
	_instanceDescriptor = instanceDescriptor;
	_serverDescriptor = serverDescriptor;
	clearChildren();
	
	boolean isEditable = (instanceDescriptor == null);
	_propertiesHolder = new PropertiesHolder(serverDescriptor);	

	if(serverDescriptor instanceof IceBoxDescriptor)
	{
	    IceBoxDescriptor iceBoxDescriptor = (IceBoxDescriptor)serverDescriptor;
	   
	    _services = new Services(iceBoxDescriptor.services,
				     isEditable ? this : null, _resolver, application);
	    addChild(_services);
	    _services.setParent(this);
	    
	    //
	    // IceBox has not dbEnv
	    //
	    assert serverDescriptor.dbEnvs.size() == 0;
	    _dbEnvs = null;
	}
	else
	{
	    _services = null;   	
	    _dbEnvs = new DbEnvs(serverDescriptor.dbEnvs, 
				 isEditable, _resolver, _model);
	    addChild(_dbEnvs);
	    _dbEnvs.setParent(this);
	}

	_adapters = new Adapters(serverDescriptor.adapters, 
				 isEditable, _services != null, 
				 _resolver, application, _model);
	addChild(_adapters);
	_adapters.setParent(this);
    }

    public void unregister()
    {
	getApplication().unregisterServer(_resolver.find("node"),
					  _id,
					  this);
	_adapters.unregister();
	if(_services != null)
	{
	    _services.unregister();
	}
    }
    
    void cascadeDeleteServiceInstance(String templateId)
    {
	if(_services != null)
	{
	    _services.cascadeDeleteServiceInstance(templateId);
	}
    }

    
    void updateDynamicInfo(ServerState state, int pid)
    {
	if(state != _state || pid != _pid)
	{
	    _state = state;
	    _pid = pid;
	    
	    _toolTip = toolTip(_state, _pid);
	    _stateIconIndex = _state.value() + 1;
	
	    //
	    // Change the node representation
	    //
	    fireNodeChangedEvent(this);
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
	    _model.getStatusBar().setText("Could not reach the node for server '" + _id 
					  + "'.");
	}
	catch(Ice.LocalException e)
	{
	    _model.getStatusBar().setText("Starting server '" + _id + "'... failed: " 
					  + e.toString());
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
	    _model.getStatusBar().setText("Could not reach the node for server '" 
					  + _id + "'.");
	}
	catch(Ice.LocalException e)
	{
	    _model.getStatusBar().setText("Stopping server '" + _id + "'... failed: " 
					  + e.toString());
	}
	_model.getStatusBar().setText("Stopping server '" + _id + "'... done.");
    }

    ServerState getState()
    {
	return _state;
    }

    ServerInstanceDescriptor getInstanceDescriptor()
    {
	return _instanceDescriptor;
    }

    public String toString()
    {
	if(_instanceDescriptor == null)
	{
	    return _id;
	}
	else
	{
	    return _id + ": " + templateLabel(_instanceDescriptor.template,
					      _resolver.getParameters().values());
	}
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

    private ServerInstanceDescriptor _instanceDescriptor;
    private ServerDescriptor _serverDescriptor;

    private Utils.Resolver _resolver;

    private PropertiesHolder _propertiesHolder;

    //
    // Children
    //
    private Services _services;
    private Adapters _adapters;
    private DbEnvs _dbEnvs;

    static private DefaultTreeCellRenderer _cellRenderer;
    static private Icon[] _icons;
    static private PopupMenu _popup;

    static private Editor _fullEditor; // writable server descriptor
    static private Editor _editor; // read-only server descriptor
  
}
