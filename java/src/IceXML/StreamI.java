// **********************************************************************
//
// Copyright (c) 2002
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package IceXML;

public class StreamI implements Ice.Stream
{
    private
    StreamI()
    {
    }

    public
    StreamI(Ice.Communicator communicator, java.io.PrintWriter writer)
    {
        _communicator = communicator;
        _os = new IceUtil.XMLOutput(writer);
        _level = 0;
        _nextId = 0;
        _objects = new java.util.IdentityHashMap();
    }

    public
    StreamI(Ice.Communicator communicator, java.io.Reader input)
    {
        this(communicator, input, true);
    }

    public
    StreamI(Ice.Communicator communicator, java.io.Reader input, boolean schema)
    {
        _communicator = communicator;
        _nodeStack = new java.util.LinkedList();
        _readObjects = new java.util.HashMap();
        _nextId = 0;

        Ice.Logger logger = communicator.getLogger();

        //
        // Create our parser, then attach an error handler to the parser.
        // The parser will call back to methods of the ErrorHandler if it
        // discovers errors during the course of parsing the XML
        // document.
        //
        javax.xml.parsers.DocumentBuilderFactory factory = javax.xml.parsers.DocumentBuilderFactory.newInstance();
        //
        // TODO: Using the standard JDK parser, calling setValidating(true)
        // appears to enable DTD validation, not schema validation. This
        // needs to be generalized to support other implementations.
        //
        factory.setValidating(false);
        if(schema)
        {
            factory.setNamespaceAware(true);
            //factory.setDoSchema(true); // TODO
        }

        javax.xml.parsers.DocumentBuilder parser = null;
        try
        {
            parser = factory.newDocumentBuilder();
        }
        catch(javax.xml.parsers.ParserConfigurationException ex)
        {
            logger.error("parser configuration error: " + ex.getMessage());
            Ice.MarshalException e = new Ice.MarshalException();
            e.initCause(ex);
            throw e;
        }

        parser.setErrorHandler(new DOMTreeErrorReporter(logger));
        //parser.setCreateEntityReferenceNodes(false); // TODO
        //parser.setToCreateXMLDeclTypeNode(true); // TODO

        // TODO:
        // parser.setEntityResolver

        //
        // Parse the XML file, catching any XML exceptions that might propagate
        // out of it.
        //
        boolean errorsOccurred = false;
        try
        {
            _document = parser.parse(new org.xml.sax.InputSource(input));
        }
        catch(java.io.IOException ex)
        {
            logger.error("I/O error during parsing: " + ex.getMessage());
            errorsOccurred = true;
        }
        catch(org.xml.sax.SAXException ex)
        {
            logger.error("parsing error: " + ex.getMessage());
            errorsOccurred = true;
        }

        if(errorsOccurred)
        {
            throw new Ice.MarshalException();
        }

        //
        // The first child of the document is the root node - ignore
        // that. Move to the top-level node in the document content.
        //
        _current = _document.getFirstChild();
        _current = _current.getFirstChild();
    }

    public
    StreamI(Ice.Communicator communicator, org.w3c.dom.Document document)
    {
        _communicator = communicator;
        _nodeStack = new java.util.LinkedList();
        _readObjects = new java.util.HashMap();
        _nextId = 0;
        _document = document;

        //
        // The first child of the document is the root node - ignore
        // that. Move to the top-level node in the document content.
        //
        _current = _document.getFirstChild();
        _current = _current.getFirstChild();
    }

    public void
    startWriteDictionary(String name, int size)
    {
        String s = name + " length=\"" + size + "\"";
        startWrite(s);
    }

    public void
    endWriteDictionary()
    {
        endWrite();
    }

    public void
    startWriteDictionaryElement()
    {
        startWrite(seqElementName);
    }

    public void
    endWriteDictionaryElement()
    {
        endWrite();
    }

    public int
    startReadDictionary(String name)
    {
        startRead(name);
        int size = readLength();
        _current = _current.getFirstChild();
        return size;
    }

    public void
    endReadDictionary()
    {
        endRead();
    }

    public void
    startReadDictionaryElement()
    {
        startRead(seqElementName);
        _current = _current.getFirstChild();
    }

