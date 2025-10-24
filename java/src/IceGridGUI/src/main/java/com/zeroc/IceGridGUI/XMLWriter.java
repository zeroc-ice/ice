// Copyright (c) ZeroC, Inc.

package com.zeroc.IceGridGUI;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.io.UnsupportedEncodingException;
import java.io.Writer;
import java.util.List;

/** Helper class to write XML files */
public class XMLWriter {
    public XMLWriter(File file) throws FileNotFoundException, IOException {
        try {
            _writer = new BufferedWriter(new OutputStreamWriter(new FileOutputStream(file), "UTF-8"));
        } catch (UnsupportedEncodingException e) {
            assert false;
        }
        _writer.write("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>" + _newline);
        _writer.write("<!-- This file was written by IceGrid GUI -->" + _newline);
    }

    public void writeElement(String name, List<String[]> attributes) throws IOException {
        _writer.write(_indent);
        _writer.write("<" + name);
        writeAttributes(attributes);
        _writer.write("/>" + _newline);
    }

    public void writeElement(String name) throws IOException {
        writeElement(name, (List<String[]>) null);
    }

    public void writeElement(String name, String content) throws IOException {
        _writer.write(_indent);
        _writer.write("<" + name + ">" + escape(content) + "</" + name + ">" + _newline);
    }

    public void writeStartTag(String name, List<String[]> attributes) throws IOException {
        _writer.write(_indent);
        _writer.write("<" + name);
        writeAttributes(attributes);
        _writer.write(">");
        _writer.write(_newline);
        increaseIndent();
    }

    public void writeStartTag(String name) throws IOException {
        writeStartTag(name, null);
    }

    public void writeEndTag(String name) throws IOException {
        decreaseIndent();
        _writer.write(_indent);
        _writer.write("</" + name + ">" + _newline);
    }

    public void close() throws IOException {
        _writer.close();
    }

    public void flush() throws IOException {
        _writer.flush();
    }

    private void writeAttributes(List<String[]> attributes) throws IOException {
        if (attributes != null) {
            for (String[] p : attributes) {
                _writer.write(" " + p[0] + "=\"" + escape(p[1]) + "\"");
            }
        }
    }

    private void increaseIndent() {
        _indent += "   ";
    }

    private void decreaseIndent() {
        if (!_indent.isEmpty()) {
            _indent = _indent.substring(3);
        }
    }

    private String escape(String input) {
        String v = input;

        // Find out whether there is a reserved character to avoid conversion if not necessary.
        final String allReserved = "<>'\"&";
        boolean hasReserved = false;
        char[] arr = input.toCharArray();
        for (char c : arr) {
            if (allReserved.indexOf(c) != -1) {
                hasReserved = true;
                break;
            }
        }
        if (hasReserved) {
            // First convert all '&' to '&amp';
            if (v.indexOf('&') != -1) {
                v = v.replaceAll("&", "&amp;");
            }

            // Next convert remaining reserved characters.
            if (v.indexOf('>') != -1) {
                v = v.replaceAll(">", "&gt;");
            }
            if (v.indexOf('<') != -1) {
                v = v.replaceAll("<", "&lt;");
            }
            if (v.indexOf('\'') != -1) {
                v = v.replaceAll("'", "&apos;");
            }
            if (v.indexOf('"') != -1) {
                v = v.replaceAll("\"", "&quot;");
            }
        }
        return v;
    }

    private Writer _writer;
    private String _indent = "";
    private static final String _newline = System.getProperty("line.separator");
}
