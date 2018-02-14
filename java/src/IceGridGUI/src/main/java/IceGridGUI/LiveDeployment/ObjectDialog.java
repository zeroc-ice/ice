// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceGridGUI.LiveDeployment;

import java.awt.Container;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JComboBox;
import javax.swing.JComponent;
import javax.swing.JDialog;
import javax.swing.JFrame;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JTextField;
import javax.swing.WindowConstants;

import com.jgoodies.forms.builder.DefaultFormBuilder;
import com.jgoodies.forms.builder.ButtonBarBuilder;
import com.jgoodies.forms.factories.Borders;
import com.jgoodies.forms.layout.CellConstraints;
import com.jgoodies.forms.layout.FormLayout;
import com.jgoodies.forms.util.LayoutStyle;

@SuppressWarnings("unchecked")
class ObjectDialog extends JDialog
{
    ObjectDialog(final Root root, boolean readOnly)
    {
        super(root.getCoordinator().getMainFrame(),
              (readOnly ? "" : "New ") + "Dynamic Well-Known Object - IceGrid Admin", true);

        setDefaultCloseOperation(WindowConstants.HIDE_ON_CLOSE);
        _mainFrame = root.getCoordinator().getMainFrame();

        _proxy.setLineWrap(true);

        if(readOnly)
        {
            _proxy.setEditable(false);
            _proxy.setOpaque(false);
            _type.setEditable(false);
        }
        else
        {
            _typeCombo.setEditable(true);
        }

        FormLayout layout = new FormLayout("right:pref, 3dlu, pref", "");
        DefaultFormBuilder builder = new DefaultFormBuilder(layout);
        builder.border(Borders.DIALOG);
        builder.rowGroupingEnabled(true);
        builder.lineGapSize(LayoutStyle.getCurrent().getLinePad());

        builder.append("Proxy");
        builder.nextLine();
        builder.append("");
        builder.nextRow(-2);
        CellConstraints cc = new CellConstraints();
        JScrollPane scrollPane = new JScrollPane(_proxy);
        builder.add(scrollPane, cc.xywh(builder.getColumn(), builder.getRow(), 1, 3));
        builder.nextRow(2);
        builder.nextLine();

        if(readOnly)
        {
            builder.append("Type", _type);
        }
        else
        {
            builder.append("Type", _typeCombo);
        }
        builder.nextLine();

        Container contentPane = getContentPane();
        if(readOnly)
        {
            contentPane.add(builder.getPanel());
        }
        else
        {
            JButton okButton = new JButton("OK");
            ActionListener okListener = new ActionListener()
            {
                @Override
                public void actionPerformed(ActionEvent e)
                {
                    if(_proxy.isEditable())
                    {
                        String type = null;
                        if(_typeCombo.getSelectedItem() != QUERY_OBJECT)
                        {
                            type = _typeCombo.getSelectedItem().toString();
                        }

                        root.addObject(_proxy.getText(), type, ObjectDialog.this);
                    }
                    else
                    {
                        setVisible(false);
                    }
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
            
            JComponent buttonBar = new ButtonBarBuilder().addGlue().addButton(okButton, cancelButton).build();
            buttonBar.setBorder(Borders.DIALOG);
            
            contentPane.setLayout(new BoxLayout(contentPane, BoxLayout.Y_AXIS));
            contentPane.add(builder.getPanel());
            contentPane.add(buttonBar);
        }

        pack();
        setResizable(false);
    }

    void showDialog()
    {
        assert _proxy.isEditable();

        _proxy.setText("");
        _typeCombo.setSelectedItem(QUERY_OBJECT);
        setLocationRelativeTo(_mainFrame);
        setVisible(true);
    }

    void showDialog(String proxy, String type)
    {
        assert !_proxy.isEditable();

        _proxy.setText(proxy);
        _type.setText(type);

        setLocationRelativeTo(_mainFrame);
        setVisible(true);
    }

    private JTextArea _proxy = new JTextArea(3, 40);
    private JTextField _type = new JTextField(40);
    private JComboBox _typeCombo = new JComboBox(new Object[]{QUERY_OBJECT});
    private JFrame _mainFrame;

    static private final Object QUERY_OBJECT = new Object()
        {
            @Override
            public String toString()
            {
                return "Query object to retrieve type";
            }
        };
}
