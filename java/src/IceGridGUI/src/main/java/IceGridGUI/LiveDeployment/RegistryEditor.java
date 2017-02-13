// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceGridGUI.LiveDeployment;

import java.awt.event.ActionEvent;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JPopupMenu;
import javax.swing.JScrollPane;
import javax.swing.JTextField;
import javax.swing.KeyStroke;

import com.jgoodies.forms.builder.DefaultFormBuilder;
import com.jgoodies.forms.layout.CellConstraints;

import IceGrid.*;
import IceGridGUI.*;

class RegistryEditor extends Editor
{
    RegistryEditor()
    {
        _hostname.setEditable(false);

        Action openDefinition = new AbstractAction("Open definition")
            {
                @Override
                public void actionPerformed(ActionEvent e)
                {
                    int selectedRow = _applications.getSelectedRow();
                    if(selectedRow != -1)
                    {
                        String appName = (String)_applications.getValueAt(selectedRow, 0);
                        ApplicationPane app = _target.getCoordinator().openLiveApplication(appName);

                        if(app != null && app.getRoot().getSelectedNode() == null)
                        {
                            app.getRoot().setSelectedNode(app.getRoot());
                        }
                    }
                }
            };

        Action showDetails = new AbstractAction("Show details")
            {
                @Override
                public void actionPerformed(ActionEvent e)
                {
                    int selectedRow = _applications.getSelectedRow();
                    if(selectedRow != -1)
                    {
                        String appName = (String)_applications.getValueAt(selectedRow, 0);
                        _target.showApplicationDetails(appName);
                    }
                }
            };

        final Action patch = new AbstractAction("Patch distribution")
            {
                @Override
                public void actionPerformed(ActionEvent e)
                {
                    int selectedRow = _applications.getSelectedRow();
                    if(selectedRow != -1)
                    {
                        String appName = (String)_applications.getValueAt(selectedRow, 0);
                        _target.patch(appName);
                    }
                }
            };

        Action removeApplication = new AbstractAction("Remove from registry")
            {
                @Override
                public void actionPerformed(ActionEvent e)
                {
                    int selectedRow = _applications.getSelectedRow();
                    if(selectedRow != -1)
                    {
                        String appName = (String)_applications.getValueAt(selectedRow, 0);

                        int confirm = JOptionPane.showConfirmDialog(
                            _target.getCoordinator().getMainFrame(),
                            "You are about to remove application '" + appName + "' from the IceGrid registry. "
                            + "Do you want to proceed?",
                            "Remove Confirmation",
                            JOptionPane.YES_NO_OPTION);

                        if(confirm == JOptionPane.YES_OPTION)
                        {
                            _target.getCoordinator().removeApplicationFromRegistry(appName);
                        }
                    }
                }
            };

        removeApplication.putValue(Action.ACCELERATOR_KEY, KeyStroke.getKeyStroke("DELETE"));

        _applications.getActionMap().put("delete", removeApplication);
        _applications.getInputMap().put(KeyStroke.getKeyStroke("DELETE"), "delete");

        final JPopupMenu appPopup = new JPopupMenu();
        appPopup.add(openDefinition);
        appPopup.add(showDetails);
        appPopup.addSeparator();
        appPopup.add(patch);
        appPopup.addSeparator();
        appPopup.add(removeApplication);

        _applications.addMouseListener(new MouseAdapter()
            {
                @Override
                public void mouseClicked(MouseEvent e)
                {
                    if(e.getClickCount() == 2)
                    {
                        int selectedRow = _applications.getSelectedRow();
                        if(selectedRow != -1)
                        {
                            String appName = (String)_applications.getValueAt(selectedRow, 0);
                            _target.showApplicationDetails(appName);
                        }
                    }
                }

                @Override
                public void mousePressed(MouseEvent e)
                {
                    maybeShowPopup(e);
                }

                @Override
                public void mouseReleased(MouseEvent e)
                {
                    maybeShowPopup(e);
                }

                private void maybeShowPopup(MouseEvent e)
                {
                    int selectedRow = _applications.getSelectedRow();
                    if (e.isPopupTrigger() && selectedRow != -1)
                    {
                        String appName = (String)_applications.getValueAt(selectedRow, 0);
                        ApplicationDescriptor desc = _target.getApplicationDescriptor(appName);
                        patch.setEnabled(desc != null && desc.distrib.icepatch.length() > 0);
                        appPopup.show(_applications, e.getX(), e.getY());
                    }
                }
            });


        Action deleteObject = new AbstractAction("Remove selected object")
            {
                @Override
                public void actionPerformed(ActionEvent e)
                {
                    if(_target.getCoordinator().connectedToMaster())
                    {
                        int selectedRow = _objects.getSelectedRow();
                        if(selectedRow != -1)
                        {
                            _target.removeObject((String)_objects.getValueAt(selectedRow, 0));
                        }
                    }
                }
            };
        deleteObject.putValue(Action.ACCELERATOR_KEY, KeyStroke.getKeyStroke("DELETE"));

        _objects.getActionMap().put("delete", deleteObject);
        _objects.getInputMap().put(KeyStroke.getKeyStroke("DELETE"), "delete");

        Action showObject = new AbstractAction("Show details")
            {
                @Override
                public void actionPerformed(ActionEvent e)
                {
                    int selectedRow = _objects.getSelectedRow();
                    if(selectedRow != -1)
                    {
                        String proxy = (String)_objects.getValueAt(selectedRow, 0);
                        String type = (String)_objects.getValueAt(selectedRow, 1);
                        _target.showObject(proxy, type);
                    }
                }
            };

        Action addObject = new AbstractAction("Add a new well-known object")
            {
                @Override
                public void actionPerformed(ActionEvent e)
                {
                    if(_target.getCoordinator().connectedToMaster())
                    {
                        _target.addObject();
                    }
                }
            };
        addObject.putValue(Action.ACCELERATOR_KEY, KeyStroke.getKeyStroke("INSERT"));

        _objects.getActionMap().put("insert", addObject);
        _objects.getInputMap().put(KeyStroke.getKeyStroke("INSERT"), "insert");

        _objects.setToolTipText("<html>Well-known objects registered through the Admin interface.<br>"
                                + "Well-known objects registered using Adapter or Replica Group<br>"
                                + "definitions are not displayed here.</html>");

        final JPopupMenu objectsPopup = new JPopupMenu();
        objectsPopup.add(addObject);
        objectsPopup.addSeparator();
        final JMenuItem deleteObjectMenuItem = objectsPopup.add(deleteObject);
        objectsPopup.addSeparator();
        final JMenuItem showObjectMenuItem = objectsPopup.add(showObject);

        _objects.addMouseListener(new MouseAdapter()
            {
                @Override
                public void mouseClicked(MouseEvent e)
                {
                    if(e.getClickCount() == 2)
                    {
                        int selectedRow = _objects.getSelectedRow();
                        if(selectedRow != -1)
                        {
                            String proxy = (String)_objects.getValueAt(selectedRow, 0);
                            String type = (String)_objects.getValueAt(selectedRow, 1);
                            _target.showObject(proxy, type);
                        }
                    }
                }

                @Override
                public void mousePressed(MouseEvent e)
                {
                    maybeShowPopup(e);
                }

                @Override
                public void mouseReleased(MouseEvent e)
                {
                    maybeShowPopup(e);
                }

                private void maybeShowPopup(MouseEvent e)
                {
                    if (e.isPopupTrigger())
                    {
                        showObjectMenuItem.setEnabled(_objects.getSelectedRow() != -1);
                        deleteObjectMenuItem.setEnabled(_objects.getSelectedRow() != -1);
                        objectsPopup.show(_objects, e.getX(), e.getY());
                    }
                }
            });

        Action deleteAdapter = new AbstractAction("Remove selected adapter")
            {
                @Override
                public void actionPerformed(ActionEvent e)
                {
                    if(_target.getCoordinator().connectedToMaster())
                    {
                        int selectedRow = _adapters.getSelectedRow();
                        if(selectedRow != -1)
                        {
                            _target.removeAdapter((String)_adapters.getValueAt(selectedRow, 0));
                        }
                    }
                }
            };
        deleteAdapter.putValue(Action.ACCELERATOR_KEY, KeyStroke.getKeyStroke("DELETE"));

        _adapters.getActionMap().put("delete", deleteAdapter);
        _adapters.getInputMap().put(KeyStroke.getKeyStroke("DELETE"), "delete");
        _adapters.setToolTipText("<html>Object adapters registered at run time.</html>");

        final JPopupMenu adaptersPopup = new JPopupMenu();
        adaptersPopup.add(deleteAdapter);

        _adapters.addMouseListener(new MouseAdapter()
            {
                @Override
                public void mousePressed(MouseEvent e)
                {
                    maybeShowPopup(e);
                }

                @Override
                public void mouseReleased(MouseEvent e)
                {
                    maybeShowPopup(e);
                }

                private void maybeShowPopup(MouseEvent e)
                {
                    if (e.isPopupTrigger() && _adapters.getSelectedRow() != -1)
                    {
                        adaptersPopup.show(_adapters, e.getX(), e.getY());
                    }
                }
            });
    }