    public void
    endReadDictionaryElement()
    {
        endRead();
    }

    public void
    startWriteSequence(String name, int size)
    {
        String s = name + " length=\"" + size + "\"";
        startWrite(s);
    }

    public void
    endWriteSequence()
    {
        endWrite();
    }

    public void
    startWriteSequenceElement()
    {
        startWrite(seqElementName);
    }

    public void
    endWriteSequenceElement()
    {
        endWrite();
    }

    public int
    startReadSequence(String name)
    {
        startRead(name);
        int size = readLength();
        _current = _current.getFirstChild();
        return size;
    }

    public void
    endReadSequence()
    {
        endRead();
    }

    public void
    startReadSequenceElement()
    {
        startRead(seqElementName);
    }

    public void
    endReadSequenceElement()
    {
        endRead();
    }

    public void
    startWriteStruct(String name)
    {
        startWrite(name);
    }

    public void
    endWriteStruct()
    {
        endWrite();
    }

    public void
    startReadStruct(String name)
    {
        startRead(name);
        _current = _current.getFirstChild();
    }

    public void
    endReadStruct()
    {
        endRead();
    }

    public void
    startWriteException(String name)
    {
        startWrite(name);
    }

    public void
    endWriteException()
    {
        endWrite();
    }

    public void
    startReadException(String name)
    {
        startRead(name);
        _current = _current.getFirstChild();
    }

    public void
    endReadException()
    {
        endRead();
    }

    public void
    writeEnum(String name, String[] table, int ordinal)
    {
        // No attributes
        assert(name.indexOf(' ') == -1 && name.indexOf('\t') == -1);
        _os.nl();
        _os.print("<" + name + ">" + table[ordinal] + "</" + name + ">");
    }

    public int
    readEnum(String name, String[] table)
    {
        startRead(name);

        org.w3c.dom.Node child = _current.getFirstChild();
        if(child == null || child.getNodeType() != org.w3c.dom.Node.TEXT_NODE)
        {
            throw new Ice.MarshalException();
        }

        String value = child.getNodeValue();
        int ordinal;
        for(ordinal = 0; ordinal < table.length; ordinal++)
        {
            if(value.equals(table[ordinal]))
            {
                break;
            }
        }
        if(ordinal == table.length)
        {
            throw new Ice.MarshalException();
        }

        endRead();

        return ordinal;
    }

    public void
    writeByte(String name, byte value)
    {
        // No attributes
        assert(name.indexOf(' ') == -1 && name.indexOf('\t') == -1);

        // The schema encoding for xs:byte is a value from -127 to 128.
        _os.nl();
        _os.print("<" + name + ">" + (int)value + "</" + name + ">");
    }

    public void
    writeByteSeq(String name, byte[] value)
    {
        startWrite(name);
        for(int i = 0; i < value.length; i++)
        {
            _os.nl();
            _os.print("<e>" + (int)value[i] + "</e>");
        }
        endWrite();
    }

    public byte
    readByte(String name)
    {
        startRead(name);

        org.w3c.dom.Node child = _current.getFirstChild();
        if(child == null || child.getNodeType() != org.w3c.dom.Node.TEXT_NODE)
        {
            throw new Ice.MarshalException();
        }

        String s = child.getNodeValue();
        int i = 0;
        try
        {
            i = Integer.parseInt(s);
        }
        catch(NumberFormatException ex)
        {
            Ice.MarshalException e = new Ice.MarshalException();
            e.initCause(ex);
            throw e;
        }

        endRead();

        return (byte)i;
    }

    public byte[]
    readByteSeq(String name)
    {
        startRead(name);

        int size = readLength();
        byte[] value = new byte[size];
        if(size > 0)
        {
            _current = _current.getFirstChild();
            for(int i = 0; i < size; i++)
            {
                value[i] = readByte(seqElementName);
            }
        }

        endRead();

        return value;
    }

    public void
    writeBool(String name, boolean value)
    {
        // No attributes
        assert(name.indexOf(' ') == -1 && name.indexOf('\t') == -1);
        _os.nl();
        _os.print("<" + name + ">" + (value ? "true" : "false") + "</" + name + ">");
    }

