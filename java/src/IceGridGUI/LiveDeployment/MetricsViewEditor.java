// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceGridGUI.LiveDeployment;

import java.awt.Component;
import java.awt.Cursor;
import java.awt.Dimension;
import java.awt.event.ActionEvent;
import java.awt.event.MouseEvent;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseListener;
import java.awt.event.ActionListener;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;

import java.util.Map;
import java.text.DecimalFormat;


import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.DefaultComboBoxModel;
import javax.swing.JButton;
import javax.swing.AbstractCellEditor;
import javax.swing.DefaultCellEditor;
import javax.swing.JOptionPane;

import javax.swing.event.TreeSelectionEvent;
import javax.swing.event.TreeSelectionListener;

import javax.swing.tree.TreePath;

import javax.swing.JComponent;
import javax.swing.JFrame;
import javax.swing.SwingConstants;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JTextField;
import javax.swing.JTable;
import javax.swing.SwingUtilities;
import javax.swing.UIManager;
import javax.swing.JTree;

import javax.swing.table.TableColumnModel;
import javax.swing.table.DefaultTableCellRenderer;
import javax.swing.table.DefaultTableModel;
import javax.swing.table.TableCellRenderer;
import javax.swing.table.TableCellEditor;
import javax.swing.table.TableColumn;

import com.jgoodies.forms.builder.DefaultFormBuilder;
import com.jgoodies.forms.layout.CellConstraints;

import IceGrid.*;
import IceGridGUI.*;

class MetricsViewEditor extends Editor
{
    private class RefreshThread extends Thread
    {
        RefreshThread(long period)
        {
            _period = period;
            _done = false;
        }

        synchronized public void
        run()
        {
            while(true)
            {
                if(_node != null && _propertiesPanel != null && _propertiesPanel.isVisible())
                {
                    _node.fetchMetricsView();
                }
                if(!_done)
                {
                    try
                    {
                        wait(_period);
                    }
                    catch(InterruptedException ex)
                    {
                    }
                }

                if(_done)
                {
                    break;
                }
            }
        }

        synchronized public void
        done()
        {
            if(!_done)
            {
                _done = true;
                notify();
            }
        }

        private final long _period;
        private boolean _done = false;
    }


    //
    // This class allow to render a button in JTable cell.
    //
    public class ButtonRenderer extends DefaultTableCellRenderer
    {
        @Override public Component 
        getTableCellRendererComponent(JTable table, Object value, boolean selected, boolean hasFocus, int row, 
                                      int column)
        {
            if(value == null)
            {
                return null;
            }
            JButton button = (JButton)value;
            if(selected)
            {
                button.setForeground(table.getSelectionForeground());
                button.setBackground(table.getSelectionBackground());
            }
            else
            {
                button.setForeground(table.getForeground());
                button.setBackground(UIManager.getColor("Button.background"));
            }
            return button;  
        }
    }

    //
    // Handle button clicks when buttons are embedded in a JTable cell.
    //
    public class ButtonMouseListener extends MouseAdapter
    {
        public ButtonMouseListener(JTable table)
        {
            _table = table;
        }

        @Override public void 
        mouseClicked(MouseEvent e)
        {
            int column = _table.getColumnModel().getColumnIndexAtX(e.getX());
            int row = e.getY() / _table.getRowHeight(); 

            if(row < _table.getRowCount() && row >= 0 && column < _table.getColumnCount() && column >= 0)
            {
                Object value = _table.getValueAt(row, column);
                if(value instanceof JButton)
                {
                    ((JButton)value).doClick();
                }
            }
        }

        private final JTable _table;
    }

    private class SelectionListener implements TreeSelectionListener
    {
        public void valueChanged(TreeSelectionEvent e)
        {
            TreePath path = null;
            if(e.isAddedPath())
            {
                path = e.getPath();
            }
            
            if(path != null && path.getLastPathComponent() instanceof MetricsView)
            {
                startRefreshThread();
            }
            else
            {
                stopRefreshThread();
            }
        }
    }

    MetricsViewEditor(Root root)
    {
        JTree tree = root.getTree();
        tree.addTreeSelectionListener(new SelectionListener());
    }

    void startRefreshThread()
    {
        if(_refreshThread == null)
        {
            _refreshThread = new RefreshThread(5000);
            _refreshThread.start();
        }
    }

    void stopRefreshThread()
    {
        if(_refreshThread != null)
        {
            _refreshThread.done();
            _refreshThread = null;
        }
    }

