// Copyright (c) ZeroC, Inc.

package com.zeroc.IceGridGUI;

import com.zeroc.IceGrid.PropertyDescriptor;

import java.awt.Component;
import java.awt.Dimension;
import java.awt.Frame;
import java.awt.Graphics2D;
import java.awt.GraphicsDevice;
import java.awt.GraphicsEnvironment;
import java.awt.Image;
import java.awt.Rectangle;
import java.awt.Window;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.awt.image.BufferedImage;
import java.net.URL;
import java.util.Arrays;
import java.util.Collection;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.SortedMap;
import java.util.TreeMap;
import java.util.prefs.BackingStoreException;
import java.util.prefs.Preferences;

import javax.swing.Icon;
import javax.swing.ImageIcon;
import javax.swing.JComponent;
import javax.swing.JDialog;
import javax.swing.KeyStroke;

public class Utils {
    public static ImageIcon getIcon(String path) {
        URL imgURL = Utils.class.getResource(path);
        if (imgURL == null) {
            System.err.println("Could not find icon " + path);
            return null;
        } else {
            return new ImageIcon(imgURL);
        }
    }

    public static Image iconToImage(Icon icon) {
        if (icon instanceof ImageIcon) {
            return ((ImageIcon) icon).getImage();
        } else {
            Graphics2D g = null;
            try {
                BufferedImage image =
                    GraphicsEnvironment.getLocalGraphicsEnvironment()
                        .getDefaultScreenDevice()
                        .getDefaultConfiguration()
                        .createCompatibleImage(icon.getIconWidth(), icon.getIconHeight());
                g = image.createGraphics();
                icon.paintIcon(null, g, 0, 0);
                return image;
            } finally {
                if (g != null) {
                    g.dispose();
                }
            }
        }
    }

    public static void addEscapeListener(final JDialog dialog) {
        dialog.getRootPane()
            .registerKeyboardAction(
                new ActionListener() {
                    @Override
                    public void actionPerformed(ActionEvent e) {
                        dialog.dispose();
                    }
                },
                KeyStroke.getKeyStroke(KeyEvent.VK_ESCAPE, 0),
                JComponent.WHEN_IN_FOCUSED_WINDOW);
    }

    public static void removeEscapeListener(JDialog dialog) {
        dialog.getRootPane()
            .unregisterKeyboardAction(KeyStroke.getKeyStroke(KeyEvent.VK_ESCAPE, 0));
    }

    // Extract Ice version in the form XXYYZZ, e.g.  030201 (for 3.2.1)
    //      0 == empty string
    //     -1 == error
    public static int getIntVersion(String version) {
        int result = 0;
        version = version.trim();
        if (!version.isEmpty()) {
            try {
                int firstDotPos = version.indexOf('.');

                if (firstDotPos == -1) {
                    result = -1;
                } else {
                    result = Integer.parseInt(version.substring(0, firstDotPos));
                    if (result == 0) {
                        return -1;
                    }
                    result *= 100;

                    int secondDotPos = version.indexOf('.', firstDotPos + 1);
                    if (secondDotPos == -1) {
                        result += Integer.parseInt(version.substring(firstDotPos + 1));
                        result *= 100;
                    } else {
                        result +=
                            Integer.parseInt(version.substring(firstDotPos + 1, secondDotPos));
                        result *= 100;
                        result += Integer.parseInt(version.substring(secondDotPos + 1));
                    }
                }
            } catch (NumberFormatException e) {
                result = -1;
            }
        }
        return result;
    }

    public static void storeWindowBounds(
            Window window, Preferences prefs) {
        if (window instanceof Frame) {
            Frame frame = (Frame) window;
            boolean maximized = frame.getExtendedState() == Frame.MAXIMIZED_BOTH;
            prefs.putBoolean("maximized", maximized);
        }

        Rectangle rect = window.getBounds();
        prefs.putInt("x", rect.x);
        prefs.putInt("y", rect.y);
        prefs.putInt("width", rect.width);
        prefs.putInt("height", rect.height);
    }

