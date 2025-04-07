// Copyright (c) ZeroC, Inc.

package com.zeroc.IceGridGUI.Application;

import com.zeroc.Ice.ToStringMode;
import com.zeroc.Ice.Util;
import com.zeroc.IceGrid.*;
import com.zeroc.IceGridGUI.*;

import java.awt.event.ActionEvent;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.List;
import java.util.Set;
import java.util.Vector;

import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.JTable;
import javax.swing.KeyStroke;
import javax.swing.event.TableModelEvent;
import javax.swing.event.TableModelListener;
import javax.swing.table.DefaultTableCellRenderer;
import javax.swing.table.DefaultTableModel;

/** A special field used to show/edit properties */
public class PropertiesField extends JTable {
    public PropertiesField(Editor editor) {
        _columnNames = new Vector<>(2);
        _columnNames.add("Name");
        _columnNames.add("Value");

        _editor = editor;

        // Adjust row height for larger fonts
        int fontSize = getFont().getSize();
        int minRowHeight = fontSize + fontSize / 3;
        if (rowHeight < minRowHeight) {
            setRowHeight(minRowHeight);
        }

        Action deleteRow =
                new AbstractAction("Delete selected row(s)") {
                    @Override
                    public void actionPerformed(ActionEvent e) {
                        if (_editable) {
                            if (isEditing()) {
                                getCellEditor().stopCellEditing();
                            }

                            for (; ; ) {
                                int selectedRow = getSelectedRow();
                                if (selectedRow == -1) {
                                    break;
                                } else {
                                    _model.removeRow(selectedRow);
                                }
                            }
                        }
                    }
                };
        getActionMap().put("delete", deleteRow);
        getInputMap().put(KeyStroke.getKeyStroke("DELETE"), "delete");
    }

    public void setProperties(
            List<PropertyDescriptor> properties,
            List<AdapterDescriptor> adapters,
            String[] logs,
            Utils.Resolver resolver,
            boolean editable) {
        _editable = editable;

        // We don't show the .Endpoint and .PublishedEndpoints of adapters,
        // since they already appear in the Adapter pages
        Set<String> hiddenPropertyNames = new HashSet<>();

        // We also hide properties whose value match an object or allocatable
        Set<String> hiddenPropertyValues = new HashSet<>();

        _hiddenProperties.clear();

        if (adapters != null) {
            // Note that we don't substitute *on purpose*, i.e. the names or values must match
            // before substitution.
            for (AdapterDescriptor p : adapters) {
                hiddenPropertyNames.add(p.name + ".Endpoints");
                hiddenPropertyNames.add(p.name + ".PublishedEndpoints");
                hiddenPropertyNames.add(p.name + ".ProxyOptions");

                for (ObjectDescriptor q : p.objects) {
                    hiddenPropertyValues.add(
                            Util.identityToString(
                                    q.id, ToStringMode.Unicode));
                }
                for (ObjectDescriptor q : p.allocatables) {
                    hiddenPropertyValues.add(
                            Util.identityToString(
                                    q.id, ToStringMode.Unicode));
                }
            }
        }

        if (logs != null) {
            for (String log : logs) {
                hiddenPropertyValues.add(log);
            }
        }

        Vector<Vector<String>> vector =
                new Vector<>(properties.size());
        for (PropertyDescriptor p : properties) {
            if (hiddenPropertyNames.contains(p.name)) {
                // We keep them at the top of the list
                if (_editable) {
                    _hiddenProperties.add(p);
                }

                // We hide only the first occurrence
                hiddenPropertyNames.remove(p.name);
            } else if (hiddenPropertyValues.contains(p.value)) {
                // We keep them at the top of the list
                if (_editable) {
                    _hiddenProperties.add(p);
                }

                // We hide only the first occurrence
                hiddenPropertyValues.remove(p.value);
            } else {
                Vector<String> row = new Vector<>(2);
                row.add(Utils.substitute(p.name, resolver));
                row.add(Utils.substitute(p.value, resolver));
                vector.add(row);
            }
        }

        if (_editable) {
            Vector<String> newRow = new Vector<>(2);
            newRow.add("");
            newRow.add("");
            vector.add(newRow);
        }

        _model =
                new DefaultTableModel(vector, _columnNames) {
                    @Override
                    public boolean isCellEditable(int row, int column) {
                        return _editable;
                    }
                };

        _model.addTableModelListener(
                new TableModelListener() {
                    @Override
                    public void tableChanged(TableModelEvent e) {
                        if (_editable) {
                            Object lastKey = _model.getValueAt(_model.getRowCount() - 1, 0);
                            if (lastKey != null && !"".equals(lastKey)) {
                                _model.addRow(new Object[]{"", ""});
                            }
                            _editor.updated();
                        }
                    }
                });
        setModel(_model);

        setCellSelectionEnabled(_editable);
        setOpaque(_editable);
        setPreferredScrollableViewportSize(getPreferredSize());

        DefaultTableCellRenderer cr = (DefaultTableCellRenderer) getDefaultRenderer(String.class);
        cr.setOpaque(_editable);
    }

    public LinkedList<PropertyDescriptor> getProperties() {
        assert _editable;

        if (isEditing()) {
            getCellEditor().stopCellEditing();
        }
        @SuppressWarnings("unchecked")
        Vector<Vector> vector = _model.getDataVector();

        LinkedList<PropertyDescriptor> result =
                new LinkedList<>(_hiddenProperties);

        for (Vector row : vector) {
            // Eliminate rows with null or empty keys
            String key = row.elementAt(0).toString();
            if (key != null) {
                key = key.trim();
                if (!key.isEmpty()) {
                    String val = row.elementAt(1).toString();
                    if (val == null) {
                        val = "";
                    }

                    result.add(new PropertyDescriptor(key, val));
                }
            }
        }
        return result;
    }

    private DefaultTableModel _model;
    private final Vector<String> _columnNames;
    private boolean _editable;

    private final LinkedList<PropertyDescriptor> _hiddenProperties =
            new LinkedList<>();

    private final Editor _editor;
}
