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
	    _image = Toolkit.getDefaultToolkit().getImage("resources/RedIceCrystal.jpg");
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
	    Dimension d = _rightPane.getSize(null);
	    
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
		    displayWelcomePanel();
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
		if(_model.displayEnabled())
		{
		    _model.showActions(newNode);
		    newNode.displayProperties();
		}
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
	_rightPane = new SimpleInternalFrame("");

	//
	// Welcome panel
	//
	_welcomePanel = new WelcomePanel();
	_welcomePanel.setBackground(Color.RED);
	displayWelcomePanel();

	_model.setPropertiesFrame(_rightPane);
	
	setLeftComponent(leftPane);
	setRightComponent(_rightPane);
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

    private void displayWelcomePanel()
    {
	_rightPane.setTitle("             ");
	_rightPane.setContent(_welcomePanel);
	_rightPane.validate();
	_rightPane.repaint();
    }


    private Model _model;
    private SimpleInternalFrame _rightPane;

    private JPanel _welcomePanel;
}
