// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceGridGUI.LiveDeployment;

import java.awt.Container;
import javax.swing.JDialog;
import javax.swing.JFrame;
import javax.swing.JTextField;
import javax.swing.WindowConstants;

import com.jgoodies.forms.builder.DefaultFormBuilder;
import com.jgoodies.forms.factories.Borders;
import com.jgoodies.forms.layout.FormLayout;
import com.jgoodies.forms.util.LayoutStyle;

import com.zeroc.IceGrid.*;

class ApplicationDetailsDialog extends JDialog
{
    ApplicationDetailsDialog(final Root root)
    {
        super(root.getCoordinator().getMainFrame(), "Application Details - IceGrid GUI", true);
        setDefaultCloseOperation(WindowConstants.HIDE_ON_CLOSE);
        _mainFrame = root.getCoordinator().getMainFrame();

        _name.setEditable(false);
        _uuid.setEditable(false);
        _createTime.setEditable(false);
        _createUser.setEditable(false);
        _updateTime.setEditable(false);
        _updateUser.setEditable(false);
        _revision.setEditable(false);

        FormLayout layout = new FormLayout("right:pref, 3dlu, pref", "");
        DefaultFormBuilder builder = new DefaultFormBuilder(layout);
        builder.border(Borders.DIALOG);
        builder.rowGroupingEnabled(true);
        builder.lineGapSize(LayoutStyle.getCurrent().getLinePad());

        builder.append("Name", _name);
        builder.nextLine();
        builder.append("UUID", _uuid);
        builder.nextLine();
        builder.append("Created", _createTime);
        builder.nextLine();
        builder.append("Created by", _createUser);
        builder.nextLine();
        builder.append("Last Update", _updateTime);
        builder.nextLine();
        builder.append("Updated by", _updateUser);
        builder.nextLine();
        builder.append("Revision", _revision);
        builder.nextLine();

        Container contentPane = getContentPane();
        contentPane.add(builder.getPanel());

        pack();
        setResizable(false);
    }

    void showDialog(ApplicationInfo info)
    {
        _name.setText(info.descriptor.name);
        _uuid.setText(info.uuid);
        _createTime.setText(java.text.DateFormat.getDateTimeInstance().format(new java.util.Date(info.createTime)));
        _createUser.setText(info.createUser);
        _updateTime.setText(java.text.DateFormat.getDateTimeInstance().format(new java.util.Date(info.updateTime)));
        _updateUser.setText(info.updateUser);
        _revision.setText(Integer.toString(info.revision));

        setLocationRelativeTo(_mainFrame);
        setVisible(true);
    }

    private JTextField _name = new JTextField(30);
    private JTextField _uuid = new JTextField(30);
    private JTextField _createTime = new JTextField(30);
    private JTextField _createUser = new JTextField(30);
    private JTextField _updateTime = new JTextField(30);
    private JTextField _updateUser = new JTextField(30);
    private JTextField _revision = new JTextField(30);
    private JFrame _mainFrame;
}