    public void
    writeBoolSeq(String name, boolean[] value)
    {
        startWrite(name);
        for(int i = 0; i < value.length; i++)
        {
            _os.nl();
            _os.print("<e>" + (value[i] ? "true" : "false") + "</e>");
        }
        endWrite();
    }

    public boolean
    readBool(String name)
    {
        startRead(name);

        org.w3c.dom.Node child = _current.getFirstChild();
        if(child == null || child.getNodeType() != org.w3c.dom.Node.TEXT_NODE)
        {
            throw new Ice.MarshalException();
        }

        String s = child.getNodeValue();
        boolean value = s.equals("true");

        endRead();

        return value;
    }

    public boolean[]
    readBoolSeq(String name)
    {
        startRead(name);

        int size = readLength();
        boolean[] value = new boolean[size];
        if(size > 0)
        {
            _current = _current.getFirstChild();
            for(int i = 0; i < size; i++)
            {
                value[i] = readBool(seqElementName);
            }
        }

        endRead();

        return value;
    }

    public void
    writeShort(String name, short value)
    {
        // No attributes
        assert(name.indexOf(' ') == -1 && name.indexOf('\t') == -1);
        _os.nl();
        _os.print("<" + name + ">" + value + "</" + name + ">");
    }

    public void
    writeShortSeq(String name, short[] value)
    {
        startWrite(name);
        for(int i = 0; i < value.length; i++)
        {
            _os.nl();
            _os.print("<e>" + value[i] + "</e>");
        }
        endWrite();
    }

    public short
    readShort(String name)
    {
        startRead(name);

        org.w3c.dom.Node child = _current.getFirstChild();
        if(child == null || child.getNodeType() != org.w3c.dom.Node.TEXT_NODE)
        {
            throw new Ice.MarshalException();
        }

        String s = child.getNodeValue();
        short i = 0;
        try
        {
            i = Short.parseShort(s);
        }
        catch(NumberFormatException ex)
        {
            Ice.MarshalException e = new Ice.MarshalException();
            e.initCause(ex);
            throw e;
        }

        endRead();

        return i;
    }

    public short[]
    readShortSeq(String name)
    {
        startRead(name);

        int size = readLength();
        short[] value = new short[size];
        if(size > 0)
        {
            _current = _current.getFirstChild();
            for(int i = 0; i < size; i++)
            {
                value[i] = readShort(seqElementName);
            }
        }

        endRead();

        return value;
    }

    public void
    writeInt(String name, int value)
    {
        // No attributes
        assert(name.indexOf(' ') == -1 && name.indexOf('\t') == -1);
        _os.nl();
        _os.print("<" + name + ">" + value + "</" + name + ">");
    }

    public void
    writeIntSeq(String name, int[] value)
    {
        startWrite(name);
        for(int i = 0; i < value.length; i++)
        {
            _os.nl();
            _os.print("<e>" + value[i] + "</e>");
        }
        endWrite();
    }

    public int
    readInt(String name)
    {
        startRead(name);

        org.w3c.dom.Node child = _current.getFirstChild();
        if(child == null || child.getNodeType() != org.w3c.dom.Node.TEXT_NODE)
        {
            throw new Ice.MarshalException();
        }

        String s = child.getNodeValue();
        int i = 0;
        try
        {
            i = Integer.parseInt(s);
        }
        catch(NumberFormatException ex)
        {
            Ice.MarshalException e = new Ice.MarshalException();
            e.initCause(ex);
            throw e;
        }

        endRead();

        return i;
    }

    public int[]
    readIntSeq(String name)
    {
        startRead(name);

        int size = readLength();
        int[] value = new int[size];
        if(size > 0)
        {
            _current = _current.getFirstChild();
            for(int i = 0; i < size; i++)
            {
                value[i] = readInt(seqElementName);
            }
        }

        endRead();

        return value;
    }

    public void
    writeLong(String name, long value)
    {
        // No attributes
        assert(name.indexOf(' ') == -1 && name.indexOf('\t') == -1);
        _os.nl();
        _os.print("<" + name + ">" + value + "</" + name + ">");
    }

