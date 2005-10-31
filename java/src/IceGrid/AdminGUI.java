// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid;

import java.awt.Dimension;
import java.awt.FlowLayout;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;

import javax.swing.Icon;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.SwingConstants;
import javax.swing.SwingUtilities;
import javax.swing.UIManager;

import javax.swing.border.EmptyBorder;
import java.awt.BorderLayout;

import java.util.prefs.Preferences;

public class AdminGUI extends JFrame
{
    class StatusBarI extends JPanel implements StatusBar
    {
	StatusBarI()
	{
	    super(new BorderLayout());
	    setBorder(new EmptyBorder(0, 13, 10, 13));

	    _text = new JLabel();
	    _text.setHorizontalAlignment(SwingConstants.LEFT);
	    add(_text, BorderLayout.LINE_START);

	    _connectedLabel = new JLabel("Working Offline");
	    _connectedLabel.setHorizontalAlignment(SwingConstants.RIGHT);
	    add(_connectedLabel, BorderLayout.LINE_END);
	}

	public void setText(String text)
	{
	    _text.setText(text);
	}

	public void setConnected(boolean connected)
	{
	    if(connected)
	    {
		_connectedLabel.setText("Working Online");
	    }
	    else
	    {
		_connectedLabel.setText("Working Offline");
	    }
	}

	JLabel _connectedLabel;
	JLabel _text;
    }

    public static void main(final String[] args) 
    {
        SwingUtilities.invokeLater(new Runnable() 
	    {
		public void run() 
		{
		    createAndShowGUI(args);
		}
	    });
    }

    AdminGUI(String[] args)
    {
	super("IceGrid Admin");	
	setIconImage(Utils.getIcon("/icons/24x24/grid.png").getImage());

	setDefaultCloseOperation(JFrame.DO_NOTHING_ON_CLOSE);
	addWindowListener(new WindowAdapter() 
	    { 
		public void windowClosing(WindowEvent e) 
		{
		    if(_model != null)
		    {
			_model.exit(0);
		    }
		}
	    });

	_model = new Model(this, args, Preferences.userNodeForPackage(getClass()),
			   new StatusBarI());
	
	initComponents();
	_model.showMainFrame();

	_model.getSessionKeeper().createSession(true);
    }
   
    private void initComponents()
    {
	//
	// MainPane
	//
	getContentPane().add(new MainPane(_model), BorderLayout.CENTER);
	
	//
	// Menu and tool bars
	//
	_model.createMenuBar();
	_model.createToolBar();
	_model.showActions(null);

	//
	// Status bar
	//
	getContentPane().add((StatusBarI)_model.getStatusBar(), BorderLayout.PAGE_END);
    }

    private static void createAndShowGUI(String[] args) 
    {
	try 
	{
	    if(UIManager.getSystemLookAndFeelClassName().equals("apple.laf.AquaLookAndFeel"))
	    {
		System.setProperty("apple.laf.useScreenMenuBar", "true");
		UIManager.setLookAndFeel("apple.laf.AquaLookAndFeel");
	    }
	    else  // JGoodies L&F
	    {
		UIManager.setLookAndFeel("com.jgoodies.looks.plastic.PlasticXPLookAndFeel");
	    }
	} 
	catch(Exception e) 
	{
	    System.err.println(e.toString());
	}
	
	//
        // Create and set up the window.
	//
        new AdminGUI(args);
     
    }

    private Model _model;
}


