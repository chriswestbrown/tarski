##################################################################
#
# Master Makefile for my extensions to QEPCAD.
#
##################################################################

both:	opt deb

UNAME_S := $(shell uname -s)
ifneq (,$(findstring MINGW,$(UNAME_S)))
    MINGW = 1
endif

opt:	
	@echo Making the optimized libraries ...
	cd sfext; $(MAKE) ${SUBMAKEFLAGS}
	cd adj2d; $(MAKE) ${SUBMAKEFLAGS}
	cd rend ; $(MAKE) ${SUBMAKEFLAGS} 
	cd newadj; $(MAKE) ${SUBMAKEFLAGS}
	cd lift2D; $(MAKE) ${SUBMAKEFLAGS}


deb:	
	@echo Making the debugging libraries ...
	cd sfext; $(MAKE) "deb=x"  ${SUBMAKEFLAGS}
	cd adj2d; $(MAKE) "deb=x"  ${SUBMAKEFLAGS}
	cd rend ; $(MAKE) "deb=x" ${SUBMAKEFLAGS}
	cd newadj; $(MAKE) "deb=x"  ${SUBMAKEFLAGS}
	cd lift2D; $(MAKE) "deb=x"  ${SUBMAKEFLAGS}

