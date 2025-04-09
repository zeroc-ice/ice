// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PushbackInputStream;
import java.io.StringWriter;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;

/**
 * A property set used to configure Ice and Ice applications. Properties are key/value pairs, with
 * both keys and values being strings. By convention, property keys should have the form
 * <em>application-name</em>[.<em>category</em>[.<em>sub-category</em>]].<em>name</em>.
 */
public final class Properties {
    static class PropertyValue {
        public PropertyValue(String v, boolean u) {
            value = v;
            used = u;
        }

        public PropertyValue clone() {
            return new PropertyValue(value, used);
        }

        public String value;
        public boolean used;
    }

    /** Creates a new empty property set. */
    public Properties() {
    }

    /**
     * Creates a property set initialized with a list of opt-in prefixes.
     *
     * @param optInPrefixes A list of prefixes that are opt-in.
     */
    public Properties(List<String> optInPrefixes) {
        _optInPrefixes.addAll(optInPrefixes);
    }

    /**
     * Creates a property set initialized from an argument vector.
     *
     * @param args A command-line argument vector, possibly containing options to set properties. If
     *     the command-line options include a <code>--Ice.Config</code> option, the corresponding
     *     configuration files are parsed. If the same property is set in a configuration file and
     *     in the argument vector, the argument vector takes precedence.
     */
    public Properties(String[] args) {
        this(args, null, null);
    }

    /**
     * Creates a property set initialized from an argument vector and return the remaining
     * arguments.
     *
     * @param args A command-line argument vector, possibly containing options to set properties. If
     *     the command-line options include a <code>--Ice.Config</code> option, the corresponding
     *     configuration files are parsed. If the same property is set in a configuration file and
     *     in the argument vector, the argument vector takes precedence.
     * @param remainingArgs If non null, the given list will contain on return the command-line
     *     arguments that were not used to set properties.
     */
    public Properties(String[] args, List<String> remainingArgs) {
        this(args, null, remainingArgs);
    }

    /**
     * Creates a property set initialized from an argument vector.
     *
     * @param args A command-line argument vector, possibly containing options to set properties. If
     *     the command-line options include a <code>--Ice.Config</code> option, the corresponding
     *     configuration files are parsed. If the same property is set in a configuration file and
     *     in the argument vector, the argument vector takes precedence.
     * @param defaults Default values for the property set. Settings in configuration files and
     *     <code>
     *     args</code> override these defaults.
     */
    public Properties(String[] args, Properties defaults) {
        this(args, defaults, null);
    }

    /**
     * Creates a property set initialized from an argument vector and return the remaining
     * arguments.
     *
     * @param args A command-line argument vector, possibly containing options to set properties. If
     *     the command-line options include a <code>--Ice.Config</code> option, the corresponding
     *     configuration files are parsed. If the same property is set in a configuration file and
     *     in the argument vector, the argument vector takes precedence.
     * @param defaults Default values for the property set. Settings in configuration files and
     *     <code>
     *     args</code> override these defaults.
     * @param remainingArgs If non null, the given list will contain on return the command-line
     *     arguments that were not used to set properties.
     */
    public Properties(String[] args, Properties defaults, List<String> remainingArgs) {
        if (defaults != null) {
            //
            // NOTE: we can't just do a shallow copy of the map as the map values
            // would otherwise be shared between the two Properties object.
            //
            for (Map.Entry<String, PropertyValue> p : defaults._properties.entrySet()) {
                _properties.put(p.getKey(), p.getValue().clone());
            }

            _optInPrefixes.addAll(defaults._optInPrefixes);
        }

        boolean loadConfigFiles = false;

        for (int i = 0; i < args.length; i++) {
            if (args[i].startsWith("--Ice.Config")) {
                String line = args[i];
                if (line.indexOf('=') == -1) {
                    line += "=1";
                }
                parseLine(line.substring(2));
                loadConfigFiles = true;

                String[] arr = new String[args.length - 1];
                System.arraycopy(args, 0, arr, 0, i);
                if (i < args.length - 1) {
                    System.arraycopy(args, i + 1, arr, i, args.length - i - 1);
                }
                args = arr;
            }
        }

        if (!loadConfigFiles) {
            //
            // If Ice.Config is not set, load from ICE_CONFIG (if set)
            //
            loadConfigFiles = !_properties.containsKey("Ice.Config");
        }

        if (loadConfigFiles) {
            loadConfig();
        }

        args = parseIceCommandLineOptions(args);
        if (remainingArgs != null) {
            remainingArgs.clear();
            if (args.length > 0) {
                remainingArgs.addAll(Arrays.asList(args));
            }
        }
    }

