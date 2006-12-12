// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
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

import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.JButton;
import javax.swing.JComboBox;
import javax.swing.JComponent;
import javax.swing.JDialog;
import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JRadioButton;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JToolBar;
import javax.swing.SwingUtilities;

import com.jgoodies.looks.BorderStyle;
import com.jgoodies.looks.HeaderStyle;
import com.jgoodies.looks.Options;
import com.jgoodies.looks.plastic.PlasticLookAndFeel;

import IceGrid.*;
import IceGridGUI.*;

class ShowLogDialog extends JDialog
{
    static interface FileIteratorFactory
    {
	FileIteratorPrx open(int count)
	    throws Ice.UserException;

	String getTitle();
    }

    private class FIFOTextArea extends JTextArea
    {
	FIFOTextArea(int rows, int colums)
	{
	    super(rows, colums);
	    setEditable(false);
	    setLineWrap(true);
	}

	public void appendLines(final String[] lines, final int maxLines, final int maxSize)
	{
	    SwingUtilities.invokeLater(new Runnable()
		{
		    public void run()
		    {	
			for(int i = 0; i < lines.length; ++i)
			{
			    //
			    // The last line is always incomplete
			    //
			    if(i + 1 != lines.length)
			    {
				append(lines[i] + "\n");
			    }
			    else
			    {
				append(lines[i]);
			    }
			    removeLines(maxLines, maxSize);
			}
		    }
		});
	}

	public void removeLines(int maxLines, int maxSize)
	{
	    javax.swing.text.Document doc = getDocument();
	    javax.swing.text.Element rootElt = doc.getDefaultRootElement();
	 
	    //
	    // We keep at least one line, no matter its length
	    //
	    int lineCount = getLineCount();
	    while(lineCount > 1 && (doc.getLength() > maxSize || (lineCount > maxLines)))
	    {
		javax.swing.text.Element firstLine = rootElt.getElement(0);
		try
		{
		    doc.remove(0, firstLine.getEndOffset());
		}
		catch(javax.swing.text.BadLocationException ble)
		{
		    assert false;
		}
		lineCount--;
	    }
	    setCaretPosition(doc.getLength());
	}

    }

    private class ReaderThread extends Thread
    {
	ReaderThread()
	{
	    _threadInitialLines = _initialLines;
	    _threadMaxLines = _maxLines;
	    _threadMaxSize = _maxSize;
	    _threadMaxReadLines = _maxReadLines;

	    _play.setEnabled(false);
	    _pause.setEnabled(true);
	    _stop.setEnabled(true);
	}

	private void openError(final String message)
	{
	    SwingUtilities.invokeLater(
		new Runnable()
		{
		    public void run()
		    {
			if(_textArea.getText() == null || _textArea.getText().length() == 0)
			{
			    close(true);
			}
			else
			{
			    stopReading();
			}
			
			JOptionPane.showMessageDialog(
			    ShowLogDialog.this,
			    message,
			    _factory.getTitle() + ": cannot open file",
			    JOptionPane.ERROR_MESSAGE);
		    }
		});
	}

	
	public void run()
	{
	    //
	    // Open file
	    //
	    int initialLines;

	    synchronized(this)
	    {
		initialLines = _threadInitialLines;
	    }

	    try
	    {
		_p = _factory.open(initialLines);
	    }
	    catch(Ice.UserException e)
	    {
		openError(e.toString());
		return;
	    }
	    catch(Ice.LocalException e)
	    {
		openError(e.toString());
		return;
	    }

	    SwingUtilities.invokeLater(
		new Runnable()
		{
		    public void run()
		    {
			if(isVisible())
			{
			    _textArea.setText(null);
			}
			else
			{
			    setVisible(true);
			}
		    }
		});

	    boolean firstRun = true;
	    for(;;)
	    {
		synchronized(this)
		{
		    if(!_done)
		    {
			if(_paused)
			{
			    while(_paused && !_done)
			    {
				try
				{
				    wait();
				}
				catch(InterruptedException e)
				{
				}
			    }
			}
			else if(!firstRun)
			{
			    try
			    {
				wait(_period);
			    }
			    catch(InterruptedException e)
			    {
			    }
			}
			else
			{
			    firstRun = false;
			}
		    }
		    
		    if(_done)
		    {
			cleanupIterator();
			return;
		    }
		}

		boolean eofEncountered = false;
		
		while(!eofEncountered)
		{
		    int maxLines;
		    int maxSize;
		    int maxReadLines;

		    synchronized(this)
		    {
			if(_done || _paused)
			{
			    break; // while(!eofEncountered)
			}

			maxLines = _threadMaxLines;
			maxSize = _threadMaxSize;
			maxReadLines = _threadMaxReadLines;
		    }
		    
		    Ice.StringSeqHolder linesHolder = new Ice.StringSeqHolder();

		    try
		    {
			eofEncountered = _p.read(_maxReadSize, linesHolder); 
		    }
		    catch(IceGrid.FileNotAvailableException e)
		    {
			_textArea.appendLines(new String[]
			    {
				"---------------------------",
				"IceGridAdmin caught: " + e.toString(),
				"---------------------------"
			    }, maxLines, maxSize);
			SwingUtilities.invokeLater(
			    new Runnable()
			    {
				public void run()
				{
				    stopReading();
				}
			    });
			cleanupIterator();
			return;
		    }
		    catch(Ice.LocalException e)
		    {
			_textArea.appendLines(new String[]
			    {
				"---------------------------",
				"IceGridAdmin caught: " + e.toString(),
				"---------------------------"
			    }, maxLines, maxSize);
			SwingUtilities.invokeLater(
			    new Runnable()
			    {
				public void run()
				{
				    stopReading();
				}
			    });
			return;
		    }
		    
		    _textArea.appendLines(linesHolder.value, maxLines, maxSize);
		}
	    }
	}
	