    @Override
    protected void appendProperties(DefaultFormBuilder builder)
    {
        CellConstraints cc = new CellConstraints();

        builder.append("Hostname" );
        builder.append(_hostname, 3);
        builder.nextLine();

        builder.appendSeparator("Deployed Applications");
        builder.append("");
        builder.nextLine();
        builder.append("");
        builder.nextLine();
        builder.append("");
        builder.nextLine();
        builder.append("");
        builder.nextLine();
        builder.append("");
        builder.nextLine();
        builder.append("");
        builder.nextLine();
        builder.append("");
        builder.nextLine();
        builder.append("");
        builder.nextRow(-14);
        JScrollPane scrollPane = new JScrollPane(_applications);
        scrollPane.setToolTipText(_applications.getToolTipText());
        builder.add(scrollPane, cc.xywh(builder.getColumn(), builder.getRow(), 3, 14));
        builder.nextRow(14);
        builder.nextLine();

        builder.appendSeparator("Dynamic Well-Known Objects");
        builder.append("");
        builder.nextLine();
        builder.append("");
        builder.nextLine();
        builder.append("");
        builder.nextLine();
        builder.append("");
        builder.nextLine();
        builder.append("");
        builder.nextLine();
        builder.append("");
        builder.nextLine();
        builder.append("");
        builder.nextLine();
        builder.append("");
        builder.nextRow(-14);
        scrollPane = new JScrollPane(_objects);
        scrollPane.setToolTipText(_objects.getToolTipText());
        builder.add(scrollPane, cc.xywh(builder.getColumn(), builder.getRow(), 3, 14));
        builder.nextRow(14);
        builder.nextLine();

        builder.appendSeparator("Dynamic Object Adapters");
        builder.append("");
        builder.nextLine();
        builder.append("");
        builder.nextLine();
        builder.append("");
        builder.nextLine();
        builder.append("");
        builder.nextLine();
        builder.append("");
        builder.nextLine();
        builder.append("");
        builder.nextLine();
        builder.append("");
        builder.nextLine();
        builder.append("");
        builder.nextRow(-14);
        scrollPane = new JScrollPane(_adapters);
        scrollPane.setToolTipText(_adapters.getToolTipText());
        builder.add(scrollPane, cc.xywh(builder.getColumn(), builder.getRow(), 3, 14));
        builder.nextRow(14);
        builder.nextLine();
    }

    @Override
    protected void buildPropertiesPanel()
    {
        super.buildPropertiesPanel();
        _propertiesPanel.setName("Registry Properties");
    }

    void show(Root root)
    {
        _target = root;
        _hostname.setText(root.getRegistryInfo().hostname);
        _applications.setSortedMap(root.getApplicationMap());
        _objects.setObjects(root.getObjects());
        _adapters.setAdapters(root.getAdapters());
    }

    private JTextField _hostname = new JTextField(20);
    private TableField _applications = new TableField("Name", "Last Update");
    private TableField _objects = new TableField("Proxy", "Type");
    private TableField _adapters = new TableField("ID", "Endpoints", "Replica Group");

    private Root _target;
}
