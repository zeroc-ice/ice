// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGridGUI;

import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.Frame;

import java.awt.event.ActionEvent;

import javax.swing.JDialog;
import javax.swing.JEditorPane;
import javax.swing.JPanel;
import javax.swing.JScrollPane;

import javax.swing.text.html.HTMLDocument;
import javax.swing.text.html.HTMLFrameHyperlinkEvent;

import javax.swing.event.HyperlinkEvent;
import javax.swing.event.HyperlinkListener;

import com.jgoodies.forms.factories.Borders;

//
// Shows Ice license and warranty
//
public class LicenseDialog extends JDialog
{
    public LicenseDialog(Frame parentFrame) 
    {
	super(parentFrame, "License - IceGrid Admin", false);
	setDefaultCloseOperation(JDialog.HIDE_ON_CLOSE);
	
	try
	{
	    _pane = new JEditorPane(Utils.class.getResource("/license.html"));
	}
	catch(java.io.IOException e)
	{
	    _pane = new JEditorPane();
	    _pane.setText("Cannot find license.html");
	}
	_pane.setEditable(false);

	_pane.addHyperlinkListener(new HyperlinkListener()
	    {
		public void hyperlinkUpdate(HyperlinkEvent e) 
		{
		    if (e.getEventType() == HyperlinkEvent.EventType.ACTIVATED) 
		    {
			_pane.scrollToReference(e.getURL().getRef());
		    }
		}
	    });
	Dimension prefSize = new Dimension(700, 500);
	_pane.setPreferredSize(prefSize);

	JScrollPane scrollPane = new JScrollPane(_pane);
	scrollPane.setBorder(Borders.DIALOG_BORDER);
	getContentPane().add(scrollPane, BorderLayout.CENTER);
	pack();
    }

    
    public void show(String ref)
    {
	setLocationRelativeTo(null);
	setVisible(true);
	_pane.scrollToReference(ref);
    }

    JEditorPane _pane;
}



