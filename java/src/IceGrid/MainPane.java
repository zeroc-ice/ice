// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid;

import java.util.prefs.Preferences;
import java.util.prefs.BackingStoreException;
import javax.swing.*;

import javax.swing.event.ChangeListener;
import javax.swing.event.ChangeEvent;
import javax.swing.event.TreeSelectionEvent;
import javax.swing.event.TreeSelectionListener;

import javax.swing.tree.TreeCellRenderer;
import javax.swing.tree.TreeSelectionModel;
import javax.swing.tree.TreePath;
import java.awt.*;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.awt.event.ActionEvent;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;


import com.jgoodies.looks.Options;
import com.jgoodies.looks.plastic.PlasticLookAndFeel;
import com.jgoodies.looks.windows.WindowsLookAndFeel;
import com.jgoodies.forms.factories.Borders;
import com.jgoodies.uif_lite.panel.SimpleInternalFrame;


import javax.swing.border.EmptyBorder;
import javax.swing.plaf.SplitPaneUI;
import javax.swing.plaf.basic.BasicSplitPaneUI;
import javax.swing.border.AbstractBorder;

import IceGrid.TreeNode.CommonBase;


public class MainPane extends JSplitPane implements Model.TreeNodeSelector
{
    static class PopupListener extends MouseAdapter
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
		    CommonBase node = (CommonBase)path.getLastPathComponent();
		    JPopupMenu popup = node.getPopupMenu();
		    if(popup != null)
		    {
			popup.show(tree, e.getX(), e.getY());
		    }
		}
	    }
	}
    }

    class SelectionListener implements TreeSelectionListener
    {
	SelectionListener(int view)
	{
	    _view = view;
	}

	public void valueChanged(TreeSelectionEvent e)
	{
	    TreePath path = e.getPath();
	    if(path != null)
	    {
		CommonBase node = (CommonBase)path.getLastPathComponent();
		node.displayProperties(_rightPane, _view);
	    }
	}

	private int _view;

    }

    class TabListener implements ChangeListener
    {
	public void stateChanged(ChangeEvent e)
	{
	    JTabbedPane tabbedPane = (JTabbedPane)e.getSource();
	    int selectedPane = tabbedPane.getSelectedIndex();
	    if(selectedPane >= 0)
	    {
		if(_treeList.size() > selectedPane)
		{
		    JTree tree = (JTree)_treeList.get(selectedPane);
		    TreePath path = tree.getSelectionPath();
		    if(path != null)
		    {
			CommonBase node = (CommonBase)path.getLastPathComponent();

			//
			// Assumes the pane indexing matches the view indexing
			//
			node.displayProperties(_rightPane, selectedPane);
			return;
		    }
		}
	    }
	    if(_rightPane != null)
	    {
		_rightPane.setTitle("Properties");
		_rightPane.setContent(_emptyPanel);
		_rightPane.validate();
		_rightPane.repaint();
	    }  
	}

	void add(JTree tree)
	{
	    _treeList.add(tree);
	}

	private java.util.List _treeList = new java.util.Vector();
    }


    public void updateUI()
    {
	super.updateUI();
	setEmptyDividerBorder();
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
  

    MainPane(Model model)
    {
	super(JSplitPane.HORIZONTAL_SPLIT, true);
	_model = model;
	_model.setTreeNodeSelector(this);

	setBorder(new EmptyBorder(10, 10, 10, 10));
	
	//
	// Left pane
	//
	_tabbedPane = new JTabbedPane();
	_tabbedPane.setMinimumSize(new Dimension(200, 300));
	_tabbedPane.putClientProperty(Options.NO_CONTENT_BORDER_KEY, Boolean.TRUE);
	_tabbedPane.setBorder(new ShadowBorder());
	TabListener tabListener = new TabListener();
	_tabbedPane.addChangeListener(tabListener);

	TreeCellRenderer renderer = new CellRenderer();
	PopupListener popupListener = new PopupListener();


	JTree nodeTree = new JTree(_model.getTreeModel(TreeModelI.NODE_VIEW));
	nodeTree.setCellRenderer(renderer);
        ToolTipManager.sharedInstance().registerComponent(nodeTree);
	nodeTree.addMouseListener(popupListener);
	
	nodeTree.getSelectionModel().setSelectionMode(TreeSelectionModel.SINGLE_TREE_SELECTION);
	SelectionListener treeSelectionListener = new SelectionListener(TreeModelI.NODE_VIEW);
	nodeTree.addTreeSelectionListener(treeSelectionListener);
	nodeTree.setRootVisible(true);
	_treeArray[0] = nodeTree;


	JScrollPane nodeScroll = 
	    new JScrollPane(nodeTree, 
			    JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED, 
			    JScrollPane.HORIZONTAL_SCROLLBAR_NEVER);
	nodeScroll.setBorder(Borders.DIALOG_BORDER);
	

	_tabbedPane.addTab("Node View", nodeScroll);
	tabListener.add(nodeTree);
	
	JTree appTree = new JTree(_model.getTreeModel(TreeModelI.APPLICATION_VIEW));
	appTree.setCellRenderer(renderer);
	ToolTipManager.sharedInstance().registerComponent(appTree);
	appTree.addMouseListener(popupListener);

	appTree.getSelectionModel().setSelectionMode(TreeSelectionModel.SINGLE_TREE_SELECTION);
	SelectionListener appSelectionListener = new SelectionListener(TreeModelI.APPLICATION_VIEW);
	appTree.addTreeSelectionListener(appSelectionListener);
	appTree.setRootVisible(true);
	_treeArray[1] = appTree;
		
	JScrollPane appScroll = 
	    new JScrollPane(appTree,
			    JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED, 
			    JScrollPane.HORIZONTAL_SCROLLBAR_NEVER);
	appScroll.setBorder(Borders.DIALOG_BORDER);
	
	_tabbedPane.addTab("Application View", appScroll);
	tabListener.add(appTree);
	
	JPanel leftPane = new JPanel(new BorderLayout());
	leftPane.add(_tabbedPane);

	//
	// Right pane
	//
	_rightPane = new SimpleInternalFrame("Properties");
	_emptyPanel = new JPanel();
	_emptyPanel.setBackground(Color.RED);
	_rightPane.setContent(_emptyPanel);
	
	setLeftComponent(leftPane);
	setRightComponent(_rightPane);
    }

    public void selectNode(TreePath path, int view)
    {
	if(_tabbedPane.getSelectedIndex() != view)
	{
	    _tabbedPane.setSelectedIndex(view);
	}
	_treeArray[view].setSelectionPath(path);
    }


    //
    // Adapted from JGoodies SimpleInternalFrame
    //
    private static class ShadowBorder extends AbstractBorder 
    {
        private static final Insets INSETS = new Insets(1, 1, 3, 3);
        public Insets getBorderInsets(Component c) { return INSETS; }

        public void paintBorder(Component c, Graphics g,
            int x, int y, int w, int h) 
	{
            Color shadow        = UIManager.getColor("controlShadow");
            if (shadow == null) 
	    {
                shadow = Color.GRAY;
            }
            Color lightShadow   = new Color(shadow.getRed(),
                                            shadow.getGreen(),
                                            shadow.getBlue(),
                                            170);
            Color lighterShadow = new Color(shadow.getRed(),
                                            shadow.getGreen(),
                                            shadow.getBlue(),
                                            70);
            g.translate(x, y);
         
            g.setColor(shadow);

	    int tabAdjustment = 24;

            // g.fillRect(0, 0, w - 3, 1);
            g.fillRect(0, 0 + tabAdjustment, 1, h - 3 - tabAdjustment);
	    g.fillRect(w - 3, 1 + tabAdjustment , 1, h - 3 - tabAdjustment);
            g.fillRect(1, h - 3, w - 3, 1);
            // Shadow line 1
            g.setColor(lightShadow);
            g.fillRect(w - 3, 0 + tabAdjustment, 1, 1);
            g.fillRect(0, h - 3, 1, 1);
            g.fillRect(w - 2, 1 + tabAdjustment, 1, h - 3 - tabAdjustment);
            g.fillRect(1, h - 2, w - 3, 1);
            // Shadow line2
            g.setColor(lighterShadow);
            g.fillRect(w - 2, 0 + tabAdjustment, 1, 1);
            g.fillRect(0, h - 2, 1, 1);
            g.fillRect(w-2, h - 2, 1, 1);
            g.fillRect(w - 1, 1 + tabAdjustment, 1, h - 2 - tabAdjustment);
            g.fillRect(1, h - 1, w - 2, 1);
            g.translate(-x, -y);
        }
    }

    private Model _model;

    private JTabbedPane _tabbedPane;
    private JTree[] _treeArray = new JTree[TreeModelI.VIEW_COUNT];
    private SimpleInternalFrame _rightPane;

    static private JPanel _emptyPanel;
}
