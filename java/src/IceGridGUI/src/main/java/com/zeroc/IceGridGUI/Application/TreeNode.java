// Copyright (c) ZeroC, Inc.

package com.zeroc.IceGridGUI.Application;

import com.zeroc.Ice.ToStringMode;
import com.zeroc.Ice.Util;
import com.zeroc.IceGrid.*;
import com.zeroc.IceGridGUI.*;

import java.io.IOException;
import java.util.*;

public abstract class TreeNode extends TreeNodeBase {
    public abstract Editor getEditor();

    protected abstract Editor createEditor();

    abstract Object getDescriptor();

    abstract void write(XMLWriter writer) throws IOException;

    // Ephemeral objects are destroyed when discard their changes
    public boolean isEphemeral() {
        return false;
    }

    // Destroys this node
    public void destroy() {
        assert false;
    }

    TreeNode(TreeNode parent, String id) {
        super(parent, id);
    }

    public Root getRoot() {
        assert _parent != null;
        return ((TreeNode) _parent).getRoot();
    }

    TreeNode findChildLike(TreeNode other) {
        // Default implementation just use id; not always appropriate
        return (TreeNode) findChild(other.getId());
    }

    // Get variable-resolver
    Utils.Resolver getResolver() {
        if (isEphemeral()) {
            return null;
        } else {
            return ((TreeNode) _parent).getResolver();
        }
    }

    // Find child whose descriptor == the given descriptor
    TreeNode findChildWithDescriptor(Object descriptor) {
        Enumeration p = children();
        while (p.hasMoreElements()) {
            TreeNode node = (TreeNode) p.nextElement();
            if (node.getDescriptor() == descriptor) {
                return node;
            }
        }
        return null;
    }

    static String[] createAttribute(String name, String value) {
        return new String[]{name, value};
    }

    static void writeVariables(XMLWriter writer, Map<String, String> variables)
            throws IOException {
        for (Map.Entry<String, String> p : variables.entrySet()) {
            List<String[]> attributes = new LinkedList<>();
            attributes.add(createAttribute("name", p.getKey()));
            attributes.add(createAttribute("value", p.getValue()));

            writer.writeElement("variable", attributes);
        }
    }

    static void writePropertySet(
            XMLWriter writer,
            PropertySetDescriptor psd,
            List<AdapterDescriptor> adapters,
            String[] logs)
            throws IOException {
        writePropertySet(writer, "", "", psd, adapters, logs);
    }