    /**
     * Get a property by key. If the property is not set, an empty string is returned.
     *
     * @param key The property key.
     * @return The property value.
     * @see #setProperty
     */
    public synchronized String getProperty(String key) {
        PropertyValue pv = _properties.get(key);
        if (pv != null) {
            pv.used = true;
            return pv.value;
        } else {
            return "";
        }
    }

    /**
     * Get an Ice property by key. If the property is not set, its default value is returned.
     *
     * @param key The property key.
     * @return The property value or the default value.
     * @see #setProperty
     */
    public synchronized String getIceProperty(String key) {
        PropertyValue pv = _properties.get(key);
        if (pv != null) {
            pv.used = true;
            return pv.value;
        } else {
            return getDefaultProperty(key);
        }
    }

    /**
     * Get a property by key. If the property is not set, the given default value is returned.
     *
     * @param key The property key.
     * @param value The default value to use if the property does not exist.
     * @return The property value or the default value.
     * @see #setProperty
     */
    public synchronized String getPropertyWithDefault(String key, String value) {
        PropertyValue pv = _properties.get(key);
        if (pv != null) {
            pv.used = true;
            return pv.value;
        } else {
            return value;
        }
    }

    /**
     * Get a property as an integer. If the property is not set, 0 is returned.
     *
     * @param key The property key.
     * @return The property value interpreted as an integer.
     * @throws PropertyException Raised if the property value is not a valid integer.
     * @see #setProperty
     */
    public int getPropertyAsInt(String key) {
        return getPropertyAsIntWithDefault(key, 0);
    }

    /**
     * Get an Ice property as an integer. If the property is not set, its default value is returned.
     *
     * @param key The property key.
     * @return The property value interpreted as an integer, or the default value.
     * @throws PropertyException Raised if the property is not a known Ice property or the value is
     *     not a valid integer.
     * @see #setProperty
     */
    public synchronized int getIcePropertyAsInt(String key) {
        String defaultValueString = getDefaultProperty(key);
        int defaultValue = 0;
        if (defaultValueString != "") {
            defaultValue = Integer.parseInt(defaultValueString);
        }

        return getPropertyAsIntWithDefault(key, defaultValue);
    }

    /**
     * Get a property as an integer. If the property is not set, the given default value is
     * returned.
     *
     * @param key The property key.
     * @param value The default value to use if the property does not exist.
     * @return The property value interpreted as an integer, or the default value.
     * @throws PropertyException Raised if the property value is not a valid integer.
     * @see #setProperty
     */
    public synchronized int getPropertyAsIntWithDefault(String key, int value) {
        PropertyValue pv = _properties.get(key);
        if (pv != null) {
            pv.used = true;

            try {
                return Integer.parseInt(pv.value);
            } catch (NumberFormatException ex) {
                throw new PropertyException(
                    "property '" + key + "' has an invalid integer value: '" + pv.value + "'");
            }
        }

        return value;
    }

    /**
     * Get a property as a list of strings. The strings must be separated by whitespace or comma. If
     * the property is not set, an empty list is returned. The strings in the list can contain
     * whitespace and commas if they are enclosed in single or double quotes. If quotes are
     * mismatched, an empty list is returned. Within single quotes or double quotes, you can escape
     * the quote in question with a backslash, e.g. O'Reilly can be written as O'Reilly, "O'Reilly"
     * or 'O\'Reilly'.
     *
     * @param key The property key.
     * @return The property value interpreted as a list of strings.
     * @see #setProperty
     */
    public String[] getPropertyAsList(String key) {
        return getPropertyAsListWithDefault(key, null);
    }

    /**
     * Get an Ice property as a list of strings. The strings must be separated by whitespace or
     * comma. If the property is not set, its default list is returned. The strings in the list can
     * contain whitespace and commas if they are enclosed in single or double quotes. If quotes are
     * mismatched, the default list is returned. Within single quotes or double quotes, you can
     * escape the quote in question with a backslash, e.g. O'Reilly can be written as O'Reilly,
     * "O'Reilly" or 'O\'Reilly'.
     *
     * @param key The property key.
     * @return The property value interpreted as list of strings, or the default value.
     * @see #setProperty
     */
    public synchronized String[] getIcePropertyAsList(String key) {
        String[] defaultList = StringUtil.splitString(getDefaultProperty(key), ", \t\r\n");
        return getPropertyAsListWithDefault(key, defaultList);
    }

