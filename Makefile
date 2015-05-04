
TOPDIR := $(shell pwd)
TOPRPMDIR := $(TOPDIR)/build/rpmbuild
all: 
	mkdir -p build && cd build && cmake .. && make && cd -

optimized:
	mkdir -p build && cd build && cmake -DCMAKE_BUILD_TYPE=Release .. && make && cd -

install:
	cd build; make install

.PHONY:check
check:
	make test  -C build/

.PHONY:release
release: all
	mkdir -p build/rpmbuild/{BUILD,RPMS,SOURCES,SPECS,SRPMS}
	rpmbuild --quiet -bb $(TOPDIR)/scripts/nixia.spec --define '_topdir $(TOPRPMDIR)'\
		--define '_mypath $(TOPDIR)'\
		--define '_builddir $(TOPRPMDIR)/BUILD'\
		--define '_specdir $(TOPDIR)/SPECS'\
		--define '_rpmdir $(TOPRPMDIR)/RPMS'\
		--define '_sourcedir $(TOPRPMDIR)/SOURCES'\
		--define '_specdir $(TOPRPMDIR)/SPECS'\
		--define '_srcrpmdir $(TOPRPMDIR)/SRPMS'\
		$(TOPDIR)/scripts/nixia.spec && \
		cp $(TOPRPMDIR)/RPMS/*/*.rpm $(TOPDIR)/build/

clean:
	rm -rf build/src  $(TOPRPMDIR)/ build/*.rpm output* core*

distclean:
	rm -rf build; rm -rf core* output*