    public MetricsView currentView()
    {
        return _node;
    }

    public void show(final MetricsView node)
    {
        _node = node;

        //
        // Call showInternal later on the Swing thread. Without this JTable
        // doesn't repaint correctly, when call addRow in the model.
        //
        SwingUtilities.invokeLater(new Runnable()
        {
            public void run()
            {
                showInternal(node);
            }
        });
    }

    public void showInternal(final MetricsView node)
    {
        java.util.Map<java.lang.String, IceMX.Metrics[]> data = node.data();
        _endpointLookups.getDataVector().removeAllElements();
        _dispatch.getDataVector().removeAllElements();
        _threads.getDataVector().removeAllElements();
        _connections.getDataVector().removeAllElements();
        _invocations.getDataVector().removeAllElements();
        _connectionsEstablishments.getDataVector().removeAllElements();

        if(data == null)
        {
            return;
        }

        for(Map.Entry<String, IceMX.Metrics[]> entry : data.entrySet())
        {
            String key = entry.getKey();
            IceMX.Metrics[] values = entry.getValue();
            if(key.equals("EndpointLookup"))
            {
                for(int i = 0; i < values.length; ++i)
                {
                    IceMX.Metrics o = values[i];
                    Object[] row = new Object[5];
                    row[0] = o.id;
                    row[1] = Integer.toString(o.current);
                    row[2] = Long.toString(o.total);
                    row[3] = metricAvg(o, 1000.0f);
                    row[4] = failures(key, node, o);
                    _endpointLookups.addRow(row);
                }
            }
            else if(key.equals("Dispatch"))
            {
                for(int i = 0; i < values.length; ++i)
                {
                    IceMX.Metrics o = values[i];
                    Object[] row = new Object[5];
                    row[0] = o.id;
                    row[1] = Integer.toString(o.current);
                    row[2] = Long.toString(o.total);
                    row[3] = metricAvg(o, 1000.0f);
                    row[4] = failures(key, node, o);
                    _dispatch.addRow(row);
                }

            }
            else if(key.equals("Thread"))
            {
                for(int i = 0; i < values.length; ++i)
                {
                    IceMX.ThreadMetrics o = (IceMX.ThreadMetrics)values[i];
                    Object[] row = new Object[8];
                    row[0] = o.id;
                    row[1] = Integer.toString(o.current);
                    row[2] = Long.toString(o.total);
                    row[3] = Integer.toString(o.inUseForIO);
                    row[4] = Integer.toString(o.inUseForUser);
                    row[5] = Integer.toString(o.inUseForOther);
                    row[6] = metricAvg(o, 1000000.0f);
                    row[7] = failures(key, node, o);
                    _threads.addRow(row);
                }
            }
            else if(key.equals("Connection"))
            {
                for(int i = 0; i < values.length; ++i)
                {
                    IceMX.ConnectionMetrics o = (IceMX.ConnectionMetrics)values[i];
                    Object[] row = new Object[9];
                    row[0] = o.id;
                    row[1] = Integer.toString(o.current);
                    row[2] = Long.toString(o.total);
                    row[3] = Long.toString(o.receivedBytes);
                    row[4] = Long.toString(o.receivedTime);
                    row[5] = Long.toString(o.sentBytes);
                    row[6] = Long.toString(o.sentTime);
                    row[7] = metricAvg(o, 1000000.0f);
                    row[8] = failures(key, node, o);
                    _connections.addRow(row);
                }
            }
            else if(key.equals("Invocation"))
            {
                for(int i = 0; i < values.length; ++i)
                {
                    IceMX.InvocationMetrics o = (IceMX.InvocationMetrics)values[i];
                    Object[] row = new Object[6];
                    row[0] = o.id;
                    row[1] = Integer.toString(o.current);
                    row[2] = Long.toString(o.total);
                    row[3] = Integer.toString(o.retry);
                    row[4] = metricAvg(o, 1000.0f);
                    row[5] = failures(key, node, o);
                    _invocations.addRow(row);
                }
            }
            else if(key.equals("ConnectionEstablishment"))
            {
                for(int i = 0; i < values.length; ++i)
                {
                    IceMX.Metrics o = values[i];
                    Object[] row = new Object[5];
                    row[0] = o.id;
                    row[1] = Integer.toString(o.current);
                    row[2] = Long.toString(o.total);
                    row[3] = metricAvg(o, 1000.0f);
                    row[4] = failures(key, node, o);
                    _connectionsEstablishments.addRow(row);
                }
            }
        }
    }