    /**
     * Get a property as a list of strings. The strings must be separated by whitespace or comma. If
     * the property is not set, the default list is returned. The strings in the list can contain
     * whitespace and commas if they are enclosed in single or double quotes. If quotes are
     * mismatched, the default list is returned. Within single quotes or double quotes, you can
     * escape the quote in question with a backslash, e.g. O'Reilly can be written as O'Reilly,
     * "O'Reilly" or 'O\'Reilly'.
     *
     * @param key The property key.
     * @param value The default value to use if the property is not set.
     * @return The property value interpreted as list of strings, or the default value.
     * @see #setProperty
     */
    public synchronized String[] getPropertyAsListWithDefault(String key, String[] value) {
        if (value == null) {
            value = new String[0];
        }

        PropertyValue pv = _properties.get(key);
        if (pv != null) {
            pv.used = true;

            String[] result = StringUtil.splitString(pv.value, ", \t\r\n");
            if (result == null) {
                Util.getProcessLogger()
                    .warning(
                        "mismatched quotes in property "
                            + key
                            + "'s value, returning default value");
                return value;
            }
            if (result.length == 0) {
                result = value;
            }
            return result;
        } else {
            return value;
        }
    }

    /**
     * Get all properties whose keys begins with <em>prefix</em>. If <em>prefix</em> is an empty
     * string, then all properties are returned.
     *
     * @param prefix The prefix to search for (empty string if none).
     * @return The matching property set.
     */
    public synchronized Map<String, String> getPropertiesForPrefix(String prefix) {
        HashMap<String, String> result = new HashMap<>();
        for (Map.Entry<String, PropertyValue> p : _properties.entrySet()) {
            String key = p.getKey();
            if (prefix.isEmpty() || key.startsWith(prefix)) {
                PropertyValue pv = p.getValue();
                pv.used = true;
                result.put(key, pv.value);
            }
        }
        return result;
    }

    /**
     * Set a property. To unset a property, set it to the empty string.
     *
     * @param key The property key.
     * @param value The property value.
     * @see #getProperty
     */
    public void setProperty(String key, String value) {
        //
        // Trim whitespace
        //
        if (key != null) {
            key = key.trim();
        }

        if (key == null || key.isEmpty()) {
            throw new InitializationException("Attempt to set property with empty key");
        }

        // Check if the property is in an Ice property prefix. If so, check that it's a valid
        // property.
        PropertyArray propertyArray = findIcePropertyArray(key);
        if (propertyArray != null) {
            if (propertyArray.isOptIn()
                && _optInPrefixes.stream().noneMatch(propertyArray.name()::equals)) {

                throw new PropertyException(
                    "unable to set '"
                        + key
                        + "': property prefix '"
                        + propertyArray.name()
                        + "' is opt-in and must be explicitly enabled");
            }

            Property prop =
                findProperty(key.substring(propertyArray.name().length() + 1), propertyArray);
            if (prop == null) {
                throw new PropertyException("unknown Ice property: " + key);
            }
            // If the property is deprecated, log a warning
            if (prop.deprecated()) {
                Util.getProcessLogger().warning("setting deprecated property: " + key);
            }
        }
        synchronized (this) {
            //
            // Set or clear the property.
            //
            if (value != null && !value.isEmpty()) {
                PropertyValue pv = _properties.get(key);
                if (pv != null) {
                    pv.value = value;
                } else {
                    pv = new PropertyValue(value, false);
                }
                _properties.put(key, pv);
            } else {
                _properties.remove(key);
            }
        }
    }

    /**
     * Get a sequence of command-line options that is equivalent to this property set. Each element
     * of the returned sequence is a command-line option of the form <code>
     * --<em>key</em>=<em>value</em>
     * </code>.
     *
     * @return The command line options for this property set.
     */
    public synchronized String[] getCommandLineOptions() {
        String[] result = new String[_properties.size()];
        int i = 0;
        for (Map.Entry<String, PropertyValue> p : _properties.entrySet()) {
            result[i++] = "--" + p.getKey() + "=" + p.getValue().value;
        }
        assert (i == result.length);
        return result;
    }