    public void
    writeLongSeq(String name, long[] value)
    {
        startWrite(name);
        for(int i = 0; i < value.length; i++)
        {
            _os.nl();
            _os.print("<e>" + value[i] + "</e>");
        }
        endWrite();
    }

    public long
    readLong(String name)
    {
        startRead(name);

        org.w3c.dom.Node child = _current.getFirstChild();
        if(child == null || child.getNodeType() != org.w3c.dom.Node.TEXT_NODE)
        {
            throw new Ice.MarshalException();
        }

        String s = child.getNodeValue();
        long i = 0;
        try
        {
            i = Long.parseLong(s);
        }
        catch(NumberFormatException ex)
        {
            Ice.MarshalException e = new Ice.MarshalException();
            e.initCause(ex);
            throw e;
        }

        endRead();

        return i;
    }

    public long[]
    readLongSeq(String name)
    {
        startRead(name);

        int size = readLength();
        long[] value = new long[size];
        if(size > 0)
        {
            _current = _current.getFirstChild();
            for(int i = 0; i < size; i++)
            {
                value[i] = readLong(seqElementName);
            }
        }

        endRead();

        return value;
    }

    public void
    writeFloat(String name, float value)
    {
        // No attributes
        assert(name.indexOf(' ') == -1 && name.indexOf('\t') == -1);
        _os.nl();
        _os.print("<" + name + ">" + value + "</" + name + ">");
    }

    public void
    writeFloatSeq(String name, float[] value)
    {
        startWrite(name);
        for(int i = 0; i < value.length; i++)
        {
            _os.nl();
            _os.print("<e>" + value[i] + "</e>");
        }
        endWrite();
    }

    public float
    readFloat(String name)
    {
        startRead(name);

        org.w3c.dom.Node child = _current.getFirstChild();
        if(child == null || child.getNodeType() != org.w3c.dom.Node.TEXT_NODE)
        {
            throw new Ice.MarshalException();
        }

        String s = child.getNodeValue();
        float i = 0;
        try
        {
            i = Float.parseFloat(s);
        }
        catch(NumberFormatException ex)
        {
            Ice.MarshalException e = new Ice.MarshalException();
            e.initCause(ex);
            throw e;
        }

        endRead();

        return i;
    }

    public float[]
    readFloatSeq(String name)
    {
        startRead(name);

        int size = readLength();
        float[] value = new float[size];
        if(size > 0)
        {
            _current = _current.getFirstChild();
            for(int i = 0; i < size; i++)
            {
                value[i] = readFloat(seqElementName);
            }
        }

        endRead();

        return value;
    }

    public void
    writeDouble(String name, double value)
    {
        // No attributes
        assert(name.indexOf(' ') == -1 && name.indexOf('\t') == -1);
        _os.nl();
        _os.print("<" + name + ">" + value + "</" + name + ">");
    }

    public void
    writeDoubleSeq(String name, double[] value)
    {
        startWrite(name);
        for(int i = 0; i < value.length; i++)
        {
            _os.nl();
            _os.print("<e>" + value[i] + "</e>");
        }
        endWrite();
    }

    public double
    readDouble(String name)
    {
        startRead(name);

        org.w3c.dom.Node child = _current.getFirstChild();
        if(child == null || child.getNodeType() != org.w3c.dom.Node.TEXT_NODE)
        {
            throw new Ice.MarshalException();
        }

        String s = child.getNodeValue();
        double i = 0;
        try
        {
            i = Double.parseDouble(s);
        }
        catch(NumberFormatException ex)
        {
            Ice.MarshalException e = new Ice.MarshalException();
            e.initCause(ex);
            throw e;
        }

        endRead();

        return i;
    }

    public double[]
    readDoubleSeq(String name)
    {
        startRead(name);

        int size = readLength();
        double[] value = new double[size];
        if(size > 0)
        {
            _current = _current.getFirstChild();
            for(int i = 0; i < size; i++)
            {
                value[i] = readDouble(seqElementName);
            }
        }

        endRead();

        return value;
    }

    public void
    writeString(String name, String value)
    {
        // No attributes
        assert(name.indexOf(' ') == -1 && name.indexOf('\t') == -1);
        _os.nl();
        _os.print("<" + name + ">");
        _os.printEscaped(value);
        _os.print("</" + name + ">");
    }