    static void writePropertySet(
            XMLWriter writer,
            String id,
            String idAttrName,
            PropertySetDescriptor psd,
            List<AdapterDescriptor> adapters,
            String[] logs)
            throws IOException {
        if (id.isEmpty() && psd.references.length == 0 && psd.properties.isEmpty()) {
            return;
        }

        // We don't show the .Endpoint of adapters,
        // since they already appear in the Adapter descriptors
        Set<String> hiddenPropertyNames = new HashSet<>();
        Set<String> hiddenPropertyValues = new HashSet<>();

        if (adapters != null) {
            for (AdapterDescriptor p : adapters) {
                hiddenPropertyNames.add(p.name + ".Endpoints");
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

        List<String[]> attributes = new LinkedList<>();
        if (!id.isEmpty()) {
            attributes.add(createAttribute(idAttrName, id));
        }
        if (psd.references.length == 0 && psd.properties.isEmpty()) {
            writer.writeElement("properties", attributes);
        } else {
            writer.writeStartTag("properties", attributes);

            for (String ref : psd.references) {
                attributes.clear();
                attributes.add(createAttribute("refid", ref));
                writer.writeElement("properties", attributes);
            }

            for (PropertyDescriptor p : psd.properties) {
                if (hiddenPropertyNames.contains(p.name)) {
                    // We hide only the first occurrence
                    hiddenPropertyNames.remove(p.name);
                } else if (hiddenPropertyValues.contains(p.value)) {
                    hiddenPropertyValues.remove(p.value);
                } else {
                    attributes.clear();
                    attributes.add(createAttribute("name", p.name));
                    attributes.add(createAttribute("value", p.value));
                    writer.writeElement("property", attributes);
                }
            }
            writer.writeEndTag("properties");
        }
    }

    static void writeLogs(
            XMLWriter writer, String[] logs, List<PropertyDescriptor> properties)
            throws IOException {
        for (String log : logs) {
            List<String[]> attributes = new LinkedList<>();
            attributes.add(createAttribute("path", log));
            String prop = lookupName(log, properties);
            if (prop != null) {
                attributes.add(createAttribute("property", prop));
            }
            writer.writeElement("log", attributes);
        }
    }

    static String lookupName(String val, List<PropertyDescriptor> properties) {
        for (PropertyDescriptor p : properties) {
            if (p.value.equals(val)) {
                return p.name;
            }
        }
        return null;
    }

    static void writeObjects(
            String elt,
            XMLWriter writer,
            List<ObjectDescriptor> objects,
            List<PropertyDescriptor> properties)
            throws IOException {
        for (ObjectDescriptor p : objects) {
            List<String[]> attributes = new LinkedList<>();
            String strId =
                    Util.identityToString(p.id, ToStringMode.Unicode);
            attributes.add(createAttribute("identity", strId));
            if (!p.type.isEmpty()) {
                attributes.add(createAttribute("type", p.type));
            }
            if (properties != null) {
                String prop = lookupName(strId, properties);
                if (prop != null) {
                    attributes.add(createAttribute("property", prop));
                }
            }
            if (p.proxyOptions != null && !p.proxyOptions.isEmpty()) {
                attributes.add(createAttribute("proxy-options", p.proxyOptions));
            }
            writer.writeElement(elt, attributes);
        }
    }

    static void writeParameters(
            XMLWriter writer,
            List<String> parameters,
            Map<String, String> defaultValues)
            throws IOException {
        for (String p : new LinkedHashSet<String>(parameters)) {
            String val = defaultValues.get(p);
            List<String[]> attributes = new LinkedList<>();
            attributes.add(createAttribute("name", p));
            if (val != null) {
                attributes.add(createAttribute("default", val));
            }
            writer.writeElement("parameter", attributes);
        }
    }

    static LinkedList<String[]> parameterValuesToAttributes(
            Map<String, String> parameterValues, List<String> parameters) {
        LinkedList<String[]> result = new LinkedList<>();

        // We use a LinkedHashSet to maintain order while eliminating duplicates
        for (String p : new LinkedHashSet<String>(parameters)) {
            String val = parameterValues.get(p);
            if (val != null) {
                result.add(createAttribute(p, val));
            }
        }
        return result;
    }

    // Actions
    public static final int NEW_ADAPTER = 0;
    public static final int NEW_NODE = 1;
    public static final int NEW_PROPERTY_SET = 2;
    public static final int NEW_REPLICA_GROUP = 3;
    public static final int NEW_SERVER = 4;
    public static final int NEW_SERVER_ICEBOX = 5;
    public static final int NEW_SERVER_FROM_TEMPLATE = 6;
    public static final int NEW_SERVICE = 7;
    public static final int NEW_SERVICE_FROM_TEMPLATE = 8;
    public static final int NEW_TEMPLATE_SERVER = 9;
    public static final int NEW_TEMPLATE_SERVER_ICEBOX = 10;
    public static final int NEW_TEMPLATE_SERVICE = 11;

    public static final int COPY = 12;
    public static final int PASTE = 13;
    public static final int DELETE = 14;

    public static final int SHOW_VARS = 15;
    public static final int SUBSTITUTE_VARS = 16;

    public static final int MOVE_UP = 17;
    public static final int MOVE_DOWN = 18;

    public static final int ACTION_COUNT = 19;

    public boolean[] getAvailableActions() {
        return new boolean[ACTION_COUNT];
    }

    public void newAdapter() {
        assert false;
    }

    public void newNode() {
        assert false;
    }

    public void newPropertySet() {
        assert false;
    }

    public void newReplicaGroup() {
        assert false;
    }

    public void newServer() {
        assert false;
    }

    public void newServerIceBox() {
        assert false;
    }

    public void newServerFromTemplate() {
        assert false;
    }

    public void newService() {
        assert false;
    }

    public void newServiceFromTemplate() {
        assert false;
    }

    public void newTemplateServer() {
        assert false;
    }

    public void newTemplateServerIceBox() {
        assert false;
    }

    public void newTemplateService() {
        assert false;
    }

    public void copy() {
        assert false;
    }

    public void paste() {
        assert false;
    }

    public void delete() {
        boolean enabled = getRoot().isSelectionListenerEnabled();

        if (enabled) {
            getRoot().disableSelectionListener();
        }
        destroy();
        getCoordinator().getCurrentTab().showNode(null);
        if (enabled) {
            getRoot().enableSelectionListener();
        }

        if (_parent != null) {
            getRoot().setSelectedNode((TreeNode) _parent);
        }
    }

    public void moveUp() {
        assert false;
    }

    public void moveDown() {
        assert false;
    }
}
