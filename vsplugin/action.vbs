
Dim vs80CsPath
Dim vs80CppPath

Dim vs90CsPath
Dim vs90CppPath


vs90CsPath = Session.Property("VS_90_CS_TEMPLATE_DIR")
vs90CppPath = Session.Property("VS_90_CPP_TEMPLATE_DIR")

vs80CsPath = Session.Property("VS_80_CS_TEMPLATE_DIR")
vs80CppPath = Session.Property("VS_80_CPP_TEMPLATE_DIR")


Session.Property("VS_90_CS_TEMPLATE_DIR_MOD") = Replace(vs90CsPath, "\.\", "\")
Session.Property("VS_90_CPP_TEMPLATE_DIR_MOD") = Replace(vs90CppPath, "\.\", "\")

Session.Property("VS_80_CS_TEMPLATE_DIR_MOD") = Replace(vs80CsPath, "\.\", "\")
Session.Property("VS_80_CPP_TEMPLATE_DIR_MOD") = Replace(vs80CppPath, "\.\", "\")
