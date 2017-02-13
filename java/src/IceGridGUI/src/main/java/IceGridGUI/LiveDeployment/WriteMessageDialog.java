// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceGridGUI.LiveDeployment;

import java.awt.Container;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.ButtonGroup;
import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JComponent;
import javax.swing.JDialog;
import javax.swing.JFrame;
import javax.swing.JOptionPane;
import javax.swing.JRadioButton;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.ScrollPaneConstants;
import javax.swing.SwingUtilities;
import javax.swing.WindowConstants;

import com.jgoodies.forms.builder.DefaultFormBuilder;
import com.jgoodies.forms.builder.ButtonBarBuilder;
import com.jgoodies.forms.factories.Borders;
import com.jgoodies.forms.layout.FormLayout;
import com.jgoodies.forms.util.LayoutStyle;

import IceGrid.*;
import IceGridGUI.*;

class WriteMessageDialog extends JDialog
{
    WriteMessageDialog(final Root root)
    {
        super(root.getCoordinator().getMainFrame(), "Write Message - IceGrid Admin", true);
        setDefaultCloseOperation(WindowConstants.HIDE_ON_CLOSE);

        _mainFrame = root.getCoordinator().getMainFrame();

        _stdOut = new JRadioButton("Write to stdout");
        _stdOut.setSelected(true);
        JRadioButton stdErr = new JRadioButton("Write to stderr");
        ButtonGroup bg = new ButtonGroup();
        bg.add(_stdOut);
        bg.add(stdErr);

        JButton okButton = new JButton("OK");
        ActionListener okListener = new ActionListener()
            {
                @Override
                public void actionPerformed(ActionEvent e)
                {
                    final Coordinator c = root.getCoordinator();

                    AdminPrx admin = c.getAdmin();
                    if(admin == null)
                    {
                        JOptionPane.showMessageDialog(
                            _mainFrame,
                            "No longer connected to IceGrid Registry",
                            "Writing message to server '" + _target + "' failed",
                            JOptionPane.ERROR_MESSAGE);
                    }
                    else
                    {

                        Ice.Identity adminId = new Ice.Identity(_target, c.getServerAdminCategory());

                        final Ice.ProcessPrx process = Ice.ProcessPrxHelper.uncheckedCast(
                            admin.ice_identity(adminId).ice_facet("Process"));

                        final String prefix = "Writing message to server '" + _target + "'...";
                        c.getStatusBar().setText(prefix);

                        Ice.Callback_Process_writeMessage cb = new Ice.Callback_Process_writeMessage()
                            {
                                @Override
                                public void response()
                                {
                                    SwingUtilities.invokeLater(new Runnable()
                                        {
                                            @Override
                                            public void run()
                                            {
                                                c.getStatusBar().setText(prefix + "done.");
                                            }
                                        });
                                }

                                @Override
                                public void exception(final Ice.LocalException e)
                                {
                                    SwingUtilities.invokeLater(new Runnable()
                                        {
                                            @Override
                                            public void run()
                                            {
                                                handleFailure("Communication exception: " + e.toString());
                                            }
                                        });
                                }

                                private void handleFailure(String message)
                                {
                                    c.getStatusBar().setText(prefix + "failed!");

                                    JOptionPane.showMessageDialog(
                                        _mainFrame,
                                        message,
                                        "Writing message to server '" + process.ice_getIdentity().name + "' failed",
                                        JOptionPane.ERROR_MESSAGE);
                                }
                            };

                        try
                        {
                            process.begin_writeMessage(_message.getText(), _stdOut.isSelected() ? 1 : 2, cb);
                        }
                        catch(Ice.LocalException ex)
                        {
                            c.getStatusBar().setText(prefix + "failed.");
                            JOptionPane.showMessageDialog(
                                _mainFrame,
                                "Communication exception: " + ex.toString(),
                                "Writing message to server '" + _target + "' failed",
                                JOptionPane.ERROR_MESSAGE);

                            return;
                        }
                    }

                    setVisible(false);
                }
            };
        okButton.addActionListener(okListener);
        getRootPane().setDefaultButton(okButton);

        JButton cancelButton = new JButton("Cancel");
        ActionListener cancelListener = new ActionListener()
            {
                @Override
                public void actionPerformed(ActionEvent e)
                {
                    setVisible(false);
                }
            };
        cancelButton.addActionListener(cancelListener);

        FormLayout layout = new FormLayout("left:pref, 3dlu, fill:pref:grow", "");
        DefaultFormBuilder builder = new DefaultFormBuilder(layout);
        builder.border(Borders.DIALOG);
        builder.rowGroupingEnabled(true);
        builder.lineGapSize(LayoutStyle.getCurrent().getLinePad());

        _message.setLineWrap(true);
        JScrollPane scrollPane = new JScrollPane(_message,
                                                 ScrollPaneConstants.VERTICAL_SCROLLBAR_AS_NEEDED,
                                                 ScrollPaneConstants.HORIZONTAL_SCROLLBAR_NEVER);
        builder.append(scrollPane, 3);
        builder.nextLine();
        builder.append(_stdOut);
        builder.append(stdErr);
        builder.nextLine();

        JComponent buttonBar = new ButtonBarBuilder().addGlue().addButton(okButton, cancelButton).build();
        buttonBar.setBorder(Borders.DIALOG);

        Container contentPane = getContentPane();
        contentPane.setLayout(new BoxLayout(contentPane, BoxLayout.Y_AXIS));
        contentPane.add(builder.getPanel());
        contentPane.add(buttonBar);

        pack();
        setResizable(false);
    }

    void showDialog(String serverId)
    {
        _target = serverId;
        _message.setText("");
        setLocationRelativeTo(_mainFrame);
        setVisible(true);
    }

    private JRadioButton _stdOut;
    private JTextArea _message = new JTextArea(3, 40);
    private String _target;
    private JFrame _mainFrame;
}
