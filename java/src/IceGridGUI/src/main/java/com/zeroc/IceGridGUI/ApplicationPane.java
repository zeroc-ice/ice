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
import javax.swing.tree.TreePath;
import javax.swing.tree.TreeSelectionModel;

import com.jgoodies.forms.factories.Borders;

import com.zeroc.IceGridGUI.Application.Editor;
import com.zeroc.IceGridGUI.Application.Root;
import com.zeroc.IceGridGUI.Application.TreeNode;

public class ApplicationPane extends JSplitPane implements Tab
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

        c.getShowLiveDeploymentFiltersAction().setEnabled(false);
        c.getCloseApplicationAction().setEnabled(true);

        c.getSaveAction().setEnabled(_root.needsSaving() && (_root.isLive() && c.connectedToMaster() ||
                                     _root.hasFile()));
        c.getDiscardUpdatesAction().setEnabled(_root.needsSaving() && (_root.isLive() || _root.hasFile()));

        if(_root.isLive())
        {
            c.getSaveToRegistryAction().setEnabled(_root.needsSaving() && c.connectedToMaster());
            c.getSaveToRegistryWithoutRestartAction().setEnabled(_root.needsSaving() && c.connectedToMaster());
        }
        else
        {
            c.getSaveToRegistryAction().setEnabled(c.connectedToMaster());
            c.getSaveToRegistryWithoutRestartAction().setEnabled(c.connectedToMaster());
        }
        c.getSaveToFileAction().setEnabled(true);

        c.getBackAction().setEnabled(_previousNodes.size() > 0);
        c.getForwardAction().setEnabled(_nextNodes.size() > 0);
        c.showActions(_currentNode);
    }

    @Override
    public void refresh()
    {
        if(_currentNode != null)
        {
            _currentEditor = _currentNode.getEditor();
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
            if(_currentNode != null && _currentNode.isEphemeral() && _root.hasNode(_currentNode))
            {
                _currentNode.destroy();
                _currentNode = null;
            }

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
            _currentEditor = _currentNode.getEditor();
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
    public void back()
    {
        //
        // Auto-apply changes
        //
        if(_currentEditor != null && !_currentEditor.save(false))
        {
            return;
        }

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
            _root.disableSelectionListener();
            _root.setSelectedNode(_currentNode);
            _root.enableSelectionListener();
            showCurrentNode();
        }
    }

    @Override
    public void forward()
    {
        if(_currentEditor != null && !_currentEditor.save(false))
        {
            return;
        }

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
            _root.disableSelectionListener();
            _root.setSelectedNode(_currentNode);
            _root.enableSelectionListener();
            showCurrentNode();
        }
    }

    public Root getRoot()
    {
        return _root;
    }

    //
    // E.g. to replace an ephemeral root
    //
    public void setRoot(Root newRoot)
    {
        boolean reset = (_root != null);

        if(reset)
        {
            ToolTipManager.sharedInstance().unregisterComponent(_root.getTree());
            _currentNode = null;
            _previousNodes.clear();
            _nextNodes.clear();
        }

        _root = newRoot;
        _root.setPane(this);

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

        tree.setRootVisible(true);

        JScrollPane leftScroll =
            new JScrollPane(tree,
                            ScrollPaneConstants.VERTICAL_SCROLLBAR_AS_NEEDED,
                            ScrollPaneConstants.HORIZONTAL_SCROLLBAR_AS_NEEDED);
        leftScroll.setBorder(Borders.EMPTY);

        _leftPane.setContent(leftScroll);

        if(reset)
        {
            _root.getCoordinator().getMainPane().resetIcon(_root);
            _leftPane.validate();
            _leftPane.repaint();
        }
    }

    @Override
    public void save()
    {
        if(_currentEditor == null || _currentEditor.save(true))
        {
            _root.save();
        }
    }

    @Override
    public void saveToRegistry(boolean restart)
    {
        if(_currentEditor == null || _currentEditor.save(true))
        {
            _root.saveToRegistry(restart);
        }
    }

    @Override
    public void saveToFile()
    {
        if(_currentEditor == null || _currentEditor.save(true))
        {
            _root.saveToFile();
        }
    }

    @Override
    public void discardUpdates()
    {
        _root.discardUpdates();
    }

    @Override
    public boolean close()
    {
        _root.getCoordinator().getMainPane().remove(this);
        return true;
    }

    public boolean applyUpdates(boolean refresh)
    {
        if(_currentEditor == null)
        {
            return true;
        }
        else
        {
            return _currentEditor.save(refresh);
        }
    }

    private void registerAction(Coordinator c, int index)
    {
        Action action = c.getActionsForMenu().get(index);

        javax.swing.ActionMap am = _leftPane.getActionMap();
        javax.swing.InputMap im = _leftPane.getInputMap();

        im.put((KeyStroke)action.getValue(Action.ACCELERATOR_KEY), (String)action.getValue(Action.NAME));
        am.put(action.getValue(Action.NAME), action);
    }

    ApplicationPane(Root root)
    {
        super(JSplitPane.HORIZONTAL_SPLIT, true);
        setBorder(new EmptyBorder(10, 10, 10, 10));

        _leftPane = new SimpleInternalFrame("Descriptors");
        _leftPane.setPreferredSize(new Dimension(280, 350));
        Coordinator c = root.getCoordinator();
        registerAction(c, TreeNode.COPY);
        registerAction(c, TreeNode.PASTE);
        registerAction(c, TreeNode.DELETE);

        //
        // Right pane
        //
        _propertiesFrame = new SimpleInternalFrame("Properties");

        setLeftComponent(_leftPane);
        setRightComponent(_propertiesFrame);

        setRoot(root);
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
            _currentEditor = null;
        }
        else
        {
            _currentEditor = _currentNode.getEditor();
            Component currentProperties = _currentEditor.getProperties();
            _propertiesFrame.setContent(currentProperties);
            _propertiesFrame.setTitle(currentProperties.getName());
            _propertiesFrame.setToolBar(_currentEditor.getToolBar());
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

    private class SelectionListener implements TreeSelectionListener
    {
        @Override
        public void valueChanged(TreeSelectionEvent e)
        {
            if(_root.isSelectionListenerEnabled())
            {
                //
                // Auto-apply changes
                //
                if(_currentEditor != null && !_currentEditor.save(false))
                {
                    //
                    // Go back to this path
                    //
                    _root.disableSelectionListener();
                    _root.setSelectedNode(_currentEditor.getTarget());
                    _root.enableSelectionListener();
                }
                else
                {
                    if(e.isAddedPath())
                    {
                        TreePath path = e.getPath();

                        if(path == null)
                        {
                            showNode(null);
                        }
                        else
                        {
                            TreeNode node = (TreeNode)path.getLastPathComponent();
                            Root root = node.getRoot();
                            if(root.hasNode(node))
                            {
                                showNode(node);
                            }
                            else
                            {
                                node = root.findNodeLike(path, false);
                                if(node == null)
                                {
                                    node = root;
                                }
                                root.setSelectedNode(node);
                            }
                        }
                    }
                    else
                    {
                        showNode(null);
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
    private Editor _currentEditor;

    static private final int HISTORY_MAX_SIZE = 20;
}