    public static Preferences restoreWindowBounds(
            Window window,
            Preferences parent,
            String node,
            Component parentComponent) {
        Preferences prefs = null;

        try {
            if (parent.nodeExists(node)) {
                prefs = parent.node(node);
            }
        } catch (BackingStoreException ex) {}

        boolean locationVisible = false;

        if (prefs != null) {
            int x = prefs.getInt("x", 0);
            int y = prefs.getInt("y", 0);
            int width = prefs.getInt("width", 0);
            int height = prefs.getInt("height", 0);
            boolean maximized = prefs.getBoolean("maximized", false);

            Rectangle visibleBounds = new Rectangle();
            GraphicsEnvironment ge = GraphicsEnvironment.getLocalGraphicsEnvironment();
            GraphicsDevice[] screens = ge.getScreenDevices();
            for (GraphicsDevice s : screens) {
                visibleBounds.add(s.getDefaultConfiguration().getBounds());
            }
            locationVisible =
                visibleBounds.contains(x, y)
                    || (maximized && visibleBounds.contains(x + 20, y + 20));

            if (locationVisible) {
                if (maximized) {
                    Frame frame = (Frame) window;
                    frame.setBounds(new Rectangle(x + 20, y + 20, width, height));
                    frame.setExtendedState(Frame.MAXIMIZED_BOTH);
                } else {
                    window.setBounds(new Rectangle(x, y, width, height));
                }

            } else {
                window.setSize(width, height);
            }
        }

        if (!locationVisible) {
            if (parentComponent != null) {
                Dimension parentSize = parentComponent.getSize();
                Dimension thisSize = window.getSize();
                if (parentSize.width < thisSize.width || parentSize.height < thisSize.height) {
                    window.setLocationRelativeTo(null);
                } else {
                    window.setLocationRelativeTo(parentComponent);
                }
            } else {
                window.setLocation(100, 100);
            }
        }

        return prefs;
    }

    public static interface Stringifier {
        String toString(Object obj);
    }

    public static class StringifyResult {
        public String returnValue;
        public String toolTip;
    }

    // Stringify helpers
    public static StringifyResult stringify(
            Collection<?> col, Stringifier stringifier, String separator) {
        StringifyResult r = new StringifyResult();
        r.returnValue = "";

        Iterator<?> p = col.iterator();

        boolean firstElement = true;
        while (p.hasNext()) {
            String elt = stringifier.toString(p.next());
            if (elt != null) {
                if (firstElement) {
                    firstElement = false;
                    r.toolTip = "<html>";
                } else {
                    r.returnValue += separator;
                    r.toolTip += "<br>";
                }

                if (elt.isEmpty()) {
                    r.returnValue += "\"\"";
                } else if (elt.matches("\\S*")) {
                    // Only non-whitespace characters
                    r.returnValue += elt;
                } else {
                    r.returnValue += '"' + elt + '"';
                }

                r.toolTip += elt;
            }
        }
        if (r.toolTip != null) {
            r.toolTip += "</html>";
        }

        return r;
    }

    public static StringifyResult stringify(Collection<?> col, String separator) {
        Stringifier stringifier =
            new Stringifier() {
                @Override
                public String toString(Object obj) {
                    return (String) obj;
                }
            };
        return stringify(col, stringifier, separator);
    }

    public static StringifyResult stringify(String[] stringSeq, String separator) {
        return stringify(Arrays.asList(stringSeq), separator);
    }

    public static StringifyResult stringify(
            Map<String, String> stringMap, final String pairSeparator, String separator) {
        Stringifier stringifier =
            new Stringifier() {
                @Override
                public String toString(Object obj) {
                    @SuppressWarnings("unchecked")
                    Map.Entry<String, String> entry =
                        (Map.Entry<String, String>) obj;
                    return entry.getKey() + pairSeparator + entry.getValue();
                }
            };

        return stringify(stringMap.entrySet(), stringifier, separator);
    }

    public static class Resolver {
        // Application-level resolver
        @SuppressWarnings("unchecked") // unchecked conversion
        public Resolver(Map<String, String> variables) {
            this(new Map[]{variables});
        }

        public Resolver(Map<String, String>[] variables) {
            _variables = variables;
            _predefinedVariables = new HashMap<>();

            _parameters = null;
            _subResolver = this;
        }

        // Resolver for instance; in-parameters are not yet substituted
        public Resolver(
                Resolver parent,
                Map<String, String> parameters,
                Map<String, String> defaults) {
            _variables = parent._variables;

            // Whenever the parent's predefined variables change, the resolver must be recreated
            reset(parent, parameters, defaults);
        }

        // Resolver for plain server or service
        public Resolver(Resolver parent) {
            _variables = parent._variables;
            _predefinedVariables = new HashMap<>(parent._predefinedVariables);
            _parameters = parent._parameters;
            if (_parameters == null) {
                _subResolver = this;
            } else {
                _subResolver = new Resolver(_variables, _predefinedVariables);
            }
        }

        private Resolver(
                Map<String, String>[] variables,
                Map<String, String> predefinedVariables) {
            _variables = variables;
            _predefinedVariables = predefinedVariables;

            _parameters = null;
            _subResolver = this;
        }