    private String metricAvg(IceMX.Metrics o, float divisor)
    {
        if(o.total > o.current)
        {
            DecimalFormat df = new DecimalFormat("0.###");
            return df.format(o.totalLifetime / divisor / (o.total - o.current)).toString();
        }
        else
        {
            return "0.000";
        }
    }

    private Object failures(final String map, final MetricsView node, final IceMX.Metrics o)
    {
        JButton button = new JButton("Show Failures (" + Integer.toString(o.failures) + ")");
        if(o.failures > 0)
        {
            button.addActionListener(new ActionListener()
                {
                    public void actionPerformed(ActionEvent e)
                    {
                        final DefaultTableModel model = new DefaultTableModel();
                        model.addColumn("Count");
                        model.addColumn("Type");
                        model.addColumn("Identity");
                        JTable table = new JTable(model);
                        table.setPreferredSize(new Dimension(550, 200));
                        table.setPreferredScrollableViewportSize(table.getPreferredSize());
                        table.setCellSelectionEnabled(false);
                        table.setOpaque(false);

                        DefaultTableCellRenderer cr = (DefaultTableCellRenderer)table.getDefaultRenderer(String.class);
                        cr.setOpaque(false);

                        DefaultTableCellRenderer rr = new DefaultTableCellRenderer();
                        rr.setHorizontalAlignment(SwingConstants.RIGHT);
                        rr.setOpaque(false);
                        table.getColumnModel().getColumn(0).setCellRenderer(rr);

                        table.setAutoResizeMode(JTable.AUTO_RESIZE_OFF);
                        table.getColumnModel().getColumn(0).setPreferredWidth(50);
                        table.getColumnModel().getColumn(1).setPreferredWidth(250);
                        table.getColumnModel().getColumn(2).setPreferredWidth(250);

                        JScrollPane scrollPane = new JScrollPane(table);

                        IceMX.Callback_MetricsAdmin_getMetricsFailures cb = 
                            new IceMX.Callback_MetricsAdmin_getMetricsFailures()
                                {
                                    public void response(final IceMX.MetricsFailures data)
                                    {
                                        SwingUtilities.invokeLater(new Runnable()
                                            {
                                                public void run()
                                                {
                                                    for(Map.Entry<String, Integer> entry : data.failures.entrySet())
                                                    {
                                                        Object[] row = new Object[3];
                                                        row[0] = entry.getValue().toString();
                                                        row[1] = entry.getKey();
                                                        row[2] = o.id;
                                                        model.addRow(row);
                                                    }
                                                    node.getCoordinator().getMainFrame().setCursor(Cursor.getPredefinedCursor(Cursor.DEFAULT_CURSOR));
                                                }
                                            });
                                    }

                                    public void exception(final Ice.LocalException e)
                                    {
                                        SwingUtilities.invokeLater(new Runnable()
                                            {
                                                public void run()
                                                {
                                                    node.getCoordinator().getMainFrame().setCursor(Cursor.getPredefinedCursor(Cursor.DEFAULT_CURSOR));
                                                    e.printStackTrace();
                                                    if(e instanceof Ice.ObjectNotExistException)
                                                    {
                                                        // Server is down.
                                                    }
                                                    else if(e instanceof Ice.FacetNotExistException)
                                                    {
                                                        // MetricsAdmin facet not present.
                                                    }
                                                    else
                                                    {
                                                        e.printStackTrace();
                                                        JOptionPane.showMessageDialog(node.getCoordinator().getMainFrame(), 
                                                                                    "Error: " + e.toString(), "Error",
                                                                                    JOptionPane.ERROR_MESSAGE);
                                                    }
                                                }
                                            });
                                    }

                                    public void exception(final Ice.UserException e)
                                    {
                                        SwingUtilities.invokeLater(new Runnable()
                                            {
                                                public void run()
                                                {
                                                    node.getCoordinator().getMainFrame().setCursor(Cursor.getPredefinedCursor(Cursor.DEFAULT_CURSOR));
                                                    e.printStackTrace();
                                                    JOptionPane.showMessageDialog(node.getCoordinator().getMainFrame(), 
                                                                                    "Error: " + e.toString(), "Error",
                                                                                    JOptionPane.ERROR_MESSAGE);
                                                }
                                            });
                                    }
                                };
                        node.getCoordinator().getMainFrame().setCursor(Cursor.getPredefinedCursor(Cursor.WAIT_CURSOR));
                        node.fetchMetricsFailures(map, o.id, cb);
                        JOptionPane.showMessageDialog(_node.getCoordinator().getMainFrame(),
                            scrollPane,
                            "Metrics Failures",
                            JOptionPane.PLAIN_MESSAGE );
                        node.getCoordinator().getMainFrame().setCursor(Cursor.getPredefinedCursor(Cursor.DEFAULT_CURSOR));
                    }
                });
        }
        return button;

    }

