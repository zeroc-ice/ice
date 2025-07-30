// Copyright (c) ZeroC, Inc.

using System;
using System.IO;
using System.Xml;

namespace ZeroC.Ice.Slice.Tools.Common;

static class DependUtil
{
    internal static XmlNode? FindSourceNode(XmlDocument dependsDoc, string sourcePath)
    {
        var sourceFile = new FileInfo(sourcePath);

        foreach (XmlNode sourceNode in dependsDoc.SelectNodes("/dependencies/source"))
        {
            if (sourceNode.Attributes?["name"] is XmlAttribute nameAttr)
            {
                var nodeFile = new FileInfo(nameAttr.Value);
                if (sourceFile.FullName.Equals(nodeFile.FullName, StringComparison.OrdinalIgnoreCase))
                {
                    return sourceNode;
                }
            }
        }
        return null;
    }
}
