// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceGridGUI.Application;

import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;

import javax.swing.JComboBox;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JTextField;

import com.jgoodies.forms.builder.DefaultFormBuilder;
import com.jgoodies.forms.layout.CellConstraints;

import com.zeroc.IceGrid.*;
import com.zeroc.IceGridGUI.*;

@SuppressWarnings("unchecked")
class ReplicaGroupEditor extends Editor
{
    @Override
    protected boolean applyUpdate(boolean refresh)
    {
        ReplicaGroup replicaGroup = (ReplicaGroup)_target;
        Root root = replicaGroup.getRoot();

        root.disableSelectionListener();
        try
        {
            if(replicaGroup.isEphemeral())
            {
                ReplicaGroups replicaGroups = (ReplicaGroups)replicaGroup.getParent();
                writeDescriptor();
                ReplicaGroupDescriptor descriptor = (ReplicaGroupDescriptor)replicaGroup.getDescriptor();
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
                        replicaGroups.insertChild(replicaGroup, true);
                    }
                    catch(UpdateFailedException die)
                    {
                        assert false;
                    }
                    root.setSelectedNode(replicaGroup);

                    JOptionPane.showMessageDialog(
                        root.getCoordinator().getMainFrame(),
                        e.toString(),
                        "Apply failed",
                        JOptionPane.ERROR_MESSAGE);
                    return false;
                }

                //
                // Success
                //
                _target = replicaGroups.findChildWithDescriptor(descriptor);
                root.updated();
            }
            else if(isSimpleUpdate())
            {
                writeDescriptor();
                root.updated();
                replicaGroup.getEditable().markModified();
            }
            else
            {
                //
                // Save to be able to rollback
                //
                Object savedDescriptor = replicaGroup.saveDescriptor();
                ReplicaGroups replicaGroups = (ReplicaGroups)replicaGroup.getParent();
                writeDescriptor();
                ReplicaGroupDescriptor descriptor = (ReplicaGroupDescriptor)replicaGroup.getDescriptor();

                replicaGroups.removeChild(replicaGroup);
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
                        replicaGroups.insertChild(replicaGroup, true);
                    }
                    catch(UpdateFailedException die)
                    {
                        assert false;
                    }
                    replicaGroup.restoreDescriptor(savedDescriptor);
                    root.setSelectedNode(_target);