    /**
     * Convert a sequence of command-line options into properties. All options that begin with
     * <code>
     * --<em>prefix</em>.</code> are converted into properties. If the prefix is empty, all options
     * that begin with <code>--</code> are converted to properties.
     *
     * @param prefix The property prefix, or an empty string to convert all options starting with
     *     <code>--</code>.
     * @param options The command-line options.
     * @return The command-line options that do not start with the specified prefix, in their
     *     original order.
     */
    public String[] parseCommandLineOptions(String prefix, String[] options) {
        if (!prefix.isEmpty() && prefix.charAt(prefix.length() - 1) != '.') {
            prefix += '.';
        }
        prefix = "--" + prefix;

        ArrayList<String> result = new ArrayList<>();
        for (String opt : options) {
            if (opt.startsWith(prefix)) {
                if (opt.indexOf('=') == -1) {
                    opt += "=1";
                }

                parseLine(opt.substring(2));
            } else {
                result.add(opt);
            }
        }
        return result.toArray(new String[0]);
    }

    /**
     * Convert a sequence of command-line options into properties. All options that begin with one
     * of the following prefixes are converted into properties: <code>--Ice</code>, <code>--IceBox
     * </code> , <code>--IceGrid</code>, <code>--Ice.SSL</code>, <code>--IceStorm</code>, and <code>
     * --Glacier2</code>.
     *
     * @param options The command-line options.
     * @return The command-line options that do not start with one of the listed prefixes, in their
     *     original order.
     */
    public String[] parseIceCommandLineOptions(String[] options) {
        String[] args = options;
        for (PropertyArray props : PropertyNames.validProps) {
            args = parseCommandLineOptions(props.name(), args);
        }
        return args;
    }

    /**
     * Load properties from a file.
     *
     * @param file The property file.
     */
    public void load(String file) {
        if (System.getProperty("os.name").startsWith("Windows")
            && (file.startsWith("HKCU\\") || file.startsWith("HKLM\\"))) {
            try {
                java.lang.Process process =
                    Runtime.getRuntime().exec(new String[]{"reg", "query", file});
                process.waitFor();
                if (process.exitValue() != 0) {
                    throw new InitializationException(
                        "Could not read Windows registry key '" + file + "'");
                }

                java.io.InputStream is = process.getInputStream();
                StringWriter sw = new StringWriter();
                int c;
                while ((c = is.read()) != -1) {
                    sw.write(c);
                }
                String[] result = sw.toString().split("\n");

                for (String line : result) {
                    int pos = line.indexOf("REG_SZ");
                    if (pos != -1) {
                        setProperty(
                            line.substring(0, pos).trim(),
                            line.substring(pos + 6, line.length()).trim());
                        continue;
                    }

                    pos = line.indexOf("REG_EXPAND_SZ");
                    if (pos != -1) {
                        String name = line.substring(0, pos).trim();
                        line = line.substring(pos + 13, line.length()).trim();
                        while (true) {
                            int start = line.indexOf('%', 0);
                            int end = line.indexOf('%', start + 1);

                            //
                            // If there isn't more %var% break the loop
                            //
                            if (start == -1 || end == -1) {
                                break;
                            }

                            String envKey = line.substring(start + 1, end);
                            String envValue = System.getenv(envKey);
                            if (envValue == null) {
                                envValue = "";
                            }

                            envKey = "%" + envKey + "%";
                            do {
                                line = line.replace(envKey, envValue);
                            } while (line.indexOf(envKey) != -1);
                        }
                        setProperty(name, line);
                        continue;
                    }
                }
            } catch (LocalException ex) {
                throw ex;
            } catch (Exception ex) {
                throw new InitializationException(
                    "Could not read Windows registry key `" + file + "'", ex);
            }
        } else {
            PushbackInputStream is = null;
            try {
                java.io.InputStream f = Util.openResource(getClass().getClassLoader(), file);
                if (f == null) {
                    throw new FileException("failed to open '" + file + "'");
                }
                //
                // Skip UTF-8 BOM if present.
                //
                byte[] bom = new byte[3];
                is = new PushbackInputStream(f, bom.length);
                int read = is.read(bom, 0, bom.length);
                if (read < 3
                    || bom[0] != (byte) 0xEF
                    || bom[1] != (byte) 0xBB
                    || bom[2] != (byte) 0xBF) {
                    if (read > 0) {
                        is.unread(bom, 0, read);
                    }
                }

                InputStreamReader isr = new InputStreamReader(is, "UTF-8");
                BufferedReader br = new BufferedReader(isr);
                parse(br);
            } catch (IOException ex) {
                throw new FileException("Cannot read '" + file + "'", ex);
            } finally {
                if (is != null) {
                    try {
                        is.close();
                    } catch (Throwable ex) {
                        // Ignore.
                    }
                }
            }
        }
    }

