// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceGridGUI.Application;

import java.awt.Component;
import javax.swing.JPopupMenu;
import javax.swing.JTree;
import javax.swing.tree.DefaultTreeCellRenderer;

import IceGrid.*;
import IceGridGUI.*;

class ServiceInstance extends TreeNode implements Service, Cloneable
{
    static public ServiceInstanceDescriptor
    copyDescriptor(ServiceInstanceDescriptor instanceDescriptor)
    {
        ServiceInstanceDescriptor copy = instanceDescriptor.clone();

        copy.propertySet = PropertySet.copyDescriptor(copy.propertySet);

        if(copy.descriptor != null)
        {
            copy.descriptor = PlainService.copyDescriptor(copy.descriptor);
        }
        return copy;
    }

    static public java.util.List<ServiceInstanceDescriptor>
    copyDescriptors(java.util.List<ServiceInstanceDescriptor> descriptors)
    {
        java.util.List<ServiceInstanceDescriptor> copy = new java.util.LinkedList<ServiceInstanceDescriptor>();
        for(ServiceInstanceDescriptor p : descriptors)
        {
            copy.add(copyDescriptor(p));
        }
        return copy;
    }

    @Override
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
            _cellRenderer = new DefaultTreeCellRenderer();
            _cellRenderer.setLeafIcon(Utils.getIcon("/icons/16x16/service.png"));
        }

        return _cellRenderer.getTreeCellRendererComponent(tree, value, sel, expanded, leaf, row, hasFocus);
    }

    //
    // Actions
    //
    @Override
    public boolean[] getAvailableActions()
    {
        boolean[] actions = new boolean[ACTION_COUNT];
        actions[COPY] = !_ephemeral;

        if(((TreeNode)_parent).getAvailableActions()[PASTE])
        {
            actions[PASTE] = true;
        }

        actions[DELETE] = true;

        if(_parent instanceof Server && !_ephemeral)
        {
            actions[SHOW_VARS] = true;
            actions[SUBSTITUTE_VARS] = true;
        }

        actions[MOVE_UP] = canMove(true);
        actions[MOVE_DOWN] = canMove(false);
        return actions;
    }

    @Override
    public JPopupMenu getPopupMenu()
    {
        ApplicationActions actions = getCoordinator().getActionsForPopup();
        if(_popup == null)
        {
            _popup = new JPopupMenu();
            _popup.add(actions.get(MOVE_UP));
            _popup.add(actions.get(MOVE_DOWN));
        }
        actions.setTarget(this);
        return _popup;
    }

    @Override
    public void copy()
    {
        getCoordinator().setClipboard(copyDescriptor(_descriptor));
        getCoordinator().getActionsForMenu().get(PASTE).setEnabled(true);
    }

    @Override
    public void paste()
    {
        ((TreeNode)_parent).paste();
    }

    @Override
    public void moveUp()
    {
        move(true);
    }

    @Override
    public void moveDown()
    {
        move(false);
    }

    @Override
    public Object getDescriptor()
    {
        return _descriptor;
    }

    @Override
    public Object saveDescriptor()
    {
        //
        // Must be a shallow copy
        //
        ServiceInstanceDescriptor saved = _descriptor.clone();
        assert saved.descriptor == null;
        return saved;
    }

    @Override
    public void restoreDescriptor(Object savedDescriptor)
    {
        ServiceInstanceDescriptor sd = (ServiceInstanceDescriptor)savedDescriptor;
        _descriptor.template = sd.template;
        _descriptor.parameterValues = sd.parameterValues;
        _descriptor.propertySet = sd.propertySet;
    }

    @Override
    public void destroy()
    {
        ((Communicator)_parent).getServices().destroyChild(this);
    }

    @Override
    public Editor getEditor()
    {
        if(_editor == null)
        {
            _editor = (ServiceInstanceEditor)getRoot().getEditor(ServiceInstanceEditor.class, this);
        }
        _editor.show(this);
        return _editor;
    }

    @Override
    protected Editor createEditor()
    {
        return new ServiceInstanceEditor();
    }

    @Override
    public String toString()
    {
        if(_displayString != null)
        {
            return _displayString;
        }
        else
        {
            return super.toString();
        }
    }

    private boolean canMove(boolean up)
    {
        if(_ephemeral)
        {
            return false;
        }
        else
        {
            return ((Communicator)_parent).getServices().canMove(this, up);
        }
    }

    private void move(boolean up)
    {
        assert canMove(up);
        ((Communicator)_parent).getServices().move(this, up);
    }

    Editable getEnclosingEditable()
    {
        return ((Communicator)_parent).getEnclosingEditable();
    }

    static private class Backup
    {
        java.util.Map<String, String> parameterValues;
        ServiceInstance clone;
    }

    @Override
    public Object rebuild(java.util.List<Editable> editables)
        throws UpdateFailedException
    {
        Backup backup = new Backup();

        //
        // Fix-up _descriptor if necessary
        //
        if(_descriptor.template.length() > 0)
        {
            TemplateDescriptor templateDescriptor = getRoot().findServiceTemplateDescriptor(_descriptor.template);

            java.util.Set<String> parameters = new java.util.HashSet<String>(templateDescriptor.parameters);
            if(!parameters.equals(_descriptor.parameterValues.keySet()))
            {
                backup.parameterValues = _descriptor.parameterValues;
                _descriptor.parameterValues = Editor.makeParameterValues(
                    _descriptor.parameterValues, templateDescriptor.parameters);
                editables.add(getEnclosingEditable());
            }
        }

        Communicator communicator = (Communicator)_parent;
        Communicator.Services services = communicator.getServices();
        ServiceInstance newService = null;

        try
        {
            newService = (ServiceInstance)services.createChild(_descriptor);
        }
        catch(UpdateFailedException e)
        {
            if(backup.parameterValues != null)
            {
                _descriptor.parameterValues = backup.parameterValues;
            }
            throw e;
        }

        try
        {
            backup.clone = (ServiceInstance)clone();
        }
        catch(CloneNotSupportedException e)
        {
            assert false;
        }

        reset(newService);

        if(backup.parameterValues != null)
        {
            editables.add(getEnclosingEditable());
        }
        getRoot().getTreeModel().nodeChanged(this);
        return backup;
    }

    @Override
    public void restore(Object backupObj)
    {
        Backup backup = (Backup)backupObj;

        if(backup.parameterValues != null)
        {
            _descriptor.parameterValues = backup.parameterValues;
        }

        reset(backup.clone);
        getRoot().getTreeModel().nodeChanged(this);
    }

    private void reset(ServiceInstance from)
    {
        _id = from._id;
        _displayString = from._displayString;
        _resolver = from._resolver;
    }

    ServiceInstance(Communicator parent,
                    String name,
                    String displayString,
                    ServiceInstanceDescriptor instanceDescriptor,
                    Utils.Resolver resolver)
        throws UpdateFailedException
    {
        super(parent, name);
        _displayString = displayString;
        _descriptor = instanceDescriptor;
        _ephemeral = false;
        _resolver = resolver;
    }

    //
    // New temporary object
    //
    ServiceInstance(Communicator parent, String name, ServiceInstanceDescriptor instanceDescriptor)
    {
        super(parent, name);
        _descriptor = instanceDescriptor;
        _ephemeral = true;
    }

    @Override
    void write(XMLWriter writer)
        throws java.io.IOException
    {
        if(!_ephemeral)
        {
            TemplateDescriptor templateDescriptor = getRoot().findServiceTemplateDescriptor(_descriptor.template);

            java.util.LinkedList<String[]> attributes =
                parameterValuesToAttributes(_descriptor.parameterValues, templateDescriptor.parameters);
            attributes.addFirst(createAttribute("template", _descriptor.template));

            if(_descriptor.propertySet.references.length == 0 && _descriptor.propertySet.properties.size() == 0)
            {
                writer.writeElement("service-instance", attributes);
            }
            else
            {
                writer.writeStartTag("service-instance", attributes);
                writePropertySet(writer, _descriptor.propertySet, null, null);
                writer.writeEndTag("service-instance");
            }
        }
    }

    @Override
    Utils.Resolver getResolver()
    {
        return _resolver;
    }

    @Override
    public boolean isEphemeral()
    {
        return _ephemeral;
    }

    private ServiceInstanceDescriptor _descriptor;

    private String _displayString;
    private final boolean _ephemeral;

    private Utils.Resolver _resolver;
    private ServiceInstanceEditor _editor;

    static private DefaultTreeCellRenderer _cellRenderer;
    static private JPopupMenu _popup;
}
