
Dim vs2008CsPath
Dim vs2010CsPath
Dim vs2010CppPath
Dim vs2012CsPath
Dim vs2012CppPath

vs2008CsPath = Session.Property("VS_2008_CS_TEMPLATE_DIR")
vs2010CsPath = Session.Property("VS_2010_CS_TEMPLATE_DIR")
vs2010CppPath = Session.Property("VS_2010_CPP_TEMPLATE_DIR")
vs2012CsPath = Session.Property("VS_2012_CS_TEMPLATE_DIR")
vs2012CppPath = Session.Property("VS_2012_CPP_TEMPLATE_DIR")

Session.Property("VS_2008_CS_TEMPLATE_DIR_MOD") = Replace(vs2008CsPath, "\.\", "\")
Session.Property("VS_2010_CS_TEMPLATE_DIR_MOD") = Replace(vs2010CsPath, "\.\", "\")
Session.Property("VS_2010_CPP_TEMPLATE_DIR_MOD") = Replace(vs2010CppPath, "\.\", "\")
Session.Property("VS_2012_CS_TEMPLATE_DIR_MOD") = Replace(vs2012CsPath, "\.\", "\")
Session.Property("VS_2012_CPP_TEMPLATE_DIR_MOD") = Replace(vs2012CppPath, "\.\", "\")
