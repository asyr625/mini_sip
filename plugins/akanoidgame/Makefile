# Use another prefix than home directory: make PREFIX=/usr
# Create .deb: "make deb"
# Upload trunk version to packages.minisip.org: "make upload"
# Upload stable version to packages.minisip.org: "make upload COMPONENT=stable"



VERSION=1.0
ifndef COMPONENT
COMPONENT=trunk
endif
REVISION=`svn info |grep '^Revision: ' |cut -d' ' -f 2 `
RELEASE=`lsb_release -c -s`

OPENGL_LIBS=-lSDL -lGL -lGLU -lSDL_ttf

ifndef PREFIX
PREFIX=${HOME}
LD_LIBRARY_PATH=${PREFIX}/lib
LINK_EXTRA_FLAGS=-L${PREFIX}/lib
endif


ENGINE_PATH=Akanoid/GameEngine Akanoid/Util

Akanoid: engine
	g++ -shared ${OPENGL_LIBS} ${LINK_EXTRA_FLAGS} -lm $(wildcard build/*) -o akanoidgame.so -lmutil -lGL -lGLU -lminisip
engine:
	test -d build/ || mkdir build/
	#build engine
	for dir in $(ENGINE_PATH); do \
		cd $$dir; \
		g++ *.cpp -std=c++0x -fPIC -DPIC -Wall -g -c -D__STDC_CONSTANT_MACROS -I${PREFIX}/include -L${PREFIX}/lib -lminisip -lmutil; \
		mv *.o ../../build; \
		cd -; \
	done
	g++ Akanoid/*.cpp -std=c++0x -fPIC -DPIC -Wall -g -c -D__STDC_CONSTANT_MACROS -I${PREFIX}/include -L${PREFIX}/lib -lminisip -lmutil; \
	#build Plugin
	g++ -fPIC -DPIC -Wall -g -c -D__STDC_CONSTANT_MACROS -I${PREFIX}/include -L${PREFIX}/lib -lminisip -lmutil GamePlugin.cxx -o GamePlugin.o;
	#build all file to build folder
	mv *.o build/;

install: Akanoid
ifdef DESTDIR
	cp akanoidgame.so ${DESTDIR}/usr/lib/libminisip/plugins
	cp Resource/akanoid.raw ${DESTDIR}/usr/share/minisip
	cp Resource/akanoid_up.raw ${DESTDIR}/usr/share/minisip
else
	cp akanoidgame.so ${PREFIX}/lib/libminisip/plugins
	cp Resource/akanoid.raw ${PREFIX}/share/minisip
	cp Resource/akanoid_up.raw ${PREFIX}/share/minisip
endif

uninstall:
ifdef DESTDIR
	rm ${DESTDIR}/usr/lib/libminisip/plugins/akanoidgame.so
else
	rm ${PREFIX}/lib/libminisip/plugins/akanoidgame.so
endif


dist:
	dch -v "${VERSION}+${REVISION}~${RELEASE}" 'latest trunk version.'
	tar --transform 's,^,akanoidgame-1.0/,' -zcvf akanoidgame-1.0.tar.gz Makefile GamePlugin.cxx Akanoid Resource debian
distclean: clean
	rm -rf akanoidgame-1.0.tar.gz *.changes *.tar.gz *.dsc
deb: dist
	tar zxvf akanoidgame-1.0.tar.gz
	make -C akanoidgame-1.0 realdeb
realdeb:
	echo rev=${REVISION}
	dpkg-buildpackage -rfakeroot -us -uc
#upload:
#	scp `find . -name "*.deb" -or -name "*.dsc" -or -name "*.tar.gz"` erik@minisip.org:deb/
#	ssh erik@minisip.org deb/do_scan.sh ${RELEASE} ${COMPONENT}
clean:
	rm -f akanoidgame.so
	rm -rf build/



