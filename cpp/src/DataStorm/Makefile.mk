# Copyright (c) ZeroC, Inc.

$(project)_libraries    = DataStorm

$(project)_generated_includedir := $(project)/generated/DataStorm

DataStorm_sliceflags    := --include-dir DataStorm
DataStorm_targetdir     := $(libdir)
DataStorm_cppflags      := -DDATASTORM_API_EXPORTS $(api_exports_cppflags)
DataStorm_dependencies  := Ice

projects += $(project)
