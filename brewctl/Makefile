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
    adc.cc application.cc config.cc error.cc gpioport.cc lcd.cc log.cc main.cc registry.cc         \
    sensor.cc session.cc sessionmanager.cc shiftreg.cc spiport.cc sqlite.cc sqlitecolumn.cc        \
    sqlitestmt.cc temperature.cc temperaturesensor.cc tempsensor.cc thermistor.cc

CFLAGS := -g -Wno-psabi -Wall -Wextra -Werror -pedantic -std=c++17

DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.d

DEPDIR := .dep
OBJDIR := .obj

OBJECTS := $(addprefix $(OBJDIR)/,$(patsubst %.cc,%.o,$(SOURCES)))

LIBS := -lwiringPi -lsqlite3 -lm

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

