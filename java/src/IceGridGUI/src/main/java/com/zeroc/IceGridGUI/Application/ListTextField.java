// Copyright (c) ZeroC, Inc.

package com.zeroc.IceGridGUI.Application;

import com.zeroc.IceGridGUI.Utils;

import java.util.LinkedList;
import java.util.List;

import javax.swing.JTextField;

/** A special field used to show/edit a list of strings separated by whitespace */
public class ListTextField extends JTextField {
    public ListTextField(int columns) {
        super(columns);
    }

    public void setList(List<String> list, final Utils.Resolver resolver) {
        Utils.Stringifier stringifier =
            new Utils.Stringifier() {
                @Override
                public String toString(Object obj) {
                    return Utils.substitute((String) obj, resolver);
                }
            };

        setText(Utils.stringify(list, stringifier, " ").returnValue);
    }

    public LinkedList<String> getList() {
        String text = getText().trim();
        LinkedList<String> result = new LinkedList<>();

        while (!text.isEmpty()) {
            if (text.startsWith("\"")) {
                int last = text.indexOf("\"", 1);
                if (last == -1) {
                    result.add(text.substring(1));
                    text = "";
                } else {
                    result.add(text.substring(1, last));
                    text = text.substring(last + 1).trim();
                }
            } else {
                String[] strings = text.split("\\s", 2);
                if (strings.length == 1) {
                    result.add(strings[0]);
                    text = "";
                } else {
                    result.add(strings[0]);
                    text = strings[1].trim();
                }
            }
        }
        return result;
    }
}