    public void
    writeStringSeq(String name, String[] value)
    {
        startWrite(name);
        for(int i = 0; i < value.length; i++)
        {
            _os.nl();
            _os.print("<e>");
            _os.printEscaped(value[i]);
            _os.print("</e>");
        }
        endWrite();
    }

    public String
    readString(String name)
    {
        String value = null;

        startRead(name);

        org.w3c.dom.Node child = _current.getFirstChild();
        if(child != null)
        {
            if(child.getNodeType() != org.w3c.dom.Node.TEXT_NODE)
            {
                throw new Ice.MarshalException();
            }
            value = child.getNodeValue();
        }
        else
        {
            value = "";
        }

        endRead();

        return value;
    }

    public String[]
    readStringSeq(String name)
    {
        startRead(name);

        int size = readLength();
        String[] value = new String[size];
        if(size > 0)
        {
            _current = _current.getFirstChild();
            for(int i = 0; i < size; i++)
            {
                value[i] = readString(seqElementName);
            }
        }

        endRead();

        return value;
    }

    public void
    writeProxy(String name, Ice.ObjectPrx value)
    {
        // No attributes
        assert(name.indexOf(' ') == -1 && name.indexOf('\t') == -1);
        String s = _communicator.proxyToString(value);
        _os.nl();
        _os.print("<" + name + ">" + s + "</" + name + ">");
    }

    public Ice.ObjectPrx
    readProxy(String name)
    {
        Ice.ObjectPrx value = null;

        startRead(name);

        org.w3c.dom.Node child = _current.getFirstChild();
        String s = "";
        if(child != null)
        {
            if(child.getNodeType() != org.w3c.dom.Node.TEXT_NODE)
            {
                throw new Ice.MarshalException();
            }
            s = child.getNodeValue();
        }

        value = _communicator.stringToProxy(s);

        endRead();

        return value;
    }

    public void
    writeObject(String name, Ice.Object value)
    {
        //
        // If at the top level of the document then the object itself must
        // be written, otherwise write a reference.
        //
        boolean writeReference = (_level != 0);

        //
        // If the object doesn't exist in the map add it.
        //
        ObjectInfo info = (ObjectInfo)_objects.get(value);
        if(info == null)
        {
            String s = "object" + _nextId++;
            info = new ObjectInfo(s, !writeReference);
            _objects.put(value, info);
        }

        if(writeReference)
        {
            _os.nl();
            _os.print("<" + name + " href =\"#" + info.id + "\"/>");
        }
        else
        {
            //
            // Precondition: If the actual object is being written the
            // written flag should be true.
            //
            assert(info.written);
            writeObjectData(name, info.id, value);
        }
    }