        public String find(String name) {
            if (_parameters != null) {
                String val = _parameters.get(name);
                if (val != null) {
                    return val;
                }
            }

            String val = _predefinedVariables.get(name);
            if (val != null) {
                return val;
            }

            for (Map<String, String> map : _variables) {
                val = map.get(name);
                if (val != null) {
                    return _subResolver.substitute(val);
                }
            }
            return null;
        }

        // Set a pre-defined variable; returns true if value was updated
        public boolean put(String name, String value) {
            String oldVal = _predefinedVariables.get(name);
            if (oldVal == null || !oldVal.equals(value)) {
                _predefinedVariables.put(name, value);
                return true;
            } else {
                return false;
            }
        }

        // Reset parameters and pre-defined variables
        public void reset(
                Resolver parent,
                Map<String, String> parameters,
                Map<String, String> defaults) {
            assert _variables == parent._variables;
            _predefinedVariables = new HashMap<>(parent._predefinedVariables);

            _parameters = parent.substituteParameterValues(parameters, defaults);
            _subResolver = new Resolver(_variables, _predefinedVariables);
        }

        public void reset(Resolver parent) {
            assert _variables == parent._variables;
            _predefinedVariables = new HashMap<>(parent._predefinedVariables);

            assert _parameters == parent._parameters;
            if (_parameters == null) {
                _subResolver = this;
            } else {
                _subResolver = new Resolver(_variables, _predefinedVariables);
            }
        }

        // The sorted substituted parameters
        public Map<String, String> getParameters() {
            return _parameters;
        }

        public String substitute(String input) {
            if (input == null) {
                return input;
            }

            int beg = 0;
            int end = 0;

            while ((beg = input.indexOf("${", beg)) != -1) {
                if (beg > 0 && input.charAt(beg - 1) == '$') {
                    int escape = beg - 1;
                    while (escape > 0 && input.charAt(escape - 1) == '$') {
                        --escape;
                    }

                    input = input.substring(0, escape) + input.substring(beg - (beg - escape) / 2);
                    if ((beg - escape) % 2 != 0) {
                        ++beg;
                        continue;
                    } else {
                        beg -= (beg - escape) / 2;
                    }
                }

                end = input.indexOf('}', beg);
                if (end == -1) {
                    // Malformed variable, can't substitute anything else
                    return input;
                }

                String name = input.substring(beg + 2, end);

                // Resolve name
                String val = find(name);
                if (val != null) {
                    input = input.substring(0, beg) + val + input.substring(end + 1);
                    beg += val.length();
                } else {
                    // No substitution, keep ${name} in the result
                    ++beg;
                }
            }
            return input;
        }

        // Substitute all the values from the input map
        public Map<String, String> substituteParameterValues(
                Map<String, String> input, Map<String, String> defaults) {
            Map<String, String> result = new HashMap<>();
            for (Map.Entry<String, String> p : input.entrySet()) {
                result.put(p.getKey(), substitute(p.getValue()));
            }
            for (Map.Entry<String, String> p : defaults.entrySet()) {
                if (!result.containsKey(p.getKey())) {
                    result.put(p.getKey(), substitute(p.getValue()));
                }
            }
            return result;
        }

        private Map<String, String>[] _variables;
        private Map<String, String> _parameters;
        private Map<String, String> _predefinedVariables;

        private Resolver _subResolver;
    }

    public static String substitute(String input, Resolver resolver) {
        if (resolver != null) {
            return resolver.substitute(input);
        } else {
            return input;
        }
    }

    // An expanded property set (i.e. containing other property sets)
    public static class ExpandedPropertySet {
        public ExpandedPropertySet[] references;
        public List<PropertyDescriptor> properties; // list of PropertyDescriptor
    }

    public static SortedMap<String, String> propertySetsToMap(
            List<ExpandedPropertySet> propertySets, Resolver resolver) {
        SortedMap<String, String> toMap = new TreeMap<>();
        for (ExpandedPropertySet p : propertySets) {
            addSet(p, resolver, toMap);
        }
        return toMap;
    }

    public static SortedMap<String, String> propertySetToMap(
            ExpandedPropertySet propertySet, Resolver resolver) {
        List<ExpandedPropertySet> list = new LinkedList<>();
        list.add(propertySet);
        return propertySetsToMap(list, resolver);
    }

    private static void addSet(
            ExpandedPropertySet set, Resolver resolver, SortedMap<String, String> toMap) {
        for (ExpandedPropertySet s : set.references) {
            addSet(s, resolver, toMap);
        }

        for (PropertyDescriptor p : set.properties) {
            String name = substitute(p.name, resolver);
            String val = substitute(p.value, resolver);
            toMap.put(name, val);
        }
    }

    private Utils() {}
}
