// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceGridGUI.LiveDeployment;

import javafx.scene.*;

import java.util.Map;
import java.util.List;
import java.util.HashMap;
import java.util.ArrayList;

import java.util.Date;
import java.util.TimeZone;

import java.text.DateFormat;
import java.text.SimpleDateFormat;

import java.awt.datatransfer.DataFlavor;
import java.awt.datatransfer.Transferable;
import java.awt.datatransfer.UnsupportedFlavorException;

import java.awt.BorderLayout;
import java.awt.Component;
import java.awt.Container;
import java.awt.Dimension;
import java.awt.Font;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;

import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.BorderFactory;

import javax.swing.border.CompoundBorder;
import javax.swing.border.EmptyBorder;

import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JComboBox;
import javax.swing.JComponent;
import javax.swing.JCheckBoxMenuItem;
import javax.swing.JDialog;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JSpinner;
import javax.swing.JTable;
import javax.swing.JOptionPane;
import javax.swing.KeyStroke;
import javax.swing.SwingUtilities;
import javax.swing.SpinnerNumberModel;
import javax.swing.SwingConstants;

import javax.swing.table.DefaultTableModel;

import javafx.application.Platform;

import javafx.embed.swing.JFXPanel;

import javafx.scene.chart.LineChart;
import javafx.scene.chart.NumberAxis;
import javafx.scene.chart.XYChart;

import javafx.scene.control.cell.PropertyValueFactory;
import javafx.scene.control.cell.TextFieldTableCell;
import javafx.scene.control.Label;
import javafx.scene.control.ScrollPane;
import javafx.scene.control.ScrollPane.ScrollBarPolicy;
import javafx.scene.shape.Line;

import javafx.scene.layout.VBox;
import javafx.scene.layout.HBox;
import javafx.scene.layout.Priority;
import javafx.scene.layout.FlowPane;

import javafx.scene.Scene;

import javafx.util.StringConverter;

import com.jgoodies.forms.layout.FormLayout;
import com.jgoodies.forms.builder.DefaultFormBuilder;
import com.jgoodies.forms.factories.Borders;
import com.jgoodies.forms.builder.ButtonBarBuilder;

import IceGrid.*;
import IceGridGUI.*;

public class GraphView extends JFrame
{

    public enum GraphCategory
    {
        DispatchGraphCategory,
        InvocationsGraphCategory,
        ConnectionsGraphCategory,
        ThreadsGraphCategory,
        BandwidthGraphCategory
    }

    public enum GraphType
    {
        TotalAverageGraphType,
        CurrentCountGraphType,
        BandwidhGraphType
    }

    class TimeFormatter extends StringConverter<java.lang.Number>
    {
        TimeFormatter()
        {
            _dateFormat.setTimeZone(TimeZone.getDefault());
        }

        @Override
        public synchronized String toString(Number timestamp)
        {
            Date date = new Date(timestamp.longValue());
            if(date.getTime() < 1000)
            {
                //
                // When the x axis is first draw we don't have times to display.
                //
                return "";
            }
            else
            {
                return _dateFormat.format(date);
            }
        }

        @Override
        public synchronized Number fromString(String time)
        {
            try
            {
                long t = _dateFormat.parse(time).getTime();
                return t;
            }
            catch(java.text.ParseException e)
            {
                return 0;
            }
        }

        public synchronized void setDateFormat(String format)
        {
            _dateFormat = new SimpleDateFormat(format);
        }

        private DateFormat _dateFormat = new SimpleDateFormat(getDateFormat());
    }

    class TransferHandler extends javax.swing.TransferHandler
    {
        @Override
        public boolean 
        canImport(TransferHandler.TransferSupport support)
        {
            boolean supported = false;
            for(DataFlavor f : support.getDataFlavors())
            {
                if(f.getMimeType().equals(NodeMimeType) || f.getMimeType().equals(ServerMimeType) || 
                   f.getMimeType().equals(MetricsViewMimeType))
                {
                    supported = true;
                    break;
                }
            }
            return supported;
        }

        @Override
        public boolean
        importData(TransferHandler.TransferSupport support)
        {
            if(!canImport(support))
            {
                return false;
            }

            Transferable t = support.getTransferable();
            
            for(DataFlavor flavor : support.getDataFlavors())
            {
                try
                {
                    if(flavor.getMimeType().equals(NodeMimeType))
                    {
                        Node node = (Node)t.getTransferData(flavor);
                        importNode(node);
                        break;
                    }
                    else if(flavor.getMimeType().equals(ServerMimeType))
                    {
                        Server server = (Server)t.getTransferData(flavor);
                        importServer(server);
                        break;
                    }
                    else if(flavor.getMimeType().equals(MetricsViewMimeType))
                    {
                        MetricsView metricsView = (MetricsView)t.getTransferData(flavor);
                        importMetricsView(metricsView);
                        break;
                    }
                }
                catch(UnsupportedFlavorException ex)
                {
                }
                catch(java.io.IOException ex)
                {
                }
            }

            return true;
        }
    }

    private void importNode(Node node)
    {
        List<Server> servers = node.getServers();
        for(Server s : servers)
        {
            importServer(s);
        }
    }