                    JOptionPane.showMessageDialog(
                        root.getCoordinator().getMainFrame(),
                        e.toString(),
                        "Apply failed",
                        JOptionPane.ERROR_MESSAGE);
                    return false;
                }

                //
                // Success
                //

                // replaced by brand new ReplicaGroup
                replicaGroups.getEditable().
                    removeElement(replicaGroup.getId(), replicaGroup.getEditable(), ReplicaGroup.class);

                _target = replicaGroups.findChildWithDescriptor(descriptor);
                root.updated();

                if(refresh)
                {
                    root.setSelectedNode(_target);
                }
            }

            if(refresh)
            {
                root.getCoordinator().getCurrentTab().showNode(_target);
            }
            _applyButton.setEnabled(false);
            _discardButton.setEnabled(false);
            return true;
        }
        finally
        {
            root.enableSelectionListener();
        }
    }

    @Override
    Utils.Resolver getDetailResolver()
    {
        Root root = _target.getRoot();

        if(root.getCoordinator().substitute())
        {
            return root.getResolver();
        }
        else
        {
            return null;
        }
    }

    ReplicaGroupEditor()
    {
        _objects = new ArrayMapField(this, true, "Identity", "Type", "Proxy Options");

        //
        // load balancing
        //
        _loadBalancing.addItemListener(new ItemListener()
            {
                @Override
                public void itemStateChanged(ItemEvent e)
                {
                    if(e.getStateChange() == ItemEvent.SELECTED)
                    {
                        updated();

                        Object item = e.getItem();
                        _loadSampleLabel.setVisible(item == ADAPTIVE);
                        _loadSample.setVisible(item == ADAPTIVE);
                    }
                }
            });
        _loadBalancing.setToolTipText(
            "<html>Specifies how IceGrid selects adapters and return<br>"
            + "their endpoints when resolving a replica group ID</html>");

        //
        // Associate updateListener with various fields
        //
        _id.getDocument().addDocumentListener(_updateListener);
        _id.setToolTipText("Must be unique within this IceGrid deployment");

        _description.getDocument().addDocumentListener(_updateListener);
        _description.setToolTipText("An optional description for this replica group");

        _nReplicas.getDocument().addDocumentListener(_updateListener);
        _nReplicas.setToolTipText("<html>IceGrid returns the endpoints of "
                                  + "up to <i>number</i> adapters<br>"
                                  + "when resolving a replica group ID.<br>"
                                  + "Enter 0 to returns the endpoints of all adapters.</html>");

        _loadSample.setEditable(true);
        JTextField loadSampleTextField = (JTextField)_loadSample.getEditor().getEditorComponent();
        loadSampleTextField.getDocument().addDocumentListener(_updateListener);
        _loadSample.setToolTipText("Use the load average or CPU usage over the last 1, 5 or 15 minutes?");

        _proxyOptions.getDocument().addDocumentListener(_updateListener);
        _proxyOptions.setToolTipText("The proxy options used for proxies created by IceGrid for the replica group");

        _filter.getDocument().addDocumentListener(_updateListener);
        _filter.setToolTipText("An optional filter for this replica group. Filters are installed by registry" +
                               "plugin to provide custom load balancing for replica groups.");
    }

    void writeDescriptor()
    {
        ReplicaGroupDescriptor descriptor = (ReplicaGroupDescriptor)getReplicaGroup().getDescriptor();

        descriptor.id = _id.getText().trim();
        descriptor.description = _description.getText();
        descriptor.objects = _objectList;
        descriptor.proxyOptions = _proxyOptions.getText().trim();
        descriptor.filter = _filter.getText();
        Object loadBalancing = _loadBalancing.getSelectedItem();
        if(loadBalancing == ORDERED)
        {
            descriptor.loadBalancing =  new OrderedLoadBalancingPolicy(_nReplicas.getText().trim());
        }
        else if(loadBalancing == RANDOM)
        {
            descriptor.loadBalancing = new RandomLoadBalancingPolicy(_nReplicas.getText().trim());
        }
        else if(loadBalancing == ROUND_ROBIN)
        {
            descriptor.loadBalancing = new RoundRobinLoadBalancingPolicy(_nReplicas.getText().trim());
        }
        else if(loadBalancing == ADAPTIVE)
        {
            descriptor.loadBalancing = new AdaptiveLoadBalancingPolicy(
                _nReplicas.getText().trim(), _loadSample.getSelectedItem().toString().trim());
        }
        else
        {
            assert false;
        }
    }

    boolean isSimpleUpdate()
    {
        ReplicaGroupDescriptor descriptor = (ReplicaGroupDescriptor)getReplicaGroup().getDescriptor();
        return descriptor.id.equals(_id.getText().trim());
    }

    @Override
    protected void appendProperties(DefaultFormBuilder builder)
    {
        builder.append("Replica Group ID");
        builder.append(_id, 3);
        builder.nextLine();

        builder.append("Description");
        builder.nextLine();
        builder.append("");
        builder.nextRow(-2);
        CellConstraints cc = new CellConstraints();
        JScrollPane scrollPane = new JScrollPane(_description);
        builder.add(scrollPane, cc.xywh(builder.getColumn(), builder.getRow(), 3, 3));
        builder.nextRow(2);
        builder.nextLine();

        builder.append("Proxy Options");
        builder.append(_proxyOptions, 3);
        builder.nextLine();

        builder.append("Filter");
        builder.append(_filter, 3);
        builder.nextLine();

        builder.append("Well-known Objects");
        builder.nextLine();
        builder.append("");
        builder.nextLine();
        builder.append("");
        builder.nextLine();
        builder.append("");
        builder.nextRow(-6);
        scrollPane = new JScrollPane(_objects);
        builder.add(scrollPane, cc.xywh(builder.getColumn(), builder.getRow(), 3, 7));
        builder.nextRow(6);
        builder.nextLine();

        builder.append("Load Balancing Policy");
        builder.append(_loadBalancing, 3);
        builder.nextLine();
        builder.append("How many Adapters? (0 = all)");
        builder.append(_nReplicas, 3);
        builder.nextLine();
        _loadSampleLabel = builder.append("Load Sample");
        builder.append(_loadSample, 3);
        builder.nextLine();
    }

    @Override
    protected void buildPropertiesPanel()
    {
        super.buildPropertiesPanel();
        _propertiesPanel.setName("Replica Group Properties");
    }

    @Override
    protected boolean validate()
    {
        //
        // First validate stringified identities
        //
        _objectList = mapToObjectDescriptorSeq(_objects.get());

        if(_objectList == null)
        {
            return false;
        }

        return check(new String[]{"Replica Group ID", _id.getText().trim()});
    }

    void show(ReplicaGroup replicaGroup)
    {
        //
        // Make sure everything is built
        //
        getProperties();

        detectUpdates(false);
        _target = replicaGroup;

        Utils.Resolver resolver = getDetailResolver();
        boolean isEditable = (resolver == null);

        ReplicaGroupDescriptor descriptor = (ReplicaGroupDescriptor)replicaGroup.getDescriptor();

        _id.setText(descriptor.id);
        _id.setEditable(isEditable);

        _description.setText(Utils.substitute(descriptor.description, resolver));
        _description.setEditable(isEditable);
        _description.setOpaque(isEditable);

        _proxyOptions.setText(descriptor.proxyOptions);
        _proxyOptions.setEditable(isEditable);

        _filter.setText(descriptor.filter);
        _filter.setEditable(isEditable);

        _objects.set(objectDescriptorSeqToMap(descriptor.objects), resolver, isEditable);

        _loadBalancing.setEnabled(true);

        if(descriptor.loadBalancing == null)
        {
            _loadBalancing.setSelectedItem(RANDOM);
            _nReplicas.setText("0");
            _loadSample.setSelectedItem("1");
        }
        else if(descriptor.loadBalancing instanceof RandomLoadBalancingPolicy)
        {
            _loadBalancing.setSelectedItem(RANDOM);
            _nReplicas.setText(Utils.substitute(descriptor.loadBalancing.nReplicas, resolver));
            _loadSample.setSelectedItem("1");
        }
        else if(descriptor.loadBalancing instanceof OrderedLoadBalancingPolicy)
        {
            _loadBalancing.setSelectedItem(ORDERED);
            _nReplicas.setText(Utils.substitute(descriptor.loadBalancing.nReplicas, resolver));
            _loadSample.setSelectedItem("1");
        }
        else if(descriptor.loadBalancing instanceof RoundRobinLoadBalancingPolicy)
        {
            _loadBalancing.setSelectedItem(ROUND_ROBIN);
            _nReplicas.setText(Utils.substitute(descriptor.loadBalancing.nReplicas, resolver));
            _loadSample.setSelectedItem("1");
        }
        else if(descriptor.loadBalancing instanceof AdaptiveLoadBalancingPolicy)
        {
            _loadBalancing.setSelectedItem(ADAPTIVE);
            _nReplicas.setText(Utils.substitute(descriptor.loadBalancing.nReplicas, resolver));

            _loadSample.setSelectedItem(
                Utils.substitute(((AdaptiveLoadBalancingPolicy)descriptor.loadBalancing).loadSample, resolver));
        }
        else
        {
            assert false;
        }
        _nReplicas.setEditable(isEditable);
        _loadSample.setEditable(isEditable);
        _loadBalancing.setEnabled(isEditable);

        _applyButton.setEnabled(replicaGroup.isEphemeral());
        _discardButton.setEnabled(replicaGroup.isEphemeral());
        detectUpdates(true);
        if(replicaGroup.isEphemeral())
        {
            updated();
        }
    }

    private ReplicaGroup getReplicaGroup()
    {
        return (ReplicaGroup)_target;
    }

    private java.util.Map<String, String[]> objectDescriptorSeqToMap(java.util.List<ObjectDescriptor> objects)
    {
        java.util.Map<String, String[]> result = new java.util.TreeMap<>();
        for(ObjectDescriptor p : objects)
        {
            result.put(com.zeroc.Ice.Util.identityToString(p.id), new String[]{p.type, p.proxyOptions});
        }
        return result;
    }

    private java.util.LinkedList<ObjectDescriptor> mapToObjectDescriptorSeq(java.util.Map<String, String[]> map)
    {
        String badIdentities = "";
        java.util.LinkedList<ObjectDescriptor> result = new java.util.LinkedList<>();

        for(java.util.Map.Entry<String, String[]> p : map.entrySet())
        {
            try
            {
                com.zeroc.Ice.Identity id = com.zeroc.Ice.Util.stringToIdentity(p.getKey());
                String[] val = p.getValue();
                result.add(new ObjectDescriptor(id, val[0], val[1]));
            }
            catch(com.zeroc.Ice.IdentityParseException ex)
            {
                badIdentities += "- " + p.getKey() + "\n";
            }
        }
        if(!badIdentities.equals(""))
        {
            JOptionPane.showMessageDialog(
                _target.getCoordinator().getMainFrame(),
                "The following identities could not be parsed properly:\n" + badIdentities,
                "Validation failed",
                JOptionPane.ERROR_MESSAGE);

            return null;
        }
        else
        {
            return result;
        }
    }

    static private String ORDERED = "Ordered";
    static private String RANDOM = "Random";
    static private String ROUND_ROBIN = "Round-robin";
    static private String ADAPTIVE = "Adaptive";

    private JTextField _id = new JTextField(20);
    private JTextArea _description = new JTextArea(3, 20);
    private JTextField _proxyOptions = new JTextField(20);
    private JTextField _filter = new JTextField(20);

    private JComboBox _loadBalancing = new JComboBox(new String[] {ADAPTIVE, 
                                                                   ORDERED, 
                                                                   RANDOM, 
                                                                   ROUND_ROBIN});

    private JTextField _nReplicas = new JTextField(20);

    private JLabel _loadSampleLabel;
    private JComboBox _loadSample = new JComboBox(new String[] {"1", "5", "15"});

    private ArrayMapField _objects;
    private java.util.LinkedList<ObjectDescriptor> _objectList;
}
