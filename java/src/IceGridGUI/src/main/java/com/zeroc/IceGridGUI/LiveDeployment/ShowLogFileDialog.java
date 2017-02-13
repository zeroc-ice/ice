// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceGridGUI.LiveDeployment;

import java.awt.BorderLayout;
import java.awt.event.ActionEvent;
import java.awt.event.KeyEvent;

import java.util.prefs.Preferences;

import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.ButtonGroup;
import javax.swing.JDialog;
import javax.swing.JFileChooser;
import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JOptionPane;
import javax.swing.JRadioButtonMenuItem;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JToggleButton;
import javax.swing.JToolBar;
import javax.swing.KeyStroke;
import javax.swing.ScrollPaneConstants;
import javax.swing.SwingUtilities;
import javax.swing.WindowConstants;

import com.jgoodies.looks.BorderStyle;
import com.jgoodies.looks.HeaderStyle;
import com.jgoodies.looks.Options;
import com.jgoodies.looks.plastic.PlasticLookAndFeel;

import com.zeroc.IceGrid.*;
import com.zeroc.IceGridGUI.*;

class ShowLogFileDialog extends JDialog
{
    static interface FileIteratorFactory
    {
        FileIteratorPrx open(int count)
            throws com.zeroc.Ice.UserException;

        String getTitle();

        String getDefaultFilename();
    }

    private class FIFOTextArea extends JTextArea
    {
        FIFOTextArea(int rows, int colums)
        {
            super(rows, colums);
            setEditable(false);
            setLineWrap(true);
        }

        void appendLines(final String[] lines, final int maxLines, final int maxSize)
        {
            SwingUtilities.invokeLater(() ->
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
                });
        }

        void removeLines(int maxLines, int maxSize)
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
            _threadMaxLines = _maxLines;
            _threadMaxSize = _maxSize;
            _threadInitialLines = _initialLines;
            _threadMaxReadSize = _maxReadSize;
            _threadPeriod = _period;

