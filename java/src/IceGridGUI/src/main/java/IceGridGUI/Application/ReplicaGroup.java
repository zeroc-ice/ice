// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceGridGUI.Application;

import java.awt.Component;
import javax.swing.JTree;
import javax.swing.tree.DefaultTreeCellRenderer;

import IceGrid.*;
import IceGridGUI.*;

class ReplicaGroup extends TreeNode
{
    static public ReplicaGroupDescriptor
    copyDescriptor(ReplicaGroupDescriptor d)
    {
        return d.clone();
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
            _cellRenderer.setLeafIcon(Utils.getIcon("/icons/16x16/replica_group.png"));
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

        if(!_ephemeral)
        {
            actions[SHOW_VARS] = true;
            actions[SUBSTITUTE_VARS] = true;
        }
        return actions;
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
    public void destroy()
    {
        ReplicaGroups replicaGroups = (ReplicaGroups)_parent;
        replicaGroups.removeChild(this);

        if(!_ephemeral)
        {
            replicaGroups.removeDescriptor(_descriptor);
            replicaGroups.getEditable().removeElement(_id, _editable, ReplicaGroup.class);
            getRoot().updated();
        }
    }

    @Override
    public Editor getEditor()
    {
        if(_editor == null)
        {
            _editor = (ReplicaGroupEditor)getRoot().getEditor(ReplicaGroupEditor.class, this);
        }
        _editor.show(this);
        return _editor;
    }

    @Override
    protected Editor createEditor()
    {
        return new ReplicaGroupEditor();
    }

    @Override
    public boolean isEphemeral()
    {
        return _ephemeral;
    }

    @Override
    Object getDescriptor()
    {
        return _descriptor;
    }

    Object saveDescriptor()
    {
        return _descriptor.clone();
    }

    void restoreDescriptor(Object savedDescriptor)
    {
        ReplicaGroupDescriptor clone = (ReplicaGroupDescriptor)savedDescriptor;
        _descriptor.id = clone.id;
        _descriptor.description = clone.description;
        _descriptor.objects = clone.objects;
        _descriptor.loadBalancing = clone.loadBalancing;
        _descriptor.proxyOptions = clone.proxyOptions;
    }

    void commit()
    {
        _editable.commit();
    }

    Editable getEditable()
    {
        return _editable;
    }

    ReplicaGroup(boolean brandNew, TreeNode parent, ReplicaGroupDescriptor descriptor)
    {
        super(parent, descriptor.id);
        _ephemeral = false;
        _editable = new Editable(brandNew);
        rebuild(descriptor);
    }

    ReplicaGroup(TreeNode parent, ReplicaGroupDescriptor descriptor)
    {
        super(parent, descriptor.id);
        _ephemeral = true;
        _editable = null;
        rebuild(descriptor);
    }

    @Override
    void write(XMLWriter writer)
        throws java.io.IOException
    {
        if(!_ephemeral)
        {
            java.util.List<String[]> attributes = new java.util.LinkedList<String[]>();
            attributes.add(createAttribute("id", _descriptor.id));
            if(_descriptor.proxyOptions.length() > 0)
            {
                attributes.add(createAttribute("proxy-options", _descriptor.proxyOptions));
            }
            
            if(_descriptor.loadBalancing == null &&
               _descriptor.description.length() == 0 && 
               _descriptor.objects.isEmpty())
            {
                writer.writeElement("replica-group", attributes);
            }
            else
            {
                writer.writeStartTag("replica-group", attributes);

                if(_descriptor.description.length() > 0)
                {
                    writer.writeElement("description", _descriptor.description);
                }
                assert _descriptor.loadBalancing != null;

                attributes.clear();
                if(_descriptor.loadBalancing instanceof RandomLoadBalancingPolicy)
                {
                    attributes.add(createAttribute("type", "random"));
                }
                else if(_descriptor.loadBalancing instanceof OrderedLoadBalancingPolicy)
                {
                    attributes.add(createAttribute("type", "ordered"));
                }
                else if(_descriptor.loadBalancing instanceof RoundRobinLoadBalancingPolicy)
                {
                    attributes.add(createAttribute("type", "round-robin"));
                }
                else if(_descriptor.loadBalancing instanceof AdaptiveLoadBalancingPolicy)
                {
                    attributes.add(createAttribute("type", "adaptive"));
                    AdaptiveLoadBalancingPolicy policy = (AdaptiveLoadBalancingPolicy)_descriptor.loadBalancing;
                    attributes.add(createAttribute("load-sample", policy.loadSample));
                }
                attributes.add(createAttribute("n-replicas", _descriptor.loadBalancing.nReplicas));
                writer.writeElement("load-balancing", attributes);

                writeObjects("object", writer, _descriptor.objects, null);
                writer.writeEndTag("replica-group");
            }
        }
    }

    void rebuild(ReplicaGroupDescriptor descriptor)
    {
        _descriptor = descriptor;
        //
        // And that's it since there is no children
        //
    }

    private ReplicaGroupDescriptor _descriptor;
    private final boolean _ephemeral;
    private final Editable _editable;
    private ReplicaGroupEditor _editor;

    static private DefaultTreeCellRenderer _cellRenderer;
}
