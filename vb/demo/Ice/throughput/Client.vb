' **********************************************************************
'
' Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
'
' This copy of Ice is licensed to you under the terms described in the
' ICE_LICENSE file included in this distribution.
'
' **********************************************************************

Imports System
Imports Demo

Module ThroughputC

    Class Client
        Inherits Ice.Application

        Private Sub menu()
            Console.Out.WriteLine("usage:")
            Console.Out.WriteLine()
            Console.Out.WriteLine("toggle type of data to send:")
            Console.Out.WriteLine("1: sequence of bytes (default)")
            Console.Out.WriteLine("2: sequence of strings(""hello"")")
            Console.Out.WriteLine("3: sequence of structs with a string (""hello"") and a double")
            Console.Out.WriteLine("4: sequence of structs with two ints and a double")
            Console.Out.WriteLine()
            Console.Out.WriteLine("select test to run:")
            Console.Out.WriteLine("t: Send sequence as twoway")
            Console.Out.WriteLine("o: Send sequence as oneway")
            Console.Out.WriteLine("r: Receive sequence")
            Console.Out.WriteLine("e: Echo (send and receive) sequence")
            Console.Out.WriteLine()
            Console.Out.WriteLine("other commands")
            Console.Out.WriteLine("s: shutdown server")
            Console.Out.WriteLine("x: exit")
            Console.Out.WriteLine("?: help")
        End Sub

        Public Overloads Overrides Function run(ByVal args() As String) As Integer
            Dim throughput As ThroughputPrx = ThroughputPrxHelper.checkedCast(communicator.propertyToProxy("Throughput.Throughput"))
            If throughput Is Nothing Then
                Console.Error.WriteLine("invalid proxy")
                Return 1
            End If
            Dim throughputOneway As ThroughputPrx = ThroughputPrxHelper.uncheckedCast(throughput.ice_oneway())

            Dim byteSeq() As Byte = New Byte(ByteSeqSize.value - 1) {}

            Dim stringSeq() As String = New String(StringSeqSize.value - 1) {}
            For i As Integer = 0 To StringSeqSize.value - 1
                stringSeq(i) = "hello"
            Next

            Dim structSeq() As StringDouble = New StringDouble(StringDoubleSeqSize.value - 1) {}
            For i As Integer = 0 To StringDoubleSeqSize.value - 1
                structSeq(i) = New StringDouble
                structSeq(i).s = "hello"
                structSeq(i).d = 3.14
            Next

            Dim fixedSeq() As Fixed = New Fixed(FixedSeqSize.value - 1) {}
            For i As Integer = 0 To FixedSeqSize.value - 1
                fixedSeq(i).i = 0
                fixedSeq(i).j = 0
                fixedSeq(i).d = 0
            Next

            '
            ' A method needs to be invoked thousands of times before the JIT compiler
            ' will convert it to native code. To ensure an accurate throughput measurement,
            ' we need to "warm up" the JIT compiler.
            '
            Dim emptyBytes() As Byte = New Byte(0) {}
            Dim emptyStrings() As String = New String(0) {}
            Dim emptyStructs() As StringDouble = New StringDouble(0) {}
            emptyStructs(0) = New StringDouble
            Dim emptyFixed() As Fixed = New Fixed(0) {}
            emptyFixed(0) = New Fixed

            Dim repetitions As Integer = 10000
            Console.Out.Write("warming up the JIT compiler...")
            Console.Out.Flush()
            For i As Integer = 0 To repetitions - 1
               throughput.sendByteSeq(emptyBytes)
               throughput.sendStringSeq(emptyStrings)
               throughput.sendStructSeq(emptyStructs)
               throughput.sendFixedSeq(emptyFixed)

               throughput.recvByteSeq()
               throughput.recvStringSeq()
               throughput.recvStructSeq()
               throughput.recvFixedSeq()

               throughput.echoByteSeq(emptyBytes)
               throughput.echoStringSeq(emptyStrings)
               throughput.echoStructSeq(emptyStructs)
               throughput.echoFixedSeq(emptyFixed)
            Next
            throughput.endWarmup()
            Console.Out.WriteLine("ok")

            menu()

            '
            ' By default use bytes sequence.
            '
            Dim currentType As Char = "1"
            Dim seqSize As Integer = ByteSeqSize.value

            Dim line As String = Nothing
            Do
                Try
                    Console.Out.Write("==> ")
                    Console.Out.Flush()
                    line = Console.In.ReadLine()
                    If line Is Nothing Then
                        Exit Do
                    End If

                    Dim tmsec As Long = System.DateTime.Now.Ticks / 10000

                    repetitions = 100

                    If line.Equals("1") Or line.Equals("2") Or line.Equals("3") Or line.Equals("4") Then
                        currentType = line.Chars(0)
                        Select Case currentType
                            Case "1"
                                Console.WriteLine("using byte sequences")
                                seqSize = ByteSeqSize.value
                            Case "2"
                                Console.WriteLine("using string sequences")
                                seqSize = StringSeqSize.value
                            Case "3"
                                Console.WriteLine("using variable-length struct sequences")
                                seqSize = StringDoubleSeqSize.value
                            Case "4"
                                Console.WriteLine("using fixed-length struct sequences")
                                seqSize = FixedSeqSize.value
                        End Select
                    ElseIf line.Equals("t") Or line.Equals("o") Or line.Equals("r") Or line.Equals("e") Then
                        Dim c As Char = line.Chars(0)

                        Select Case c
                            Case "t"
                                Console.Out.Write("sending")
                            Case "o"
                                Console.Out.Write("sending")
                            Case "r"
                                Console.Out.Write("receiving")
                            Case "e"
                                Console.Out.Write("sending and receiving")
                        End Select

                        Console.Out.Write(" " & repetitions)
                        Select Case currentType
                            Case "1"
                                Console.Write(" byte")
                            Case "2"
                                Console.Write(" string")
                            Case "3"
                                Console.Write(" variable-length struct")
                            Case "4"
                                Console.Write(" fixed-length struct")
                        End Select
                        Console.Write(" sequences of size " & seqSize)

                        If c = "o" Then
                            Console.Out.Write(" as oneway")
                        End If

                        Console.Out.WriteLine("...")

                        For i As Integer = 0 To repetitions - 1
                            Select Case currentType
                                Case "1"
                                    Select Case c
                                        Case "t"
                                            throughput.sendByteSeq(byteSeq)
                                        Case "o"
                                            throughputOneway.sendByteSeq(byteSeq)
                                        Case "r"
                                            throughput.recvByteSeq()
                                        Case "e"
                                            throughput.echoByteSeq(byteSeq)
                                    End Select

                                Case "2"
                                    Select Case c
                                        Case "t"
                                            throughput.sendStringSeq(stringSeq)
                                        Case "o"
                                            throughputOneway.sendStringSeq(stringSeq)
                                        Case "r"
                                            throughput.recvStringSeq()
                                        Case "e"
                                            throughput.echoStringSeq(stringSeq)
                                    End Select

                                Case "3"
                                    Select Case c
                                        Case "t"
                                            throughput.sendStructSeq(structSeq)
                                        Case "o"
                                            throughputOneway.sendStructSeq(structSeq)
                                        Case "r"
                                            throughput.recvStructSeq()
                                        Case "e"
                                            throughput.echoStructSeq(structSeq)
                                    End Select
                                Case "4"
                                    Select Case c
                                        Case "t"
                                            throughput.sendFixedSeq(fixedSeq)
                                        Case "o"
                                            throughputOneway.sendFixedSeq(fixedSeq)
                                        Case "r"
                                            throughput.recvFixedSeq()
                                        Case "e"
                                            throughput.echoFixedSeq(fixedSeq)
                                    End Select
                            End Select
                        Next

                        Dim dmsec As Double = System.DateTime.Now.Ticks / 10000 - tmsec
                        Console.Out.WriteLine("time for " & repetitions & " sequences: " & dmsec.ToString("F") & "ms")
                        Console.Out.WriteLine("time per sequence: " & CType(dmsec / repetitions, Double).ToString("F") & "ms")
                        Dim wireSize As Integer = 0
                        Select Case currentType
                            Case "1"
                                wireSize = 1
                            Case "2"
                                wireSize = stringSeq(0).Length
                            Case "3"
                                wireSize = structSeq(0).s.Length
                                wireSize += 8 ' Size of double on the wire.
                            Case "4"
                                wireSize = 16 ' Size of two ints and a double on the wire.
                        End Select
                        Dim mbit As Double = repetitions * seqSize * wireSize * 8.0 / dmsec / 1000.0
                        If c = "e" Then
                            mbit *= 2
                        End If
                        Console.Out.WriteLine("throughput: " & mbit.ToString("#.##") & "Mbps")
                    ElseIf line.Equals("s") Then
                        throughput.shutdown()
                    ElseIf line.Equals("x") Then
                        '  Nothing to do
                    ElseIf line.Equals("?") Then
                        menu()
                    Else
                        Console.Out.WriteLine("unknown command `" & line & "'")
                        menu()
                    End If
                Catch ex As System.Exception
                    Console.Error.WriteLine(ex)
                End Try
            Loop While Not line.Equals("x")

            Return 0
        End Function
    End Class

    Public Sub Main(ByVal args() As String)
        Dim app As Client = New Client
        Dim status As Integer = app.main(args, "config.client")
        System.Environment.Exit(status)
    End Sub

End Module
