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

import javax.swing.border.EmptyBorder;
import javax.swing.plaf.SplitPaneUI;
import javax.swing.plaf.basic.BasicSplitPaneUI;
import javax.swing.border.AbstractBorder;

import IceGrid.TreeNode.CommonBase;


public class MainPane extends JSplitPane
{
    private class WelcomePanel extends JPanel
    {
	WelcomePanel()
	{
	    MediaTracker mt = new MediaTracker(this);
	    java.net.URL imgURL = Utils.class.getResource("/RedIceCrystal.jpg");
	    assert imgURL != null;
	    _image = Toolkit.getDefaultToolkit().getImage(imgURL);
	    mt.addImage(_image, 0);

	    try
	    {
		mt.waitForAll();
	    }
	    catch(InterruptedException e)
	    {
	    }

	    _aspectRatio = _image.getWidth(null)/_image.getHeight(null);
	}
	
	protected void paintComponent(Graphics g)
	{
	    super.paintComponent(g);    
	    Dimension d = _model.getPropertiesFrame().getSize(null);
	    
	    //
	    // Keep the aspect ratio and make the image fill all the space
	    //
	    if(d.height * _aspectRatio < d.width)
	    {
		d.height = (int)((float)d.width / _aspectRatio);
	    }
	    else
	    {
		d.width = (int)((float)d.height * _aspectRatio);
	    }

	    g.drawImage(_image, 0, 0, d.width + 100, d.height, null); 
	}

	private Image _image;
	private float _aspectRatio;
    }



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
			node.getModel().showActions(node);
			popup.show(tree, e.getX(), e.getY());
		    }
		}
	    }
	}
    }

    class SelectionListener implements TreeSelectionListener
    {
	public void valueChanged(TreeSelectionEvent e)
	{
	    TreePath path = null;
	    if(e.isAddedPath())
	    {
		path = e.getPath();
	    }
	    
	    if(path == null)
	    {
		if(_model.displayEnabled())
		{
		    _model.show(_model.getRoot());
		    // displayWelcomePanel();
		}
	    }
	    else
	    {
		CommonBase newNode = (CommonBase)path.getLastPathComponent();

		if(_previousNode != null && _previousNode.isEphemeral()
		   && _previousNode != newNode)
		{
		    _previousNode.destroy();
		}
		
		//
		// Must be a valid node
		//
		assert newNode.getParent() != null;
		_previousNode = newNode;
		_model.show(newNode);
	    }	    
	}
	private CommonBase _previousNode;
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

	setBorder(new EmptyBorder(10, 10, 10, 10));

	TreeCellRenderer renderer = new CellRenderer();
	PopupListener popupListener = new PopupListener();

	JTree tree = new JTree(_model.getTreeModel());
	//
	// Work-around for Java bug #4833524
	//
	// tree.setUI(new PlasticTreeUI());

	tree.setBorder(new EmptyBorder(5, 5, 5, 5));
	tree.setCellRenderer(renderer);
	ToolTipManager.sharedInstance().registerComponent(tree);
	tree.addMouseListener(popupListener);

	tree.getSelectionModel().setSelectionMode
	    (TreeSelectionModel.SINGLE_TREE_SELECTION);
	
	SelectionListener appSelectionListener = new SelectionListener();
	tree.addTreeSelectionListener(appSelectionListener);
	tree.setRootVisible(false);
	_model.setTree(tree);
		
	JScrollPane appScroll = 
	    new JScrollPane(tree,
			    JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED, 
			    JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);
	appScroll.setBorder(Borders.EMPTY_BORDER);
	
	SimpleInternalFrame leftPane = new SimpleInternalFrame("Applications");
	leftPane.setContent(appScroll);
	leftPane.setPreferredSize(new Dimension(280, 350));

	//
	// Right pane
	//
	JPanel rightPane = new JPanel();
	rightPane.setLayout(new BoxLayout(rightPane, BoxLayout.Y_AXIS));
	rightPane.setBorder(Borders.EMPTY_BORDER);
	rightPane.add(_model.getCurrentStatusFrame());
	rightPane.add(Box.createRigidArea(new Dimension(0, 5)));
	rightPane.add(_model.getPropertiesFrame());

	//_model.show(_model.getRoot());
	
	//
	// Welcome panel
	//
	//_welcomePanel = new WelcomePanel();
	//displayWelcomePanel();
	
	setLeftComponent(leftPane);
	setRightComponent(rightPane);
    }

    private void displayWelcomePanel()
    {
	SimpleInternalFrame propertiesFrame = _model.getPropertiesFrame();
	propertiesFrame.setContent(_welcomePanel);
	propertiesFrame.validate();
	propertiesFrame.repaint();
    }


    private Model _model;
    private JPanel _welcomePanel;
}
