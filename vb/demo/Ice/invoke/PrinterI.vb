' **********************************************************************
'
' Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
'
' This copy of Ice is licensed to you under the terms described in the
' ICE_LICENSE file included in this distribution.
'
' **********************************************************************

Imports System
Imports System.Collections
Imports InvokeDemo

Public Class PrinterI
    Inherits Ice.Blobject

    Public Overrides Function ice_invoke(ByVal inparams As Byte(), ByRef outParams As Byte(), ByVal current As Ice.Current) As Boolean

        outParams = Nothing

        Dim communicator As Ice.Communicator = current.adapter.getCommunicator()

        Dim inStream As Ice.InputStream = Nothing
        If inparams.Length > 0 Then
            inStream = Ice.Util.createInputStream(communicator, inparams)
        End If

        If current.operation.Equals("printString") Then
            Dim message As String = inStream.readString()
            inStream.destroy()
            Console.WriteLine("Printing string `" & message & "'")
            Return True
        ElseIf current.operation.Equals("printStringSequence") Then
            Dim seq As String() = StringSeqHelper.read(inStream)
            inStream.destroy()
            Console.Write("Printing string sequence {")
            For i As Integer = 0 To seq.Length - 1
                If i > 0 Then
                    Console.Write(", ")
                End If
                Console.Write("'" & seq(i) & "'")
            Next
            Console.WriteLine("}")
            Return True
        ElseIf current.operation.Equals("printDictionary") Then
            Dim dict As StringDict = StringDictHelper.read(inStream)
            inStream.destroy()
            Console.Write("Printing dictionary {")
            Dim first As Boolean = True
            For Each e As DictionaryEntry In dict
                If Not first Then
                    Console.Write(", ")
                End If
                first = False
                Console.Write(e.Key.ToString() & "=" & e.Value.ToString())
            Next
            Console.WriteLine("}")
            Return True
        ElseIf current.operation.Equals("printEnum") Then
            Dim c As Color = ColorHelper.read(inStream)
            inStream.destroy()
            Console.WriteLine("Printing enum " & c)
            Return True
        ElseIf current.operation.Equals("printStruct") Then
            Dim s As [Structure] = New [Structure]
            s.ice_read(inStream)
            inStream.destroy()
            Console.WriteLine("Printing struct: name=" & s.name & ", value=" & s.value)
            Return True
        ElseIf current.operation.Equals("printStructSequence") Then
            Dim seq As [Structure]() = StructureSeqHelper.read(inStream)
            inStream.destroy()
            Console.Write("Printing struct sequence: {")
            For i As Integer = 0 To seq.Length - 1
                If i > 0 Then
                    Console.Write(", ")
                End If
                Console.Write(seq(i).name & "=" & seq(i).value)
            Next
            Console.WriteLine("}")
            Return True
        ElseIf current.operation.Equals("printClass") Then
            Dim ch As CHelper = New CHelper(inStream)
            ch.read()
            inStream.readPendingObjects()
            inStream.destroy()
            Dim c As InvokeDemo.C = ch.value
            Console.WriteLine("Printing class: s.name=" & c.s.name & ", s.value=" & c.s.value)
            Return True
        ElseIf current.operation.Equals("getValues") Then
            Dim c As InvokeDemo.C = New InvokeDemo.C
            c.s = New [Structure]
            c.s.name = "green"
            c.s.value = Color.green
            Dim outStream As Ice.OutputStream = Ice.Util.createOutputStream(communicator)
            CHelper.write(outStream, c)
            outStream.writeString("hello")
            outStream.writePendingObjects()
            outParams = outStream.finished()
            Return True
        ElseIf current.operation.Equals("throwPrintFailure") Then
            Console.WriteLine("Throwing PrintFailure")
            Dim ex As PrintFailure = New PrintFailure
            ex.reason = "paper tray empty"
            Dim outStream As Ice.OutputStream = Ice.Util.createOutputStream(communicator)
            outStream.writeException(ex)
            outParams = outStream.finished()
            Return False
        ElseIf current.operation.Equals("shutdown") Then
            current.adapter.getCommunicator().shutdown()
            Return True
        Else
            Dim ex As Ice.OperationNotExistException = New Ice.OperationNotExistException
            ex.id = current.id
            ex.facet = current.facet
            ex.operation = current.operation
            Throw ex
        End If
    End Function

End Class