	private void cleanupIterator()
	{
	    try
	    {
		_p.destroy();
	    }
	    catch(Ice.LocalException e)
	    {
		// Ignored, maybe should log warning
	    }
	}

	synchronized void pause()
	{
	    assert !_paused;
	    _paused = true;
	    notify();
	}

	synchronized void terminate()
	{
	    _done = true;
	    notify();
	}

	synchronized void play()
	{
	    assert _paused;
	    _paused = false;
	    notify();
	}
	
	synchronized void setPrefs(int initialLines, int maxLines, int maxSize, int maxReadLines)
	{
	    _threadInitialLines = initialLines;
	    _threadMaxLines = maxLines;
	    _threadMaxSize = maxSize;
	    _threadMaxReadLines = maxReadLines;
	}

	private  FileIteratorPrx _p;
	private boolean _done = false;
	private boolean _paused = false;

	private int _threadInitialLines;
	private int _threadMaxLines;
	private int _threadMaxSize;
	private int _threadMaxReadLines;

    }
    
    private class MenuBar extends JMenuBar
    {
	private MenuBar()
	{
	    putClientProperty(Options.HEADER_STYLE_KEY, HeaderStyle.BOTH);
	    putClientProperty(PlasticLookAndFeel.BORDER_STYLE_KEY, BorderStyle.SEPARATOR);

	    //
	    // File menu
	    // 
	    JMenu fileMenu = new JMenu("File");
	    fileMenu.setMnemonic(java.awt.event.KeyEvent.VK_F);
	    add(fileMenu);
	    fileMenu.add(_pause);
	    fileMenu.add(_play);
	    fileMenu.add(_stop);
	    fileMenu.addSeparator();
	    fileMenu.add(new AbstractAction("Save as...")
		{
		    public void actionPerformed(ActionEvent e)
		    {
			// todo
		    }

		});
	    fileMenu.addSeparator();

	    fileMenu.add(new AbstractAction("Close")
		{
		    public void actionPerformed(ActionEvent e)
		    {
			close(true);
		    }
		});
	    JMenu editMenu = new JMenu("Edit");
	    editMenu.setMnemonic(java.awt.event.KeyEvent.VK_E);
	    add(editMenu);
	    editMenu.add(new AbstractAction("Copy")
		{
		    public void actionPerformed(ActionEvent e)
		    {
			_textArea.copy();
		    }

		});
	    editMenu.addSeparator();
	    editMenu.add(new AbstractAction("Select all")
		{
		    public void actionPerformed(ActionEvent e)
		    {
			_textArea.grabFocus();
			_textArea.selectAll();
		    }

		});
	    editMenu.addSeparator();
	    editMenu.add(new AbstractAction("Preferences")
		{
		    public void actionPerformed(ActionEvent e)
		    {
			// todo
		    }
		});	  
	}
    }

