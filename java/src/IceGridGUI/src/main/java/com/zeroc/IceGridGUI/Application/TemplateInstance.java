// Copyright (c) ZeroC, Inc.

package com.zeroc.IceGridGUI.Application;

import java.util.List;

interface TemplateInstance {
    Object rebuild(List<Editable> editables) throws UpdateFailedException;

    void restore(Object backupObj);
}
