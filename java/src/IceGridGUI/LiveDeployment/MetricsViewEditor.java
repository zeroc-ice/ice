// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceGridGUI.LiveDeployment;

import java.lang.reflect.Field;
import java.lang.reflect.Modifier;

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
import javax.swing.ListSelectionModel;
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
        final java.util.Map<java.lang.String, IceMX.Metrics[]> data = node.data();

        boolean rebuildPanel = false;

        for(Map.Entry<String, IceMX.Metrics[]> entry : data.entrySet())
        {
            if(_tables.get(entry.getKey()) == null)
            {
                IceMX.Metrics[] objects = entry.getValue();
                if(objects.length > 0)
                {
                    java.util.HashMap<String, Integer> columnPositions = new java.util.HashMap<String, Integer>();
                    _columnPositions.put(entry.getKey(), columnPositions);
                    DefaultTableModel model = new TableModel();
                    JTable table = new JTable(model);

                    table.setDragEnabled(true);
                    table.setSelectionMode(ListSelectionModel.MULTIPLE_INTERVAL_SELECTION);
                    table.setCellSelectionEnabled(true);
                    table.addMouseListener(new ButtonMouseListener(table));

                    DefaultTableCellRenderer defaultCellRender = 
                                                    (DefaultTableCellRenderer)table.getDefaultRenderer(String.class);

                    DefaultTableCellRenderer buttonCellRender = new ButtonRenderer();

                    DefaultTableCellRenderer numberCellRender = new DefaultTableCellRenderer();
                    numberCellRender.setHorizontalAlignment(SwingConstants.RIGHT);

                    Class cls = objects[0].getClass();
                    Map<String, Field> fields = new java.util.HashMap<String, Field>();
                    for(Field f : cls.getFields())
                    {
                        if(Modifier.isStatic(f.getModifiers()))
                        {
                            continue;
                        }

                        Class fieldClass = f.getType();

                        if(!fieldClass.equals(int.class) && !fieldClass.equals(long.class) && 
                           !fieldClass.equals(float.class) && !fieldClass.equals(double.class) && 
                           !fieldClass.equals(boolean.class) && !fieldClass.equals(byte.class) && 
                           !fieldClass.equals(String.class) && !fieldClass.isArray())
                        {
                            continue;
                        }
                        fields.put(f.getName(), f);
                    }

                    //
                    // Add fields using the default column sort.
                    //
                    for(String name : _columnSort)
                    {
                        Field field = fields.remove(name);
                        if(field != null)
                        {
                            addField(field, model, columnPositions);
                        }
                    }
            
                    //
                    // Append field names that are not in default column
                    // sort to the end, using the order in class definition.
                    //
                    if(fields.size() > 0)
                    {
                        for(Field f : cls.getFields())
                        {
                            Field field = fields.remove(f.getName());
                            if(field != null)
                            {
                                addField(field, model, columnPositions);    
                            }
                        }
                    }

                    for(Map.Entry<String, Integer> e : columnPositions.entrySet())
                    {
                        try
                        {
                            Field field = cls.getField(e.getKey());
                            Class fieldClass = field.getType();

                            //
                            // XXX Open arrays in a dialog with a table.
                            //
//                             if(field.getName().equals("failures") || fieldClass.isArray())
//                             {
//                                 table.getColumnModel().getColumn(e.getValue().intValue()).
//                                                                                     setCellRenderer(buttonCellRender);
//                             }
                            if(fieldClass.equals(int.class) || fieldClass.equals(long.class) || 
                               fieldClass.equals(float.class) || fieldClass.equals(double.class))
                            {
                                table.getColumnModel().getColumn(e.getValue().intValue()).
                                                                                    setCellRenderer(numberCellRender);
                            }
                        }
                        catch(NoSuchFieldException ex)
                        {
                        }
                    }
                    _tables.put(entry.getKey(), table);
                    rebuildPanel = true;
                }
            }
        }

        if(rebuildPanel)
        {
            buildPropertiesPanel();
        }

        //
        // Load the data.
        //
        for(Map.Entry<String, IceMX.Metrics[]> entry : data.entrySet())
        {
            String key = entry.getKey();
            IceMX.Metrics[] values = entry.getValue();
            JTable table = _tables.get(key);
            DefaultTableModel  model = (DefaultTableModel)table.getModel();
            model.getDataVector().removeAllElements();
            for(IceMX.Metrics m : values)
            {
                Class cls = m.getClass();
                int idx = 0;
                Object[] row = new Object[table.getColumnCount()];
                java.util.Map<String, Integer> columnPositions = _columnPositions.get(key);

                for(Field f : cls.getFields())
                {
                    Integer position = columnPositions.get(f.getName());
                    if(position == null)
                    {
                        continue;
                    }
                    try
                    {
                        row[position.intValue()] = f.get(m).toString();
                    }
                    catch(IllegalAccessException ex)
                    {
                        row[position.intValue()] = null;
                    }
                }
                model.addRow(row);
            }
        }
    }

    private void addField(Field f, DefaultTableModel model, java.util.HashMap<String, Integer> columnPositions)
    {
        String columnName = _columnNames.get(f.getName());
        if(columnName == null)
        {
            columnName = f.getName();
        }
        model.addColumn(columnName);
        columnPositions.put(f.getName(), columnPositions.size());
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

    protected void appendProperties(DefaultFormBuilder builder)
    {
        java.util.Map<String, JTable> tables = new java.util.HashMap<String, JTable>(_tables);
        for(String name : _sectionSort)
        {
            JTable table = tables.remove(name);
            if(table == null)
            {
                continue;
            }
            String section = _sectionNames.get(name);
            if(section == null)
            {
                section = name;
            }
            createScrollTable(builder, section, table);
        }
        for(Map.Entry<String, JTable> entry : tables.entrySet())
        {
            createScrollTable(builder, entry.getKey(), entry.getValue());
        }
    }

    private void createScrollTable(DefaultFormBuilder builder, String title, JTable table)
    {
        CellConstraints cc = new CellConstraints();
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

        JScrollPane scrollPane = new JScrollPane(table);
        builder.add(scrollPane, cc.xywh(builder.getColumn(), builder.getRow(), 3, 10));
        builder.nextRow(10);
        builder.nextLine();
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
    private java.util.Map<String, JTable> _tables = new java.util.HashMap<String, JTable>();
    private MetricsView _node;

    static class ColumnInfo
    {
        public ColumnInfo(String name, String displayName)
        {
            this.name = name;
            this.displayName = displayName;
        }

        public String name;
        public String displayName;
    }

    //
    // Map attributes to column names.
    //
    private static final Map<String, String> _columnNames;
    static
    {
        Map<String, String> m = new java.util.HashMap<String, String>();
        m.put("id", "Identity");
        m.put("current", "Current");
        m.put("total", "Total");
        m.put("retry", "Retry");
        m.put("remotes", "Remotes");
        m.put("inUseForIO", "IO");
        m.put("inUseForUser", "User");
        m.put("inUseForOther", "Other");
        m.put("remotes", "Remotes");
        m.put("validating", "Validating");
        m.put("holding", "Holding");
        m.put("active", "Active");
        m.put("closing", "Closing");
        m.put("closed", "Closed");
        m.put("receivedBytes", "RX Bytes");
        m.put("sentBytes", "TX Bytes");
        m.put("totalLifetime", "Total Time");
        m.put("failures", "Failures");
        _columnNames = java.util.Collections.unmodifiableMap(m);
    }

    //
    // Default column sort
    //
    private static final String[] _columnSort = {"id", "current", "total", "retry", "remotes", "inUseForIO", "inUseForUser", 
                                                 "inUseForOther", "remotes", "validating", "holding",
                                                 "active", "closing", "closed", "receivedBytes", "sentBytes",
                                                 "totalLifetime", "failures"};


    private static final Map<String, String> _sectionNames;
    static
    {
        Map<String, String> m = new java.util.HashMap<String, String>();
        m.put("Dispatch", "Operation Dispatch");
        m.put("Invocation", "Remote Invocations");
        m.put("Connection", "Connections");
        m.put("Thread", "Threads");
        m.put("ConnectionEstablishment", "Connection Establishments");
        m.put("EndpointLookup", "Endpoint Lookups");
        _sectionNames = java.util.Collections.unmodifiableMap(m);
    }

    //
    // Default section sort
    //
    private static final String[] _sectionSort = {"Dispatch", "Invocation", "Connection", "Thread", 
                                                 "ConnectionEstablishment", "EndpointLookup"};

    private static final Map<String, Map<String, Integer>> _columnPositions = new java.util.HashMap<String, Map<String, Integer>>();
}
