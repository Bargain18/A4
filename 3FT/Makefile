#--------------------------------------------------------------------
# Makefile for Assignment 4, Part 3
# Authors: Michael Wei, Lakshit Ahuja
#--------------------------------------------------------------------

all: ft

ft: ft.o ft_client.o fileNode.o dirNode.o
	gcc217 ft.o ft_client.o -o ft

clean:
	rm -f $(TARGETS) meminfo*.out


ft.o:

fileNode.o:

dirNode.o:

dynarray.o: dynarray.c dynarray.h
	gcc217 dynarray.c

path.o: path.c dynarray.h path.h a4def.h
	gcc217 -c path.c

ft_client.o: ft_client.c ft.h a4def.h
	gcc217 -c ft_client.c

#checkerFT.o: checkerFT.c dynarray.h checkerDT.h nodeDT.h path.h a4def.h


clobber: clean
	rm -f dynarray.o path.o dt_client.o checkerDT.o nodeDTGood.o dtGood.o *~