    /**
     * Create a copy of this property set.
     *
     * @return A copy of this property set.
     */
    public synchronized Properties _clone() {
        Properties clonedProperties = new Properties();
        //
        // NOTE: we can't just do a shallow copy of the map as the map values
        // would otherwise be shared between the two Properties objects.
        //
        // _properties = new java.util.HashMap<String, PropertyValue>(props._properties);
        for (Map.Entry<String, PropertyValue> p : _properties.entrySet()) {
            clonedProperties._properties.put(p.getKey(), p.getValue().clone());
        }
        return clonedProperties;
    }

    public synchronized List<String> getUnusedProperties() {
        List<String> unused = new ArrayList<>();
        for (Map.Entry<String, PropertyValue> p : _properties.entrySet()) {
            PropertyValue pv = p.getValue();
            if (!pv.used) {
                unused.add(p.getKey());
            }
        }
        return unused;
    }

    private void parse(BufferedReader in) {
        try {
            String line;
            while ((line = in.readLine()) != null) {
                parseLine(line);
            }
        } catch (IOException ex) {
            throw new SyscallException(ex);
        }
    }

    private void parseLine(String line) {
        String key = "";
        String value = "";

        int state = ParseStateKey;

        String whitespace = "";
        String escapedspace = "";
        boolean finished = false;
        for (int i = 0; i < line.length(); i++) {
            char c = line.charAt(i);
            switch (state) {
                case ParseStateKey:
                {
                    switch (c) {
                        case '\\':
                            if (i < line.length() - 1) {
                                c = line.charAt(++i);
                                switch (c) {
                                    case '\\':
                                    case '#':
                                    case '=':
                                        key += whitespace;
                                        whitespace = "";
                                        key += c;
                                        break;

                                    case ' ':
                                        if (!key.isEmpty()) {
                                            whitespace += c;
                                        }
                                        break;

                                    default:
                                        key += whitespace;
                                        whitespace = "";
                                        key += '\\';
                                        key += c;
                                        break;
                                }
                            } else {
                                key += whitespace;
                                key += c;
                            }
                            break;

                        case ' ':
                        case '\t':
                        case '\r':
                        case '\n':
                            if (!key.isEmpty()) {
                                whitespace += c;
                            }
                            break;

                        case '=':
                            whitespace = "";
                            state = ParseStateValue;
                            break;

                        case '#':
                            finished = true;
                            break;

                        default:
                            key += whitespace;
                            whitespace = "";
                            key += c;
                            break;
                    }
                    break;
                }

                case ParseStateValue:
                {
                    switch (c) {
                        case '\\':
                            if (i < line.length() - 1) {
                                c = line.charAt(++i);
                                switch (c) {
                                    case '\\':
                                    case '#':
                                    case '=':
                                        value += value.isEmpty() ? escapedspace : whitespace;
                                        whitespace = "";
                                        escapedspace = "";
                                        value += c;
                                        break;

                                    case ' ':
                                        whitespace += c;
                                        escapedspace += c;
                                        break;

                                    default:
                                        value += value.isEmpty() ? escapedspace : whitespace;
                                        whitespace = "";
                                        escapedspace = "";
                                        value += '\\';
                                        value += c;
                                        break;
                                }
                            } else {
                                value += value.isEmpty() ? escapedspace : whitespace;
                                value += c;
                            }
                            break;

                        case ' ':
                        case '\t':
                        case '\r':
                        case '\n':
                            if (!value.isEmpty()) {
                                whitespace += c;
                            }
                            break;

                        case '#':
                            finished = true;
                            break;

                        default:
                            value += value.isEmpty() ? escapedspace : whitespace;
                            whitespace = "";
                            escapedspace = "";
                            value += c;
                            break;
                    }
                    break;
                }
            }
            if (finished) {
                break;
            }
        }
        value += escapedspace;

        if ((state == ParseStateKey && !key.isEmpty())
            || (state == ParseStateValue && key.isEmpty())) {
            Util.getProcessLogger().warning("invalid config file entry: \"" + line + "\"");
            return;
        } else if (key.isEmpty()) {
            return;
        }

        setProperty(key, value);
    }

