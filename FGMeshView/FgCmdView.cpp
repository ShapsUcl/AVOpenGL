//
// Copyright (C) Singular Inversions Inc. 2009
//
// Authors: Andrew Beatty
// Created: June 23, 2009
//

#include "stdafx.h"

#include "FgCommand.hpp"
#include "FgFileSystem.hpp"
#include "Fg3dDisplay.hpp"
#include "Fg3dMeshIo.hpp"
#include "Fg3dMeshOps.hpp"
#include "FgSyntax.hpp"
#include "FgImgDisplay.hpp"
#include "FgDraw.hpp"
#include "FgAffineCwC.hpp"
#include "FgBuild.hpp"

using namespace std;

// **************************************************************************************
// **************************************************************************************

//static
void
viewMesh(const FgArgs & args)
{
 	TRACE("Calling viewmesh command\n");
   FgSyntax            syntax(args,
        "[-c] [-r] (<mesh>.<ext> [<texImage> [<transparencyImage>]])+\n"
        "    -c    - Compare meshes rather than view all at once\n"
        "    -r    - Remove unused vertices for viewing\n"
        "    <ext> - " + fgLoadMeshFormatsDescription());
    bool                compare = false,
                        ru = false;
    int ct=0;
	TRACE("Options to viewMesh: \n");
	while (syntax.peekNext()[0] == '-') {
        if (syntax.next() == "-c")
            compare = true;
        else if (syntax.curr() == "-r")
            ru = true;
        else
            syntax.error("Unrecognized option: ",syntax.curr());
    }
    vector<Fg3dMesh>    meshes;
	while (syntax.more()) {
        ct++;
		string          fname = syntax.next(),
                        ext = fgToLower(fgPathToExt(fname));
		//TRACE("Viewmesh Loaded %s.%s\n", fname, ext); 
        Fg3dMesh        mesh = fgLoadMeshAnyFormat(fname);
        fgout << fgnl << "Mesh " << meshes.size() << ": " << fgpush << mesh;
        size_t          origVerts = mesh.verts.size();
        if (ru) {
            mesh = fgRemoveUnusedVerts(mesh);
            if (mesh.verts.size() < origVerts)
                fgout << fgnl << origVerts-mesh.verts.size() << " unused vertices removed for viewing";
        }
        if(syntax.more() && fgIsImgFilename(syntax.peekNext())) {
            if (mesh.uvs.empty())
                fgout << fgnl << "WARNING: " << syntax.curr() << " has no UVs, texture image "
                    << syntax.peekNext() << " will not be seen.";
            FgImgRgbaUb         texture;
            fgLoadImgAnyFormat(syntax.next(),texture);
            fgout << fgnl << "Texture image: " << texture;
            if(syntax.more() && fgIsImgFilename(syntax.peekNext())) {
                FgImgRgbaUb     trans;
                fgout << fgnl << "Transparency image:" << trans;
                fgLoadImgAnyFormat(syntax.next(),trans);
                mesh.texImages.push_back(fgImgApplyTransparencyPow2(texture,trans));
            }
            else {
                FgImgRgbaUb     texPow;
                fgPower2Ceil(texture,texPow);
                mesh.texImages.push_back(texPow);
            }
        }
        meshes.push_back(mesh);
        fgout << fgpop;
    }
	TRACE("%d arguments found\n", ct);
    if (compare && (meshes.size() > 1))
        fgDisplayMeshes(meshes,true);
    else
        fgDisplayMeshes(meshes,false);
}

FgCmd
fgCmdViewMeshInfo()
{return FgCmd(viewMesh,"mesh","Interactively view 3D meshes"); }

FgCmd
fgCmdViewMesh3D(const char * args)
{
	return (FgCmd(viewMesh, "mesh", args)); 
}

void
fgViewImage(const FgArgs & args)
{
    FgSyntax    syntax(args,"<imageFileName>");
    if (args.size() > 2)
        syntax.incorrectNumArgs();
    FgImgRgbaUb     img;
    fgLoadImgAnyFormat(syntax.next(),img);
    fgout << fgnl << img;
    fgImgDisplay(img);
}

FgCmd
fgCmdViewImageInfo()
{return FgCmd(fgViewImage,"image","Basic image viewer"); }

void
fgViewFloatImage(const FgArgs & args)
{
    FgSyntax    syntax(args,"<imageFileName> [<saveName>]");
    FgImgF      img;
    if (fgToLower(fgPathToExt(syntax.next())) == "fgpbn")
        fgLoadPBin(syntax.curr(),img);
    else {
        if (fgCurrentOS() != "win")
            fgout << "WARNING: This functionality currently only works properly under windows";
        fgLoadImgAnyFormat(syntax.curr(),img);
    }
    if (syntax.more())
        fgSavePBin(syntax.next(),img);
    fgout << fgnl << img;
    fgImgDisplay(img);
}

FgCmd
fgCmdViewFloatImageInfo()
{return FgCmd(fgViewFloatImage,"floatImage","Floating point image viewer"); }

void
fgCmdViewUvs(const FgArgs & args)
{
    FgSyntax            syntax(args,
        "<mesh>.<ext> [<texImage>]\n"
        "     <ext> = " + fgLoadMeshFormatsDescription());
    string          fname = syntax.next(),
                    ext = fgToLower(fgPathToExt(fname));
    Fg3dMesh        mesh = fgLoadMeshAnyFormat(fname);
    if (mesh.uvs.empty()) {
        fgout << fgnl << "Mesh has no UVs";
        return;
    }
    FgImgRgbaUb     img;
    if(syntax.more()) {
        if (fgIsImgFilename(syntax.peekNext()))
            fgLoadImgAnyFormat(syntax.next(),img);
        else
            syntax.error("Not an image file",syntax.next());
    }
    fgout << fgnl << "UV Bounds: " << fgBounds(mesh.uvs);
    fgImgDisplay(fgUvImage(mesh,img));
}

FgCmd
fgCmdViewUvsInfo()
{return FgCmd(fgCmdViewUvs,"uvs","View the UV layout of a 3D mesh"); }
