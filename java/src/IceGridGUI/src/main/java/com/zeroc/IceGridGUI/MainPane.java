// Copyright (c) ZeroC, Inc.

package com.zeroc.IceGridGUI;

import com.zeroc.IceGridGUI.Application.Root;

import javax.swing.*;
import javax.swing.border.EmptyBorder;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

public class MainPane extends JTabbedPane {
    public void addApplication(ApplicationPane application) {
        Root root = application.getRoot();
        super.addTab(computeTitle(root.getId()), getIcon(root), application);
    }

    @Override
    public void setTitleAt(int index, String title) {
        super.setTitleAt(index, computeTitle(title));
    }

    public void resetTitle(Root root) {
        int i = findIndex(root);
        if (i > 0) {
            setTitleAt(i, root.getId());
        }
    }

    public void resetIcon(Root root) {
        int i = findIndex(root);
        if (i > 0) {
            setIconAt(i, getIcon(root));
        }
    }

    public int findIndex(Root root) {
        for (int i = 1; i < getTabCount(); i++) {
            ApplicationPane pane = (ApplicationPane) getComponentAt(i);
            if (pane.getRoot() == root) {
                return i;
            }
        }
        return -1;
    }

    public ApplicationPane findApplication(Root root) {
        for (int i = 1; i < getTabCount(); i++) {
            ApplicationPane pane = (ApplicationPane) getComponentAt(i);
            if (pane.getRoot() == root) {
                return pane;
            }
        }
        return null;
    }

    public void removeApplication(Root root) {
        for (int i = 1; i < getTabCount(); i++) {
            ApplicationPane pane = (ApplicationPane) getComponentAt(i);
            if (pane.getRoot() == root) {
                removeTabAt(i);
                break;
            }
        }
    }

    MainPane(Coordinator coordinator) {
        _coordinator = coordinator;
        setBorder(new EmptyBorder(10, 10, 10, 10));

        addChangeListener(
                new ChangeListener() {
                    @Override
                    public void stateChanged(ChangeEvent evt) {
                        Tab tab = (Tab) getSelectedComponent();
                        tab.selected();
                    }
                });

        _registryIcon = Utils.getIcon("/icons/16x16/registry_bound_application.png");
        _fileIcon = Utils.getIcon("/icons/16x16/file_bound_application.png");

        addTab(
                "Live Deployment",
                Utils.getIcon("/icons/16x16/live_deployment.png"),
                _coordinator.getLiveDeploymentPane());
    }

    private String computeTitle(String name) {
        String title = name;
        int i = 0;
        while (hasTitle(title)) {
            title = name + " (" + Integer.toString(++i) + ")";
        }
        return title;
    }

    private boolean hasTitle(String title) {
        int tabCount = getTabCount();
        for (int i = 0; i < tabCount; i++) {
            if (title.equals(getTitleAt(i))) {
                return true;
            }
        }
        return false;
    }

    private ImageIcon getIcon(Root root) {
        if (root.isLive()) {
            return _registryIcon;
        } else if (root.hasFile()) {
            return _fileIcon;
        } else {
            return null;
        }
    }

    private final Coordinator _coordinator;
    private final ImageIcon _registryIcon;
    private final ImageIcon _fileIcon;
}
