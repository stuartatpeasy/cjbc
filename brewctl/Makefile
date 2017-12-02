# Makefile for brewctl
#
# Stuart Wallace <stuartw@atom.net>, 2017.
#
#

ifndef VERBOSE
.SILENT:
endif

APPNAME := brewctl

SOURCES := \
    application.cc avahiservice.cc config.cc error.cc httpservice.cc log.cc main.cc peripherals/adc.cc                 \
    peripherals/defaulteffector.cc peripherals/defaulttempsensor.cc peripherals/effector.cc peripherals/gpiopin.cc     \
    peripherals/gpioport.cc peripherals/lcd.cc peripherals/shiftreg.cc peripherals/spiport.cc                          \
    peripherals/tempsensor.cc registry.cc session.cc sessionmanager.cc sqlite/sqlite.cc sqlite/sqlitecolumn.cc         \
    sqlite/sqlitestmt.cc temperature.cc thermistor.cc thread.cc util/net.cc util/random.cc util/sys.cc util/thread.cc

CFLAGS := -O3 -Wno-psabi -Wall -Wextra -Werror -pedantic -std=c++17 -pthread -I.
LDFLAGS := -pthread

DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.d

DEPDIR := .dep
OBJDIR := .obj

OBJECTS := $(addprefix $(OBJDIR)/,$(patsubst %.cc,%.o,$(SOURCES)))

LIBS := -lwiringPi -lsqlite3 -lm -lavahi-common -lavahi-client -lmicrohttpd

$(OBJDIR)/%.o : %.cc $(DEPDIR)/%.d
	mkdir -p $(dir $@)
	echo "(CXX) $<"
	$(CXX) $(DEPFLAGS) $(CFLAGS) -c -o$@ $<

$(DEPDIR)/%.d:
	mkdir -p $(dir $@)

.PRECIOUS: $(DEPDIR)/%.d
.PRECIOUS: $(OBJDIR)/%.o

.PHONY: all
.PHONY: clean

all: $(APPNAME)

-include $(patsubst %,$(DEPDIR)/%.d,$(basename $(SOURCES)))

$(APPNAME): $(OBJECTS)
	echo "(LD) $(APPNAME)"
	$(CXX) $(OBJECTS) $(LIBS) -o$(APPNAME) $(LDFLAGS)

clean:
	rm -f $(APPNAME) $(OBJECTS)
	rm -rf $(DEPDIR) $(OBJDIR)

