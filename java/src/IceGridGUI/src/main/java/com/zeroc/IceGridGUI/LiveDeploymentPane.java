// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceGridGUI;

import java.awt.*;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;

import javax.swing.*;
import javax.swing.border.EmptyBorder;
import javax.swing.event.TreeSelectionEvent;
import javax.swing.event.TreeSelectionListener;
import javax.swing.plaf.SplitPaneUI;
import javax.swing.plaf.basic.BasicSplitPaneUI;
import javax.swing.tree.TreeCellRenderer;
import javax.swing.tree.TreeSelectionModel;
import javax.swing.tree.TreePath;
import com.jgoodies.forms.factories.Paddings;

import com.zeroc.IceGridGUI.LiveDeployment.Editor;
import com.zeroc.IceGridGUI.LiveDeployment.Root;
import com.zeroc.IceGridGUI.LiveDeployment.TreeNode;

public class LiveDeploymentPane extends JSplitPane implements Tab
{
    @Override
    public void updateUI()
    {
        super.updateUI();
        setEmptyDividerBorder();
    }

    @Override
    public void selected()
    {
        Coordinator c = _root.getCoordinator();

        if(c.connected())
        {
            c.getShowLiveDeploymentFiltersAction().setEnabled(true);
        }
        c.getCloseApplicationAction().setEnabled(false);
        c.getSaveAction().setEnabled(false);
        c.getSaveToRegistryAction().setEnabled(false);
        c.getSaveToRegistryWithoutRestartAction().setEnabled(false);
        c.getSaveToFileAction().setEnabled(false);
        c.getDiscardUpdatesAction().setEnabled(false);

        c.getBackAction().setEnabled(_previousNodes.size() > 0);
        c.getForwardAction().setEnabled(_nextNodes.size() > 0);
        c.showActions(_currentNode);
    }

    @Override
    public void refresh()
    {
        if(_currentNode != null)
        {
            _currentNode.getEditor();

            if(_root.getCoordinator().getCurrentTab() == this)
            {
                //
                // Refresh actions as well
                //
                _root.getCoordinator().showActions(_currentNode);
            }
        }
    }

    @Override
    public void showNode(TreeNodeBase node)
    {
        TreeNode newNode = (TreeNode)node;

        if(newNode != _currentNode)
        {
            if(newNode == null)
            {
                _currentNode = null;
                showCurrentNode();
            }
            else
            {
                if(_currentNode != null && _root.hasNode(_currentNode))
                {
                    _previousNodes.add(_currentNode);
                    while(_previousNodes.size() >= HISTORY_MAX_SIZE)
                    {
                        _previousNodes.removeFirst();
                    }
                    _root.getCoordinator().getBackAction().setEnabled(true);
                }
                _nextNodes.clear();
                _root.getCoordinator().getForwardAction().setEnabled(false);
                _currentNode = newNode;
                showCurrentNode();
            }
        }
        else
        {
            refresh();
        }
    }

    @Override
    public void back()
    {
        TreeNode previousNode = null;
        do
        {
            previousNode = _previousNodes.removeLast();
        } while(_previousNodes.size() > 0 && (previousNode == _currentNode || !_root.hasNode(previousNode)));

        if(_previousNodes.size() == 0)
        {
            _root.getCoordinator().getBackAction().setEnabled(false);
        }

        if(previousNode != _currentNode)
        {
            if(_currentNode != null)
            {
                _nextNodes.addFirst(_currentNode);
                _root.getCoordinator().getForwardAction().setEnabled(true);
            }

            _currentNode = previousNode;
            _selectionListenerEnabled = false;
            _root.setSelectedNode(_currentNode);
            _selectionListenerEnabled = true;
            showCurrentNode();
        }
    }

    @Override
    public void forward()
    {
        TreeNode nextNode = null;
        do
        {
            nextNode = _nextNodes.removeFirst();
        } while(_nextNodes.size() > 0 && (nextNode == _currentNode || !_root.hasNode(nextNode)));

        if(_nextNodes.size() == 0)
        {
            _root.getCoordinator().getForwardAction().setEnabled(false);
        }

        if(nextNode != _currentNode)
        {
            if(_currentNode != null)
            {
                _previousNodes.add(_currentNode);
                _root.getCoordinator().getBackAction().setEnabled(true);
            }

            _currentNode = nextNode;
            _selectionListenerEnabled = false;
            _root.setSelectedNode(_currentNode);
            _selectionListenerEnabled = true;
            showCurrentNode();
        }
    }

    @Override
    public void save()
    {
        assert false;
    }

    @Override
    public void saveToRegistry(boolean restart)
    {
        assert false;
    }

