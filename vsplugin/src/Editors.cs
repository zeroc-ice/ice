// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.IO;
using System.Text;
using System.ComponentModel;
using System.Globalization;
using System.Drawing;
using System.Drawing.Design;
using System.Windows.Forms.Design;
using System.Windows.Forms;

using EnvDTE;
using Microsoft.VisualStudio.VCProjectEngine;
using Microsoft.VisualStudio.VCProject;
using System.Runtime.InteropServices;

namespace Ice.VisualStudio
{
    [ComVisible(false)]
    public class IncludePathEditor : UITypeEditor
    {
        override public Object EditValue(ITypeDescriptorContext context, IServiceProvider provider, Object value)
        {
            Project project = Util.getSelectedProject();

            IWindowsFormsEditorService service = ((IWindowsFormsEditorService)provider.GetService(
                                                                                typeof(IWindowsFormsEditorService)));
            if(service != null)
            {
                IncludeDirForm dialog = new IncludeDirForm((IncludePathList)value, project);
                service.ShowDialog(dialog);
            }
            return new IncludePathList(Util.getProjectProperty(project, Util.PropertyNames.IceIncludePath));
        }

        public override UITypeEditorEditStyle GetEditStyle(ITypeDescriptorContext context)
        {
            return UITypeEditorEditStyle.Modal;
        }
    }

    [ComVisible(false)]
    public class IceComponentEditor : UITypeEditor
    {
        override public Object EditValue(ITypeDescriptorContext context, IServiceProvider provider, Object value)
        {
            EnvDTE.Project project = Util.getSelectedProject();
            IWindowsFormsEditorService service = ((IWindowsFormsEditorService)provider.GetService(
                                                                                typeof(IWindowsFormsEditorService)));
            if(service != null)
            {
                ProductReferencesForm dialog = new ProductReferencesForm(project);
                service.ShowDialog(dialog);
            }
            return Util.getIceCSharpComponents(project).ToString();
        }

        public override UITypeEditorEditStyle GetEditStyle(ITypeDescriptorContext context)
        {
            return UITypeEditorEditStyle.Modal;
        }
    }

    [ComVisible(false)]
    public class IceLibraryEditor : UITypeEditor
    {
        override public Object EditValue(ITypeDescriptorContext context, IServiceProvider provider, Object value)
        {
            EnvDTE.Project project = Util.getSelectedProject();
            IWindowsFormsEditorService service = ((IWindowsFormsEditorService)provider.GetService(
                                                                                typeof(IWindowsFormsEditorService)));
            if(service != null)
            {
                LibraryReferencesForm dialog = new LibraryReferencesForm(project);
                service.ShowDialog(dialog);
            }
            return Util.getIceCppComponents(project);
        }
        public override UITypeEditorEditStyle GetEditStyle(ITypeDescriptorContext context)
        {
            // We're using a drop down style UITypeEditor.
            return UITypeEditorEditStyle.Modal;
        }
    }

    [ComVisible(false)]
    public class PreprocessorMacroEditor : UITypeEditor
    {
        override public Object EditValue(ITypeDescriptorContext context, IServiceProvider provider, Object value)
        {
            IWindowsFormsEditorService service = ((IWindowsFormsEditorService)provider.GetService(
                                                                                typeof(IWindowsFormsEditorService)));
            Project project = Util.getSelectedProject();
            if (service != null)
            {
                PreprocessorMacrosForm dialog = new PreprocessorMacrosForm(project, (String)value);
                service.ShowDialog(dialog);
            }
            return Util.getProjectProperty(project, Util.PropertyNames.IceMacros);
        }

        public override UITypeEditorEditStyle GetEditStyle(ITypeDescriptorContext context)
        {
            return UITypeEditorEditStyle.Modal;
        }
    }

    [ComVisible(false)]
    public class IceHomeEditor : UITypeEditor
    {
        override public Object EditValue(ITypeDescriptorContext context, IServiceProvider provider, Object value)
        {
            IWindowsFormsEditorService service = ((IWindowsFormsEditorService)provider.GetService(
                                                                                typeof(IWindowsFormsEditorService)));
            Project project = Util.getSelectedProject();
            if (service != null)
            {
                FolderBrowserDialog dialog = new FolderBrowserDialog();
                dialog.SelectedPath = Util.getIceHome(project);
                dialog.Description = "Select Ice Home Installation Directory";
                DialogResult result = dialog.ShowDialog();
                if (result == DialogResult.OK)
                {
                    return dialog.SelectedPath;
                }
                return value;
            }
            return Util.getProjectProperty(project, Util.PropertyNames.IceMacros);
        }

        public override UITypeEditorEditStyle GetEditStyle(ITypeDescriptorContext context)
        {
            return UITypeEditorEditStyle.Modal;
        }
    }
}