    public Ice.Object
    readObject(String name, String signatureType, Ice.ObjectFactory factory)
    {
        Ice.Object value = null;

        startRead(name);

        //
        // Node attributes. Either id & type will be set, or href.
        //
        // TODO: Eliminate Holders?
        //
        Ice.StringHolder id = new Ice.StringHolder();
        Ice.StringHolder type = new Ice.StringHolder();
        Ice.StringHolder href = new Ice.StringHolder();
        readAttributes(id, type, href);

        //
        // If this object is an href then locate the object. If the object
        // has been previously read then it will be contained in the
        // readObjects list. Otherwise, it should be in the object-set at
        // the top-level of the document.
        //
        if(href.value != null && href.value.length() > 0)
        {
            href.value = href.value.substring(1); // Skip the '#'

            value = (Ice.Object)_readObjects.get(href.value);
            if(value != null)
            {
                endRead();
                return value;
            }

            //
            // The first child of the document is the root node - ignore
            // that. Move to the top-level node in the document content.
            //
            _current = _document.getFirstChild();
            _current = _current.getFirstChild();
            while(_current != null)
            {
                if(_current.getNodeType() == org.w3c.dom.Node.ELEMENT_NODE)
                {
                    Ice.StringHolder dummy = new Ice.StringHolder();
                    readAttributes(id, type, dummy);
                    if(id.value.equals(href.value))
                    {
                        break;
                    }
                }
                _current = _current.getNextSibling();
            }
            //
            // If the object isn't found, that's an error.
            //
            if(_current == null)
            {
                throw new Ice.MarshalException();
            }
        }

        //
        // Empty type indicates nil object.
        //
        if(type.value.length() == 0)
        {
            value = null;
        }
        else
        {
            if(type.value.equals("::Ice::Object"))
            {
                value = new Ice.Object();
            }
            else
            {
                //
                // First, determine if there is an installed factory for this
                // type. If so use that. If that factory doesn't exist, or it
                // doesn't create an object of that type then use the supplied
                // factory provided that the types are the same. If no value
                // can be created then throw a NoObjectFactoryException.
                //
                Ice.ObjectFactory userFactory = _communicator.findObjectFactory(type.value);
                if(userFactory != null)
                {
                    value = userFactory.create(type.value);
                }

                if(value == null && type.value.equals(signatureType))
                {
                    assert(factory != null);
                    value = factory.create(type.value);
                    assert(value != null);
                }

                if(value == null)
                {
                    throw new Ice.NoObjectFactoryException();
                }
            }

            //
            // Add the object to the readObjects map, move to the first
            // child node & unmarshal the object.
            //
            _readObjects.put(id.value, value);
            _current = _current.getFirstChild();
            value.__unmarshal(this);
        }

        endRead();

        return value;
    }

    private void
    startWrite(String name)
    {
        _os.se(name);
        ++_level;
    }

    private void
    endWrite()
    {
        --_level;

        _os.ee();

        if(_level == 0)
        {
            dumpUnwrittenObjects();
        }
    }

    private void
    startRead(String name)
    {
        while(_current != null && _current.getNodeType() != org.w3c.dom.Node.ELEMENT_NODE)
        {
            _current = _current.getNextSibling();
        }
        if(_current == null)
        {
            throw new Ice.MarshalException();
        }

        String nodeName = _current.getNodeName();
        //
        // TODO: Work around for bug in xerces
        //
        final String facets = "facets";
        final String facetsNS = "ice:facets";
        if((!name.equals(facetsNS) || !nodeName.equals(facets)) && !name.equals(nodeName))
        {
            throw new Ice.MarshalException();
        }

        _nodeStack.add(_current);
    }

    private void
    endRead()
    {
        _current = (org.w3c.dom.Node)_nodeStack.removeLast();
        _current = _current.getNextSibling();
    }

    private void
    dumpUnwrittenObjects()
    {
        //
        // Precondition: Must be at the top-level.
        //
        assert(_level == 0);

        //
        // It's necessary to run through the set of unwritten objects
        // repeatedly until all referenced objects have been written
        // (since referenced objects can themselves reference other
        // objects).
        //
        int nwritten;
        do
        {
            nwritten = 0;
            java.util.Iterator p = _objects.entrySet().iterator();
            while(p.hasNext())
            {
                java.util.Map.Entry e = (java.util.Map.Entry)p.next();
                Ice.Object obj = (Ice.Object)e.getKey();
                ObjectInfo info = (ObjectInfo)e.getValue();
                if(!info.written)
                {
                    info.written = true;
                    writeObjectData("ice:object", info.id, obj);
                    ++nwritten;
                }
                else
                {
                    ++nwritten;
                }
            }
        }
        while(_objects.size() != nwritten);
    }

    private void
    writeObjectData(String name, String id, Ice.Object obj)
    {
        String xsdType = "";
        String typeId = "";

        if(obj != null)
        {
            //
            // Convert the type-id to the equivalent schema type
            // definition name.
            //
            typeId = obj.ice_id(null);

            final String xsdPrefix = "_internal.";
            final String xsdSuffix = "Type";
            xsdType += xsdPrefix;
            int i = 0;
            if(typeId.charAt(0) == ':') // Skip the preceeding "::".
            {
                i = 2;
            }
            for(; i < typeId.length(); ++i)
            {
                if(typeId.charAt(i) == ':')
                {
                    xsdType += '.';
                    ++i;
                }
                else
                {
                    xsdType += typeId.charAt(i);
                }
            }
            xsdType += xsdSuffix;
        }
        else
        {
            xsdType = "";
        }

        String s = name + " id=\"" + id + "\" type=\"" + typeId + "\"" +
           " xsi:type=\"" + xsdType + "\"";
        if(obj == null)
        {
            s += " xsi:nil=\"true\"";
        }

        startWrite(s);
        if(obj != null)
        {
            obj.__marshal(this);
        }
        endWrite();
    }