    @Override
    public void saveToFile()
    {
        assert false;
    }

    @Override
    public void discardUpdates()
    {
        assert false;
    }

    @Override
    public boolean close()
    {
        return false;
    }

    LiveDeploymentPane(Root root)
    {
        super(JSplitPane.HORIZONTAL_SPLIT, true);
        _root = root;
        setBorder(new EmptyBorder(10, 10, 10, 10));

        //
        // Tree display
        //
        TreeCellRenderer renderer = new CellRenderer();
        PopupListener popupListener = new PopupListener();

        JTree tree = _root.getTree();

        tree.setBorder(new EmptyBorder(5, 5, 5, 5));
        tree.setCellRenderer(renderer);
        ToolTipManager.sharedInstance().registerComponent(tree);
        tree.addMouseListener(popupListener);

        tree.getSelectionModel().setSelectionMode(TreeSelectionModel.SINGLE_TREE_SELECTION);

        tree.addTreeSelectionListener(new SelectionListener());

        tree.setRootVisible(false);

        JScrollPane leftScroll =
            new JScrollPane(tree,
                            ScrollPaneConstants.VERTICAL_SCROLLBAR_AS_NEEDED,
                            ScrollPaneConstants.HORIZONTAL_SCROLLBAR_AS_NEEDED);
        leftScroll.setBorder(Paddings.EMPTY);

        _leftPane = new SimpleInternalFrame("Runtime Components");
        _leftPane.setContent(leftScroll);
        _leftPane.setPreferredSize(new Dimension(280, 350));

        //
        // Right pane
        //
        _propertiesFrame = new SimpleInternalFrame("Properties");

        setLeftComponent(_leftPane);
        setRightComponent(_propertiesFrame);
    }

    private void showCurrentNode()
    {
        _root.getCoordinator().showActions(_currentNode);
        if(_currentNode == null)
        {
            Component oldContent = _propertiesFrame.getContent();
            if(oldContent != null)
            {
                _propertiesFrame.remove(oldContent);
            }
            _propertiesFrame.setTitle("Properties");
            _propertiesFrame.setToolBar(null);
        }
        else
        {
            Editor editor = _currentNode.getEditor();
            Component currentProperties = editor.getProperties();
            _propertiesFrame.setContent(currentProperties);
            _propertiesFrame.setTitle(currentProperties.getName());
            _propertiesFrame.setToolBar(editor.getToolBar());
        }
        _propertiesFrame.validate();
        _propertiesFrame.repaint();
    }

    private void setEmptyDividerBorder()
    {
        SplitPaneUI splitPaneUI = getUI();
        if(splitPaneUI instanceof BasicSplitPaneUI)
        {
            BasicSplitPaneUI basicUI = (BasicSplitPaneUI)splitPaneUI;
            basicUI.getDivider().setBorder(BorderFactory.createEmptyBorder());
        }
    }

    private class SelectionListener implements TreeSelectionListener
    {
        @Override
        public void valueChanged(TreeSelectionEvent e)
        {
            if(_selectionListenerEnabled)
            {
                TreePath path = null;
                if(e.isAddedPath())
                {
                    path = e.getPath();
                }

                if(path == null)
                {
                    showNode(null);
                }
                else
                {
                    showNode((TreeNode)path.getLastPathComponent());
                }
            }
        }
    }

    private class PopupListener extends MouseAdapter
    {
        @Override
        public void mousePressed(MouseEvent e)
        {
            maybeShowPopup(e);
        }

        @Override
        public void mouseReleased(MouseEvent e)
        {
            maybeShowPopup(e);
        }

        private void maybeShowPopup(MouseEvent e)
        {
            if (e.isPopupTrigger())
            {
                JTree tree = (JTree)e.getComponent();

                TreePath path = tree.getPathForLocation(e.getX(), e.getY());

                if(path != null)
                {
                    TreeNode node = (TreeNode)path.getLastPathComponent();
                    JPopupMenu popup = node.getPopupMenu();
                    if(popup != null)
                    {
                        popup.show(tree, e.getX(), e.getY());
                    }
                }
            }
        }
    }

    private Root _root;
    private SimpleInternalFrame _leftPane;
    private SimpleInternalFrame _propertiesFrame;

    //
    // back/forward navigation
    //
    private java.util.LinkedList<TreeNode> _previousNodes = new java.util.LinkedList<>();
    private java.util.LinkedList<TreeNode> _nextNodes = new java.util.LinkedList<>();
    private TreeNode _currentNode;

    private boolean _selectionListenerEnabled = true;

    static private final int HISTORY_MAX_SIZE = 20;
}
