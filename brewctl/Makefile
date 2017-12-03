# Makefile for brewctl
#
# Stuart Wallace <stuartw@atom.net>, 2017.
#
#

ifndef VERBOSE
.SILENT:
endif

APPNAME := brewctl

SOURCES := $(shell find . -name '*.cc')

CFLAGS := -g -Wno-psabi -Wall -Wextra -Werror -pedantic -std=c++17 -pthread -I.
LDFLAGS := -pthread

DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.d

DEPDIR := .dep
OBJDIR := .obj

OBJECTS := $(addprefix $(OBJDIR)/,$(patsubst %.cc,%.o,$(SOURCES)))

LIBS := -lavahi-client -lavahi-common -ljson-c -lm -lmicrohttpd -lsqlite3 -lwiringPi

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

