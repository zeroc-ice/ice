// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceGridGUI.LiveDeployment;

import java.lang.reflect.Field;

import java.awt.datatransfer.DataFlavor;
import java.awt.datatransfer.UnsupportedFlavorException;

import java.awt.Component;
import java.awt.Cursor;
import java.awt.Dimension;
import java.awt.event.ActionEvent;
import java.awt.event.MouseEvent;
import java.awt.event.MouseAdapter;
import java.awt.event.ActionListener;
import java.awt.BorderLayout;

import java.util.List;
import java.util.ArrayList;
import java.util.Map;
import java.util.HashMap;
import java.util.Set;
import java.util.LinkedHashSet;
import java.util.prefs.Preferences;

import java.text.DecimalFormat;

import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;

import javax.swing.JButton;
import javax.swing.JOptionPane;
import javax.swing.BorderFactory;

import javax.swing.border.TitledBorder;

import javax.swing.event.TreeSelectionEvent;
import javax.swing.event.TreeSelectionListener;

import javax.swing.tree.TreePath;
import javax.swing.ListSelectionModel;
import javax.swing.JComponent;
import javax.swing.JScrollPane;
import javax.swing.JSplitPane;
import javax.swing.JTable;
import javax.swing.SwingUtilities;
import javax.swing.UIManager;
import javax.swing.JTree;
import javax.swing.JMenu;
import javax.swing.JMenuItem;
import javax.swing.JPopupMenu;
import javax.swing.JPanel;

import javax.swing.table.DefaultTableCellRenderer;
import javax.swing.table.DefaultTableModel;
import javax.swing.table.TableCellRenderer;
import javax.swing.table.JTableHeader;

import com.zeroc.IceGridGUI.*;

public class MetricsViewEditor extends Editor implements MetricsFieldContext
{
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

