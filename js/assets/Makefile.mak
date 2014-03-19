# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..

TARGETS	= common.min.js common.css

!if "$(GZIP_PATH)" != ""
TARGETS = $(TARGETS) common.min.js.gz common.css.gz
!endif

!include $(top_srcdir)\config\Make.rules.mak.js

CLOSUREFLAGS	= $(CLOSUREFLAGS) --warning_level QUIET

SCRIPTS = foundation\js\vendor\modernizr.js \
	foundation\js\vendor\jquery.js \
	foundation\js\foundation.min.js \
	nouislider\5.0.0\minified\jquery.nouislider.min.js \
	animo.js\animo.js \
	spin.js\spin.js \
	spin.js\jquery.spin.js \
	highlight\highlight.pack.js \
	icejs.js
	
STYLE_SHEETS = foundation\css\foundation.min.css \
	animate.css \
	highlight\styles\vs.css \
	nouislider\5.0.0\minified\jquery.nouislider.min.css \
	icejs.css
	
common.min.js: $(SCRIPTS)
	-del /q common.min.js
	java -jar $(CLOSURE_PATH)\compiler.jar $(CLOSUREFLAGS) --js $(SCRIPTS) --js_output_file common.min.js

!if "$(GZIP_PATH)" != ""
common.min.js.gz: common.min.js
	@del /q common.min.js.gz
	"$(GZIP_PATH)" -c9 common.min.js > common.min.js.gz
!endif

common.css: $(STYLE_SHEETS)
	-del /q common.css
	node concat.js $(STYLE_SHEETS) > common.css

!if "$(GZIP_PATH)" != ""
common.css.gz: common.css
	@del /q common.css.gz
	"$(GZIP_PATH)" -c9 common.css > common.css.gz
!endif