    private void importServer(Server server)
    {
        List<MetricsView> metrics = server.getMetrics();
        for(MetricsView m : metrics)
        {
            importMetricsView(m);
        }
    }

    private void importMetricsView(MetricsView metricsView)
    {
        Server server = (Server)metricsView.getParent();
        Node node = (Node)server.getParent();
        MetricsViewInfo s = new MetricsViewInfo(node.getId(), server.getId(), metricsView.getId());
        if(!_metrics.contains(s))
        {
            if(_metrics.size() == 0)
            {
                startRefreshThread();
            }
            _metrics.add(s);
            addSeries(s, metricsView.data().get(_key));
        }
    }

    class MetricsViewTargetInfo
    {
        MetricsViewTargetInfo(MetricsViewInfo parent, String id)
        {
            this.parent = parent;
            this.id = id;
            this.total = 0;
            this.totalLifetime = 0;
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
            else if(!(other instanceof MetricsViewTargetInfo))
            {
                return false;
            }
            MetricsViewTargetInfo that = (MetricsViewTargetInfo)other;
            return this.parent.equals(that.parent) && this.id.equals(that.id);
        }

        @Override
        public int hashCode()
        {
            return IceInternal.HashUtil.hashAdd(parent.hashCode(), id);
        }

        @Override 
        public String toString()
        {
            StringBuilder sb = new StringBuilder();
            sb.append(parent.toString());
            sb.append(" - ");
            sb.append(id);
            return sb.toString();
        }

        public String id;
        public MetricsViewInfo parent;
        //
        // Totals from last delta.
        //
        public long total;
        public long totalLifetime;
    };

    class BandwidhMetricsInfo extends MetricsViewTargetInfo
    {
        BandwidhMetricsInfo(MetricsViewInfo parent, String id, boolean in)
        {
            super(parent, id);
            this.in = in;
        }

        @Override public boolean
        equals(Object other)
        {
            if(other == null)
            {
                return false;
            }
            else if(other == this)
            {
                return true;
            }
            else if(!(other instanceof BandwidhMetricsInfo))
            {
                return false;
            }
            BandwidhMetricsInfo that = (BandwidhMetricsInfo)other;
            return super.equals(that) && this.in == that.in;
        }

        @Override
        public int hashCode()
        {
            return IceInternal.HashUtil.hashAdd(super.hashCode(), in);
        }

        @Override 
        public String toString()
        {
            StringBuilder sb = new StringBuilder();
            sb.append(super.toString());
            sb.append(" - ");
            sb.append((in ? "bytes received" : "bytes sent"));
            return sb.toString();
        }

        public boolean in;
    };

    public class MetricsViewInfo
    {
        MetricsViewInfo(String node, String server, String view)
        {
            this.node = node;
            this.server = server;
            this.view = view;
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
            return this.node.equals(that.node) && this.server.equals(that.server) && this.view.equals(that.view);
        }

        @Override
        public int hashCode()
        {
            int h = IceInternal.HashUtil.hashAdd(5381, node);
            h = IceInternal.HashUtil.hashAdd(h, server);
            return IceInternal.HashUtil.hashAdd(h, view);
        }

        @Override 
        public String toString()
        {
            StringBuilder sb = new StringBuilder();
            sb.append(node);
            sb.append(" - ");
            sb.append(server);
            sb.append(" - ");
            sb.append(view);
            return sb.toString();
        }

        public String node;
        public String server;
        public String view;
    }

    public class DeltaInfo
    {
        public DeltaInfo(long total, long totalLifetime)
        {
            this.total = total;
            this.totalLifetime = totalLifetime;
        }

        public long total;
        public long totalLifetime;
    }

    private class RefreshThread extends Thread
    {
        RefreshThread(long period)
        {
            _period = period;
            _done = false;
        }

        synchronized void setRefreshPeriod(long period)
        {
           _period = period;
        }

