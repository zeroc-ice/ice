// Copyright (c) ZeroC, Inc.

package com.zeroc.IceGridGUI.Application;

import com.zeroc.IceGridGUI.Utils;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Vector;

import javax.swing.DefaultCellEditor;
import javax.swing.JComboBox;
import javax.swing.JTable;
import javax.swing.event.TableModelEvent;
import javax.swing.event.TableModelListener;
import javax.swing.table.DefaultTableCellRenderer;
import javax.swing.table.DefaultTableModel;
import javax.swing.table.TableCellEditor;

// A special field used to show/edit parameter values
@SuppressWarnings("unchecked")
public class ParameterValuesField extends JTable {
    public ParameterValuesField(Editor editor) {
        _editor = editor;

        _columnNames = new Vector<>(2);
        _columnNames.add("Name");
        _columnNames.add("Value");

        _useDefaultCombo.setEditable(true);
        _notSetCombo.setEditable(true);

        // Adjust row height for larger fonts
        int fontSize = getFont().getSize();
        int minRowHeight = fontSize + fontSize / 3;
        if (rowHeight < minRowHeight) {
            setRowHeight(minRowHeight);
        }
    }

    public void set(
            List<String> names,
            Map<String, String> values,
            Map<String, String> defaultValues,
            final Utils.Resolver resolver) {
        // Transform map into vector of vectors
        Vector<Vector<String>> vector = new Vector<>(names.size());

        _hasDefault = new boolean[names.size()];
        int i = 0;

        for (String name : names) {
            Vector<String> row = new Vector<>(2);
            row.add(name);

            _hasDefault[i] = defaultValues.get(name) != null;

            String val = values.get(name);
            if (val == null) {
                row.add(_hasDefault[i] ? _useDefault : _notSet);
            } else {
                row.add(Utils.substitute(val, resolver));
            }
            vector.add(row);
            i++;
        }

        _model =
            new DefaultTableModel(vector, _columnNames) {
                @Override
                public boolean isCellEditable(int row, int column) {
                    if (resolver == null) {
                        return column > 0;
                    } else {
                        return false;
                    }
                }
            };

        _model.addTableModelListener(
            new TableModelListener() {
                @Override
                public void tableChanged(TableModelEvent e) {
                    _editor.updated();
                }
            });
        setModel(_model);

        setCellSelectionEnabled(resolver == null);
        setOpaque(resolver == null);
        setPreferredScrollableViewportSize(getPreferredSize());

        DefaultTableCellRenderer cr = (DefaultTableCellRenderer) getDefaultRenderer(String.class);
        cr.setOpaque(resolver == null);
    }

    public Map<String, String> getValues() {
        Map<String, String> values = new HashMap<>();

        if (isEditing()) {
            getCellEditor().stopCellEditing();
        }

        Vector<Vector> vector = _model.getDataVector();

        for (Vector row : vector) {
            // Eliminate rows with null or empty names
            String name = row.elementAt(0).toString();
            assert name != null;

            String val = row.elementAt(1).toString();

            // Eliminate entries with default or not set value
            if (val != _useDefault && val != _notSet) {
                assert val != null;
                values.put(name, val);
            }
        }
        return values;
    }

    @Override
    public TableCellEditor getCellEditor(int row, int column) {
        if (column == 1) {
            return _hasDefault[row] ? _useDefaultEditor : _notSetEditor;
        } else {
            return super.getCellEditor(row, column);
        }
    }

    private static final String _useDefault = "Use default";

    private static final String _notSet = "Not set";

    private JComboBox _useDefaultCombo = new JComboBox(new String[]{_useDefault});

    private JComboBox _notSetCombo = new JComboBox(new String[]{_notSet});

    private TableCellEditor _useDefaultEditor = new DefaultCellEditor(_useDefaultCombo);
    private TableCellEditor _notSetEditor = new DefaultCellEditor(_notSetCombo);

    private DefaultTableModel _model;
    private Vector<String> _columnNames;
    private Editor _editor;

    private boolean[] _hasDefault;
}
