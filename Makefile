-include Config.mk

################ Source files ##########################################

EXE	:= ${NAME}
SRCS	:= $(wildcard *.cc)
OBJS	:= $(addprefix $O,$(SRCS:.cc=.o))
DEPS	:= ${OBJS:.o=.d}

################ Compilation ###########################################

.PHONY: all clean distclean maintainer-clean

all:	Config.mk config.h ${EXE}

run:	${EXE}
	@./${EXE}

${EXE}:	${OBJS}
	@echo "Linking $@ ..."
	@${CXX} ${LDFLAGS} -o $@ ${OBJS} ${LIBS}

$O%.o:	%.cc
	@echo "    Compiling $< ..."
	@[ -d $(dir $@) ] || mkdir -p $(dir $@)
	@${CXX} ${CXXFLAGS} -MMD -MT "$(<:.cc=.s) $@" -o $@ -c $<

%.s:	%.cc
	@echo "    Compiling $< to assembly ..."
	@${CXX} ${CXXFLAGS} -S -o $@ -c $<

################ Installation ##########################################

.PHONY:	install uninstall

ifdef BINDIR
EXEI	:= $(addprefix ${BINDIR}/,${EXE})

install:	${EXEI}
${EXEI}:	${EXE}
	@echo "Installing $< as $@ ..."
	@${INSTALLEXE} $< $@

uninstall:
	@if [ -f ${EXEI} ]; then\
	    echo "Removing ${EXEI} ...";\
	    rm -f ${EXEI};\
	fi
endif

################ Maintenance ###########################################

clean:
	@if [ -d $O ]; then\
	    rm -f ${EXE} ${OBJS} ${DEPS};\
	    rmdir $O;\
	fi
distclean:	clean
	@rm -f Config.mk config.h config.status

maintainer-clean: distclean

${OBJS}:		Makefile Config.mk config.h
Config.mk:		Config.mk.in
config.h:		config.h.in
Config.mk config.h:	configure
	@if [ -x config.status ]; then echo "Reconfiguring ..."; ./config.status; \
	else echo "Running configure ..."; ./configure; fi

-include ${DEPS}
