
CSC 471 -- Program 3: Lighting

Student: Bevin Tang
Date: Nov. 1, 2017

Description: 
	The program illustrates the effects of 4 different shaders on 2 objects: (1) the colors are equal to the vertex normals, (2) materials are shaded using Gouraud Shading, (3) materials are shaded using Phong Shading, (4) the object's silhouettes will be shown (black exterior, white interior).

	The depicted objects can take on different "materials" (both objects will have the same material):
		(1) Shiny Blue Plastic
		(2) Flat Grey
		(3) Brass
		(4) Smooth Chocolate

Usage:
--Command Line:
	There are two ways to execute the program:

	(1) One can either run the default (which shows the Stanford Bunny) by just executing the program without arguments.

	(2) Alternatively, the program can be run with 1 argument -- the file path to the desired .obj file. Make sure that the resource folder (the one containing the shaders and .obj files) exists in the previous directory, as the program is dependent on this organization of the file system.

-- Key Inputs:
	While using the program, the user has access to some features through keypress:
	"r" -- rotate both meshes about their own Y-axis
	"q" -- move light source left
	"e" -- move light source right
	"p" -- cycle through shaders
	"m" -- cycle through materials
	"esc" -- close program