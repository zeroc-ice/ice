// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGridGUI.LiveDeployment;

import java.awt.Cursor;
import java.awt.BorderLayout;
import java.awt.Container;
import java.awt.Dimension;
import java.awt.Frame;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.ButtonGroup;
import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JComboBox;
import javax.swing.JComponent;
import javax.swing.JDialog;
import javax.swing.JFrame;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JRadioButton;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.SwingUtilities;

import com.jgoodies.forms.builder.DefaultFormBuilder;
import com.jgoodies.forms.factories.Borders;
import com.jgoodies.forms.factories.ButtonBarFactory;
import com.jgoodies.forms.layout.CellConstraints;
import com.jgoodies.forms.layout.FormLayout;
import com.jgoodies.forms.util.LayoutStyle;

import IceGrid.*;
import IceGridGUI.*;

class WriteMessageDialog extends JDialog
{
    WriteMessageDialog(final Root root)
    {
        super(root.getCoordinator().getMainFrame(), 
              "Write Message - IceGrid Admin", true);
        setDefaultCloseOperation(JDialog.HIDE_ON_CLOSE);

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
                public void actionPerformed(ActionEvent e)
                {
                    final Coordinator c = root.getCoordinator();
                    final String target = _target;
                    final String prefix = "Writing message to server '" + target + "'...";


                    AMI_Admin_writeMessage cb = new AMI_Admin_writeMessage()
                        {
                            public void ice_response()
                            {
                                SwingUtilities.invokeLater(new Runnable() 
                                    {   
                                        public void run() 
                                        {
                                            c.getStatusBar().setText(prefix + "done.");
                                        }
                                    });
                            }
                            
                            public void ice_exception(final Ice.UserException e)
                            {
                                SwingUtilities.invokeLater(new Runnable() 
                                    {
                                        public void run() 
                                        {
                                            handleFailure("IceGrid exception: " + e.toString());
                                        }
                                        
                                    });
                            }
                                            
                            public void ice_exception(final Ice.LocalException e)
                            {
                                SwingUtilities.invokeLater(new Runnable() 
                                    {
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
                                    "Writing message to server '" + target + "' failed",
                                    JOptionPane.ERROR_MESSAGE);
                            }
                        };
                  
                    try
                    {
                        c.getStatusBar().setText(prefix);
                        _mainFrame.setCursor(Cursor.getPredefinedCursor(Cursor.WAIT_CURSOR));

                        c.getAdmin().writeMessage_async(
                            cb, _target, _message.getText(), _stdOut.isSelected() ? 1 : 2);
                    }
                    catch(Ice.LocalException ex)
                    {
                        c.getStatusBar().setText(prefix + "failed.");
                        JOptionPane.showMessageDialog(
                            _mainFrame,
                            "Communication exception: " + ex.toString(),
                            "Writing message to server '" + target + "' failed",
                            JOptionPane.ERROR_MESSAGE);
                
                        return;
                    }
                    finally
                    {
                        _mainFrame.setCursor(Cursor.getPredefinedCursor(Cursor.DEFAULT_CURSOR));
                    }

                    setVisible(false);
                }
            };
            okButton.addActionListener(okListener);
            getRootPane().setDefaultButton(okButton);
        
            JButton cancelButton = new JButton("Cancel");
            ActionListener cancelListener = new ActionListener()
                {
                    public void actionPerformed(ActionEvent e)
                    {
                        setVisible(false);
                    }
                };
            cancelButton.addActionListener(cancelListener);

            FormLayout layout = new FormLayout("left:pref, 3dlu, fill:pref:grow", "");
            DefaultFormBuilder builder = new DefaultFormBuilder(layout);
            builder.setDefaultDialogBorder();
            builder.setRowGroupingEnabled(true);
            builder.setLineGapSize(LayoutStyle.getCurrent().getLinePad());

            _message.setLineWrap(true);
            JScrollPane scrollPane = new JScrollPane(_message,
                                                     JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED, 
                                                     JScrollPane.HORIZONTAL_SCROLLBAR_NEVER);
            builder.append(scrollPane, 3);
            builder.nextLine();
            builder.append(_stdOut);
            builder.append(stdErr);
            builder.nextLine();
            JComponent buttonBar = 
                ButtonBarFactory.buildOKCancelBar(okButton, cancelButton);
            buttonBar.setBorder(Borders.DIALOG_BORDER);

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
