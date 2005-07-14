// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid;

import javax.swing.*;
import javax.swing.border.EmptyBorder;

import java.awt.BorderLayout;
import java.awt.Frame;
import java.awt.Rectangle;
import java.awt.Container;
import java.awt.Component;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.awt.event.ActionEvent;
import java.awt.event.ItemEvent;
import java.awt.event.ActionListener;
import java.awt.event.ItemListener;
import com.jgoodies.looks.Options;
import com.jgoodies.looks.HeaderStyle;
import com.jgoodies.looks.BorderStyle;
import com.jgoodies.looks.plastic.PlasticLookAndFeel;

import java.util.prefs.Preferences;
import java.util.prefs.BackingStoreException;

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
	setDefaultCloseOperation(JFrame.DO_NOTHING_ON_CLOSE);
	setIconImage(Utils.getIcon("/icons/grid.png").getImage());

	addWindowListener(new WindowAdapter() 
	    { 
		public void windowClosing(WindowEvent e) 
		{
		    exit(0);
		}
	    });
	
	_communicator = Ice.Util.initialize(args);
	_model = new Model(_communicator, new StatusBarI());

	initComponents();
	
	// Sizing / placement
	//
	if(!loadWindowPrefs())
	{
	    setLocation(100, 100);
	    pack();
	}

	//
        // Display the window.
	//
	setVisible(true);	
	_sessionKeeper = new SessionKeeper(this, _model, _prefs);
	_sessionKeeper.createSession(true);
	
    }

    private void exit(int status)
    {
	storeWindowPrefs();
	if(_communicator != null)
	{
	    try
	    {
		_communicator.destroy();
	    }
	    catch(Ice.LocalException e)
	    {
		//
		// TODO: log exception
		//
	    }
	}

	dispose();
	Runtime.getRuntime().exit(status);
    }

    private void storeWindowPrefs()
    {
	Preferences windowPrefs = _prefs.node("Window");
	Rectangle rect = getBounds();
	windowPrefs.putInt("x", rect.x);
	windowPrefs.putInt("y", rect.y);
	windowPrefs.putInt("width", rect.width);
	windowPrefs.putInt("height", rect.height);
	windowPrefs.putBoolean("maximized", getExtendedState() == Frame.MAXIMIZED_BOTH);
    }

    private boolean loadWindowPrefs()
    {
	try
	{
	    if(!_prefs.nodeExists("Window"))
	    {
		return false;
	    }
	}
	catch(BackingStoreException e)
	{
	    return false;
	}

	Preferences windowPrefs = _prefs.node("Window");
	int x = windowPrefs.getInt("x", 0);
	int y = windowPrefs.getInt("y", 0);
	int width = windowPrefs.getInt("width", 0);
	int height = windowPrefs.getInt("height", 0);
	setBounds(new Rectangle(x, y, width, height));
	if(windowPrefs.getBoolean("maximized", false))
	{
	    setExtendedState(Frame.MAXIMIZED_BOTH);
	}
	return true;
    }
   
    private void initComponents()
    {
	//
	// MainPane
	//
	getContentPane().add(new MainPane(_model), BorderLayout.CENTER);

	//
	// Menu bar
	//	
	JMenuBar menuBar = new JMenuBar();
	menuBar.putClientProperty(Options.HEADER_STYLE_KEY, HeaderStyle.BOTH);
	menuBar.putClientProperty(PlasticLookAndFeel.BORDER_STYLE_KEY, BorderStyle.SEPARATOR);

	JMenu fileMenu = new JMenu("File");
	fileMenu.setMnemonic(java.awt.event.KeyEvent.VK_F);
	menuBar.add(fileMenu);

	JMenu editMenu = new JMenu("Edit");
	editMenu.setMnemonic(java.awt.event.KeyEvent.VK_E);
	menuBar.add(editMenu);

	JMenu helpMenu = new JMenu("Help");
	helpMenu.setMnemonic(java.awt.event.KeyEvent.VK_H);
	menuBar.add(helpMenu);

	setJMenuBar(menuBar);

	//
	// Tool bar
	//
	JToolBar toolBar = new JToolBar();
	toolBar.putClientProperty(Options.HEADER_STYLE_KEY, HeaderStyle.BOTH);
	toolBar.putClientProperty(PlasticLookAndFeel.BORDER_STYLE_KEY, BorderStyle.SEPARATOR);

	toolBar.setFloatable(false);
        toolBar.putClientProperty("JToolBar.isRollover", Boolean.TRUE);
	getContentPane().add(toolBar, BorderLayout.PAGE_START);

	//
	// Status bar
	//
	getContentPane().add((StatusBarI)_model.getStatusBar(), BorderLayout.PAGE_END);

	//
	// Actions
	// actions that can be enabled/disabled are fields
	//

	Action connectAction = new AbstractAction("Connect...", Utils.getIcon("/icons/connect.gif"))
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    _sessionKeeper.createSession(false);
		}
	    };
	connectAction.putValue(Action.ACCELERATOR_KEY, KeyStroke.getKeyStroke("ctrl N"));    
	fileMenu.add(connectAction);
	fileMenu.addSeparator();
	toolBar.add(connectAction);
	toolBar.addSeparator();

	_saveAction = new AbstractAction("Save", Utils.getIcon("/icons/save_edit.gif"))
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    // TODO: implement
		}
	    };
	_saveAction.putValue(Action.ACCELERATOR_KEY, KeyStroke.getKeyStroke("ctrl S"));
	_saveAction.setEnabled(false);
	fileMenu.add(_saveAction);
	toolBar.add(_saveAction);
		
	_discardAction = new AbstractAction("Discard all updates...", 
					    Utils.getIcon("/icons/undo_edit.gif"))
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    // TODO: implement
		}
	    };
	_discardAction.setEnabled(false);
	fileMenu.add(_discardAction);
	fileMenu.addSeparator();
	toolBar.add(_discardAction);
	toolBar.addSeparator();

	final Action exitAction = new AbstractAction("Exit")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    exit(0);
		}
	    };
	exitAction.putValue(Action.ACCELERATOR_KEY, KeyStroke.getKeyStroke("alt F4"));
	fileMenu.add(exitAction);

	Action cutAction = new AbstractAction("Cut", Utils.getIcon("/icons/cut_edit.gif"))
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    // TODO: implement
		}
	    };
	cutAction.putValue(Action.ACCELERATOR_KEY, KeyStroke.getKeyStroke("ctrl X"));
	editMenu.add(cutAction);
	toolBar.add(cutAction);

	Action copyAction = new AbstractAction("Copy", Utils.getIcon("/icons/copy_edit.gif"))
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    // TODO: implement
		}
	    };
	copyAction.putValue(Action.ACCELERATOR_KEY, KeyStroke.getKeyStroke("ctrl C"));
	editMenu.add(copyAction);
	toolBar.add(copyAction);
	
	_pasteAction = new AbstractAction("Paste", Utils.getIcon("/icons/paste_edit.gif"))
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    // TODO: implement
		}
	    };
	_pasteAction.putValue(Action.ACCELERATOR_KEY, KeyStroke.getKeyStroke("ctrl V"));
	_pasteAction.setEnabled(false);
	editMenu.add(_pasteAction);
	editMenu.addSeparator();
	toolBar.add(_pasteAction);
	toolBar.addSeparator();

	Action deleteAction = new AbstractAction("Delete", Utils.getIcon("/icons/delete_edit.gif"))
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    // TODO: implement
		}
	    };
	deleteAction.putValue(Action.ACCELERATOR_KEY, KeyStroke.getKeyStroke("DELETE"));    
	editMenu.add(deleteAction);
	toolBar.add(deleteAction);

	toolBar.addSeparator();
	Action substituteVarAction = new AbstractAction("${}")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    _model.toggleSubstituteVariables();
		}
	    };

	substituteVarAction.putValue(Action.SHORT_DESCRIPTION, 
				     "Substitute variables and parameters in servers' properties");
    
	toolBar.add(new JToggleButton(substituteVarAction));


	Action aboutAction = new AbstractAction("About...")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    // TODO: implement
		}
	    };
	helpMenu.add(aboutAction);
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

    private Action _saveAction;
    private Action _discardAction;
    private Action _pasteAction;

    private Preferences _prefs = Preferences.userNodeForPackage(getClass());

    private Ice.Communicator _communicator;
    private int _invocationTimeout = 10000;
    private int _observerTimeout = 10000;

    private SessionKeeper _sessionKeeper;
    private Model _model;

}