        synchronized public void
        run()
        {
            while(true)
            {
                ArrayList<MetricsViewInfo> metrics = null;
                synchronized(GraphView.this)
                {
                    metrics = new ArrayList<MetricsViewInfo>(_metrics);
                }
                for(final MetricsViewInfo m : metrics)
                {
                    Ice.Identity adminId = new Ice.Identity(m.server, _coordinator.getServerAdminCategory());
                    Ice.ObjectPrx admin = _coordinator.getAdmin().ice_identity(adminId);
                    IceMX.Callback_MetricsAdmin_getMetricsView cb = new IceMX.Callback_MetricsAdmin_getMetricsView()
                        {
                            public void response(final java.util.Map<java.lang.String, IceMX.Metrics[]> data)
                            {
                                addDelta(m, data, System.currentTimeMillis());
                            }

                            public void exception(final Ice.LocalException e)
                            {
                                handleError(m, e.toString());
                            }

                            public void exception(final Ice.UserException e)
                            {
                                handleError(m, e.toString());
                            }
                        };
                    try
                    {
                        IceMX.MetricsAdminPrx metricsAdmin = 
                            IceMX.MetricsAdminPrxHelper.uncheckedCast(admin.ice_facet("MetricsAdmin"));
                        metricsAdmin.begin_getMetricsView(m.view, cb);
                    }
                    catch(Ice.LocalException e)
                    {
                        handleError(m, e.toString());
                    }
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

        private void handleError(final MetricsViewInfo metrics, final String error)
        {
            SwingUtilities.invokeLater(new Runnable()
                {
                    public void run()
                    {
                        JOptionPane.showMessageDialog(GraphView.this,
                                        "Error retrieving metrics view from `" + metrics.toString() + "'\n" + error, 
                                        "Error",
                                        JOptionPane.ERROR_MESSAGE);
                        removeMetrics(metrics);
                    }
                });
        }

        private long _period;
        private boolean _done = false;
    }

    public GraphView(Coordinator coordinator, GraphType type, GraphCategory category, 
                    final String title, String key)
    {
        _coordinator = coordinator;
        _type = type;
        _category = category;
        setTitle(title);
        _key = key;

        //
        // Don't destroy JavaFX when the frame is disposed.
        //
        Platform.setImplicitExit(false);

        setIconImage(Utils.getIcon("/icons/16x16/grid.png").getImage());

        setDefaultCloseOperation(JFrame.DO_NOTHING_ON_CLOSE);

        addWindowListener(new WindowAdapter()
        {
            public void windowClosing(WindowEvent e) 
            {
                close();
            }
        });

        //
        // Create actions
        //
        Action removeMetrics = new AbstractAction("Remove Metrics")
            {
                public void actionPerformed(ActionEvent event) 
                {
                    final DefaultTableModel model = new DefaultTableModel()
                        {
                            public Class getColumnClass(int column)
                            {
                                if(column == 3)
                                {
                                    return Boolean.class;
                                }
                                else
                                {
                                    return String.class;
                                }
                            }
                            
                            public boolean isCellEditable(int row, int column)
                            {
                                return column == 3;
                            }
                        };
                    model.addColumn("Node");
                    model.addColumn("Server");
                    model.addColumn("View");
                    model.addColumn("Show");

                    for(MetricsViewInfo m : _metrics)
                    {
                        model.addRow(new Object[]{m.node, m.server, m.view, true});
                    }
                    
                    JTable table = new JTable(model);
                    table.setPreferredSize(new Dimension(450, 300));
                    table.setPreferredScrollableViewportSize(table.getPreferredSize());
                    table.setCellSelectionEnabled(false);
                    table.setOpaque(false);
                    JScrollPane scrollPane = new JScrollPane(table);
                    JOptionPane.showMessageDialog(GraphView.this, scrollPane, "Select Servers", 
                                                  JOptionPane.PLAIN_MESSAGE);

                    for(int i = 0; i < model.getRowCount(); ++i)
                    {
                        //
                        // Remove unselected metrics from the graph.
                        //
                        boolean selected = ((Boolean)model.getValueAt(i, 3)).booleanValue();
                        if(!selected)
                        {
                            removeMetrics(new MetricsViewInfo((String)model.getValueAt(i, 0), 
                                                              (String)model.getValueAt(i, 1), 
                                                              (String)model.getValueAt(i, 2)));

                        }
                    }
                }
            };
        removeMetrics.setEnabled(true);

        //
        // Graph preferences.
        //
        Action preferences = new AbstractAction("Preferences")
            {
                public void actionPerformed(ActionEvent event) 
                {
                    //
                    // SpinnerNumberModel to set a refresh period.
                    //
                    // min value is 500 ms == 0.5 seconds
                    // max value is 60 * 1000 ms == 60 seconds == 1 minute
                    //
                    SpinnerNumberModel refreshPeriod = new SpinnerNumberModel(getRefreshPeriod(), 500, 60 * 1000, 1);
                    JPanel refreshPanel;
                    {
                        DefaultFormBuilder builder =
                                                new DefaultFormBuilder(new FormLayout("pref,2dlu,pref:grow", "pref"));
                        builder.append("Refresh period (ms):", new JSpinner(refreshPeriod));
                        refreshPanel = builder.getPanel();
                    }

                    //
                    // SpinnerNumberModel to set the number of symbols to keep in X axis.
                    //
                    // min value is 10
                    // max value is 100
                    //
                    SpinnerNumberModel horizontalAxisSymbolCount = 
                                                            new SpinnerNumberModel(_horizontaSymbolsCount, 5, 100, 1);

                    //
                    // JComboBox to select time format used in X Axis
                    //
                    JComboBox<String> dateFormats = new JComboBox<String>(_dateFormats);
                    dateFormats.setSelectedItem(getDateFormat());
                    JPanel xAxisPanel;
                    {
                        DefaultFormBuilder builder = 
                                                new DefaultFormBuilder(new FormLayout("pref,2dlu,pref:grow", "pref"));
                        builder.append("Number of horizontal symbols:", new JSpinner(horizontalAxisSymbolCount));
                        builder.append("Time format:", dateFormats);

                        xAxisPanel = builder.getPanel();
                    }

                    //
                    // JComboBox to select Y Axis units.
                    //
                    JComboBox<String> units = null;
                    JPanel yAxisPanel = null;
                    if(_type == GraphType.BandwidhGraphType || _type == GraphType.TotalAverageGraphType)
                    {
                        if(_type == GraphType.BandwidhGraphType)
                        {
                            units = new JComboBox<String>(_bandwidthUnits);
                            units.setSelectedIndex(getBandwidthUnitIndex());
                        }
                        else
                        {
                            units = new JComboBox<String>(_timeUnits);
                            units.setSelectedIndex(getTimeUnitIndex());
                        }

                        
                        {
                            DefaultFormBuilder builder = 
                                                new DefaultFormBuilder(new FormLayout("pref,2dlu,pref:grow", "pref"));
                            builder.append("Units:", units);
                            yAxisPanel = builder.getPanel();
                        }
                    }

                    FormLayout layout = new FormLayout("fill:pref:grow", "pref");
                    final DefaultFormBuilder builder = new DefaultFormBuilder(layout);
                    builder.border(Borders.DIALOG);
                    builder.appendSeparator("Refresh Thread");
                    builder.append(refreshPanel);
                    builder.nextLine();
                    builder.appendSeparator("Horizontal Axis");
                    builder.append(xAxisPanel);
                    if(yAxisPanel != null)
                    {
                        builder.appendSeparator("Vertical Axis");
                        builder.append(yAxisPanel);
                    }

                    class PreferencesDialog extends JDialog
                        {
                            PreferencesDialog(JFrame owner)
                            {
                                super(owner, true);
                                setTitle("Graph Preferences");
                                setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);

                                JButton okButton = new JButton("OK");
                                ActionListener okListener = new ActionListener()
                                    {
                                        public void actionPerformed(ActionEvent e)
                                        {
                                            _option = JOptionPane.OK_OPTION;
                                            setVisible(false);
                                            dispose();
                                        }
                                    };

                                ActionListener cancelListener = new ActionListener()
                                    {
                                        public void actionPerformed(ActionEvent e)
                                        {
                                            setVisible(false);
                                            dispose();
                                        }
                                    };

                                okButton.addActionListener(okListener);
                                getRootPane().setDefaultButton(okButton);
                                KeyStroke stroke = KeyStroke.getKeyStroke(KeyEvent.VK_ESCAPE, 0);
                                getRootPane().registerKeyboardAction(cancelListener, stroke, 
                                                                     JComponent.WHEN_IN_FOCUSED_WINDOW);

                                final JComponent buttonBar = new ButtonBarBuilder().addGlue()
                                                                                   .addButton(okButton)
                                                                                   .addGlue().build();
                                buttonBar.setBorder(Borders.DIALOG);

                                Container contentPane = getContentPane();
                                contentPane.setLayout(new BoxLayout(contentPane, BoxLayout.Y_AXIS));
                                contentPane.add(builder.getPanel());
                                contentPane.add(buttonBar);
                                pack();
                                setResizable(false);
                                setLocationRelativeTo(owner);
                            }

                            int showDialog()
                            {
                                setVisible(true);
                                return _option;
                            }

                            private int _option = JOptionPane.CLOSED_OPTION;
                        };
                    //
                    // Disabled the default dialog action, so we can use Enter for commit table cells edit.
                    //
                    PreferencesDialog dialog = new PreferencesDialog(GraphView.this);

                    if(dialog.showDialog() != JOptionPane.OK_OPTION)
                    {
                        return;
                    }
                    setRefreshPeriod(refreshPeriod.getNumber().longValue());
                    setHorizontalSymbolsCount(horizontalAxisSymbolCount.getNumber().intValue());
                    setDateFormat((String)dateFormats.getSelectedItem());
                    if((_type == GraphType.BandwidhGraphType && getBandwidthUnitIndex() != units.getSelectedIndex()) ||
                        _type == GraphType.TotalAverageGraphType && getTimeUnitIndex() != units.getSelectedIndex())
                    {
                        final int index = units.getSelectedIndex();
                        final float  cf = conversionFactor(index);
                        final String label = yAxisLabel(index);

                        if(_type == GraphType.BandwidhGraphType)
                        {
                            setBandwidthUnitIndex(index);
                        }
                        else
                        {
                            setTimeUnitIndex(index);
                        }

                        //
                        // Update the graph in JavaFX thread.
                        //
                        Platform.runLater(new Runnable()
                            {
                                @Override
                                public void run()
                                {
                                    _yAxis.setLabel(label);
                                    for(XYChart.Series<Number, Number> series : _chart.getData())
                                    {
                                        for(XYChart.Data<Number, Number> item : series.getData())
                                        {
                                            item.setYValue(item.getYValue().floatValue() * cf);
                                        }
                                    }
                                }
                            });
                    }
                }
            };

        Action showLegend = new AbstractAction("Show Legend")
            {
                public void actionPerformed(ActionEvent event) 
                {
                    Platform.runLater(new Runnable()
                            {
                                @Override
                                public void run()
                                {
                                    _legendScroll.setVisible(!_legendScroll.isVisible());
                                }
                            });
                }
            };

        //
        // Create menus
        //
        JMenuBar menuBar = new JMenuBar();

        // Create a menu
        JMenu fileMenu = new JMenu("File");
        fileMenu.setMnemonic(java.awt.event.KeyEvent.VK_F);
        fileMenu.add(removeMetrics);
        fileMenu.add(preferences);
        menuBar.add(fileMenu);

        JMenu viewMenu = new JMenu("View");
        viewMenu.setMnemonic(java.awt.event.KeyEvent.VK_V);
        _showLegendMenuItem = new JCheckBoxMenuItem(showLegend);
        _showLegendMenuItem.setState(false);
        viewMenu.add(_showLegendMenuItem);
        menuBar.add(viewMenu);

        setJMenuBar(menuBar);

        JLabel dropTarget = new JLabel("Drop runtime components here to add it to the graph.");
        dropTarget.setBorder(new CompoundBorder(BorderFactory.createLineBorder(java.awt.Color.gray, 5, true), 
                                                new EmptyBorder(20, 20, 20, 20)));
        dropTarget.setHorizontalAlignment(SwingConstants.CENTER);
        Font f = dropTarget.getFont();
        dropTarget.setFont(f.deriveFont(f.getStyle(), f.getSize() + 8));
        dropTarget.setTransferHandler(new TransferHandler());


        final JFXPanel fxPanel = new JFXPanel();
        DefaultFormBuilder builder = new DefaultFormBuilder(new FormLayout("fill:pref:grow", "fill:pref:grow, pref"));
        builder.append(fxPanel);
        builder.nextLine();
        builder.append(dropTarget);
        builder.nextLine();

        JPanel panel = builder.getPanel();
        panel.setBorder(BorderFactory.createEmptyBorder(5, 5, 5, 5));
        setContentPane(panel);
        setPreferredSize(new Dimension(1024, 768));
        pack();
        //
        // initialize the scene in JavaFX thread.
        //
        Platform.runLater(new Runnable()
            {
                @Override
                public void run()
                {
                    _xAxis = new NumberAxis();
                    _yAxis = new NumberAxis();
                    _legend = new VBox(5);
                    _legend.setPadding(new javafx.geometry.Insets(5, 5, 5, 5));
                    _legend.setAlignment(javafx.geometry.Pos.CENTER);
                    _legendScroll = new ScrollPane();
                    _legendScroll .managedProperty().bind(_legendScroll .visibleProperty());
                    _legendScroll.setHbarPolicy(ScrollBarPolicy.NEVER);
                    _legendScroll.setVbarPolicy(ScrollBarPolicy.AS_NEEDED);
                    _legendScroll.setPrefWidth(295);
                    _legendScroll.setMaxWidth(295);
                    _legendScroll.setContent(_legend);
                    _legendScroll.setVisible(false);

                    VBox legendBox = new VBox(5);
                    legendBox.setPadding(new javafx.geometry.Insets(10, 10, 10, 10));
                    legendBox.setAlignment(javafx.geometry.Pos.CENTER);
                    VBox.setVgrow(_legendScroll, Priority.ALWAYS);
                    legendBox.getChildren().add(_legendScroll);

                    _chart = new LineChart<Number, Number>(_xAxis, _yAxis);

                    _xAxis.setLabel("Time (HH:mm:ss)");
                    _xAxis.setTickLabelFormatter(_timeFormater);
                    _xAxis.setForceZeroInRange(false);
                    _yAxis.setLabel(yAxisLabel(getUnitsSelectedIndex()));
                    _chart.setTitle(title);
                    _chart.setAnimated(true);
                    _chart.setLegendVisible(false);

                    HBox hbox = new HBox(5);
                    hbox.setPadding(new javafx.geometry.Insets(0, 5, 0, 5)); 
                    hbox.setAlignment(javafx.geometry.Pos.CENTER);
                    HBox.setHgrow(_chart, Priority.ALWAYS);
                    hbox.getChildren().addAll(_chart, legendBox);
                    fxPanel.setScene(new Scene(hbox));
                }
            });
        setLocationRelativeTo(_coordinator.getMainFrame());
        setVisible(true);
    }

    public void close()
    {
        stopRefreshThread();
        setVisible(false);
        _coordinator.removeGraphView(GraphView.this);
        dispose();
    }

    synchronized private void startRefreshThread()
    {
        if(_refreshThread == null)
        {
            _refreshThread = new RefreshThread(getRefreshPeriod());
            _refreshThread.start();
        }
    }

    synchronized private void stopRefreshThread()
    {
        if(_refreshThread != null)
        {
            _refreshThread.done();
            _refreshThread = null;
        }
    }

    synchronized private void setHorizontalSymbolsCount(int count)
    {
        _horizontaSymbolsCount = count;
    }

    synchronized private int getHorizontalSymbolsCount()
    {
        return _horizontaSymbolsCount;
    }

    void addDelta(MetricsViewInfo info, java.util.Map<java.lang.String, IceMX.Metrics[]> data, 
                               final long timestamp)
    {
        IceMX.Metrics[] metrics = data.get(_key);
        Map<MetricsViewTargetInfo, XYChart.Series<Number, Number>> seriesSeq = _series.get(info);
        if(seriesSeq == null)
        {
            return;
        }

        if(metrics != null)
        {
            if(_type == GraphType.TotalAverageGraphType)
            {
                for(IceMX.Metrics m : metrics)
                {
                    MetricsViewTargetInfo targetInfo = new MetricsViewTargetInfo(info, m.id);
                    XYChart.Series<Number, Number> series = seriesSeq.get(targetInfo);
                    if(series == null)
                    {
                        continue;
                    }

                    float value = 0;
                    DeltaInfo delta = _deltas.get(targetInfo);
                    if(delta == null)
                    {
                        delta = new DeltaInfo(m.total, m.totalLifetime);
                        _deltas.put(targetInfo, delta);
                        continue; // We need two meassurements to calculate delta AVG.
                    }
                    long deltaTotal = (m.total - delta.total);
                    value = deltaTotal == 0 ? 0.0f : 
                                        (m.totalLifetime - delta.totalLifetime) / deltaTotal;
                    delta.total = m.total;
                    delta.totalLifetime = m.totalLifetime;
                    addDelta(series, timestamp, value * _timeUnitsFactors[getTimeUnitIndex()]);
                }
            }
            else if(_type == GraphType.CurrentCountGraphType)
            {
                for(IceMX.Metrics m : metrics)
                {
                    MetricsViewTargetInfo targetInfo = new MetricsViewTargetInfo(info, m.id);
                    XYChart.Series<Number, Number> series = seriesSeq.get(targetInfo);
                    if(series == null)
                    {
                        continue;
                    }
                    float value = 0;
                    value = m.current;
                    addDelta(series, timestamp, value);
                }
            }
            else if(_type == GraphType.BandwidhGraphType)
            {
                for(IceMX.Metrics m : metrics)
                {
                    IceMX.ConnectionMetrics cm = (IceMX.ConnectionMetrics)m;
                    for(boolean in : new boolean[]{true, false})
                    {
                        MetricsViewTargetInfo targetInfo = new BandwidhMetricsInfo(info, m.id, in);
                        XYChart.Series<Number, Number> series = seriesSeq.get(targetInfo);
                        if(series == null)
                        {
                            continue;
                        }

                        float value = 0;
                        DeltaInfo delta = _deltas.get(targetInfo);
                        if(delta == null)
                        {
                            if(in)
                            {
                                delta = new DeltaInfo(cm.receivedBytes, timestamp);
                            }
                            else
                            {
                                delta = new DeltaInfo(cm.sentBytes, timestamp);
                            }
                            _deltas.put(targetInfo, delta);
                            continue; // We need two meassurements to calculate delta AVG.
                        }

                        if(in)
                        {
                            boolean zero = (cm.receivedBytes - delta.total) == 0 || 
                                           (timestamp - delta.totalLifetime) == 0;
                            value = zero ? 0.0f : (cm.receivedBytes - delta.total) / 
                                                  (float)(timestamp - delta.totalLifetime);
                            delta.total = cm.receivedBytes;
                            delta.totalLifetime = timestamp;
                        }
                        else
                        {
                            boolean zero = (cm.sentBytes - delta.total) == 0 || 
                                           (timestamp - delta.totalLifetime) == 0;
                            value = zero ? 0.0f : (cm.sentBytes - delta.total) / 
                                                  (float)(timestamp - delta.totalLifetime);
                            delta.total = cm.sentBytes;
                            delta.totalLifetime = timestamp;
                        }
                        addDelta(series, timestamp, value * _bandwidthUnitsFactors[getBandwidthUnitIndex()]);
                    }
                }
            }
        }
    }

    private void addDelta(final XYChart.Series<Number, Number> series, final long timestamp, final float value)
    {
        //
        // Update the graph series in JavaFX thread.
        //
        Platform.runLater(new Runnable()
            {
                @Override
                public void run()
                {
                    series.getData().add(new XYChart.Data<Number, Number>(timestamp, value));
                    while(series.getData().size() > getHorizontalSymbolsCount())
                    {
                        series.getData().remove(0);
                    }
                    updateStyle(series, getSeriesClass(series));
                }
            });
    }

    public void addSeries(final MetricsViewInfo info, final IceMX.Metrics[] data)
    {
        //
        // Need to run in JavaFX thread.
        //
        Platform.runLater(new Runnable()
            {
                @Override
                public void run()
                {
                    if(data != null)
                    {
                        Map<MetricsViewTargetInfo, XYChart.Series<Number, Number>> seriesSeq = _series.get(info);
 
                        if(seriesSeq == null)
                        {
                            seriesSeq = new HashMap<MetricsViewTargetInfo, XYChart.Series<Number, Number>>();
                            _series.put(info, seriesSeq);
                        }
                        
                        if(_category != GraphCategory.BandwidthGraphCategory)
                        {
                            for(IceMX.Metrics m : data)
                            {
                                String color = DefaultColors[_chart.getData().size() % DefaultColors.length];
                                MetricsViewTargetInfo targetInfo = new MetricsViewTargetInfo(info, m.id);
                                XYChart.Series<Number, Number> series = new XYChart.Series<Number, Number>();
                                series.setName(targetInfo.toString());
                                seriesSeq.put(targetInfo, series);
                                _chart.getData().add(series);
                                String styleClass = getSeriesClass(series);
                                addStyle(series, styleClass, color, false);
                                updateStyle(series, styleClass);
                                seriesAdded(targetInfo.toString(), styleClass);
                            }
                        }
                        else
                        {
                            for(IceMX.Metrics m : data)
                            {
                                String color = DefaultColors[(_chart.getData().size() / 2) % DefaultColors.length];
                                for(boolean in : new boolean[]{true, false})
                                {
                                    MetricsViewTargetInfo targetInfo = new BandwidhMetricsInfo(info, m.id, in);
                                    XYChart.Series<Number, Number> series = new XYChart.Series<Number, Number>();
                                    series.setName(targetInfo.toString());
                                    seriesSeq.put(targetInfo, series);
                                    _chart.getData().add(series);
                                    String styleClass = getSeriesClass(series);
                                    addStyle(series, styleClass, color, in);
                                    updateStyle(series, styleClass);
                                    seriesAdded(targetInfo.toString(), styleClass);
                                }
                            }
                        }
                    }
                }
            });
    }

    //
    // Must be called in JavaFX thread.
    //
    // Return the class used to style a serie.
    //
    public String getSeriesClass(XYChart.Series<Number, Number> series)
    {
        String value = null;
        for(String styleClass : series.getNode().getStyleClass())
        {
            if(styleClass.startsWith("series"))
            {
                value = styleClass;
                break;
            }
        }
        return value;
    }

    private void addStyle(XYChart.Series<Number, Number> series, String seriesClass, String color, boolean dashed)
    {
        //
        // Customize the style.
        //
        StringBuilder sb = new StringBuilder();
        sb.append("-fx-stroke: ");
        sb.append(color);
        sb.append("; ");
        sb.append("-fx-background-color: ");
        sb.append(color);
        sb.append(", white;");
        if(dashed)
        {
            sb.append("-fx-stroke-dash-array: 10 10;");
        }
        sb.append("-fx-stroke-width: 5;");
        _styles.put(seriesClass, sb.toString()); 
    }

    private void 
    removeMetrics(MetricsViewInfo info)
    {
        _metrics.remove(info);
        if(_metrics.size() == 0)
        {
            stopRefreshThread();
        }
        removeSeries(info);
    }

    //
    // Remove all series associated to a Metrics View
    //
    private void removeSeries(final MetricsViewInfo info)
    {
        //
        // Need to run in JavaFX thread.
        //
        Platform.runLater(new Runnable()
            {
                @Override
                public void run()
                {
                    Map<MetricsViewTargetInfo, XYChart.Series<Number, Number>> seriesSeq = _series.get(info);
                    if(seriesSeq != null)
                    {
                        for(Map.Entry<MetricsViewTargetInfo, XYChart.Series<Number, Number>> entry : 
                            seriesSeq.entrySet())
                        {
                            String seriesClass = getSeriesClass(entry.getValue());
                            seriesRemoved(seriesClass);
                            _styles.remove(seriesClass);
                            //
                            // Don't remove the XYChart.Series object here, to avoid the series style classes 
                            // to be reasign by JavaFX.
                            //
                            // _chart.getData().remove(entry.getValue());
                            entry.getValue().getData().clear();
                        }
                        _deltas.remove(info);
                        _series.remove(info);
                    }
                }
            });
    }

    synchronized long getRefreshPeriod()
    {
        return _refreshPeriod;
    }

    synchronized void setRefreshPeriod(long refreshPeriod)
    {
        _refreshPeriod = refreshPeriod;
        if(_refreshThread != null)
        {
            _refreshThread.setRefreshPeriod(_refreshPeriod);
        }
    }

    synchronized String getDateFormat()
    {
        return _dateFormat;
    }

    synchronized void setDateFormat(String dateFormat)
    {
        _dateFormat = dateFormat;
        _timeFormater.setDateFormat(dateFormat);
    }

    synchronized String getBandwithUnit()
    {
        return _bandwidthUnits[_bandwidthUnitIndex];
    }

    synchronized int getBandwidthUnitIndex()
    {
        return _bandwidthUnitIndex;
    }

    synchronized void setBandwidthUnitIndex(int index)
    {
        _bandwidthUnitIndex = index;
    }

    synchronized String getTimeUnit()
    {
        return _timeUnits[_timeUnitIndex];
    }

    synchronized int getTimeUnitIndex()
    {
        return _timeUnitIndex;
    }

    synchronized void setTimeUnitIndex(int index)
    {
        _timeUnitIndex = index;
    }

    synchronized float conversionFactor(int index)
    {
        return _type == GraphType.BandwidhGraphType ?
                    _bandwidthUnitsFactors[index] / _bandwidthUnitsFactors[_bandwidthUnitIndex] : 
                    _timeUnitsFactors[index] / _timeUnitsFactors[_timeUnitIndex];
    }

    synchronized String yAxisLabel(int index)
    {
        if(_type == GraphType.BandwidhGraphType)
        {
            return _bandwidthUnits[index];
        }
        else if(_type == GraphType.TotalAverageGraphType)
        {
            StringBuilder sb = new StringBuilder();
            sb.append("Average ");
            if(_category == GraphCategory.DispatchGraphCategory)
            {
                sb.append("dispatch ");
            }
            else
            {
                sb.append("invocation ");
            }
            sb.append("time (");
            sb.append(_timeUnits[index]);
            sb.append(")");
            return sb.toString();
        }
        else if(_category == GraphCategory.ThreadsGraphCategory)
        {
            return "Threads";
        }
        else if(_category == GraphCategory.ConnectionsGraphCategory)
        {
            return "Connections";
        }
        else
        {
            return null;
        }
    }

    synchronized int getUnitsSelectedIndex()
    {
        if(_type == GraphType.BandwidhGraphType)
        {
            return _bandwidthUnitIndex;
        }
        else if(_type == GraphType.TotalAverageGraphType)
        {
            return _timeUnitIndex;
        }
        else
        {
            return 0;
        }
    }

    //
    // Must be called on JavaFX thread
    //
    private void updateStyle(final XYChart.Series<Number, Number> series, String seriesClass)
    {
        String style = _styles.get(seriesClass);
        java.util.Set<javafx.scene.Node> nodes = _chart.lookupAll("." + seriesClass);
        for(javafx.scene.Node n : nodes)
        {
            n.setStyle(style);
        }
    }

    //
    // Must be called on JavaFX thread
    //
    private void
    seriesAdded(String name, String styleClass)
    {
        VBox vbox = new VBox(5);
        vbox.setPadding(new javafx.geometry.Insets(5, 5, 5, 5));

        Line line = new Line(0, 10, 250, 10);
        line.setStyle(_styles.get(styleClass));
        
        
        Label label = new Label(name);
        label.setPrefWidth(250);
        label.setMaxWidth(250);
        label.setWrapText(true);

        if(!_legendScroll.isVisible() && _legend.getChildren().size() == 0)
        {
            _legendScroll.setVisible(true);
            SwingUtilities.invokeLater(new Runnable()
                {
                    public void run()
                    {
                        _showLegendMenuItem.setState(true);
                    }
                });
        }
        vbox.getChildren().addAll(line, label);

        vbox.setStyle("-fx-border-color: #c0c0c0;; -fx-border-radius: 5; -fx-background-color: #f5f5f5;");
        _legend.getChildren().add(vbox);
        _legendItems.put(styleClass, vbox);
    }

    private void
    seriesRemoved(String styleClass)
    {
        VBox vbox = _legendItems.get(styleClass);
        if(vbox != null)
        {
            _legendItems.remove(styleClass);
            _legend.getChildren().remove(vbox);
            if(_legendScroll.isVisible() && _legend.getChildren().size() == 0)
            {
                _legendScroll.setVisible(false);
                SwingUtilities.invokeLater(new Runnable()
                    {
                        public void run()
                        {
                            _showLegendMenuItem.setState(false);
                        }
                    });
            }
        }
    }

    private final Coordinator _coordinator;
    private RefreshThread _refreshThread;
    private final List<MetricsViewInfo> _metrics = new ArrayList<MetricsViewInfo>();
    private Map<MetricsViewInfo, Map<MetricsViewTargetInfo, XYChart.Series<Number, Number>>> _series = 
                            new HashMap<MetricsViewInfo, Map<MetricsViewTargetInfo, XYChart.Series<Number, Number>>>();

    private Map<MetricsViewTargetInfo, DeltaInfo> _deltas = new HashMap<MetricsViewTargetInfo, DeltaInfo>();
    private Map<String, String> _styles = new HashMap<String, String>();
    private Map<String, VBox> _legendItems = new HashMap<String, VBox>();
    
    private LineChart<Number, Number> _chart;
    private ScrollPane _legendScroll;
    private VBox _legend;
    private final GraphCategory _category;
    private final GraphType _type;
    private final String _key;
    private int _horizontaSymbolsCount = 10;

    private long _refreshPeriod = 5000;

    private String[] _dateFormats = new String[]{"HH:mm:ss", "mm:ss"};
    private String _dateFormat = _dateFormats[0];

    private String[] _bandwidthUnits = new String[]{"Bytes/s", "KBytes/s", "Bytes/min", "KBytes/min"};
    private float[] _bandwidthUnitsFactors = new float[]{1000.0f, 1.0f, 60000.0f, 60.0f};
    private int _bandwidthUnitIndex = 0;

    private String[] _timeUnits = new String[]{"\u00B5s", "ms", "s"};
    private float [] _timeUnitsFactors = new float[]{1.0f, 0.001f, 0.000001f};
    private int _timeUnitIndex = 1;

    final TimeFormatter _timeFormater = new TimeFormatter();
    final Object _monitor = new Object();

    private JCheckBoxMenuItem _showLegendMenuItem;

    NumberAxis _xAxis;
    NumberAxis _yAxis;

    private final static String NodeMimeType = 
                                "application/x-java-jvm-local-objectref; class=IceGridGUI.LiveDeployment.Node";
    private final static String ServerMimeType =
                                "application/x-java-jvm-local-objectref; class=IceGridGUI.LiveDeployment.Server";
    private final static String MetricsViewMimeType = 
                                "application/x-java-jvm-local-objectref; class=IceGridGUI.LiveDeployment.MetricsView";

    private final static String[] DefaultColors = new String[]
                                                    {
                                                        "#FF0000", // Red
                                                        "#00FF00", // Lime
                                                        "#00FFFF", // Aqua
                                                        "#FFA07A", // LightSalmon
                                                        "#FFC0CB", // Pink
                                                        "#FFD700", // Gold
                                                        "#CD5C5C", // Indian red
                                                        "#32CD32", // LimeGreen
                                                        "#AFEEEE", // PaleTurquoise
                                                        "#FF4500", // OrangeRed
                                                        "#FF69B4", // HotPink
                                                        "#BDB76B", // DarkKhaki
                                                        "#8B0000", // DarkRed
                                                        "#9ACD32", // YellowGreen
                                                        "#00BFFF", // DeepSkyBlue
                                                        "#4B0082", // Indigo
                                                    };
}