    private void
    readAttributes(Ice.StringHolder id, Ice.StringHolder type, Ice.StringHolder href)
    {
        final String idStr = "id";
        final String typeStr = "type";
        final String hrefStr = "href";

        org.w3c.dom.NamedNodeMap attributes = _current.getAttributes();
        int attrCount = attributes.getLength();
        for(int i = 0; i < attrCount; i++)
        {
            org.w3c.dom.Node attribute = attributes.item(i);
            String name = attribute.getNodeName();
            if(name.equals(idStr))
            {
                id.value = attribute.getNodeValue();
            }
            else if(name.equals(typeStr))
            {
                type.value = attribute.getNodeValue();
            }
            else if(name.equals(hrefStr))
            {
                href.value = attribute.getNodeValue();
            }
        }
    }

    private int
    readLength()
    {
        final String lengthStr = "length";

        org.w3c.dom.NamedNodeMap attributes = _current.getAttributes();
        int attrCount = attributes.getLength();
        for(int i = 0; i < attrCount; i++)
        {
            org.w3c.dom.Node attribute = attributes.item(i);
            String name = attribute.getNodeName();
            if(name.equals(lengthStr))
            {
                try
                {
                    return Integer.parseInt(attribute.getNodeValue());
                }
                catch(NumberFormatException ex)
                {
                    throw new Ice.MarshalException();
                }
            }
        }

        throw new Ice.MarshalException();
    }

    private Ice.Communicator _communicator;

    //
    // The sequence element name
    //
    private static final String seqElementName = "e";

    //
    // For reading.
    //
    private org.w3c.dom.Document _document;
    private org.w3c.dom.Node _current;
    private java.util.LinkedList _nodeStack;
    private java.util.HashMap _readObjects;

    //
    // For writing.
    //

    //
    // XML Output stream.
    //
    private IceUtil.XMLOutput _os;

    //
    // Current document level.
    //
    private int _level;

    //
    // Next id for marshalling objects.
    //
    private int _nextId;
    private static class ObjectInfo
    {
        ObjectInfo(String i, boolean w)
        {
            id = i;
            written = w;
        }

        String id; // The id of the associated object
        boolean written; // Has the object been written?
    }
    private java.util.IdentityHashMap _objects;

    private static class DOMTreeErrorReporter implements org.xml.sax.ErrorHandler
    {
        DOMTreeErrorReporter(Ice.Logger logger)
        {
            _logger = logger;
            _sawErrors = false;
        }

        public void
        warning(org.xml.sax.SAXParseException ex)
            throws org.xml.sax.SAXException
        {
            String s = "file: \"" + ex.getSystemId() +
                "\", line: " + ex.getLineNumber() +
                ", column: " + ex.getColumnNumber() +
                "\nmessage: " + ex.getMessage();
            _logger.warning(s);
        }

        public void
        error(org.xml.sax.SAXParseException ex)
            throws org.xml.sax.SAXException
        {
            _sawErrors = true;
            String s = "file: \"" + ex.getSystemId() +
                "\", line: " + ex.getLineNumber() +
                ", column: " + ex.getColumnNumber() +
                "\nmessage: " + ex.getMessage();
            _logger.error(s);
        }

        public void
        fatalError(org.xml.sax.SAXParseException ex)
            throws org.xml.sax.SAXException
        {
            _sawErrors = true;
            String s = "file: \"" + ex.getSystemId() +
                "\", line: " + ex.getLineNumber() +
                ", column: " + ex.getColumnNumber() +
                "\nmessage: " + ex.getMessage();
            _logger.error(s);
        }

        boolean
        getSawErrors()
        {
            return _sawErrors;
        }

        private Ice.Logger _logger;
        private boolean _sawErrors;
    }
}
