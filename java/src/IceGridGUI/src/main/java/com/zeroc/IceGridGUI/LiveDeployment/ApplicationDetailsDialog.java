// Copyright (c) ZeroC, Inc.

package com.zeroc.IceGridGUI.LiveDeployment;

import com.jgoodies.forms.builder.DefaultFormBuilder;
import com.jgoodies.forms.factories.Borders;
import com.jgoodies.forms.layout.FormLayout;
import com.jgoodies.forms.util.LayoutStyle;

import com.zeroc.IceGrid.ApplicationDescriptor;
import com.zeroc.IceGrid.ApplicationInfo;

import java.awt.Container;
import java.text.DateFormat;
import java.util.Date;

import javax.swing.JDialog;
import javax.swing.JFrame;
import javax.swing.JTextField;
import javax.swing.WindowConstants;

class ApplicationDetailsDialog extends JDialog {
    ApplicationDetailsDialog(final Root root) {
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

    void showDialog(ApplicationInfo info) {
        _name.setText(info.descriptor.name);
        _uuid.setText(info.uuid);
        _createTime.setText(
            DateFormat.getDateTimeInstance()
                .format(new Date(info.createTime)));
        _createUser.setText(info.createUser);
        _updateTime.setText(
            DateFormat.getDateTimeInstance()
                .format(new Date(info.updateTime)));
        _updateUser.setText(info.updateUser);
        _revision.setText(Integer.toString(info.revision));

        setLocationRelativeTo(_mainFrame);
        setVisible(true);
    }

    private final JTextField _name = new JTextField(30);
    private final JTextField _uuid = new JTextField(30);
    private final JTextField _createTime = new JTextField(30);
    private final JTextField _createUser = new JTextField(30);
    private final JTextField _updateTime = new JTextField(30);
    private final JTextField _updateUser = new JTextField(30);
    private final JTextField _revision = new JTextField(30);
    private final JFrame _mainFrame;
}
