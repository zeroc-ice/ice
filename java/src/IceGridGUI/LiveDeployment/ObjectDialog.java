// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGridGUI.LiveDeployment;

import java.awt.BorderLayout;
import java.awt.Container;
import java.awt.Dimension;
import java.awt.Frame;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JComboBox;
import javax.swing.JComponent;
import javax.swing.JDialog;
import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextField;

import com.jgoodies.forms.builder.DefaultFormBuilder;
import com.jgoodies.forms.factories.Borders;
import com.jgoodies.forms.factories.ButtonBarFactory;
import com.jgoodies.forms.layout.FormLayout;
import com.jgoodies.forms.util.LayoutStyle;

class ObjectDialog extends JDialog
{
    ObjectDialog(final Root root)
    {
	super(root.getCoordinator().getMainFrame(), 
	      "New Well-Known Object - IceGrid Admin", true);
	setDefaultCloseOperation(JDialog.HIDE_ON_CLOSE);
	_mainFrame = root.getCoordinator().getMainFrame();
	
	_type.setEditable(true);

	JButton okButton = new JButton("OK");
	ActionListener okListener = new ActionListener()
	    {
		public void actionPerformed(ActionEvent e)
		{
		    String type = null;
		    if(_type.getSelectedItem() != QUERY_OBJECT)
		    {
			type = _type.getSelectedItem().toString();
		    }

		    if(root.addObject(_proxy.getText(),type))
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
		    public void actionPerformed(ActionEvent e)
		    {
			setVisible(false);
		    }
		};
	    cancelButton.addActionListener(cancelListener);

	    FormLayout layout = new FormLayout("right:pref, 3dlu, pref", "");
	    DefaultFormBuilder builder = new DefaultFormBuilder(layout);
	    builder.setDefaultDialogBorder();
	    builder.setRowGroupingEnabled(true);
	    builder.setLineGapSize(LayoutStyle.getCurrent().getLinePad());
	    
	    builder.append("Proxy", _proxy);
	    builder.nextLine();
	    builder.append("Type", _type);
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

    void showDialog()
    {
	_proxy.setText("");
	_type.setSelectedItem(QUERY_OBJECT);
	setLocationRelativeTo(_mainFrame);
	setVisible(true);
    }

    private JTextField _proxy = new JTextField(60);
    private JComboBox _type = new JComboBox(new Object[]{QUERY_OBJECT});
    private JFrame _mainFrame;

    static private final Object QUERY_OBJECT = new Object()
	{
	    public String toString()
	    {
		return "Query object to retrieve type";
	    }
	};
    
}
