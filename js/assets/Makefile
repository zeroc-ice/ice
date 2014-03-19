# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..

TARGETS	= common.min.js common.min.js.gz common.css common.css.gz

include $(top_srcdir)/config/Make.rules.js

CLOSUREFLAGS := $(CLOSUREFLAGS) --warning_level QUIET

SCRIPTS = foundation/js/vendor/modernizr.js \
	foundation/js/vendor/jquery.js \
	foundation/js/foundation.min.js \
	nouislider/5.0.0/minified/jquery.nouislider.min.js \
	animo.js/animo.js \
	spin.js/spin.js \
	spin.js/jquery.spin.js \
	highlight/highlight.pack.js \
	icejs.js
	
STYLE_SHEETS = foundation/css/foundation.min.css \
	animate.css \
	highlight/styles/vs.css \
	nouislider/5.0.0/minified/jquery.nouislider.min.css \
	icejs.css
	
common.min.js common.min.js.gz: $(SCRIPTS)
	@rm -f common.min.js common.min.js.gz
	java -jar $(CLOSURE_PATH)/compiler.jar $(CLOSUREFLAGS) --js $(SCRIPTS) --js_output_file common.min.js
	gzip -c9 common.min.js > common.min.js.gz

common.css common.css.gz: $(STYLE_SHEETS)
	@rm -f common.css common.css.gz
	node concat.js $(STYLE_SHEETS) > common.css
	gzip -c9 common.css > common.css.gz
