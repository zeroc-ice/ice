// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGridGUI;

import java.awt.*;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.awt.event.ActionEvent;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;

import javax.swing.*;
import javax.swing.border.AbstractBorder;
import javax.swing.border.EmptyBorder;
import javax.swing.event.TreeSelectionEvent;
import javax.swing.event.TreeSelectionListener;
import javax.swing.plaf.SplitPaneUI;
import javax.swing.plaf.basic.BasicSplitPaneUI;
import javax.swing.tree.TreeCellRenderer;
import javax.swing.tree.TreeSelectionModel;
import javax.swing.tree.TreePath;

import com.jgoodies.looks.Options;
import com.jgoodies.looks.plastic.PlasticLookAndFeel;
import com.jgoodies.looks.windows.WindowsLookAndFeel;
import com.jgoodies.forms.factories.Borders;

import IceGrid.*;
import IceGridGUI.LiveDeployment.Editor;
import IceGridGUI.LiveDeployment.Root;
import IceGridGUI.LiveDeployment.TreeNode;

public class LiveDeploymentPane extends JSplitPane implements Tab
{
    public void updateUI()
    {
        super.updateUI();
        setEmptyDividerBorder();
    }

    public void selected()
    {
        Coordinator c = _root.getCoordinator();

        c.getCloseApplicationAction().setEnabled(false);
        c.getSaveAction().setEnabled(false);
        c.getSaveToRegistryAction().setEnabled(false);
        c.getSaveToFileAction().setEnabled(false);
        c.getDiscardUpdatesAction().setEnabled(false);
        
        c.getBackAction().setEnabled(_previousNodes.size() > 0);
        c.getForwardAction().setEnabled(_nextNodes.size() > 0);
        c.showActions(_currentNode);
    }

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

    public void back()
    {
        TreeNode previousNode = null;
        do
        {
            previousNode = (TreeNode)_previousNodes.removeLast();
        } while(_previousNodes.size() > 0 
                && (previousNode == _currentNode || !_root.hasNode(previousNode)));
                
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

    public void forward()
    {
        TreeNode nextNode = null;
        do
        {
            nextNode = (TreeNode)_nextNodes.removeFirst();
        } while(_nextNodes.size() > 0 
                && (nextNode == _currentNode || !_root.hasNode(nextNode)));
        
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

    
    public void save()
    {
        assert false;
    }
    public void saveToRegistry()
    {
        assert false;
    }
    public void saveToFile()
    {
        assert false;
    }
    public void discardUpdates()
    {
        assert false;
    }
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

        tree.getSelectionModel().setSelectionMode
            (TreeSelectionModel.SINGLE_TREE_SELECTION);

        tree.addTreeSelectionListener(new SelectionListener());
        
        tree.setRootVisible(false);

        JScrollPane leftScroll = 
            new JScrollPane(tree,
                            JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED, 
                            JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);
        leftScroll.setBorder(Borders.EMPTY_BORDER);
        
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
        public void mousePressed(MouseEvent e) 
        {
            maybeShowPopup(e);
        }

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
    private java.util.LinkedList _previousNodes = new java.util.LinkedList();
    private java.util.LinkedList _nextNodes = new java.util.LinkedList();
    private TreeNode _currentNode;
    
    private boolean _selectionListenerEnabled = true;

    static private final int HISTORY_MAX_SIZE = 20;
}
