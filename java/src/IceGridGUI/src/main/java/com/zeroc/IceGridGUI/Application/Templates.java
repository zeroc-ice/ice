// Copyright (c) ZeroC, Inc.

package com.zeroc.IceGridGUI.Application;

import com.zeroc.IceGrid.*;

import java.util.LinkedList;
import java.util.List;
import java.util.Vector;

abstract class Templates extends ListTreeNode {
    abstract void tryAdd(String newId, TemplateDescriptor descriptor) throws UpdateFailedException;

    protected Templates(Root parent, String id) {
        super(false, parent, id);
    }

    void tryUpdate(Communicator child) throws UpdateFailedException {
        List<? extends TemplateInstance> instanceList = child.findInstances();
        List<Object> backupList = new Vector<>();

        List<Editable> editables = new LinkedList<>();

        for (TemplateInstance p : instanceList) {
            try {
                backupList.add(p.rebuild(editables));
            } catch (UpdateFailedException e) {
                for (int i = backupList.size() - 1; i >= 0; i--) {
                    TemplateInstance instance = instanceList.get(i);
                    instance.restore(backupList.get(i));
                }
                throw e;
            }
        }

        for (Editable p : editables) {
            p.markModified();
        }
    }
}
