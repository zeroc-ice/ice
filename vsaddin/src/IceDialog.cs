// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections.Generic;
using System.Text;
using EnvDTE;

namespace Ice.VisualStudio
{
    public interface IceConfigurationDialog
    {
        void unsetCancelButton();
        void setCancelButton();        
        void needSave();
        void endEditIncludeDir(bool save);
        bool editingIncludeDir();
    }
}