    private void loadConfig() {
        String value = getIceProperty("Ice.Config");

        if (value.isEmpty() || "1".equals(value)) {
            try {
                value = System.getenv("ICE_CONFIG");
                if (value == null) {
                    value = "";
                }
            } catch (java.lang.SecurityException ex) {
                value = "";
            }
        }

        if (!value.isEmpty()) {
            for (String file : value.split(",")) {
                load(file.trim());
            }

            _properties.put("Ice.Config", new PropertyValue(value, true));
        }
    }

    /*
     * Find a property by key in a property array.
     * @param key The property key.
     * @param propertyArray The property array to search.
     * @return The property if found, null otherwise.
     */
    static Property findProperty(String key, PropertyArray propertyArray) {
        for (Property prop : propertyArray.properties()) {
            String pattern = prop.pattern();
            // If the key is an exact match, return the property unless it has a property class
            // which is prefix only. If the key is a regex match, return the property. A property
            // cannot have a property class and use regex.
            if (key.equals(pattern)) {
                if (prop.propertyArray() != null && prop.propertyArray().prefixOnly()) {
                    return null;
                }
                return prop;
            } else if (prop.usesRegex() && key.matches(pattern)) {
                return prop;
            }

            // If the property has a property class, check if the key is a prefix of the property.
            if (prop.propertyArray() != null) {
                // Check if the key is a prefix of the property.
                // The key must be:
                //  - shorter than the property pattern
                //  - the property pattern must start with the key
                // - the pattern character after the key must be a dot
                if (key.length() > pattern.length()
                    && key.startsWith(pattern)
                    && key.charAt(pattern.length()) == '.') {
                    String substring = key.substring(pattern.length() + 1);
                    // Check if the suffix is a valid property. If so, return it. If it's not,
                    // continue searching the current property array.
                    Property foundProp = findProperty(substring, prop.propertyArray());
                    if (foundProp != null) {
                        return foundProp;
                    }
                }
            }
        }
        return null;
    }

    /*
     * Validate properties with a given prefix.
     * @param prefix The property prefix.
     * @param properties The properties to validate.
     * @param propertyArray The property array to validate against.
     * @throws PropertyException if any unknown properties are found.
     */
    static void validatePropertiesWithPrefix(
            String prefix, Properties properties, PropertyArray propertyArray) {
        // Do not check for unknown properties if Ice prefix, ie Ice, Glacier2, etc
        for (PropertyArray props : PropertyNames.validProps) {
            if (prefix.startsWith(props.name() + ".")) {
                return;
            }
        }

        var unknownProperties =
            properties.getPropertiesForPrefix(prefix + ".").keySet().stream()
                .filter(
                    key ->
                        findProperty(
                            key.substring(prefix.length() + 1),
                            propertyArray)
                            == null)
                .collect(Collectors.toList());

        if (unknownProperties.size() > 0) {
            throw new PropertyException(
                "found unknown properties for "
                    + propertyArray.name()
                    + ": '"
                    + prefix
                    + "'\n    "
                    + String.join("\n    ", unknownProperties));
        }
    }

    /*
     * Find an Ice property array by key.
     * @param key The property key.
     * @return The property array if found, null otherwise.
     */
    private static PropertyArray findIcePropertyArray(String key) {
        int dotPos = key.indexOf('.');

        // If the key doesn't contain a dot, it's not a valid Ice property.
        if (dotPos == -1) {
            return null;
        }

        String prefix = key.substring(0, dotPos);

        return Arrays.stream(PropertyNames.validProps)
            .filter(properties -> properties.name().equals(prefix))
            .findFirst()
            .orElse(null);
    }

    /*
     * Gets the default value for a given Ice property.
     * @param key The property key.
     * @return The default value.
     * @throws PropertyException if the property is unknown.
     */
    private static String getDefaultProperty(String key) {
        PropertyArray propertyArray = findIcePropertyArray(key);
        if (propertyArray == null) {
            throw new PropertyException("unknown Ice property: " + key);
        }

        Property prop =
            findProperty(key.substring(propertyArray.name().length() + 1), propertyArray);
        if (prop == null) {
            throw new PropertyException("unknown Ice property: " + key);
        }
        return prop.defaultValue();
    }

    private static final int ParseStateKey = 0;
    private static final int ParseStateValue = 1;
    private final HashMap<String, PropertyValue> _properties = new HashMap<>();
    private final List<String> _optInPrefixes = new ArrayList<>();
}
