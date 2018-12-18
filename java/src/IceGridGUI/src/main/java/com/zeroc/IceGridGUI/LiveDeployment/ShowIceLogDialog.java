// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceGridGUI.LiveDeployment;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Dimension;
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
import javax.swing.JTable;
import javax.swing.JToggleButton;
import javax.swing.JToolBar;
import javax.swing.KeyStroke;
import javax.swing.ScrollPaneConstants;
import javax.swing.SwingUtilities;
import javax.swing.table.*;
import javax.swing.WindowConstants;

import com.jgoodies.looks.BorderStyle;
import com.jgoodies.looks.HeaderStyle;
import com.jgoodies.looks.Options;
import com.jgoodies.looks.plastic.PlasticLookAndFeel;

import com.zeroc.Ice.Current;
import com.zeroc.Ice.LocalException;
import com.zeroc.Ice.LogMessage;
import com.zeroc.Ice.LogMessageType;
import com.zeroc.Ice.RemoteLoggerPrx;
import com.zeroc.Ice.UserException;
import com.zeroc.IceGridGUI.*;

class ShowIceLogDialog extends JDialog
{
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
                        JFileChooser fileChooser = _parent.getRoot().getCoordinator().getSaveIceLogChooser();

                        fileChooser.setSelectedFile(
                            new java.io.File(fileChooser.getCurrentDirectory(), _defaultFileName + ".csv"));

                        java.io.File file = null;

