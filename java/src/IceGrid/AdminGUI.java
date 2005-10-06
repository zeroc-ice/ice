// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid;

import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;

import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.SwingConstants;
import javax.swing.SwingUtilities;
import javax.swing.UIManager;

import javax.swing.border.EmptyBorder;
import java.awt.BorderLayout;

import com.jgoodies.looks.plastic.PlasticLookAndFeel;

import java.util.prefs.Preferences;

public class AdminGUI extends JFrame
{
    class StatusBarI extends JPanel implements StatusBar
    {
	StatusBarI()
	{
	    super(new BorderLayout());
	    setBorder(new EmptyBorder(0, 10, 10, 10));
	    _text = new JLabel("Starting up");
	    _text.setHorizontalAlignment(SwingConstants.LEFT);
	    add(_text, BorderLayout.LINE_START);
	    
	    JLabel copyright = new JLabel("Copyright \u00A9 2005 ZeroC, Inc.");
	    copyright.setHorizontalAlignment(SwingConstants.RIGHT);
	    add(copyright, BorderLayout.LINE_END);
	}

	public void setText(String text)
	{
	    _text.setText(text);
	}

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
	setIconImage(Utils.getIcon("/icons/grid.png").getImage());

	_model = new Model(this, args, Preferences.userNodeForPackage(getClass()),
			   new StatusBarI());

	setDefaultCloseOperation(JFrame.DO_NOTHING_ON_CLOSE);
	addWindowListener(new WindowAdapter() 
	    { 
		public void windowClosing(WindowEvent e) 
		{
		    _model.exit(0);
		}
	    });
	
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
	_model.setActions(_model.getDefaultActions());

	//
	// Status bar
	//
	getContentPane().add((StatusBarI)_model.getStatusBar(), BorderLayout.PAGE_END);
    }

    private static void createAndShowGUI(String[] args) {
     
	// JGoodies L&F
	try 
	{
	    UIManager.setLookAndFeel(new com.jgoodies.looks.plastic.PlasticXPLookAndFeel());
	} 
	catch(Exception e) 
	{}

	//
        // Create and set up the window.
	//
        new AdminGUI(args);
     
    }

    private Model _model;
}


