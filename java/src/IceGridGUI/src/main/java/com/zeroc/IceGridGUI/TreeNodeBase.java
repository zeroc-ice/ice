// Copyright (c) ZeroC, Inc.

package com.zeroc.IceGridGUI;

import java.awt.Component;
import java.util.Arrays;
import java.util.Comparator;
import java.util.Enumeration;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.NoSuchElementException;

import javax.swing.JPopupMenu;
import javax.swing.JTree;
import javax.swing.tree.DefaultTreeModel;
import javax.swing.tree.TreeCellRenderer;
import javax.swing.tree.TreeNode;
import javax.swing.tree.TreePath;

// This class behaves like a leaf; derived class that represent non-leaf nodes must override various methods.
public class TreeNodeBase implements TreeNode, TreeCellRenderer {
    public Coordinator getCoordinator() {
        return _parent.getCoordinator();
    }

    @Override
    public Enumeration<TreeNode> children() {
        return new Enumeration<TreeNode>() {
            @Override
            public boolean hasMoreElements() {
                return false;
            }

            @Override
            public TreeNode nextElement() {
                throw new NoSuchElementException();
            }
        };
    }

    @Override
    public boolean getAllowsChildren() {
        return false;
    }

    @Override
    public TreeNode getChildAt(int childIndex) {
        return null;
    }

    @Override
    public int getChildCount() {
        return 0;
    }

    @Override
    public int getIndex(TreeNode node) {
        return -1;
    }

    @Override
    public TreeNode getParent() {
        return _parent;
    }

    @Override
    public boolean isLeaf() {
        return true;
    }

    @Override
    public Component getTreeCellRendererComponent(
            JTree tree,
            Object value,
            boolean sel,
            boolean expanded,
            boolean leaf,
            int row,
            boolean hasFocus) {
        return null;
    }

    @Override
    public String toString() {
        return _id;
    }

    public JPopupMenu getPopupMenu() {
        return null;
    }

    public String getId() {
        return _id;
    }

    public TreePath getPath() {
        if (_parent == null) {
            return new TreePath(this);
        } else {
            return _parent.getPath().pathByAddingChild(this);
        }
    }

    public LinkedList<String> getFullId() {
        LinkedList<String> result = _parent == null ? new LinkedList<>() : _parent.getFullId();

        result.add(_id);
        return result;
    }

    public TreeNodeBase findChild(String id) {
        Enumeration p = children();
        while (p.hasMoreElements()) {
            TreeNodeBase child = (TreeNodeBase) p.nextElement();
            if (child.getId().equals(id)) {
                return child;
            }
        }
        return null;
    }

    protected String makeNewChildId(String base) {
        String id = base;
        int i = 0;
        while (findChild(id) != null) {
            id = base + "-" + (++i);
        }
        return id;
    }

    protected TreeNodeBase(TreeNodeBase parent, String id) {
        _parent = parent;
        _id = id;
    }

    //
    // Helper functions
    //

    protected boolean insertSortedChild(TreeNodeBase newChild, List c, DefaultTreeModel treeModel) {
        @SuppressWarnings("unchecked")
        List<TreeNodeBase> children = c;

        String id = newChild.getId();
        int i;
        for (i = 0; i < children.size(); i++) {
            String otherId = children.get(i).getId();
            int cmp = id.compareTo(otherId);

            if (cmp == 0) {
                return false;
            } else if (cmp < 0) {
                break;
            }
        }
        children.add(i, newChild);
        if (treeModel != null) {
            treeModel.nodesWereInserted(this, new int[]{getIndex(newChild)});
        }
        return true;
    }

    protected String insertSortedChildren(List nc, List ic, DefaultTreeModel treeModel) {
        @SuppressWarnings("unchecked")
        List<TreeNodeBase> newChildren = nc;
        @SuppressWarnings("unchecked")
        List<TreeNodeBase> intoChildren = ic;

        TreeNodeBase[] children = newChildren.toArray(new TreeNodeBase[0]);
        Arrays.sort(children, _childComparator);

        int[] indices = new int[children.length];

        int offset = -1;

        int i = 0;
        boolean checkInsert = true;
        for (int j = 0; j < children.length; j++) {
            String id = children[j].getId();

            if (checkInsert) {
                while (i < intoChildren.size()) {
                    TreeNodeBase existingChild = intoChildren.get(i);
                    int cmp = id.compareTo(existingChild.getId());
                    if (cmp == 0) {
                        return id;
                    }
                    if (cmp < 0) {
                        break; // while
                    }
                    i++;
                }

                if (i < intoChildren.size()) {
                    // Insert here, and increment i (since children is sorted)
                    intoChildren.add(i, children[j]);
                    if (offset == -1) {
                        offset = getIndex(intoChildren.get(0));
                    }

                    indices[j] = offset + i;
                    i++;
                    continue; // for
                } else {
                    checkInsert = false;
                }
            }

            // Append
            intoChildren.add(children[j]);
            if (offset == -1) {
                offset = getIndex(intoChildren.get(0));
            }
            indices[j] = offset + i;
            i++;
        }

        if (treeModel != null) {
            treeModel.nodesWereInserted(this, indices);
        }

        return null;
    }

    protected void removeSortedChildren(String[] childIds, List fc, DefaultTreeModel treeModel) {
        @SuppressWarnings("unchecked")
        List<TreeNodeBase> fromChildren = fc;

        if (childIds.length == 0) {
            return; // nothing to do
        }
        assert !fromChildren.isEmpty();

        String[] ids = childIds.clone();
        Arrays.sort(ids);

        Object[] childrenToRemove = new Object[ids.length];
        int[] indices = new int[ids.length];

        int i = getIndex(fromChildren.get(0));
        int j = 0;

        Iterator<TreeNodeBase> p = fromChildren.iterator();
        while (p.hasNext() && j < ids.length) {
            TreeNodeBase child = p.next();
            if (ids[j].equals(child.getId())) {
                childrenToRemove[j] = child;
                indices[j] = i;
                p.remove();
                ++j;
            }
            ++i;
        }

        // Should be all removed
        assert (j == ids.length);

        if (treeModel != null) {
            treeModel.nodesWereRemoved(this, indices, childrenToRemove);
        }
    }

    protected void childrenChanged(List c, DefaultTreeModel treeModel) {
        @SuppressWarnings("unchecked")
        List<TreeNodeBase> children = c;

        for (TreeNodeBase child : children) {
            treeModel.nodeStructureChanged(child);
        }
    }

    public int[] resize(int[] array, int size) {
        int[] result = new int[size];
        System.arraycopy(array, 0, result, 0, size);
        return result;
    }

    protected TreeNodeBase find(String id, List il) {
        @SuppressWarnings("unchecked")
        List<TreeNodeBase> inList = il;

        for (TreeNodeBase node : inList) {
            if (node.getId().equals(id)) {
                return node;
            }
        }
        return null;
    }

    protected TreeNodeBase _parent;
    protected String _id;

    protected Comparator<TreeNodeBase> _childComparator =
        new Comparator<TreeNodeBase>() {
            @Override
            public int compare(TreeNodeBase o1, TreeNodeBase o2) {
                return o1.getId().compareTo(o2.getId());
            }
        };
}