    private class ToolBar extends JToolBar
    {
	private ToolBar()
	{
	    putClientProperty(Options.HEADER_STYLE_KEY, HeaderStyle.BOTH);
	    putClientProperty(PlasticLookAndFeel.BORDER_STYLE_KEY, BorderStyle.SEPARATOR);
	    setFloatable(false);
	    putClientProperty("JToolBar.isRollover", Boolean.TRUE);

	    JButton button = new JButton(_pause);
	    button.setText(null);
	    button.setIcon(Utils.getIcon("/icons/16x16/pause.png"));
	    add(button);
	    button = new JButton(_play);
	    button.setText(null);
	    button.setIcon(Utils.getIcon("/icons/16x16/start.png"));
	    add(button);
	    button = new JButton(_stop);
	    button.setText(null);
	    button.setIcon(Utils.getIcon("/icons/16x16/stop.png"));
	    add(button);
	    
	}
    }


    ShowLogDialog(Root root, FileIteratorFactory factory, 
		  int period, int initialLines, int maxLines, int maxSize, int maxReadLines)
    {
	super(root.getCoordinator().getMainFrame(), factory.getTitle() + " - IceGrid Admin", false);
	
	_period = period;
	_initialLines = initialLines;
	_maxLines = maxLines;
	_maxSize = maxSize;
	_maxReadLines = maxReadLines;
	_factory = factory;
	_root = root;

	setDefaultCloseOperation(JDialog.DO_NOTHING_ON_CLOSE);	
	addWindowListener(new java.awt.event.WindowAdapter()
	    {
		public void windowClosing(java.awt.event.WindowEvent e)
		{
		    close(true);
		}
	    });

	_pause = new AbstractAction("Pause")
	    {
		public void actionPerformed(ActionEvent e)
		{
		    pause();
		}
	    };
	_pause.setEnabled(false);

	
	_play = new AbstractAction("Play")
	    {
		public void actionPerformed(ActionEvent e)
		{
		    play();
		}
	    };
	_play.setEnabled(false);
	    
	
	_stop = new AbstractAction("Stop")
	    {
		public void actionPerformed(ActionEvent e)
		{
		    stopReading();
		}
	    };
	_stop.setEnabled(false);
	
	setJMenuBar(new MenuBar());
	getContentPane().add(new ToolBar(),
			     BorderLayout.PAGE_START);

	JScrollPane scrollPane = new JScrollPane(_textArea, 
						 JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED, 
						 JScrollPane.HORIZONTAL_SCROLLBAR_NEVER);

	getContentPane().add(scrollPane);
	
	pack();
	setResizable(true);
	
	setLocationRelativeTo(root.getCoordinator().getMainFrame());
	play();
    }

    void pause()
    {
	_thread.pause();
	_pause.setEnabled(false);
	_play.setEnabled(true);
    }

    void stopReading()
    {
	if(_thread != null)
	{
	    _thread.terminate();
	    
	    try
	    {
		_thread.join();
	    }
	    catch(InterruptedException e)
	    {
	    }
	    
	    _thread = null;
	    _stop.setEnabled(false);
	    _play.setEnabled(true);
	    _pause.setEnabled(false);
	}
    }

    void play()
    {
	if(_thread != null)
	{
	    _thread.play();
	    _play.setEnabled(false);
	    _pause.setEnabled(true);
	    _stop.setEnabled(true);
	}
	else
	{
	    _thread = new ReaderThread();
	    _thread.start();
	}
    }

    void close(boolean notifyRoot)
    {
	stopReading();

	if(notifyRoot)
	{
	    _root.removeShowLogDialog(_factory.getTitle());
	}
	dispose();
    }

    private final Root _root;
    private final FileIteratorFactory _factory;
    private final int _maxReadSize = 800000; // TODO, compute from MessageSizeMax or remove

    private int _period;
    private int _initialLines;
    private int _maxLines;
    private int _maxSize;
    private int _maxReadLines;

    private Action _play;
    private Action _pause;
    private Action _stop;

    private FIFOTextArea _textArea = new FIFOTextArea(20, 45);
    private ReaderThread _thread;
}
