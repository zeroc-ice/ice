// Copyright (c) ZeroC, Inc.

package com.zeroc.IceGridGUI.Application;

import java.awt.event.ActionEvent;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Vector;

import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.DefaultCellEditor;
import javax.swing.JComboBox;
import javax.swing.JTable;
import javax.swing.KeyStroke;
import javax.swing.event.TableModelEvent;
import javax.swing.event.TableModelListener;
import javax.swing.table.DefaultTableModel;
import javax.swing.table.TableCellEditor;
import javax.swing.table.TableColumn;

/** A special field used to show/edit parameters */
@SuppressWarnings("unchecked")
public class ParametersField extends JTable {
    public ParametersField(Editor editor) {
        _editor = editor;

        _columnNames = new Vector<>(2);
        _columnNames.add("Name");
        _columnNames.add("Default value");

        JComboBox comboBox = new JComboBox(new String[]{_noDefault});
        comboBox.setEditable(true);
        _cellEditor = new DefaultCellEditor(comboBox);

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
            };
        getActionMap().put("delete", deleteRow);
        getInputMap().put(KeyStroke.getKeyStroke("DELETE"), "delete");
    }

    public void set(List<String> names, Map<String, String> values) {
        // Transform map into vector of vectors
        Vector<Vector<String>> vector = new Vector<>(names.size());
        for (String name : names) {
            Vector<String> row = new Vector<>(2);

            row.add(name);

            String val = values.get(name);
            if (val == null) {
                row.add(_noDefault);
            } else {
                row.add(val);
            }
            vector.add(row);
        }

        Vector<String> newRow = new Vector<>(2);
        newRow.add("");
        newRow.add(_noDefault);
        vector.add(newRow);

        _model = new DefaultTableModel(vector, _columnNames);

        _model.addTableModelListener(
            new TableModelListener() {
                @Override
                public void tableChanged(TableModelEvent e) {
                    Object lastKey = _model.getValueAt(_model.getRowCount() - 1, 0);
                    if (lastKey != null && !"".equals(lastKey)) {
                        _model.addRow(new Object[]{"", _noDefault});
                    }

                    _editor.updated();
                }
            });
        setModel(_model);

        TableColumn valColumn = getColumnModel().getColumn(1);
        valColumn.setCellEditor(_cellEditor);

        setPreferredScrollableViewportSize(getPreferredSize());
    }

    public Map<String, String> get(List<String> names) {
        assert names != null;

        Map<String, String> values = new HashMap<>();

        if (isEditing()) {
            getCellEditor().stopCellEditing();
        }
        @SuppressWarnings("unchecked")
        Vector<Vector> vector = _model.getDataVector();

        for (Vector row : vector) {
            // Eliminate rows with null or empty names
            String name = row.elementAt(0).toString();
            if (name != null) {
                name = name.trim();

                if (!name.isEmpty()) {
                    names.add(name);

                    String val = row.elementAt(1).toString();

                    // Eliminate entries with "default" value
                    if (val != _noDefault) {
                        assert val != null;
                        values.put(name, val);
                    }
                }
            }
        }
        return values;
    }

    private final String _noDefault = "No default";

    private DefaultTableModel _model;
    private Vector<String> _columnNames;
    private Editor _editor;
    private TableCellEditor _cellEditor;
}
