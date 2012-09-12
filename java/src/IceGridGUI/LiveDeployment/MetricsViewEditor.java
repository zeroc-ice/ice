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
import javax.swing.table.TableRowSorter;

import com.jgoodies.forms.builder.DefaultFormBuilder;
import com.jgoodies.forms.layout.CellConstraints;

import IceGrid.*;
import IceGridGUI.*;

public class MetricsViewEditor extends Editor
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
    public static class ButtonRenderer extends DefaultTableCellRenderer
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
    // This class allow to render a number with a format
    //
    public static class FormatedNumberRenderer extends DefaultTableCellRenderer
    {

        public FormatedNumberRenderer(String format)
        {
            _format = new DecimalFormat(format);
        }

        public Component
        getTableCellRendererComponent(JTable table, Object value, boolean isSelected, boolean hasFocus,
                                      int row, int column)
        {
            if(value != null)
            { 
                this.setText(_format.format(Double.parseDouble(value.toString()))); 
            } 
            else
            { 
                this.setText(""); 
            } 
            this.setHorizontalAlignment(RIGHT);  
            return this; 
        }
        
        private final DecimalFormat _format; 
    }

    //
    // Handle button clicks when buttons are embedded in a JTable cell.
    //
    public static class ButtonMouseListener extends MouseAdapter
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
        if(_properties == null)
        {
            _properties = Ice.Util.createProperties();
            _properties.load("resources/metrics.config");
        }
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

    public void show(MetricsView node)
    {
        _node = node;
        final java.util.Map<java.lang.String, IceMX.Metrics[]> data = node.data();

        boolean rebuildPanel = false;

        for(Map.Entry<String, IceMX.Metrics[]> entry : data.entrySet())
        {
            if(_tables.get(entry.getKey()) != null)
            {
		        continue;
	        }

            IceMX.Metrics[] objects = entry.getValue();
            if(objects == null || objects.length == 0)
            {
                continue;
            }

            TableModel model = new TableModel();
            String prefix = "IceGridGUI.Metrics." + entry.getKey();
            String[] names = _properties.getPropertyAsList(prefix + ".fields");
            for(String name : names)
            {
                setupModelField(node, model, prefix, entry.getKey(), name, objects);
            }
            if(model.getMetricFields().size() == 0)
            {
                continue;
            }

            JTable table = new JTable(model);

            table.setDragEnabled(true);
            table.setSelectionMode(ListSelectionModel.MULTIPLE_INTERVAL_SELECTION);
            table.setCellSelectionEnabled(true);
            table.addMouseListener(new ButtonMouseListener(table));
            table.setAutoCreateRowSorter(true);

            for(Map.Entry<Integer, MetricsField> fieldEntry : model.getMetricFields().entrySet())
            {
                if(fieldEntry.getValue().getCellRenderer() != null)
                {
                    table.getColumnModel().getColumn(fieldEntry.getKey().intValue()).setCellRenderer(
                                                                            fieldEntry.getValue().getCellRenderer());
                }
            }

            _tables.put(entry.getKey(), table);
            rebuildPanel = true;
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
            if(table == null)
            {
                continue;
            }
            TableModel model = (TableModel)table.getModel();

            model.getDataVector().removeAllElements();
            model.fireTableDataChanged();

            for(IceMX.Metrics m : values)
            {
                model.addMetrics(m);
            }
        }
    }

    private static void setupModelField(MetricsView node, TableModel model, String prefix, String mapName, String name, 
                                        IceMX.Metrics[] objects)
    {
        String className = _properties.getPropertyWithDefault(
                                                    prefix + "." + name + ".fieldClass",
                                                    "IceGridGUI.LiveDeployment.MetricsViewEditor$DeclaredMetricsField");
        Class<?> cls = IceInternal.Util.findClass(className, null);
        if(cls == null)
        {
            //XXX class not found
            return;
        }
        try
        {
            Field objectField = null;
            try
            {
                objectField = objects[0].getClass().getField(name);
            }
            catch(NoSuchFieldException ex)
            {
            }

            java.lang.reflect.Constructor<?> ctor = cls.getConstructor(MetricsView.class, String.class, 
                                                                       String.class, Field.class);

            MetricsField field = (MetricsField)ctor.newInstance(node, mapName, name, objectField);
            Map<String, String> properties = _properties.getPropertiesForPrefix(prefix + "." + name);
            for(Map.Entry<String, String> propEntry : properties.entrySet())
            {
                if(propEntry.getKey().equals(prefix + "." + name + ".fieldClass"))
                {
                    continue;
                }

                if(!propEntry.getKey().substring(0, propEntry.getKey().lastIndexOf(".")).equals(prefix + "." + name))
                {
                    //
                    // Sub metric property.
                    //
                    continue;
                }

                String setterName = propEntry.getKey().substring(propEntry.getKey().lastIndexOf(".") + 1);
                setterName = "set" + Character.toUpperCase(setterName.charAt(0)) + setterName.substring(1);
                
                try
                {
                    java.lang.reflect.Method setter = cls.getMethod(setterName, new Class[]{String.class});
                    setter.invoke(field, new Object[]{propEntry.getValue()});
                }
                catch(NoSuchMethodException ex)
                {
                    continue;
                }
                catch(java.lang.reflect.InvocationTargetException ex)
                {
                    ex.printStackTrace();
                }
            }
            model.addField(field);
        }
        catch(NoSuchMethodException  ex)
        {
            ex.printStackTrace();
        }
        catch(InstantiationException ex)
        {
            ex.printStackTrace();
        }
        catch(java.lang.reflect.InvocationTargetException ex)
        {
            ex.printStackTrace();
        }
        catch(IllegalAccessException ex)
        {
            ex.printStackTrace();
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
    
    public static class TableModel extends DefaultTableModel
    {

        public void addMetrics(IceMX.Metrics m)
        {
            Object[] row = new Object[_fields.size()];

            for(Map.Entry<Integer, MetricsField> entry : _fields.entrySet())
            {
               row[entry.getKey().intValue()] = entry.getValue().getValue(m);
            }
            addRow(row);
        }

        public void addField(MetricsField field)
        {
            addColumn(field.getColumnName());
            _fields.put(_fields.size(), field);
        }

        public boolean isCellEditable(int row, int column)
        {
            return false;
        }

	    public Class getColumnClass(int index)
	    {
	        return _fields.get(index).getColumnClass();
	    }

        public java.util.Map<Integer, MetricsField> getMetricFields()
        {
            return _fields;
        }

        java.util.Map<Integer, MetricsField> _fields = new java.util.HashMap<Integer, MetricsField>();
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

    public interface MetricsField
    {
	    public String getColumnName();
	    public Class getColumnClass();
        public TableCellRenderer getCellRenderer();
	    public Object getValue(IceMX.Metrics m);
    }

    static public class DeclaredMetricsField implements MetricsField
    {
        public DeclaredMetricsField(MetricsView node, String mapName, String fieldName, Field field)
        {
            _node = node;
            _mapName = mapName;
            _fieldName = fieldName;
            if(field == null)
            {
                throw new IllegalArgumentException("Field argument must be non null, " +
                                                   "Metrics object: `" + mapName + "' field: `" + fieldName + "'");
            }
            Class columnClass = field.getType();
            if(columnClass.equals(int.class))
            {
                _columnClass = Integer.class;
            }
            else if(columnClass.equals(long.class))
            {
                _columnClass = Long.class;
            }
            else if(columnClass.equals(float.class))
            {
                _columnClass = Float.class;
                setFormat("#0.000"); // Set the default format
            }
            else if(columnClass.equals(double.class))
            {
                _columnClass = Double.class;
                setFormat("#0.000"); // Set the default format
            }
            else
            {
                _columnClass = columnClass;
            }
        }

        public String getFieldName()
        {
            return _fieldName;
        }

        public void setColumnName(String columnName)
        {
            _columnName = columnName;
        }

        public String getColumnName()
        {
            return _columnName == null ? _fieldName : _columnName;
        }

        public void setColumnClass(String columnClass)
        {
            _columnClass = IceInternal.Util.findClass(columnClass, null);
        }

        public Class getColumnClass()
        {
            return _columnClass;
        }

        public void setFormat(String format)
        {
            _cellRenderer = new FormatedNumberRenderer(format);
        }

        public TableCellRenderer getCellRenderer()
        {
            return _cellRenderer;
        }

        public Object getValue(IceMX.Metrics m)
        {
            try
            {
                return m.getClass().getField(_fieldName).get(m);
            }
            catch(NoSuchFieldException ex)
            {
                return null;
            }
            catch(IllegalAccessException ex)
            {
                return null;
            }
        }

        private final MetricsView _node;
        private final String _mapName;
        private final String _fieldName;
        private String _columnName;
        private Class _columnClass;
        private TableCellRenderer _cellRenderer;
    }

    static public class AverageLifetimeMetricsField implements MetricsField
    {
        public AverageLifetimeMetricsField(MetricsView node, String mapName, String fieldName, Field field)
        {
            _node = node;
            _mapName = mapName;
            _fieldName = fieldName;
            setFormat("#0.000"); // Set the default format
        }
        
        public void setColumnName(String columnName)
        {
            _columnName = columnName;
        }

        public void setFormat(String format)
        {
            _cellRenderer = new FormatedNumberRenderer(format);
        }

	    public String getColumnName()
	    {
	        return _columnName;
	    }

	    public Class getColumnClass()
	    {
	        return Float.class;
	    }

        public TableCellRenderer getCellRenderer()
        {
            return _cellRenderer;
        }

        public void setDivisor(String divisor) throws java.lang.NumberFormatException
        {
            _divisor = Float.parseFloat(divisor);
        }

        public Object getValue(IceMX.Metrics m)
        {
            if(m.totalLifetime == 0 || m.total - m.current == 0)
            {
                return 0.0f;
            }
            else
            {
                return (float)(m.totalLifetime / _divisor / (m.total - m.current));
            }
        }

        private final MetricsView _node;
        private final String _mapName;
        private final String _fieldName;
	    private float _divisor;
	    private String _columnName;
        private TableCellRenderer _cellRenderer;
    }

    static public class BandwidthDelta
    {
        public long bytes;
        public long timestamp;
    }

    static public class BandwidthMetricsField implements MetricsField
    {
        public BandwidthMetricsField(MetricsView node, String mapName, String fieldName, Field field)
	    {
            _node = node;
            _mapName = mapName;
            _fieldName = fieldName;
            setFormat("#0.000"); // Set the default format
	    }

        public void setColumnName(String columnName)
        {
            _columnName = columnName;
        }

        public String getColumnName()
        {
            return _columnName;
        }

        public void setFormat(String format)
        {
            _cellRenderer = new FormatedNumberRenderer(format);
        }

        public Class getColumnClass()
        {
            return Float.class;
        }

        public TableCellRenderer getCellRenderer()
        {
            return _cellRenderer;
        }

        public void setBytesField(String bytesField)
        {
            _bytesField = bytesField;
        }

        public Object getValue(IceMX.Metrics m)
        {
            BandwidthDelta d1 = _deltas.get(m.id);
            BandwidthDelta d2 = new BandwidthDelta();
            try
            {
                d2.bytes = (Long)m.getClass().getField(_bytesField).get(m);
            }
            catch(NoSuchFieldException ex)
            {
                return null;
            }
            catch(IllegalAccessException ex)
            {
                return null;
            }
            d2.timestamp = System.currentTimeMillis();


            _deltas.put(m.id, d2);
            if(d1 == null || d2.bytes - d1.bytes == 0 || d2.timestamp - d1.timestamp == 0)
            {
                return 0.0f;
            }
            else
            {
                return (float)((d2.bytes - d1.bytes) / (float)(d2.timestamp - d1.timestamp) * 1000.0f);
            }
        }

        private final MetricsView _node;
        private final String _mapName;
        private final String _fieldName;
        private String _columnName;
        private String _bytesField;
        private final Map<String, BandwidthDelta> _deltas = new java.util.HashMap<String, BandwidthDelta>();
        private TableCellRenderer _cellRenderer;
    }

    static public class FailuresMetricsField implements MetricsField
    {
        public FailuresMetricsField(MetricsView node, String mapName, String fieldName, Field field)
        {
            _node = node;
            _mapName = mapName;
            _fieldName = fieldName;
        }

        public void setColumnName(String columnName)
        {
            _columnName = columnName;
        }

        public String getColumnName()
        {
            return _columnName;
        }

        public Class getColumnClass()
        {
            return JButton.class;
        }

        public TableCellRenderer getCellRenderer()
        {
            return _cellRenderer;
        }

	    public Object getValue(final IceMX.Metrics m)
	    {
            JButton button = new JButton("Show Failures (" + Integer.toString(m.failures) + ")");
            if(m.failures > 0)
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
                                                            row[2] = m.id;
                                                            model.addRow(row);
                                                        }
                                                        _node.getCoordinator().getMainFrame().setCursor(
                                                                    Cursor.getPredefinedCursor(Cursor.DEFAULT_CURSOR));
                                                    }
                                                });
                                            }

                                        public void exception(final Ice.LocalException e)
                                        {
                                            SwingUtilities.invokeLater(new Runnable()
                                                {
                                                    public void run()
                                                    {
                                                        _node.getCoordinator().getMainFrame().setCursor(
                                                                    Cursor.getPredefinedCursor(Cursor.DEFAULT_CURSOR));
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
                                                            JOptionPane.showMessageDialog(
                                                                                _node.getCoordinator().getMainFrame(),
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
                                                        _node.getCoordinator().getMainFrame().setCursor(
                                                                    Cursor.getPredefinedCursor(Cursor.DEFAULT_CURSOR));
                                                        e.printStackTrace();
                                                        JOptionPane.showMessageDialog(
                                                                            _node.getCoordinator().getMainFrame(),
                                                                            "Error: " + e.toString(), "Error",
                                                                            JOptionPane.ERROR_MESSAGE);
                                                    }
                                                });
                                        }
                                    };

                            _node.getCoordinator().getMainFrame().setCursor(
                                                                        Cursor.getPredefinedCursor(Cursor.WAIT_CURSOR));
                            _node.fetchMetricsFailures(_mapName, m.id, cb);

                            JOptionPane.showMessageDialog(_node.getCoordinator().getMainFrame(), scrollPane, 
                                                          "Metrics Failures", JOptionPane.PLAIN_MESSAGE );
                            _node.getCoordinator().getMainFrame().setCursor(
                                                                    Cursor.getPredefinedCursor(Cursor.DEFAULT_CURSOR));
                        }
                    });
            }
            else
            {
                button.setEnabled(false);
            }
            return button;
	    }

        private final MetricsView _node;
        private final String _mapName;
        private final String _fieldName;
        private static final TableCellRenderer _cellRenderer = new ButtonRenderer();
        private String _columnName;
    }

    static public class SubMetricsField implements MetricsField
    {
        public SubMetricsField(MetricsView node, String mapName, String fieldName, Field field)
        {
            _node = node;
            _mapName = mapName;
            _fieldName = fieldName;
        }

        public void setColumnName(String columnName)
        {
            _columnName = columnName;
        }

        public String getColumnName()
        {
            return _columnName == null ? _fieldName : _columnName;
        }

        public Class getColumnClass()
        {
            return JButton.class;
        }

        public TableCellRenderer getCellRenderer()
        {
            return _cellRenderer;
        }

        public Object getValue(final IceMX.Metrics m)
        {
            try
            {
                final IceMX.Metrics[] objects = (IceMX.Metrics[])m.getClass().getField(_fieldName).get(m);
                JButton button = new JButton("Show " + getColumnName() + " (" + Integer.toString(objects.length) + ")");
                button.setEnabled(objects.length > 0);
                if(objects.length > 0)
                {
                    button.addActionListener(new ActionListener()
                        {
                            public void actionPerformed(ActionEvent event)
                            {
                                final TableModel model = new TableModel();
                                String prefix = "IceGridGUI.Metrics." + _mapName + "." + _fieldName;
                                String[] names = _properties.getPropertyAsList(prefix + ".fields");
                                for(String name : names)
                                {
                                    setupModelField(_node, model, prefix, _fieldName, name, objects);
                                }
                                if(model.getMetricFields().size() == 0)
                                {
                                    return;
                                }

                                JTable table = new JTable(model);
                                table.addMouseListener(new ButtonMouseListener(table));
                                table.setAutoCreateRowSorter(true);

                                for(Map.Entry<Integer, MetricsField> fieldEntry : model.getMetricFields().entrySet())
                                {
                                    if(fieldEntry.getValue().getCellRenderer() != null)
                                    {
                                        table.getColumnModel().getColumn(fieldEntry.getKey().intValue()).
                                                            setCellRenderer(fieldEntry.getValue().getCellRenderer());
                                    }
                                }

                                for(IceMX.Metrics m : objects)
                                {
                                    model.addMetrics(m);
                                }

                                JScrollPane scrollPane = new JScrollPane(table);
                                JOptionPane.showMessageDialog(_node.getCoordinator().getMainFrame(), scrollPane,
                                                              getColumnName(), JOptionPane.PLAIN_MESSAGE);

                            }
                        });
                }
                return button;
            }
            catch(NoSuchFieldException ex)
            {
                return null;
            }
            catch(IllegalAccessException ex)
            {
                return null;
            }
        }

        private final MetricsView _node;
        private final String _mapName;
        private final String _fieldName;
        private static final TableCellRenderer _cellRenderer = new ButtonRenderer();
        private String _columnName;
    }

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

    private static Ice.Properties _properties;
}
