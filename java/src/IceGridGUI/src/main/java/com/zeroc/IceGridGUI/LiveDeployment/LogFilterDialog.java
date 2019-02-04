//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.IceGridGUI.LiveDeployment;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JComponent;
import javax.swing.JDialog;
import javax.swing.JOptionPane;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.WindowConstants;

import com.zeroc.Ice.LogMessageType;

import com.jgoodies.forms.builder.DefaultFormBuilder;
import com.jgoodies.forms.builder.ButtonBarBuilder;
import com.jgoodies.forms.factories.Borders;
import com.jgoodies.forms.layout.CellConstraints;
import com.jgoodies.forms.layout.FormLayout;
import com.jgoodies.forms.util.LayoutStyle;

class LogFilterDialog extends JDialog
{
    LogFilterDialog(final ShowIceLogDialog dialog)
    {
        super(dialog, "Ice log filter - IceGrid GUI", true);
        setDefaultCloseOperation(WindowConstants.DISPOSE_ON_CLOSE);

        java.util.Set<com.zeroc.Ice.LogMessageType> messageTypeFilterSet = null;
        if(dialog.getMessageTypeFilter() != null)
        {
            messageTypeFilterSet = new java.util.HashSet<>(java.util.Arrays.asList(dialog.getMessageTypeFilter()));
        }

        final JCheckBox error = new JCheckBox("Error",
                    messageTypeFilterSet == null || messageTypeFilterSet.contains(LogMessageType.ErrorMessage));
        final JCheckBox warning = new JCheckBox("Warning",
                    messageTypeFilterSet == null || messageTypeFilterSet.contains(LogMessageType.WarningMessage));
        final JCheckBox print = new JCheckBox("Print",
                    messageTypeFilterSet == null || messageTypeFilterSet.contains(LogMessageType.PrintMessage));
        final JCheckBox trace = new JCheckBox("Trace",
                    messageTypeFilterSet == null || messageTypeFilterSet.contains(LogMessageType.TraceMessage));

        final JTextArea traceCategories = new JTextArea(3, 40);
        traceCategories.setLineWrap(true);

        String[] traceCategoryFilter = dialog.getTraceCategoryFilter();
        if(traceCategoryFilter != null)
        {
            // TODO: join with escapes!
            traceCategories.setText(
                com.zeroc.IceUtilInternal.StringUtil.joinString(java.util.Arrays.asList(traceCategoryFilter), ", "));
        }
        else
        {
            traceCategories.setText(null);
        }

        traceCategories.setToolTipText("Trace categories separated by commas; leave blank to get all categories");

        JButton okButton = new JButton("OK");
        ActionListener okListener = new ActionListener()
            {
                @Override
                public void actionPerformed(ActionEvent e)
                {
                    String[] traceCategoryFilter = null;

                    String txt = traceCategories.getText();
                    if(txt != null && !txt.isEmpty())
                    {
                        traceCategoryFilter = com.zeroc.IceUtilInternal.StringUtil.splitString(txt, ", \t\r\n");
                        if(traceCategoryFilter == null)
                        {
                            // unmatched quote
                            JOptionPane.showMessageDialog(LogFilterDialog.this,
                                                          "Unmatched quote in Trace categories field",
                                                          "Invalid entry", JOptionPane.ERROR_MESSAGE);
                            return;
                        }

                        if(traceCategoryFilter.length == 0) // only separators
                        {
                            traceCategoryFilter = null;
                        }
                    }

                    java.util.Set<LogMessageType> messageTypeFilterSet = new java.util.HashSet<>();
                    if(error.isSelected())
                    {
                        messageTypeFilterSet.add(LogMessageType.ErrorMessage);
                    }
                    if(warning.isSelected())
                    {
                        messageTypeFilterSet.add(LogMessageType.WarningMessage);
                    }
                    if(print.isSelected())
                    {
                        messageTypeFilterSet.add(LogMessageType.PrintMessage);
                    }
                    if(trace.isSelected())
                    {
                        messageTypeFilterSet.add(LogMessageType.TraceMessage);
                    }
                    if(messageTypeFilterSet.size() == 0 || messageTypeFilterSet.size() == 4)
                    {
                        // All or nothing checked equivalent of getting everything!
                        messageTypeFilterSet = null;
                    }
                    LogMessageType[] messageTypeFilter = null;
                    if(messageTypeFilterSet != null)
                    {
                        messageTypeFilter = messageTypeFilterSet.toArray(new LogMessageType[0]);
                    }

                    dispose();
                    dialog.setFilters(messageTypeFilter, traceCategoryFilter);
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
                    dispose();
                }
            };
        cancelButton.addActionListener(cancelListener);

        FormLayout layout = new FormLayout("left:pref, 3dlu, fill:pref:grow", "");
        DefaultFormBuilder builder = new DefaultFormBuilder(layout);
        builder.border(Borders.DIALOG);
        builder.rowGroupingEnabled(true);
        builder.lineGapSize(LayoutStyle.getCurrent().getLinePad());

        builder.appendSeparator("Retrieve only the following types of log messages (server-side filtering)");
        builder.nextLine();
        builder.append(error);
        builder.nextLine();
        builder.append(warning);
        builder.nextLine();
        builder.append(print);
        builder.nextLine();
        builder.append(trace);
        builder.nextLine();
        builder.append("Trace categories");
        builder.nextLine();
        builder.append("");
        builder.nextRow(-2);
        CellConstraints cc = new CellConstraints();
        JScrollPane scrollPane = new JScrollPane(traceCategories);
        builder.add(scrollPane, cc.xywh(builder.getColumn(), builder.getRow(), 1, 3));
        builder.nextRow(2);
        builder.nextLine();

        JComponent buttonBar = new ButtonBarBuilder().addGlue().addButton(okButton, cancelButton).build();
        buttonBar.setBorder(Borders.DIALOG);

        java.awt.Container contentPane = getContentPane();
        contentPane.setLayout(new BoxLayout(contentPane, BoxLayout.Y_AXIS));
        contentPane.add(builder.getPanel());
        contentPane.add(buttonBar);

        pack();
        setResizable(false);
        setLocationRelativeTo(dialog);
        setVisible(true);
    }
}