            _playButton.setSelected(true);
            _playItem.setSelected(true);
            _pause.setEnabled(true);
        }

        private void openError(final String message)
        {
            SwingUtilities.invokeLater(() ->
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
                        ShowLogFileDialog.this,
                        message,
                        _factory.getTitle() + ": cannot open file",
                        JOptionPane.ERROR_MESSAGE);
                });
        }

        @Override
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
            catch(com.zeroc.Ice.UserException e)
            {
                openError(e.toString());
                return;
            }
            catch(com.zeroc.Ice.LocalException e)
            {
                openError(e.toString());
                return;
            }

            SwingUtilities.invokeLater(() ->
                {
                    if(isVisible())
                    {
                        _textArea.setText(null);
                    }
                    else
                    {
                        setVisible(true);
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
                                wait(_threadPeriod);
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
                    int maxReadSize;

                    synchronized(this)
                    {
                        if(_done || _paused)
                        {
                            break; // while(!eofEncountered)
                        }

                        maxLines = _threadMaxLines;
                        maxSize = _threadMaxSize;
                        maxReadSize = _threadMaxReadSize;
                    }

                    FileIterator.ReadResult r = null;

                    try
                    {
                        r = _p.read(maxReadSize);
                        eofEncountered = r.returnValue;
                    }
                    catch(com.zeroc.IceGrid.FileNotAvailableException e)
                    {
                        _textArea.appendLines(new String[]
                            {
                                "---------------------------",
                                "IceGrid GUI caught: " + e.toString(),
                                "---------------------------"
                            }, maxLines, maxSize);
                        SwingUtilities.invokeLater(() -> stopReading());
                        cleanupIterator();
                        return;
                    }
                    catch(com.zeroc.Ice.LocalException e)
                    {
                        _textArea.appendLines(new String[]
                            {
                                "---------------------------",
                                "IceGrid GUI caught: " + e.toString(),
                                "---------------------------"
                            }, maxLines, maxSize);
                        SwingUtilities.invokeLater(() -> stopReading());
                        return;
                    }

                    _textArea.appendLines(r.lines, maxLines, maxSize);
                }
            }
        }

        private void cleanupIterator()
        {
            try
            {
                _p.destroy();
            }
            catch(com.zeroc.Ice.LocalException e)
            {
                // Ignored, maybe should log warning
            }
        }

        synchronized void pause()
        {
            if(!_paused)
            {
                _paused = true;
                notify();
            }
        }

        synchronized void terminate()
        {
            if(!_done)
            {
                _done = true;
                notify();
            }
        }

        synchronized void play()
        {
            if(_paused)
            {
                _paused = false;
                notify();
            }
        }

        synchronized void setPrefs()
        {
            _threadMaxLines = _maxLines;
            _threadMaxSize = _maxSize;
            _threadInitialLines = _initialLines;
            _threadMaxReadSize = _maxReadSize;
            _threadPeriod = _period;
        }

        private FileIteratorPrx _p;
        private boolean _done = false;
        private boolean _paused = false;

        private int _threadMaxLines;
        private int _threadMaxSize;
        private int _threadInitialLines;
        private int _threadMaxReadSize;
        private int _threadPeriod;
    }

    private class MenuBar extends JMenuBar
    {
        private MenuBar()
        {
            putClientProperty(Options.HEADER_STYLE_KEY, HeaderStyle.BOTH);
            putClientProperty(PlasticLookAndFeel.BORDER_STYLE_KEY, BorderStyle.SEPARATOR);

            final int MENU_MASK = java.awt.Toolkit.getDefaultToolkit().getMenuShortcutKeyMask();

            //
            // File menu
            //
            JMenu fileMenu = new JMenu("File");
            fileMenu.setMnemonic(java.awt.event.KeyEvent.VK_F);
            add(fileMenu);

            ButtonGroup bg = new ButtonGroup();

            _pauseItem = new JRadioButtonMenuItem(_pause);
            fileMenu.add(_pauseItem);
            bg.add(_pauseItem);
            _playItem = new JRadioButtonMenuItem(_play);
            fileMenu.add(_playItem);
            bg.add(_playItem);
            _stopItem = new JRadioButtonMenuItem(_stop);
            fileMenu.add(_stopItem);
            bg.add(_stopItem);
            fileMenu.addSeparator();

            Action save = new AbstractAction("Save As...")
                {
                    @Override
                    public void actionPerformed(ActionEvent e)
                    {
                        JFileChooser fileChooser = _root.getCoordinator().getSaveLogFileChooser();

                        fileChooser.setSelectedFile(new java.io.File(fileChooser.getCurrentDirectory(),
                                                                     _factory.getDefaultFilename()));

                        java.io.File file = null;

                        while(file == null)
                        {
                            int result = fileChooser.showSaveDialog(ShowLogFileDialog.this);
                            if(result == JFileChooser.APPROVE_OPTION)
                            {
                                file = fileChooser.getSelectedFile();

                                if(file != null)
                                {
                                    if(!file.exists() && file.getName().indexOf('.') == -1)
                                    {
                                        file = new java.io.File(file.getAbsolutePath() + ".log");
                                    }

                                    java.io.OutputStreamWriter os = null;

                                    try
                                    {
                                        os = new java.io.OutputStreamWriter(new java.io.FileOutputStream(file));
                                        String txt = _textArea.getText();
                                        if(txt == null)
                                        {
                                            txt = "";
                                        }
                                        os.write(txt, 0, txt.length());
                                    }
                                    catch(java.io.IOException io)
                                    {
                                        JOptionPane.showMessageDialog(
                                            ShowLogFileDialog.this,
                                            io.toString(),
                                            "Cannot write file",
                                            JOptionPane.ERROR_MESSAGE);
                                    }
                                    finally
                                    {
                                        if(os != null)
                                        {
                                            try
                                            {
                                                os.close();
                                            }
                                            catch(java.io.IOException io)
                                            {
                                            }
                                        }
                                    }
                                }
                            }
                            else
                            {
                                break; // while
                            }
                        }
                    }
                };
            save.putValue(Action.ACCELERATOR_KEY, KeyStroke.getKeyStroke(KeyEvent.VK_S, MENU_MASK));
            save.putValue(Action.SHORT_DESCRIPTION, "Save As...");
            fileMenu.add(save);
            fileMenu.addSeparator();

            fileMenu.add(new AbstractAction("Close")
                {
                    @Override
                    public void actionPerformed(ActionEvent e)
                    {
                        close(true);
                    }
                });
            JMenu editMenu = new JMenu("Edit");
            editMenu.setMnemonic(java.awt.event.KeyEvent.VK_E);
            add(editMenu);

            Action copy = new AbstractAction("Copy")
                {
                    @Override
                    public void actionPerformed(ActionEvent e)
                    {
                        _textArea.copy();
                    }

                };
            copy.putValue(Action.ACCELERATOR_KEY, KeyStroke.getKeyStroke(KeyEvent.VK_C, MENU_MASK));
            copy.putValue(Action.SHORT_DESCRIPTION, "Copy");
            editMenu.add(copy);

            editMenu.addSeparator();
            Action selectAll = new AbstractAction("Select All")
                {
                    @Override
                    public void actionPerformed(ActionEvent e)
                    {
                        _textArea.grabFocus();
                        _textArea.selectAll();
                    }
                };
            selectAll.putValue(Action.ACCELERATOR_KEY, KeyStroke.getKeyStroke(KeyEvent.VK_A, MENU_MASK));
            selectAll.putValue(Action.SHORT_DESCRIPTION, "Select All");

            editMenu.add(selectAll);
            editMenu.addSeparator();
            editMenu.add(new AbstractAction("Preferences...")
                {
                    @Override
                    public void actionPerformed(ActionEvent e)
                    {
                        new LogPrefsDialog(ShowLogFileDialog.this);
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

            _pauseButton = new JToggleButton(_pause);
            _pauseButton.setText(null);
            _pauseButton.setIcon(Utils.getIcon("/icons/16x16/pause.png"));
            add(_pauseButton);
            _playButton = new JToggleButton(_play);
            _playButton.setText(null);
            _playButton.setIcon(Utils.getIcon("/icons/16x16/start.png"));
            add(_playButton);
            _stopButton = new JToggleButton(_stop);
            _stopButton.setText(null);
            _stopButton.setIcon(Utils.getIcon("/icons/16x16/stop.png"));
            add(_stopButton);

            ButtonGroup bg = new ButtonGroup();
            bg.add(_pauseButton);
            bg.add(_playButton);
            bg.add(_stopButton);
        }
    }

    ShowLogFileDialog(Root root, FileIteratorFactory factory, int maxLines, int maxSize, int initialLines,
                      int maxReadSize, int period)
    {
        super(root.getCoordinator().getMainFrame(), factory.getTitle() + " - IceGrid GUI", false);

        _maxLines = maxLines;
        _maxSize = maxSize;
        _initialLines = initialLines;
        _maxReadSize = maxReadSize;
        _period = period;
        _factory = factory;
        _root = root;
        _preferences = Coordinator.getPreferences().node("LiveDeployment");

        setDefaultCloseOperation(WindowConstants.DO_NOTHING_ON_CLOSE);
        addWindowListener(new java.awt.event.WindowAdapter()
            {
                @Override
                public void windowClosing(java.awt.event.WindowEvent e)
                {
                    close(true);
                }
            });

        _pause = new AbstractAction("Pause")
            {
                @Override
                public void actionPerformed(ActionEvent e)
                {
                    pause();
                }
            };

        _play = new AbstractAction("Play")
            {
                @Override
                public void actionPerformed(ActionEvent e)
                {
                    play();
                }
            };

        _stop = new AbstractAction("Stop")
            {
                @Override
                public void actionPerformed(ActionEvent e)
                {
                    stopReading();
                }
            };

        setJMenuBar(new MenuBar());
        getContentPane().add(new ToolBar(), BorderLayout.PAGE_START);

        JScrollPane scrollPane = new JScrollPane(_textArea,
                                                 ScrollPaneConstants.VERTICAL_SCROLLBAR_AS_NEEDED,
                                                 ScrollPaneConstants.HORIZONTAL_SCROLLBAR_NEVER);

        getContentPane().add(scrollPane);

        setResizable(true);
        pack();
        Utils.restoreWindowBounds(this, _preferences, "LogFileDialog", _root.getCoordinator().getMainFrame());

        play();
    }

    void pause()
    {
        _thread.pause();
        _pauseItem.setSelected(true);
        _pauseButton.setSelected(true);
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
            _stopItem.setSelected(true);
            _stopButton.setSelected(true);
            _pause.setEnabled(false);
        }
    }

    void play()
    {
        if(_thread != null)
        {
            _thread.play();
            _playItem.setSelected(true);
            _playButton.setSelected(true);
            _pause.setEnabled(true);
        }
        else
        {
            _thread = new ReaderThread();
            _thread.start();
        }
    }

    int getMaxLines()
    {
        return _maxLines;
    }

    int getMaxSize()
    {
        return _maxSize;
    }

    int getInitialLines()
    {
        return _initialLines;
    }

    int getMaxReadSize()
    {
        return _maxReadSize;
    }

    int getPeriod()
    {
        return _period;
    }

    void setPrefs(int maxLines, int maxSize, int initialLines, int maxReadSize, int period)
    {
        if(maxLines < 50)
        {
            maxLines = 50;
        }
        _maxLines = maxLines;

        if(maxSize < 1000)
        {
            maxSize = 1000;
        }
        _maxSize = maxSize;

        _initialLines = initialLines;

        if(maxReadSize < 100)
        {
            maxReadSize = 100;
        }
        else if(maxReadSize + 512 > _root.getMessageSizeMax())
        {
            maxReadSize =  _root.getMessageSizeMax() - 512;
        }
        _maxReadSize = maxReadSize;

        if(period < 200)
        {
            period = 200;
        }
        else if(period > 5000)
        {
            period = 5000;
        }
        _period = period;

        if(_thread != null)
        {
            _thread.setPrefs();
        }

        _root.setLogPrefs(_maxLines, _maxSize, _initialLines, _maxReadSize, _period);
    }

    void close(boolean notifyRoot)
    {
        stopReading();

        if(notifyRoot)
        {
            _root.removeShowLogFileDialog(_factory.getTitle());
        }

        Utils.storeWindowBounds(this, _preferences.node("LogFileDialog"));

        dispose();
    }

    private final Root _root;
    private final FileIteratorFactory _factory;

    private int _maxLines;
    private int _maxSize;
    private int _initialLines;
    private int _maxReadSize;
    private int _period;

    private Action _play;
    private Action _pause;
    private Action _stop;

    private JRadioButtonMenuItem _playItem;
    private JRadioButtonMenuItem _pauseItem;
    private JRadioButtonMenuItem _stopItem;

    private JToggleButton _playButton;
    private JToggleButton _pauseButton;
    private JToggleButton _stopButton;

    private FIFOTextArea _textArea = new FIFOTextArea(20, 45);
    private ReaderThread _thread;

    private final Preferences _preferences;
}