    protected void appendProperties(DefaultFormBuilder builder)
    {
        createScrollTable(builder, "Operation Dispatch", _dispatColumnNames, _dispatch);

        createScrollTable(builder, "Application Connections", _connectionsColumnNames, _connections);

        createScrollTable(builder, "Remote Invocations", _invocationsColumnNames, _invocations);

        createScrollTable(builder, "Application Threads", _threadsColumnNames, _threads);

        createScrollTable(builder, "Connection Establishments", _connectionsEstablishmentsColumnNames,
                          _connectionsEstablishments);

        createScrollTable(builder, "Endpoint Lookups", _endpointLookupsColumnNames, _endpointLookups);
    }

    private void createScrollTable(DefaultFormBuilder builder, String title, Object[] columnNames,
                                   DefaultTableModel model)
    {
        CellConstraints cc = new CellConstraints();
        for(Object name : columnNames)
        {
            model.addColumn(name);
        }
        builder.appendSeparator(title);
        builder.append("");
        builder.nextLine();
        builder.append("");
        builder.nextLine();
        builder.append("");
        builder.nextLine();
        builder.append("");
        builder.nextLine();
        builder.append("");
        builder.nextLine();
        builder.append("");
        builder.nextRow(-10);
        JTable table = new JTable(model);
        table.setCellSelectionEnabled(false);
        table.setOpaque(false);
        table.addMouseListener(new ButtonMouseListener(table));
        JScrollPane scrollPane = new JScrollPane(table);
        builder.add(scrollPane, cc.xywh(builder.getColumn(), builder.getRow(), 3, 10));
        builder.nextRow(10);
        builder.nextLine();

        DefaultTableCellRenderer cr = (DefaultTableCellRenderer)table.getDefaultRenderer(String.class);
        cr.setOpaque(false);
        DefaultTableCellRenderer rr = new DefaultTableCellRenderer();
        rr.setHorizontalAlignment(SwingConstants.RIGHT);
        rr.setOpaque(false);
        for(int i = 1; i < columnNames.length - 1; i++)
        {
            table.getColumnModel().getColumn(i).setCellRenderer(rr);
        }
        DefaultTableCellRenderer br = new ButtonRenderer();
        br.setOpaque(false);
        table.getColumnModel().getColumn(columnNames.length - 1).setCellRenderer(br);
    }

    protected void buildPropertiesPanel()
    {
        super.buildPropertiesPanel();
        _propertiesPanel.setName("Metrics Report");
    }
    
    class TableModel extends DefaultTableModel
    {
        public boolean isCellEditable(int row, int column)
        {
            return false;
        }
    }

    private RefreshThread _refreshThread;

    private DefaultTableModel _endpointLookups = new TableModel();
    private Object[] _endpointLookupsColumnNames = new Object[]{"Identity", "Current", "Total", "Avg (ms)", 
                                                                "Failures"};

    private DefaultTableModel _threads = new TableModel();
    private Object[] _threadsColumnNames = new Object[]{"Identity", "Current", "Total", "IO", "User", "Other", 
                                                               "Avg (ms)", "Failures"};

    private DefaultTableModel _dispatch = new TableModel();
    private Object[] _dispatColumnNames = new Object[]{"Identity", "Current", "Total", "Avg (ms)", "Failures"};

    private DefaultTableModel _connections = new TableModel();
    private Object[] _connectionsColumnNames = new Object[]{"Identity", "Current", "Total", "RxBytes", "RxTime", 
                                                            "TxBytes", "TxTime", "Avg (s)", "Failures"};

    private DefaultTableModel _invocations = new TableModel();
    private Object[] _invocationsColumnNames = new Object[]{"Identity", "Current", "Total", "Retries", "Avg (ms)",
                                                            "Failures"};

    private DefaultTableModel _connectionsEstablishments = new TableModel();
    private Object[] _connectionsEstablishmentsColumnNames =  new Object[]{"Identity", "Current", "Total", "Avg (ms)", 
                                                                           "Failures"};
    private MetricsView _node;
}
