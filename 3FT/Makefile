#--------------------------------------------------------------------
# Makefile for Assignment 4, Part 3
# Authors: Michael Wei, Lakshit Ahuja
#--------------------------------------------------------------------

all: ft

ft: ft.o ft_client.o fileNode.o dirNode.o dynarray.o path.o
	gcc217 ft.o ft_client.o fileNode.o dirNode.o dynarray.o path.o -o ft

ft.o: ft.c ft.h types.h dirNode.h fileNode.h path.h dynarray.h a4def.h
	gcc217 -c ft.c

fileNode.o: fileNode.c fileNode.h types.h dirNode.h dynarray.h path.h a4def.h
	gcc217 -c fileNode.c

dirNode.o: dirNode.c dirNode.h types.h fileNode.h dynarray.h path.h a4def.h
	gcc217 -c dirNode.c

dynarray.o: dynarray.c dynarray.h
	gcc217 -c dynarray.c

path.o: path.c dynarray.h path.h a4def.h
	gcc217 -c path.c

ft_client.o: ft_client.c ft.h a4def.h
	gcc217 -c ft_client.c

#checkerFT.o: checkerFT.c dynarray.h checkerDT.h nodeDT.h path.h a4def.h