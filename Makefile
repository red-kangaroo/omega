-include Config.mk

################ Source files ##########################################

exe	:= $O${name}
srcs	:= $(wildcard *.cc)
objs	:= $(addprefix $O,$(srcs:.cc=.o))
deps	:= ${objs:.o=.d}
confs	:= Config.mk config.h
oname   := $(notdir $(abspath $O))

################ Compilation ###########################################

.SUFFIXES:
.PHONY: all clean distclean maintainer-clean

all:	${exe}

run:	${exe}
	@$<

${exe}:	${objs}
	@echo "Linking $@ ..."
	@${CC} ${ldflags} -o $@ $^ ${libs}
ifndef debug
	@strip -d -R .eh_frame -R .eh_frame_hdr -R .comment $@
endif

$O%.o:	%.cc
	@echo "    Compiling $< ..."
	@${CXX} ${cxxflags} -MMD -MT "$(<:.cc=.s) $@" -o $@ -c $<
ifndef debug
	@strip -d -R .eh_frame $@
endif

%.s:	%.cc
	@echo "    Compiling $< to assembly ..."
	@${CXX} ${cxxflags} -S -o $@ -c $<

################ Installation ##########################################

ifdef bindir
.PHONY:	install installdirs uninstall

exed	:= ${DESTDIR}${bindir}
exei	:= ${exed}/$(notdir ${exe})

${exed}:
	@echo "Creating $@ ..."
	@${INSTALL} -d $@
${exei}:	${exe} | ${exed}
	@echo "Installing $@ ..."
	@${INSTALL_PROGRAM} $< $@

installdirs:	${exed}
install:	${exei}
uninstall:
	@if [ -f ${exei} ]; then\
	    echo "Removing ${exei} ...";\
	    rm -f ${exei};\
	fi
endif

################ Maintenance ###########################################

clean:
	@if [ -d ${builddir} ]; then\
	    rm -f ${exe} ${objs} ${deps} $O.d;\
	    rmdir ${builddir};\
	fi

distclean:	clean
	@rm -f ${oname} ${confs} config.status

maintainer-clean: distclean

$O.d:	${builddir}/.d
	@[ -h ${oname} ] || ln -sf ${builddir} ${oname}
${builddir}/.d:
	@[ -d $(dir $@) ] || mkdir -p $(dir $@)
	@touch $@

${objs}:	Makefile ${confs} | $O.d
config.h:	config.h.in configure | Config.mk
Config.mk:	Config.mk.in configure
	@if [ -x config.status ]; then echo "Reconfiguring ...";\
	    ./config.status;\
	else echo "Running configure ...";\
	    ./configure;\
	fi

-include ${deps}
