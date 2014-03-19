// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This plug-in is provided to you under the terms and conditions
// of the Eclipse Public License Version 1.0 ("EPL"). A copy of
// the EPL is available at http://www.eclipse.org/legal/epl-v10.html.
//
// **********************************************************************

package com.zeroc.slice2javaplugin.properties;

import java.util.ArrayList;
import java.util.Iterator;

import org.eclipse.core.resources.IContainer;
import org.eclipse.core.resources.IFolder;
import org.eclipse.core.resources.IProject;
import org.eclipse.core.resources.IResource;
import org.eclipse.core.runtime.CoreException;
import org.eclipse.jface.viewers.CheckStateChangedEvent;
import org.eclipse.jface.viewers.CheckboxTreeViewer;
import org.eclipse.jface.viewers.ICheckStateListener;
import org.eclipse.jface.viewers.ISelectionChangedListener;
import org.eclipse.jface.viewers.ITreeContentProvider;
import org.eclipse.jface.viewers.SelectionChangedEvent;
import org.eclipse.jface.viewers.TreeSelection;
import org.eclipse.swt.SWT;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.swt.widgets.Tree;
import org.eclipse.ui.dialogs.SelectionDialog;
import org.eclipse.ui.model.WorkbenchContentProvider;
import org.eclipse.ui.model.WorkbenchLabelProvider;

class SourceSelectionDialog extends SelectionDialog
{
    // Do we allow multiple selections?
    private boolean multiple = true;
    
    // the root element to populate the viewer with
    private IProject root;

    // the visual selection widget group
    private CheckboxTreeViewer selectionGroup;
    
    // constants
    private final static int SIZING_SELECTION_WIDGET_WIDTH = 400;

    private final static int SIZING_SELECTION_WIDGET_HEIGHT = 300;

    /**
     * Creates a resource selection dialog rooted at the given element.
     * 
     * @param parentShell
     *            the parent shell
     * @param rootElement
     *            the root element to populate this dialog with
     * @param message
     *            the message to be displayed at the top of this dialog, or
     *            <code>null</code> to display a default message
     */
    public SourceSelectionDialog(Shell parentShell, IProject project, String message)
    {
        super(parentShell);
        setTitle("Source Folder Selection");
        root = project;
        if(message != null)
        {
            setMessage(message);
        }
        else
        {
            setMessage("Select source folder:");
        }
    }
    
    public void setMultiple(boolean m)
    {
        multiple = m;
    }
    
    /*
     * (non-Javadoc) Method declared in Window.
     */
    protected void configureShell(Shell shell)
    {
        super.configureShell(shell);
        //PlatformUI.getWorkbench().getHelpSystem().setHelp(shell, IIDEHelpContextIds.RESOURCE_SELECTION_DIALOG);
    }

    public void create()
    {
        super.create();
        initializeDialog();
    }

    /*
     * (non-Javadoc) Method declared on Dialog.
     */
    protected Control createDialogArea(Composite parent)
    {
        // page group
        Composite composite = (Composite) super.createDialogArea(parent);

        // create the input element, which has the root resource
        // as its only child
        ArrayList<IProject> input = new ArrayList<IProject>();
        input.add(root);

        createMessageArea(composite);
        
        Tree tree = new Tree(composite, ((multiple) ? SWT.CHECK : SWT.SINGLE) | SWT.BORDER);
        GridData data = new GridData(GridData.FILL_BOTH);
        data.widthHint = SIZING_SELECTION_WIDGET_WIDTH;
        data.heightHint = SIZING_SELECTION_WIDGET_HEIGHT;
        tree.setLayoutData(data);
        tree.setFont(parent.getFont());
        
        selectionGroup = new CheckboxTreeViewer(tree);
        selectionGroup.setContentProvider(getResourceProvider(IResource.FILE | IResource.PROJECT));
        selectionGroup.setLabelProvider(WorkbenchLabelProvider.getDecoratingWorkbenchLabelProvider());
        selectionGroup.setInput(input);

        selectionGroup.expandToLevel(2);
        
        return composite;
    }

    /**
     * Returns a content provider for <code>IResource</code>s that returns only
     * children of the given resource type.
     */
    private ITreeContentProvider getResourceProvider(final int resourceType)
    {
        return new WorkbenchContentProvider()
        {
            @SuppressWarnings("unchecked")
            public Object[] getChildren(Object o)
            {
                if(o instanceof IContainer)
                {
                    IResource[] members = null;
                    try
                    {
                        members = ((IContainer) o).members();
                    }
                    catch(CoreException e)
                    {
                        // just return an empty set of children
                        return new Object[0];
                    }

                    // filter out the desired resource types
                    ArrayList<Object> results = new ArrayList<Object>();
                    for(int i = 0; i < members.length; i++)
                    {
                        if(members[i] instanceof IFolder)
                        {
                            results.add(members[i]);
                        }
                    }
                    return results.toArray();
                }
                // input element case
                if(o instanceof ArrayList)
                {
                    return ((ArrayList<Object>) o).toArray();
                }
                return new Object[0];
            }
        };
    }

    /**
     * Initializes this dialog's controls.
     */
    private void initializeDialog()
    {
        getOkButton().setEnabled(false);
        if(multiple)
        {
            selectionGroup.addCheckStateListener(new ICheckStateListener()
            {
                public void checkStateChanged(CheckStateChangedEvent event)
                {
                    if(!event.getChecked() && selectionGroup.getGrayed(event.getElement()))
                    {
                        selectionGroup.setChecked(event.getElement(), true);
                    }
                    else
                    {
                        int count = selectionGroup.getCheckedElements().length - getInitialElementSelections().size();
                        getOkButton().setEnabled(count > 0);
                    }
                }
            });

            for(Iterator<?> iter = getInitialElementSelections().iterator(); iter.hasNext(); )
            {
                IResource cur = (IResource)iter.next();
                selectionGroup.setGrayChecked(cur, true);
            }   
        }
        else
        {
            selectionGroup.addSelectionChangedListener(new ISelectionChangedListener()
            {
                public void selectionChanged(SelectionChangedEvent event)
                {
                    getOkButton().setEnabled(true);
                }
            });
        }
    }

    /**
     * The <code>ResourceSelectionDialog</code> implementation of this
     * <code>Dialog</code> method builds a list of the selected resources for
     * later retrieval by the client and closes this dialog.
     */
    protected void okPressed()
    {
        /*
         * Iterator resultEnum = selectionGroup.getAllCheckedListItems();
         * ArrayList list = new ArrayList(); while (resultEnum.hasNext()) {
         * list.add(resultEnum.next()); } setResult(list);
         */
        ArrayList<Object> list = new ArrayList<Object>();
        if(multiple)
        {
            Object[] objs = selectionGroup.getCheckedElements();
            for(int i = 0; i < objs.length; ++i)
            {
                if(!selectionGroup.getGrayed(objs[i]))
                {
                    list.add(objs[i]);
                }
            }
        }
        else
        {
            TreeSelection obj = (TreeSelection)selectionGroup.getSelection();
            if(!obj.isEmpty())
            {
                list.add(obj.getFirstElement());
            }
        }
        setResult(list);
        super.okPressed();
    }
}