                        while(file == null)
                        {
                            int result = fileChooser.showSaveDialog(ShowIceLogDialog.this);
                            if(result == JFileChooser.APPROVE_OPTION)
                            {
                                file = fileChooser.getSelectedFile();

                                if(file != null)
                                {
                                    if(!file.exists() && file.getName().indexOf('.') == -1)
                                    {
                                        file = new java.io.File(file.getAbsolutePath() + ".csv");
                                    }

                                    java.io.OutputStreamWriter os = null;

                                    try
                                    {
                                        os = new java.io.OutputStreamWriter(new java.io.FileOutputStream(file));

                                        for(Object p : _tableModel.getDataVector())
                                        {
                                            @SuppressWarnings("unchecked")
                                            java.util.Vector<Object> row = (java.util.Vector<Object>)p;
                                            String txt = "\"" + renderDate((java.util.Date) row.elementAt(0)) + "\"," +
                                                    renderLogMessageType((LogMessageType) row.elementAt(1)) + ",\"" +
                                                    row.elementAt(2).toString().replace("\"", "\"\"") + "\",\"" +
                                                    row.elementAt(3).toString().replace("\"", "\"\"") + "\"";

                                            txt += "\r\n";
                                            os.write(txt, 0, txt.length());
                                        }
                                    }
                                    catch(java.io.IOException io)
                                    {
                                        JOptionPane.showMessageDialog(
                                            ShowIceLogDialog.this,
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
                        String txt = new String();
                        for(int i : _table.getSelectedRows())
                        {
                            int j = _table.convertRowIndexToModel(i);

                            txt += renderDate((java.util.Date)_tableModel.getValueAt(j, 0)) + "\t" +
                                    renderLogMessageType((LogMessageType)_tableModel.getValueAt(j, 1)) + "\t" +
                                    _tableModel.getValueAt(j, 2).toString() + "\t" +
                                    renderMessage(_tableModel.getValueAt(j, 3).toString()) + "\n";
                        }

                        java.awt.datatransfer.StringSelection ss = new java.awt.datatransfer.StringSelection(txt);

                        java.awt.Toolkit.getDefaultToolkit().getSystemClipboard().setContents(ss, null);
                    }

                };
            copy.putValue(Action.ACCELERATOR_KEY, KeyStroke.getKeyStroke(KeyEvent.VK_C, MENU_MASK));
            copy.putValue(Action.SHORT_DESCRIPTION, "Copy");
            _table.getActionMap().put("copy", copy);

            editMenu.add(copy);

            editMenu.addSeparator();
            Action selectAll = new AbstractAction("Select All")
                {
                    @Override
                    public void actionPerformed(ActionEvent e)
                    {
                        _table.grabFocus();
                        _table.selectAll();
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
                        new LogPrefsDialog(ShowIceLogDialog.this);
                    }
                });
            editMenu.addSeparator();
            editMenu.add(new AbstractAction("Filter...")
                {
                    @Override
                    public void actionPerformed(ActionEvent e)
                    {
                        new LogFilterDialog(ShowIceLogDialog.this);
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

    private class RemoteLoggerI implements com.zeroc.Ice.RemoteLogger
    {
        @Override
        public synchronized void init(String prefix, LogMessage[] logMessages, Current current)
        {
            // Ignore prefix

            if(!_destroyed)
            {
                _rowCount = logMessages.length +
                    _queue.size() < _maxRows ? logMessages.length + _queue.size() : _maxRows;

                final Object[][] data = new Object[_rowCount][];

                int i = _rowCount - 1;

                for(java.util.Iterator<LogMessage> p = _queue.descendingIterator(); p.hasNext() && i >= 0; i--)
                {
                    data[i] = logMessageToRow(p.next());
                }

                int j = logMessages.length - 1;
                while(i >= 0 && j >= 0)
                {
                    data[i] = logMessageToRow(logMessages[j]);
                    i--;
                    j--;
                }

               _queue.clear();
               _paused = false;

                SwingUtilities.invokeLater(() ->
                    {
                        _tableModel.setDataVector(data, _columnNames);
                        _table.scrollRectToVisible(_table.getCellRect(_table.getRowCount() - 1, 0, true));
                        _pause.setEnabled(true);
                    });
            }

        }

        @Override
        public synchronized void log(LogMessage message, Current current)
        {
            if(_destroyed)
            {
                // Nothing to do
            }
            else if(_paused)
            {
                _queue.addLast(message);
                // We keep up to _maxRow in the queue
                while(_queue.size() > _maxRows)
                {
                    _queue.removeFirst();
                }
            }
            else
            {
                showLogMessage(message);
            }
        }

        private synchronized void setMaxRows(int maxRows)
        {
            _maxRows = maxRows;

            final int rowsToRemove = _rowCount - _maxRows;

            if(rowsToRemove > 0)
            {
                _rowCount -= rowsToRemove;

                SwingUtilities.invokeLater(() ->
                    {
                        int i = rowsToRemove;
                        while(i-- > 0)
                        {
                            _tableModel.removeRow(0);
                        }
                    });
            }
        }

        private synchronized void pause()
        {
            assert(!_destroyed);
            _paused = true;
        }

        private synchronized void play()
        {
            assert(!_destroyed);
            for(LogMessage p : _queue)
            {
                showLogMessage(p);
            }
            _queue.clear();
            _paused = false;
        }

        private synchronized void stop()
        {
            _destroyed = true;
        }

        private void showLogMessage(LogMessage msg)
        {
            final Object[] row = logMessageToRow(msg);
            _rowCount++;
            final int rowsToRemove = _rowCount - _maxRows;
            if(rowsToRemove > 0)
            {
                _rowCount -= rowsToRemove;
            }

            SwingUtilities.invokeLater(() ->
                {
                    _tableModel.addRow(row);
                    int i = rowsToRemove;
                    while(i-- > 0)
                    {
                        _tableModel.removeRow(0);
                    }
                    _table.scrollRectToVisible(_table.getCellRect(_table.getRowCount() - 1, 0, true));
                });
        }

        private boolean _paused = true;
        private boolean _destroyed = false;
        private final java.util.Deque<LogMessage> _queue = new java.util.ArrayDeque<>();
        private int _rowCount = 0;
        private int _maxRows = _maxMessages;
    }

    static private class DateRenderer extends DefaultTableCellRenderer
    {
        @Override
        public void setValue(Object value)
        {
            if(value == null)
            {
                setText("");
            }
            else
            {
                setText(renderDate((java.util.Date) value));
            }
        }
    }

    static private class LogMessageTypeRenderer extends DefaultTableCellRenderer
    {
        @Override
        public void setValue(Object value)
        {
            if(value == null)
            {
                setText("");
            }
            else
            {
                setText(renderLogMessageType((LogMessageType) value));
            }
        }
    }

    static private class MessageRenderer extends DefaultTableCellRenderer
    {
        @Override
        public void setValue(Object value)
        {
            if(value == null)
            {
                setText("");
            }
            else
            {
                setText(renderMessage((String)value));
            }
        }
    }

    ShowIceLogDialog(TreeNode parent, String title, com.zeroc.Ice.LoggerAdminPrx loggerAdmin, String defaultFileName,
                     int maxMessages, int initialMessages)
    {
        super(parent.getRoot().getCoordinator().getMainFrame(), title + " - IceGrid GUI", false);

        _parent = parent;
        _title = title;
        _loggerAdmin = loggerAdmin;
        _defaultFileName = defaultFileName;
        _maxMessages = maxMessages;
        _initialMessages = initialMessages;
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
                    stop();
                }
            };

         _tableModel = new DefaultTableModel(_columnNames, 0)
         {
             @Override
             public boolean isCellEditable(int row, int column)
             {
                 return false;
             }
         };

         _table = new JTable(_tableModel)
         {
             @Override
             public java.awt.Component prepareRenderer(javax.swing.table.TableCellRenderer renderer,
                                                       int row, int column)
             {
                 java.awt.Component c = super.prepareRenderer(renderer, row, column);

                 if (!isRowSelected(row))
                 {
                     int modelRow = convertRowIndexToModel(row);
                     LogMessageType type = (LogMessageType)getModel().getValueAt(modelRow, 1);
                     if(type != null)
                     {
                         switch(type)
                         {
                             case ErrorMessage:
                             {
                                 c.setBackground(Color.RED);
                                 break;
                             }
                             case WarningMessage:
                             {
                                 c.setBackground(Color.ORANGE);
                                 break;
                             }
                            case PrintMessage:
                            {
                                c.setBackground(Color.LIGHT_GRAY);
                                break;
                            }
                            default:
                            {
                                c.setBackground(getBackground());
                                break;
                            }
                        }
                    }
                }
                return c;
            }

            @Override
            public String getToolTipText(java.awt.event.MouseEvent e)
            {
                String tip = null;
                java.awt.Point p = e.getPoint();
                int row = rowAtPoint(p);
                int col = columnAtPoint(p);

                if(col == 3 && row >= 0) // Log message
                {
                    Object obj = getValueAt(row, col);
                    if(obj != null)
                    {
                        tip = "<html>" + obj.toString().replace("\n", "<br>")+ "</html>";
                    }
                }
                return tip;
            }

        };

        _table.setAutoResizeMode(JTable.AUTO_RESIZE_LAST_COLUMN);
        _table.setAutoCreateColumnsFromModel(false);

        _table.getColumnModel().getColumn(0).setCellRenderer(new DateRenderer());
        _table.getColumnModel().getColumn(0).setPreferredWidth(200);
        _table.getColumnModel().getColumn(0).setMaxWidth(300);
        _table.getColumnModel().getColumn(1).setCellRenderer(new LogMessageTypeRenderer());
        _table.getColumnModel().getColumn(1).setPreferredWidth(75);
        _table.getColumnModel().getColumn(1).setMaxWidth(100);
        _table.getColumnModel().getColumn(2).setPreferredWidth(150);
        _table.getColumnModel().getColumn(2).setMaxWidth(300);
        _table.getColumnModel().getColumn(3).setCellRenderer(new MessageRenderer());

        _table.getTableHeader().setReorderingAllowed(false);

        //
        // Adjust row height for larger fonts
        //
        int fontSize = _table.getFont().getSize();
        int minRowHeight = fontSize + fontSize / 3;
        if(_table.getRowHeight() < minRowHeight)
        {
            _table.setRowHeight(minRowHeight);
        }

        _table.setRowSelectionAllowed(true);
        _table.setOpaque(false);
        _table.setPreferredScrollableViewportSize(new Dimension(800, 400));
        _table.setFillsViewportHeight(true);

        setJMenuBar(new MenuBar());
        getContentPane().add(new ToolBar(), BorderLayout.PAGE_START);

        JScrollPane scrollPane = new JScrollPane(_table,
                                                 ScrollPaneConstants.VERTICAL_SCROLLBAR_AS_NEEDED,
                                                 ScrollPaneConstants.HORIZONTAL_SCROLLBAR_NEVER);

        getContentPane().add(scrollPane);

        setResizable(true);
        pack();
        Utils.restoreWindowBounds(this, _preferences, "IceLogDialog", _parent.getRoot().getCoordinator().getMainFrame());
        _parent.getRoot().addShowIceLogDialog(_title, this);

        setVisible(true);

        play();
    }

    private void pause()
    {
        _pauseItem.setSelected(true);
        _pauseButton.setSelected(true);
        assert(_remoteLogger != null);
        _remoteLogger.pause();
    }

    private void play()
    {
        if(_remoteLogger == null)
        {
            _tableModel.setRowCount(0);
            if(_messageTypeFilter != null || _traceCategoryFilter != null)
            {
                setTitle(_title + " (Filtered) - IceGrid GUI");
            }
            else
            {
                setTitle(_title + " (No filter) - IceGrid GUI");
            }

            _playItem.setSelected(true);
            _playButton.setSelected(true);
            _pause.setEnabled(false);  // Init will enable Pause

            String id = _loggerAdmin.ice_getIdentity().name + "-" + java.util.UUID.randomUUID().toString();
            _remoteLogger = new RemoteLoggerI();
            _remoteLoggerPrx = RemoteLoggerPrx.uncheckedCast(
                _parent.getRoot().getCoordinator().addCallback(_remoteLogger, id, ""));

            final String prefix = "Attaching remote logger to " + _loggerAdmin.ice_getIdentity().name + "...";
            final String errorTitle = "Failed to attach remote logger to " + _loggerAdmin.ice_getIdentity().name;
            _parent.getRoot().getCoordinator().getStatusBar().setText(prefix);

            try
            {
                _loggerAdmin.attachRemoteLoggerAsync(_remoteLoggerPrx,  _messageTypeFilter, _traceCategoryFilter,
                                                     _initialMessages).whenComplete((result, ex) ->
                    {
                        if(ex == null)
                        {
                            _parent.getRoot().amiSuccess(prefix);
                        }
                        else
                        {
                            SwingUtilities.invokeLater(() ->
                                {
                                    _parent.getRoot().failure(prefix, errorTitle, ex.toString());
                                    stopped();
                                });
                        }
                    });
            }
            catch(LocalException ex)
            {
                _parent.getRoot().failure(prefix, errorTitle, ex.toString());
                stopped();
            }
        }
        else
        {
            _remoteLogger.play();
            _pause.setEnabled(true);
        }
    }

    private void stop(boolean detach)
    {
        if(_remoteLogger != null)
        {
           if(detach)
           {
               final String prefix = "Detaching remote logger from " + _loggerAdmin.ice_getIdentity().name + "...";
               _parent.getRoot().getCoordinator().getStatusBar().setText(prefix);

                try
                {
                    _loggerAdmin.detachRemoteLoggerAsync(_remoteLoggerPrx).whenComplete((result, ex) ->
                        {
                            if(ex == null)
                            {
                                if(result)
                                {
                                    _parent.getRoot().amiSuccess(prefix);
                                }
                                else
                                {
                                    _parent.getRoot().amiSuccess(prefix, "not found");
                                }
                            }
                            else
                            {
                                _parent.getRoot().amiSuccess(prefix, ex.toString());
                            }
                        });
                }
                catch(LocalException ex)
                {
                    _parent.getRoot().success(prefix, ex.ice_id());
                }
            }
        }

        if(_remoteLogger != null)
        {
            _remoteLogger.stop();
            _remoteLogger = null;
        }

        if(_remoteLoggerPrx != null)
        {
            _parent.getRoot().getCoordinator().removeCallback(_remoteLoggerPrx.ice_getIdentity().name, "");
            _remoteLoggerPrx = null;
        }

        _stopItem.setSelected(true);
        _stopButton.setSelected(true);
        _pause.setEnabled(false);
    }

    int getMaxMessages()
    {
        return _maxMessages;
    }

    int getInitialMessages()
    {
        return _initialMessages;
    }

    void setPrefs(int maxMessages, int initialMessages)
    {
        if(maxMessages < 50)
        {
            maxMessages = 50;
        }
        _maxMessages = maxMessages;
        _initialMessages = initialMessages;

        if(_remoteLogger != null)
        {
            _remoteLogger.setMaxRows(_maxMessages);
        }

        _parent.getRoot().setLogPrefs(_maxMessages, _initialMessages);
    }

    LogMessageType[] getMessageTypeFilter()
    {
        return _messageTypeFilter;
    }

    String[] getTraceCategoryFilter()
    {
        return _traceCategoryFilter;
    }

    void setFilters(LogMessageType[] messageTypeFilter, String[] traceCategoryFilter)
    {
        _messageTypeFilter = messageTypeFilter;
        _traceCategoryFilter = traceCategoryFilter;

        if(_remoteLogger != null)
        {
            stop();
            play();
        }
        // otherwise, leave it in stopped mode
    }

    void stop()
    {
        stop(true);
    }

    void stopped()
    {
        stop(false);
    }

    void close(boolean notifyRoot)
    {
        stop();
        _parent.clearShowIceLogDialog();
        if(notifyRoot)
        {
            _parent.getRoot().removeShowIceLogDialog(_title);
        }

        Utils.storeWindowBounds(this, _preferences.node("IceLogDialog"));

        dispose();
    }

    private Object[] logMessageToRow(LogMessage msg)
    {
        Object[] row = new Object[4];

        row[0] = new java.util.Date(msg.timestamp / 1000);
        row[1] = msg.type;
        row[2] = msg.traceCategory;
        row[3] = msg.message;

        return row;
    }

    private final TreeNode _parent;
    private final com.zeroc.Ice.LoggerAdminPrx _loggerAdmin;
    private final String _title;
    private final String _defaultFileName;

    private RemoteLoggerI _remoteLogger;
    private com.zeroc.Ice.RemoteLoggerPrx _remoteLoggerPrx;

    private int _maxMessages;
    private int _initialMessages;

    private LogMessageType[] _messageTypeFilter;
    private String[] _traceCategoryFilter;

    private Action _play;
    private Action _pause;
    private Action _stop;

    private JRadioButtonMenuItem _playItem;
    private JRadioButtonMenuItem _pauseItem;
    private JRadioButtonMenuItem _stopItem;

    private JToggleButton _playButton;
    private JToggleButton _pauseButton;
    private JToggleButton _stopButton;

    private final Object[] _columnNames = new Object[]{"Timestamp", "Type", "Trace Category", "Log Message"};
    private final DefaultTableModel _tableModel;
    private final JTable _table;

    private final Preferences _preferences;

    private static String renderDate(java.util.Date date)
    {
        return _dateFormat.format(date) + _timeFormat.format(date);
    }

    private static String renderLogMessageType(LogMessageType type)
    {
        // Remove "Message" from end of string.
        String s = type.toString();
        assert(s.length() > 7);
        return s.substring(0, s.length() - 7);
    }

    private static String renderMessage(String msg)
    {
        return msg.replace("\n", " ");
    }

    private static final java.text.DateFormat _dateFormat =
        java.text.DateFormat.getDateInstance(java.text.DateFormat.SHORT);

    private static final java.text.DateFormat _timeFormat = new java.text.SimpleDateFormat(" HH:mm:ss:SSS");
}