        @Override
        public Component
        getTableCellRendererComponent(JTable table, Object value, boolean isSelected, boolean hasFocus,
                                      int row, int column)
        {
            if(value != null)
            {
                setText(_format.format(Double.parseDouble(value.toString())));
            }
            else
            {
                setText("");
            }
            this.setHorizontalAlignment(RIGHT);

            if(isSelected)
            {
                setForeground(table.getSelectionForeground());
                setBackground(table.getSelectionBackground());
            }
            else
            {
                setForeground(table.getForeground());
                setBackground(table.getBackground());
            }
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

    private static class SelectionListener implements TreeSelectionListener
    {
        @Override
        public void valueChanged(TreeSelectionEvent e)
        {
            //
            // Stop the refresh thread.
            //
            MetricsViewEditor.stopRefresh();

            //
            // If selected node is a MetricsView and it is enabled; start the refresh thread.
            //
            if(e.isAddedPath() && e.getPath().getLastPathComponent() instanceof MetricsView &&
               ((MetricsView )e.getPath().getLastPathComponent()).isEnabled())
            {
                MetricsViewEditor.startRefresh((MetricsView)e.getPath().getLastPathComponent());
            }

            if(e.isAddedPath())
            {
                MetricsViewEditor.setSelectedPath(e.getPath());
            }
        }
    }

    MetricsViewEditor(Root root)
    {
        _prefs = Coordinator.getPreferences().node("MetricsView");

        if(_properties == null)
        {
            Coordinator coord = root.getCoordinator();
            JTree tree = root.getTree();
            tree.addTreeSelectionListener(new SelectionListener());

            Set<String> sectionSort = new LinkedHashSet<>();
            _properties = com.zeroc.Ice.Util.createProperties();

            _properties.load("metrics.cfg");
            sectionSort.addAll(java.util.Arrays.asList(_properties.getPropertyAsList("IceGridGUI.Metrics")));

            String metricsDefs = coord.getProperties().getProperty("IceGridAdmin.MetricsConfigs");
            if(!metricsDefs.isEmpty())
            {
                for(String s : metricsDefs.split(","))
                {
                    try
                    {
                        _properties.load(s.trim());
                    }
                    catch(com.zeroc.Ice.FileException ex)
                    {
                        coord.getCommunicator().getLogger().warning("unable to load `" + ex.path + "'");
                    }
                    sectionSort.addAll(java.util.Arrays.asList(_properties.getPropertyAsList("IceGridGUI.Metrics")));
                }
            }

            for(String name : sectionSort)
            {
                String displayName = _properties.getPropertyWithDefault("IceGridGUI.Metrics." + name, "");
                if(!displayName.equals(""))
                {
                    _sectionNames.put(name, displayName);
                }
            }

            _sectionSort = sectionSort.toArray(new String[sectionSort.size()]);
        }
    }

    @Override
    public int getRefreshPeriod()
    {
        return _refreshPeriod;
    }

    synchronized static void startRefresh(final MetricsView node)
    {
        assert(_refreshFuture == null);
        _refreshFuture = node.getCoordinator().getScheduledExecutor().scheduleAtFixedRate(() ->
            {
                node.fetchMetricsView();
            }, 0, _refreshPeriod, java.util.concurrent.TimeUnit.SECONDS);
    }

    synchronized static void stopRefresh()
    {
        if(_refreshFuture != null)
        {
            _refreshFuture.cancel(false);
            _refreshFuture = null;
        }
    }

    static void setSelectedPath(TreePath path)
    {
        _selectedPath = path;
    }

    public static class MetricsViewInfo
    {
        public MetricsViewInfo(MetricsView view)
        {
            java.util.List<String> fullId = ((Communicator)view.getParent()).getFullId();

            assert fullId.size() > 0;

            if(fullId.size() == 1)
            {
                component = "/";
            }
            else
            {
                StringBuilder builder = null;
                for(String s : fullId)
                {
                    if(builder == null)
                    {
                        builder = new StringBuilder();
                        // Skip first element
                    }
                    else
                    {
                        builder.append('/');
                        builder.append(s);
                    }
                }

                component = builder.toString();
            }

            this.view = view.getId();
            admin = view.getMetricsAdmin();
        }

        @Override
        public boolean equals(Object other)
        {
            if(other == null)
            {
                return false;
            }
            else if(other == this)
            {
                return true;
            }
            else if(!(other instanceof MetricsViewInfo))
            {
                return false;
            }
            MetricsViewInfo that = (MetricsViewInfo)other;
            return this.component.equals(that.component) && this.view.equals(that.view);
        }

        @Override
        public String toString()
        {
            StringBuilder builder = new StringBuilder();
            builder.append(component);
            builder.append("/");
            builder.append(view);
            return builder.toString();
        }

        @Override
        public int hashCode()
        {
            int h = com.zeroc.IceInternal.HashUtil.hashAdd(5381, component);
            return com.zeroc.IceInternal.HashUtil.hashAdd(h, view);
        }

        public String component;
        public String view;
        public com.zeroc.IceMX.MetricsAdminPrx admin;
    }

    public static class MetricsCell
    {
        public MetricsCell(String id, MetricsField field)
        {
            _id = id;
            _field = field;
        }

        public String getId()
        {
            return _id;
        }

        public MetricsField getField()
        {
            return _field;
        }

        @Override
        public boolean equals(Object other)
        {
            if(other == null)
            {
                return false;
            }
            else if(other == this)
            {
                return true;
            }
            else if(!(other instanceof MetricsCell))
            {
                return false;
            }
            MetricsCell that = (MetricsCell)other;
            return this._id.equals(that._id) && this._field.getFieldName().equals(that._field.getFieldName());
        }

        @Override
        public int hashCode()
        {
            int h = com.zeroc.IceInternal.HashUtil.hashAdd(5381, _id);
            return com.zeroc.IceInternal.HashUtil.hashAdd(h, _field.getFieldName());
        }

        public double getScaleFactor()
        {
            return _scaleFactor;
        }

        public void setScaleFactor(double scaleFactor)
        {
            _scaleFactor = scaleFactor;
        }

        public Number getValue(com.zeroc.IceMX.Metrics m, long timestamp)
        {
            Number value = ((Number)getField().getValue(m, timestamp));
            if(value == null)
            {
                return value;
            }
            else
            {
                _samples++;
                _last = value.doubleValue();

                _average = _average + (_last - _average) / _samples;
                if(_last < _min || _samples == 1)
                {
                    _min = _last;
                }
                if(_last > _max || _samples == 1)
                {
                    _max = _last;
                }

                return _last * _scaleFactor;
            }
        }

        public Number getLast()
        {
            return _last;
        }

        public Number getAverage()
        {
            return _average;
        }

        public Number getMin()
        {
            return _min;
        }

        public Number getMax()
        {
            return _max;
        }

        public void resetField()
        {
            _field = _field.createField();
        }

        private String _id;
        private MetricsField _field;
        private double _scaleFactor = 1.0d;
        private double _last;
        private double _average = 0;
        private int _samples = 0;
        private double _min;
        private double _max;
    }

    class Transferable implements java.awt.datatransfer.Transferable
    {
        public Transferable(MetricsViewTransferableData data)
        {
            _data = data;
            try
            {
                _flavors = new DataFlavor[]{(DataFlavor)MetricsViewTransferableData.dataFlavor().clone()};
            }
            catch(CloneNotSupportedException ex)
            {
            }
        }

        @Override
        public Object getTransferData(DataFlavor flavor) throws UnsupportedFlavorException
        {
            if(!isDataFlavorSupported(flavor))
            {
                throw new UnsupportedFlavorException(flavor);
            }
            return _data;
        }

        @Override
        public DataFlavor[] getTransferDataFlavors()
        {
            return _flavors;
        }

        @Override
        public boolean isDataFlavorSupported(DataFlavor flavor)
        {
            return _flavors[0].equals(flavor);
        }

        private DataFlavor[] _flavors;
        private MetricsViewTransferableData _data;
    }

    //
    // Class used to transfer selected metric fields with drag & drop.
    //
    public static class MetricsViewTransferableData
    {
        public MetricsViewTransferableData(MetricsViewInfo info, String name, Map<String, List<MetricsCell>> rows)
        {
            this.info = info;
            this.name = name;
            this.rows = rows;
        }

        public static DataFlavor dataFlavor()
        {
            if(_flavor == null)
            {
                try
                {
                    _flavor = new DataFlavor(DataFlavor.javaJVMLocalObjectMimeType +
                                             ";class=\"" + MetricsViewTransferableData.class.getName() + "\"");
                }
                catch(ClassNotFoundException ex)
                {
                    // Cannot happen
                    ex.printStackTrace();
                }
            }
            return _flavor;
        }

        public final MetricsViewInfo info;
        public final String name;
        public final Map<String, List<MetricsCell>> rows;
        private static DataFlavor _flavor;
    }

    public Map<String, List<MetricsCell>> getSelectedRows(JTable table, boolean numeric)
    {
        int[] selectedRows = table.getSelectedRows();
        int[] selectedColumns = table.getSelectedColumns();
        Map<String, List<MetricsCell>> rows = new HashMap<>();

        if(selectedRows.length > 0 && selectedColumns.length > 0)
        {
            TableModel model = (TableModel)table.getModel();

            int idColumn = table.getColumnModel().getColumnIndex(_properties.getProperty(
                                                    "IceGridGUI.Metrics." + model.getMetricsName() + ".id.columnName"));

            for(int row : selectedRows)
            {
                List<MetricsCell> cells = new ArrayList<>();
                String id = model.getValueAt(table.convertRowIndexToModel(row), idColumn).toString();
                for(int col : selectedColumns)
                {
                    MetricsField field = model.getMetricFields().get(table.convertColumnIndexToModel(col));
                    Class columnClass = field.getColumnClass();

                    if(!numeric)
                    {
                        cells.add(new MetricsCell(id, field.createField()));
                    }
                    else if(columnClass.equals(int.class) || columnClass.equals(Integer.class) ||
                       columnClass.equals(long.class) || columnClass.equals(Long.class) ||
                       columnClass.equals(float.class) || columnClass.equals(Float.class) ||
                       columnClass.equals(double.class) || columnClass.equals(Double.class))
                    {
                        cells.add(new MetricsCell(id, field.createField()));
                    }
                }
                if(cells.size() > 0)
                {
                    rows.put(id, cells);
                }
            }
        }
        return rows;
    }

    class TransferHandler extends javax.swing.TransferHandler
    {
        public TransferHandler(MetricsView node)
        {
            _node = node;
        }

        @Override
        public int
        getSourceActions(JComponent component)
        {
            return javax.swing.TransferHandler.COPY;
        }

        @Override
        public Transferable
        createTransferable(JComponent component)
        {
            JTable table = (JTable)component;
            TableModel model = (TableModel)table.getModel();
            Map<String, List<MetricsCell>> rows = getSelectedRows(table, true);

            if(rows.size() > 0)
            {
                return new Transferable(new MetricsViewTransferableData(
                                                            new MetricsViewInfo(_node), model.getMetricsName(), rows));
            }
            else
            {
                return null;
            }
        }

        protected void exportDone(JComponent source, Transferable data, int action)
        {
            // Does nothing as we don't support move action.
        }

        private MetricsView _node;
    }

    public void show(final MetricsView node, final Map<java.lang.String, com.zeroc.IceMX.Metrics[]> data,
                     final long timestamp)
    {
        boolean rebuildPanel = false;
        if(!node.isEnabled())
        {
            _tables.clear();
            rebuildPanel = true;
        }
        else if(data != null)
        {
            for(final Map.Entry<String, com.zeroc.IceMX.Metrics[]> entry : data.entrySet())
            {
                if(_tables.get(entry.getKey()) != null)
                {
                    continue;
                }

                com.zeroc.IceMX.Metrics[] objects = entry.getValue();
                if(objects == null || objects.length == 0)
                {
                    continue;
                }
                final TableModel model = new TableModel(entry.getKey());
                String prefix = "IceGridGUI.Metrics." + entry.getKey();
                String[] names = _properties.getPropertyAsList(prefix + ".fields");
                for(String name : names)
                {
                    Field objectField = null;
                    try
                    {
                        objectField = objects[0].getClass().getField(name);
                    }
                    catch(NoSuchFieldException ex)
                    {
                    }
                    MetricsField field =
                        createField(node, prefix + "." + name, entry.getKey(), name, objectField, this);
                    if(field != null)
                    {
                        model.addField(field);
                    }
                }
                if(model.getMetricFields().size() == 0)
                {
                    continue;
                }

                final JTable table = new JTable(model)
                    {
                        //
                        //Implement table header tool tips.
                        //
                        @Override
                        protected JTableHeader createDefaultTableHeader()
                        {
                            return new JTableHeader(columnModel)
                            {
                                @Override
                                public String getToolTipText(MouseEvent e)
                                {
                                    int index = columnModel.getColumn(
                                        columnModel.getColumnIndexAtX(e.getPoint().x)).getModelIndex();
                                    return model.getMetricFields().get(index).getColumnToolTip();
                                }
                            };
                        }
                    };

                //
                // Adjust row height for larger fonts
                //
                int fontSize = table.getFont().getSize();
                int minRowHeight = fontSize + fontSize / 3;
                if(table.getRowHeight() < minRowHeight)
                {
                    table.setRowHeight(minRowHeight);
                }

                table.setDragEnabled(true);
                table.setSelectionMode(ListSelectionModel.MULTIPLE_INTERVAL_SELECTION);
                table.setCellSelectionEnabled(true);
                table.addMouseListener(new ButtonMouseListener(table));
                table.setAutoCreateRowSorter(true);
                table.setTransferHandler(new TransferHandler(node));
                table.addMouseListener(new MouseAdapter()
                    {
                        @Override
                        public void mousePressed(MouseEvent e)
                        {
                            createAndShowMenu(e);
                        }

                        @Override
                        public void mouseReleased(MouseEvent e)
                        {
                            createAndShowMenu(e);
                        }

                        public void createAndShowMenu(MouseEvent e)
                        {
                            if(e.isPopupTrigger())
                            {
                                JPopupMenu popup = new JPopupMenu();
                                JMenu addToGraph = new JMenu("Add To Metrics Graph");
                                popup.add(addToGraph);
                                final Map<String, List<MetricsCell>> rows = getSelectedRows(table, true);
                                addToGraph.setEnabled(rows.size() > 0);
                                JMenuItem newGraph = new JMenuItem("New Metrics Graph");
                                newGraph.addActionListener(new ActionListener()
                                    {
                                        @Override
                                        public void actionPerformed(ActionEvent e)
                                        {
                                            Coordinator.IGraphView view = node.getCoordinator().createGraphView();
                                            if(view != null)
                                            {
                                                view.addSeries(
                                                    new MetricsViewTransferableData(new MetricsViewInfo(node),
                                                    entry.getKey(), rows));
                                            }
                                        }
                                    });
                                addToGraph.add(newGraph);

                                Coordinator.IGraphView[] graphs = node.getCoordinator().getGraphViews();
                                for(final Coordinator.IGraphView view : graphs)
                                {
                                    JMenuItem item = new JMenuItem(view.getTitle());
                                    addToGraph.add(item);
                                    item.addActionListener(new ActionListener()
                                        {
                                            @Override
                                            public void actionPerformed(ActionEvent e)
                                            {
                                                view.addSeries(
                                                    new MetricsViewTransferableData(new MetricsViewInfo(node),
                                                    entry.getKey(), rows));
                                            }
                                        });
                                }
                                popup.show(e.getComponent(), e.getX(), e.getY());
                            }
                        }
                    });

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
        }

        if(rebuildPanel)
        {
            buildPropertiesPanel();
        }

        if(data != null)
        {
            //
            // Load the data.
            //
            for(Map.Entry<String, com.zeroc.IceMX.Metrics[]> entry : data.entrySet())
            {
                String key = entry.getKey();
                com.zeroc.IceMX.Metrics[] values = entry.getValue();
                JTable table = _tables.get(key);
                if(table == null)
                {
                    continue;
                }
                TableModel model = (TableModel)table.getModel();

                Map<String, List<MetricsCell>> rows = getSelectedRows(table, false);

                model.getDataVector().removeAllElements();
                model.fireTableDataChanged();
                for(com.zeroc.IceMX.Metrics m : values)
                {
                    model.addMetrics(m, timestamp);
                }

                int idColumn = table.getColumnModel().getColumnIndex(_properties.getProperty(
                    "IceGridGUI.Metrics." + key + ".id.columnName"));
                if(rows.size() > 0)
                {
                    for(int i = table.getRowCount() - 1; i >= 0; --i)
                    {
                        String id = (String)table.getValueAt(i, idColumn);
                        List<MetricsCell> columns = rows.get(id);
                        if(columns != null)
                        {
                            for(MetricsCell cell : columns)
                            {
                                int j = table.getColumnModel().getColumnIndex(cell.getField().getColumnName());

                                table.getSelectionModel().addSelectionInterval(i, i);
                                table.getColumnModel().getSelectionModel().addSelectionInterval(j, j);
                            }
                        }
                    }
                }
            }
        }
    }

    private static MetricsField createField(MetricsView node, String prefix, String mapName, String name,
                                            Field objectField, MetricsFieldContext context)
    {
        String className = _properties.getPropertyWithDefault(
            prefix + ".fieldClass", "com.zeroc.IceGridGUI.LiveDeployment.MetricsViewEditor$DeclaredMetricsField");

        if(!className.startsWith("com.zeroc."))
        {
            className = "com.zeroc." + className;
        }

        Class<?> cls = com.zeroc.IceInternal.Util.findClass(className, null);
        if(cls == null)
        {
            System.err.println("Could not find class " + className);
            return null;
        }
        try
        {
            java.lang.reflect.Constructor<?> ctor = cls.getDeclaredConstructor(MetricsView.class, String.class,
                                                                               String.class, String.class, Field.class);
            MetricsField field = (MetricsField)ctor.newInstance(node, prefix, mapName, name, objectField);
            field.setContext(context);
            Map<String, String> properties = _properties.getPropertiesForPrefix(prefix);
            for(Map.Entry<String, String> propEntry : properties.entrySet())
            {
                if(propEntry.getKey().equals(prefix + "." + name + ".fieldClass"))
                {
                    continue;
                }

                if(!propEntry.getKey().substring(0, propEntry.getKey().lastIndexOf(".")).equals(prefix))
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
            return field;
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
        return null;
    }

    @Override
    protected JComponent createPropertiesPanel()
    {
        JSplitPane current = null;
        JSplitPane top = null;
        Map<String, JTable> tables = new HashMap<>(_tables);

        StringBuilder sb = new StringBuilder();
        Object[] elements = _selectedPath.getPath();
        for(Object element : elements)
        {
            sb.append(element.toString());
            sb.append(".");
        }

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
            current = createScrollTable(current, sb.toString() + name, section, table);
            if(top == null)
            {
                top = current;
            }
        }
        for(Map.Entry<String, JTable> entry : tables.entrySet())
        {
            current = createScrollTable(current, sb.toString() + entry.getKey(), entry.getKey(), entry.getValue());
            if(top == null)
            {
                top = current;
            }
        }
        if(current != null)
        {
            current.setBottomComponent(new JPanel());
        }
        return top;
    }

    private JSplitPane createScrollTable(JSplitPane currentPane,final String key, String title, JTable table)
    {
        JPanel panel = new JPanel();
        TitledBorder border = BorderFactory.createTitledBorder(BorderFactory.createEmptyBorder(),
                                                               title, TitledBorder.LEFT, TitledBorder.CENTER);
        panel.setBorder(border);
        panel.setLayout(new BorderLayout(0, 0));
        panel.add(new JScrollPane(table),  BorderLayout.CENTER);

        JSplitPane splitPane = new JSplitPane(JSplitPane.VERTICAL_SPLIT);
        splitPane.setTopComponent(panel);
        if(currentPane != null)
        {
            currentPane.setBottomComponent(splitPane);
            currentPane.setBorder(BorderFactory.createEmptyBorder(1, 1, 1, 1));
        }
        splitPane.setDividerLocation(_prefs.getInt(key, 120));
        splitPane.addPropertyChangeListener(JSplitPane.DIVIDER_LOCATION_PROPERTY,
                                            new PropertyChangeListener()
                                                {
                                                    @Override
                                                    public void propertyChange(PropertyChangeEvent e)
                                                    {
                                                        _prefs.putInt(key, Integer.valueOf((Integer)e.getNewValue()));
                                                        try
                                                        {
                                                            _prefs.flush();
                                                        }
                                                        catch(java.util.prefs.BackingStoreException ex)
                                                        {
                                                            JOptionPane.showMessageDialog(null,
                                                                                          ex.toString(),
                                                                                          "Error saving preferences",
                                                                                          JOptionPane.ERROR_MESSAGE);
                                                        }
                                                    }
                                                });
        return splitPane;
    }

    @Override
    protected void buildPropertiesPanel()
    {
        super.buildPropertiesPanel();
        _propertiesPanel.setName("Metrics Report");
    }

    public static class TableModel extends DefaultTableModel
    {
        public TableModel(String metricsName)
        {
            _metricsName = metricsName;
        }

        public void addMetrics(com.zeroc.IceMX.Metrics m, long timestamp)
        {
            Object[] row = new Object[_fields.size()];

            for(Map.Entry<Integer, MetricsField> entry : _fields.entrySet())
            {
                Object value = entry.getValue().getValue(m, timestamp);
                if(value == null)
                {
                    Class c = getColumnClass(entry.getKey().intValue());
                    if(c.equals(Integer.class))
                    {
                        value = new Integer(0);
                    }
                    else if(c.equals(Long.class))
                    {
                        value = new Long(0);
                    }
                    else if(c.equals(Float.class))
                    {
                        value = new Float(0.0f);
                    }
                    else if(c.equals(Double.class))
                    {
                        value = new Double(0.0d);
                    }
                }
                row[entry.getKey().intValue()] = value;
            }
            addRow(row);
        }

        public void addField(MetricsField field)
        {
            addColumn(field.getColumnName());
            _fields.put(_fields.size(), field);
        }

        @Override
        public boolean isCellEditable(int row, int column)
        {
            return false;
        }

        @Override
        public Class getColumnClass(int index)
        {
            return _fields.get(index).getColumnClass();
        }

        public Map<Integer, MetricsField> getMetricFields()
        {
            return _fields;
        }

        public String getMetricsName()
        {
            return _metricsName;
        }

        String _metricsName;
        Map<Integer, MetricsField> _fields = new HashMap<>();
    }

    private static java.util.concurrent.Future<?> _refreshFuture;
    private Map<String, JTable> _tables = new HashMap<>();

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
        //
        // The metrics view where the field was added.
        //
        public MetricsView getMetricsNode();

        //
        // Return the prefix of properties used to configure the field.
        //
        public String getPropertyPrefix();

        //
        // Name of the metrics object.
        //
        public String getMetricsName();

        //
        // Name to identify the field.
        //
        public String getFieldName();

        //
        // Name for display.
        //
        public String getColumnName();

        //
        // ToolTip
        //
        public String getColumnToolTip();

        //
        // The Java class correspoding to the field, is used in the table models.
        //
        public Class getColumnClass();

        //
        // Renderer used by JTable to render the field.
        //
        public TableCellRenderer getCellRenderer();

        //
        // Return the value of the field for the given metrics object.
        //
        public Object getValue(com.zeroc.IceMX.Metrics m, long timestamp);

        //
        // Set up a field identical to this but without the transient data.
        //
        public MetricsField createField();

        public MetricsFieldContext getContext();

        public void setContext(MetricsFieldContext context);
    }

    static public abstract class AbstractField implements MetricsField
    {
        public AbstractField(MetricsView node, String prefix, String metricsName, String fieldName, Field objectField)
        {
            _node = node;
            _prefix = prefix;
            _metricsName = metricsName;
            _fieldName = fieldName;
            _objectField = objectField;
        }

        @Override
        public MetricsView getMetricsNode()
        {
            return _node;
        }

        @Override
        public String getMetricsName()
        {
            return _metricsName;
        }

        @Override
        public String getFieldName()
        {
            return _fieldName;
        }

        @Override
        public String getColumnName()
        {
            return _columnName == null ? _fieldName : _columnName;
        }

        public void setColumnName(String columnName)
        {
            _columnName = columnName;
        }

        @Override
        public String getColumnToolTip()
        {
            return _columnToolTip;
        }

        public void setColumnToolTip(String columnToolTip)
        {
            _columnToolTip = columnToolTip;
        }

        @Override
        public MetricsField createField()
        {
            return MetricsViewEditor.createField(_node, _prefix, _metricsName, _fieldName, _objectField, _context);
        }

        @Override
        public String getPropertyPrefix()
        {
            return _prefix;
        }

        @Override
        public MetricsFieldContext getContext()
        {
            return _context;
        }

        @Override
        public void setContext(MetricsFieldContext context)
        {
            _context = context;
        }

        private final MetricsView _node;
        private final String _prefix;
        private final String _metricsName;
        private final String _fieldName;
        private final Field _objectField;
        private String _columnName;
        private String _columnToolTip;
        private MetricsFieldContext _context;
    }

    static public class DeclaredMetricsField extends AbstractField
    {
        public DeclaredMetricsField(MetricsView node, String prefix, String metricsName, String fieldName, Field field)
        {
            super(node, prefix, metricsName, fieldName, field);
            if(field == null)
            {
                throw new IllegalArgumentException("Field argument must be non null, " +
                                                   "Metrics object: `" + metricsName + "' field: `" + fieldName + "'");
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

        @Override
        public Class getColumnClass()
        {
            return _columnClass;
        }

        public void setFormat(String format)
        {
            _cellRenderer = new FormatedNumberRenderer(format);
        }

        @Override
        public TableCellRenderer getCellRenderer()
        {
            return _cellRenderer;
        }

        @Override
        public Object getValue(com.zeroc.IceMX.Metrics m, long timestamp)
        {
            try
            {
                return m.getClass().getField(getFieldName()).get(m);
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

        private Class _columnClass;
        private TableCellRenderer _cellRenderer;
    }

    static public class AverageLifetimeMetricsField extends AbstractField
    {
        public AverageLifetimeMetricsField(MetricsView node, String prefix, String metricsName, String fieldName,
                                           Field field)
        {
            super(node, prefix, metricsName, fieldName, field);
            setFormat("#0.000"); // Set the default format
        }

        public void setFormat(String format)
        {
            _cellRenderer = new FormatedNumberRenderer(format);
        }

        @Override
        public Class getColumnClass()
        {
            return Float.class;
        }

        @Override
        public TableCellRenderer getCellRenderer()
        {
            return _cellRenderer;
        }

        public void setScaleFactor(String scaleFactor) throws java.lang.NumberFormatException
        {
            _scaleFactor = Double.parseDouble(scaleFactor);
        }

        @Override
        public Object getValue(com.zeroc.IceMX.Metrics m2, long timestamp)
        {
            com.zeroc.IceMX.Metrics m1 = _deltas.get(m2.id);
            _deltas.put(m2.id, m2);
            if(m1 == null)
            {
                return null;
            }
            if((m2.total - m1.total) - (m2.current - m1.current) == 0 ||
               (m2.totalLifetime - m1.totalLifetime) == 0)
            {
                return 0.0f;
            }
            else
            {
                return (float)(((m2.totalLifetime - m1.totalLifetime) / _scaleFactor) /
                                (m2.total - m1.total) - (m2.current - m1.current));
            }
        }

        private double _scaleFactor = 1.0d;
        private String _columnName;
        private TableCellRenderer _cellRenderer;
        private final Map<String, com.zeroc.IceMX.Metrics> _deltas = new HashMap<>();
    }

    static public class DeltaMeasurement
    {
        public double value;
        public long timestamp;
    }

    static public class DeltaAverageMetricsField extends AbstractField
    {
        public DeltaAverageMetricsField(MetricsView node, String prefix, String metricsName, String fieldName,
                                        Field field)
        {
            super(node, prefix, metricsName, fieldName, field);
            setFormat("#0.000"); // Set the default format
        }

        public void setFormat(String format)
        {
            _cellRenderer = new FormatedNumberRenderer(format);
        }

        @Override
        public Class getColumnClass()
        {
            return Double.class;
        }

        @Override
        public TableCellRenderer getCellRenderer()
        {
            return _cellRenderer;
        }

        public void setDataField(String dataField)
        {
            _dataField = dataField;
        }

        public void setScaleFactor(String scaleFactor) throws java.lang.NumberFormatException
        {
            _scaleFactor = Double.parseDouble(scaleFactor);
        }

        @Override
        public Object getValue(com.zeroc.IceMX.Metrics m, long timestamp)
        {
            DeltaMeasurement d1 = _deltas.get(m.id);
            DeltaMeasurement d2 = new DeltaMeasurement();
            try
            {
                Object v = m.getClass().getField(_dataField).get(m);
                if(v instanceof Integer)
                {
                    d2.value = (Integer)v;
                }
                else if(v instanceof Long)
                {
                    d2.value = (Long)v;
                }
                else if(v instanceof Float)
                {
                    d2.value = (Float)v;
                }
                else if(v instanceof Double)
                {
                    d2.value = (Double)v;
                }
            }
            catch(NoSuchFieldException ex)
            {
                ex.printStackTrace();
                return null;
            }
            catch(IllegalAccessException ex)
            {
                ex.printStackTrace();
                return null;
            }
            d2.timestamp =  timestamp;

            _deltas.put(m.id, d2);

            if(d1 == null)
            {
                //
                // Return null indicate the graph to ignore this measurement. We need two measurement to calculate
                // the delta increments.
                //
                return null;
            }

            //
            // If the elapsed period is less than the refresh period, don't
            // calculate a new value.
            //
            Double last = _last.get(m.id);
            if(last == null)
            {
                if(d2.timestamp - d1.timestamp >= getContext().getRefreshPeriod() * 1000)
                {
                    last = 0.0d;
                }
                else
                {
                    return null;
                }
            }
            if(d2.timestamp - d1.timestamp >= getContext().getRefreshPeriod() * 1000)
            {
                if(d2.value - d1.value == 0 || d2.timestamp - d1.timestamp == 0)
                {
                    last = 0.0d;
                }
                else
                {
                    last = (double)((d2.value - d1.value) / ((d2.timestamp - d1.timestamp) /  _scaleFactor));
                }
            }
            _last.put(m.id, last);
            return last;
        }

        private double _scaleFactor = 1.0d;
        private String _dataField;
        private final Map<String, DeltaMeasurement> _deltas = new HashMap<>();
        private final Map<String, Double> _last = new HashMap<>();
        private TableCellRenderer _cellRenderer;
    }

    static public class FailuresMetricsField extends AbstractField
    {
        public FailuresMetricsField(MetricsView node, String prefix, String metricsName, String fieldName, Field field)
        {
            super(node, prefix, metricsName, fieldName, field);
        }

        @Override
        public Class getColumnClass()
        {
            return JButton.class;
        }

        @Override
        public TableCellRenderer getCellRenderer()
        {
            return _cellRenderer;
        }

        @Override
        public Object getValue(final com.zeroc.IceMX.Metrics m, long timestamp)
        {
            JButton button = new JButton(Integer.toString(m.failures));
            if(m.failures > 0)
            {
                button.addActionListener(new ActionListener()
                    {
                        @Override
                        public void actionPerformed(ActionEvent e)
                        {
                            final DefaultTableModel model = new DefaultTableModel();
                            model.addColumn("Count");
                            model.addColumn("Type");
                            model.addColumn("Identity");
                            final JTable table = new JTable(model);

                            //
                            // Adjust row height for larger fonts
                            //
                            int fontSize = table.getFont().getSize();
                            int minRowHeight = fontSize + fontSize / 3;
                            if(table.getRowHeight() < minRowHeight)
                            {
                                table.setRowHeight(minRowHeight);
                            }

                            table.setPreferredSize(new Dimension(550, 200));

                            table.setPreferredScrollableViewportSize(table.getPreferredSize());
                            table.setCellSelectionEnabled(false);

                            table.setAutoResizeMode(JTable.AUTO_RESIZE_OFF);
                            table.getColumnModel().getColumn(0).setPreferredWidth(50);
                            table.getColumnModel().getColumn(1).setPreferredWidth(250);
                            table.getColumnModel().getColumn(2).setPreferredWidth(250);

                            JScrollPane scrollPane = new JScrollPane(table);

                            getMetricsNode().getCoordinator().getMainFrame().setCursor(
                                Cursor.getPredefinedCursor(Cursor.WAIT_CURSOR));
                            java.util.concurrent.CompletableFuture<com.zeroc.IceMX.MetricsFailures> r =
                                getMetricsNode().fetchMetricsFailures(getMetricsName(), m.id);
                            if(r != null)
                            {
                                r.whenComplete((result, ex) ->
                                    {
                                        if(ex == null)
                                        {
                                            SwingUtilities.invokeLater(() ->
                                                {
                                                    for(Map.Entry<String, Integer> entry : result.failures.entrySet())
                                                    {
                                                        Object[] row = new Object[3];
                                                        row[0] = entry.getValue().toString();
                                                        row[1] = entry.getKey();
                                                        row[2] = m.id;
                                                        model.addRow(row);
                                                    }
                                                    getMetricsNode().getCoordinator().getMainFrame().setCursor(
                                                        Cursor.getPredefinedCursor(Cursor.DEFAULT_CURSOR));
                                                });
                                        }
                                        else
                                        {
                                            SwingUtilities.invokeLater(() ->
                                                {
                                                    getMetricsNode().getCoordinator().getMainFrame().setCursor(
                                                        Cursor.getPredefinedCursor(Cursor.DEFAULT_CURSOR));
                                                    if(ex instanceof com.zeroc.Ice.ObjectNotExistException)
                                                    {
                                                        // Server is down.
                                                    }
                                                    else if(ex instanceof com.zeroc.Ice.FacetNotExistException)
                                                    {
                                                        // MetricsAdmin facet not present.
                                                    }
                                                    else
                                                    {
                                                        ex.printStackTrace();
                                                        JOptionPane.showMessageDialog(
                                                                getMetricsNode().getCoordinator().getMainFrame(),
                                                                "Error: " + ex.toString(), "Error",
                                                                JOptionPane.ERROR_MESSAGE);
                                                    }
                                                });
                                        }
                                    });
                            }

                            JOptionPane.showMessageDialog(getMetricsNode().getCoordinator().getMainFrame(), scrollPane,
                                                          "Metrics Failures", JOptionPane.PLAIN_MESSAGE);
                            getMetricsNode().getCoordinator().getMainFrame().setCursor(
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

        private static final TableCellRenderer _cellRenderer = new ButtonRenderer();
    }

    static public class SubMetricsField extends AbstractField
    {
        public SubMetricsField(MetricsView node, String prefix, String metricsName, String fieldName, Field field)
        {
            super(node, prefix, metricsName, fieldName, field);
        }

        @Override
        public Class getColumnClass()
        {
            return JButton.class;
        }

        @Override
        public TableCellRenderer getCellRenderer()
        {
            return _cellRenderer;
        }

        @Override
        public Object getValue(final com.zeroc.IceMX.Metrics m, final long timestamp)
        {
            try
            {
                final com.zeroc.IceMX.Metrics[] objects =
                    (com.zeroc.IceMX.Metrics[])m.getClass().getField(getFieldName()).get(m);
                JButton button = new JButton(Integer.toString(objects.length));
                button.setEnabled(objects.length > 0);
                if(objects.length > 0)
                {
                    button.addActionListener(new ActionListener()
                        {
                            @Override
                            public void actionPerformed(ActionEvent event)
                            {
                                final TableModel model = new TableModel(getMetricsName());
                                String prefix = "IceGridGUI.Metrics." + getMetricsName() + "." + getFieldName();
                                String[] names = _properties.getPropertyAsList(prefix + ".fields");
                                for(String name : names)
                                {
                                    Field objectField = null;
                                    try
                                    {
                                        objectField = objects[0].getClass().getField(name);
                                    }
                                    catch(NoSuchFieldException ex)
                                    {
                                    }
                                    MetricsField field = MetricsViewEditor.createField(getMetricsNode(),
                                                                                       prefix + "." + name,
                                                                                       getFieldName(), name,
                                                                                       objectField,
                                                                                       getContext());
                                    if(field != null)
                                    {
                                        model.addField(field);
                                    }
                                }
                                if(model.getMetricFields().size() == 0)
                                {
                                    return;
                                }

                                final JTable table = new JTable(model)
                                    {
                                        //
                                        // Implement table header tool tips.
                                        //
                                        @Override
                                        protected JTableHeader createDefaultTableHeader()
                                        {
                                            return new JTableHeader(columnModel)
                                            {
                                                @Override
                                                public String getToolTipText(MouseEvent e)
                                                {
                                                    int index = columnModel.getColumn(columnModel.getColumnIndexAtX(
                                                                                       e.getPoint().x)).getModelIndex();
                                                    return model.getMetricFields().get(index).getColumnToolTip();
                                                }
                                            };
                                        }
                                    };
                                table.addMouseListener(new ButtonMouseListener(table));
                                table.setAutoCreateRowSorter(true);

                                //
                                // Adjust row height for larger fonts
                                //
                                int fontSize = table.getFont().getSize();
                                int minRowHeight = fontSize + fontSize / 3;
                                if(table.getRowHeight() < minRowHeight)
                                {
                                    table.setRowHeight(minRowHeight);
                                }

                                for(Map.Entry<Integer, MetricsField> fieldEntry : model.getMetricFields().entrySet())
                                {
                                    if(fieldEntry.getValue().getCellRenderer() != null)
                                    {
                                        table.getColumnModel().getColumn(fieldEntry.getKey().intValue()).
                                                            setCellRenderer(fieldEntry.getValue().getCellRenderer());
                                    }
                                }

                                int idColumn = table.getColumnModel().getColumnIndex(_properties.getProperty(prefix +
                                                                                                     ".id.columnName"));

                                for(com.zeroc.IceMX.Metrics m : objects)
                                {
                                    model.addMetrics(m, timestamp);
                                }

                                JScrollPane scrollPane = new JScrollPane(table);
                                scrollPane.setPreferredSize(new Dimension(800, 600));
                                JOptionPane.showMessageDialog(getMetricsNode().getCoordinator().getMainFrame(),
                                                              scrollPane, getColumnName(), JOptionPane.PLAIN_MESSAGE);

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

        private static final TableCellRenderer _cellRenderer = new ButtonRenderer();
    }

    private static final int _refreshPeriod = 5;
    private static com.zeroc.Ice.Properties _properties;
    private static String[] _sectionSort;
    private static Map<String, String> _sectionNames = new HashMap<>();
    private static TreePath _selectedPath;
    final private Preferences _prefs;
}
