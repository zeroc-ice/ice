// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceGridGUI.LiveDeployment;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JComponent;
import javax.swing.JDialog;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JTextField;

import com.jgoodies.forms.builder.DefaultFormBuilder;
import com.jgoodies.forms.builder.ButtonBarBuilder;
import com.jgoodies.forms.factories.Borders;
import com.jgoodies.forms.layout.FormLayout;
import com.jgoodies.forms.util.LayoutStyle;

import IceGrid.*;
import IceGridGUI.*;

class ShowLogPrefsDialog extends JDialog
{
    ShowLogPrefsDialog(final ShowLogDialog sld)
    {
        super(sld, "Preferences - IceGrid Admin", true);
        setDefaultCloseOperation(JDialog.DISPOSE_ON_CLOSE);

        final JTextField maxLinesField = new JTextField(10);
        maxLinesField.setText(Integer.toString(sld.getMaxLines()));
        maxLinesField.setToolTipText("Maximum number of lines in this dialog's buffer");

        final JTextField maxSizeField = new JTextField(10);
        maxSizeField.setText(Integer.toString(sld.getMaxSize()));
        maxSizeField.setToolTipText("Maximum number of characters in this dialog's buffer");

        final JTextField initialLinesField = new JTextField(10);
        initialLinesField.setText(Integer.toString(sld.getInitialLines()));
        initialLinesField.setToolTipText("Start by retrieving <num> lines from the server; -1 means retrieve all");

        final JTextField maxReadSizeField = new JTextField(10);
        maxReadSizeField.setText(Integer.toString(sld.getMaxReadSize()));
        maxReadSizeField.setToolTipText("Maximum number of bytes read by each request");

        final JTextField periodField = new JTextField(10);
        periodField.setText(Float.toString((float)sld.getPeriod() / 1000));
        periodField.setToolTipText("After reaching EOF, check every <num> seconds for new output");

        JButton okButton = new JButton("OK");
        ActionListener okListener = new ActionListener()
            {
                public void actionPerformed(ActionEvent e)
                {
                    try
                    {
                        int maxLines = parseInt(maxLinesField, "Max lines in buffer");
                        int maxSize = parseInt(maxSizeField, "Max characters in buffer");
                        int initialLines = parseInt(initialLinesField, "Initial tail (lines)");
                        int maxReadSize = parseInt(maxReadSizeField, "Max bytes read per request");
                        int period = (int)(parseFloat(periodField, "Poll period (seconds)") * 1000);

                        sld.setPrefs(maxLines, maxSize, initialLines, maxReadSize, period);
                        dispose();
                    }
                    catch(NumberFormatException ex)
                    {
                        return;
                    }
                }
            };
        okButton.addActionListener(okListener);
        getRootPane().setDefaultButton(okButton);

        JButton cancelButton = new JButton("Cancel");
        ActionListener cancelListener = new ActionListener()
            {
                public void actionPerformed(ActionEvent e)
                {
                    dispose();
                }
            };
        cancelButton.addActionListener(cancelListener);

        FormLayout layout = new FormLayout("left:pref, 3dlu, fill:pref:grow", "");
        DefaultFormBuilder builder = new DefaultFormBuilder(layout);
        builder.border(Borders.DIALOG);
        builder.rowGroupingEnabled(true);
        builder.lineGapSize(LayoutStyle.getCurrent().getLinePad());

        builder.append("Max lines in buffer", maxLinesField);
        builder.nextLine();
        builder.append("Max characters in buffer", maxSizeField);
        builder.nextLine();
        builder.append("Initial tail (lines)", initialLinesField);
        builder.nextLine();
        builder.append("Max bytes read per request", maxReadSizeField);
        builder.nextLine();
        builder.append("Poll period (seconds)", periodField);
        builder.nextLine();

        JComponent buttonBar = new ButtonBarBuilder().addGlue().addButton(okButton, cancelButton).build();
        buttonBar.setBorder(Borders.DIALOG);

        java.awt.Container contentPane = getContentPane();
        contentPane.setLayout(new BoxLayout(contentPane, BoxLayout.Y_AXIS));
        contentPane.add(builder.getPanel());
        contentPane.add(buttonBar);

        pack();
        setResizable(false);
        setLocationRelativeTo(sld);
        setVisible(true);
    }

    private int parseInt(JTextField field, String label) throws NumberFormatException
    {
        try
        {
            return Integer.parseInt(field.getText());
        }
        catch(NumberFormatException e)
        {
            JOptionPane.showMessageDialog(
                this,
                label + " must be an integer",
                "Invalid entry",
                JOptionPane.ERROR_MESSAGE);

            throw e;
        }
    }

    private float parseFloat(JTextField field, String label) throws NumberFormatException
    {
        try
        {
            return Float.parseFloat(field.getText());
        }
        catch(NumberFormatException e)
        {
            JOptionPane.showMessageDialog(
                this,
                label + " must be a decimal number",
                "Invalid entry",
                JOptionPane.ERROR_MESSAGE);

            throw e;
        }
    }
}
