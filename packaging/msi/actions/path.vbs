Option Explicit
Dim BinDir, Shell, Key, Path

BinDir = Session.Property("CustomActionData")

Set Shell = CreateObject("WScript.Shell")
Key = "HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Environment\Path"

Path = Shell.RegRead(Key)

If Path <> "" Then
    If InStr(LCase(Path), LCase(BinDir)) = 0 Then
    	Shell.RegWrite Key, Path & ";" & BinDir, "REG_EXPAND_SZ"
    End If
Else
    Shell.RegWrite Key, BinDir, "REG_EXPAND_SZ"
End If
