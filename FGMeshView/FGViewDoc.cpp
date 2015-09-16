
// FGViewDoc.cpp : implementation of the CFGViewDoc class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "FGView.h"
#endif

#include "FGViewDoc.h"
#include "Shlwapi.h"
#include <propkey.h>

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

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CFGViewDoc

IMPLEMENT_DYNCREATE(CFGViewDoc, CDocument)

BEGIN_MESSAGE_MAP(CFGViewDoc, CDocument)
	ON_COMMAND(ID_FILE_OPEN, &CFGViewDoc::OnFileOpen)
	ON_COMMAND(ID_FILE_SEND_MAIL, &CFGViewDoc::OnFileSendMail)
	ON_UPDATE_COMMAND_UI(ID_FILE_SEND_MAIL, &CFGViewDoc::OnUpdateFileSendMail)
END_MESSAGE_MAP()


// CFGViewDoc construction/destruction

CFGViewDoc::CFGViewDoc()
{
	// TODO: add one-time construction code here
	CFGViewApp *pApp = (CFGViewApp *)AfxGetApp();

	m_cur = 0;
}

BOOL CFGViewDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	using namespace std;

	TRACE("OnOpenDocument(%s)\n",lpszPathName);

	vector<Fg3dMesh>    meshes;
	// load in the meshes 

 	m_cur=meshes.size()-1;

	CString fname(lpszPathName);
	::PathStripPath(fname.GetBuffer());
	fname.Replace(".tri","bmp");
	TRACE("SetTitle %s\n",(LPCTSTR)fname);
	SetTitle((LPCTSTR)fname);
	fname.ReleaseBuffer(-1);

	UpdateAllViews(NULL);

	return TRUE;
}

void CFGViewDoc::OnFileOpen()
{
	// TODO: add one-time construction code here
	CFGViewApp *pApp = (CFGViewApp *)AfxGetApp();
	TCHAR FileNameList[8192]= {0};

	LPCTSTR strFilter = _T("FG Tri Mesh (*.tri)|*.tri*||");
	LPCTSTR ext = _T("tri");

	char * meshArgs = new char[512];

    CFileDialog FileDlg(TRUE, ext, NULL, OFN_ALLOWMULTISELECT, strFilter, NULL, 0, TRUE);
	FileDlg.m_ofn.lpstrFile = FileNameList;
	FileDlg.m_ofn.nMaxFile=8192;
	if( FileDlg.DoModal() == IDOK ) {
		CWaitCursor cw;
		CString csFname;
		CString fname, texfName;
		POSITION pos = FileDlg.GetStartPosition();
		while (pos != NULL) {
			fname = FileDlg.GetNextPathName(pos);
			csFname = fname;
			TRACE("Trifile name: %s\n", csFname);
			//if (!OnOpenDocument((LPCTSTR)csFname)) return;
			pApp->AddToRecentFileList((LPCTSTR)csFname);
			//::PathStripPath(fname.GetBuffer());
			fname.Replace(".tri",".bmp");
			fname.ReleaseBuffer(-1);
			texfName = fname;
			TRACE("Texture file name: %s\n", texfName);
			strncat(meshArgs, csFname, strlen(csFname)); 
			strncat(meshArgs, texfName, strlen(texfName)); 
		}
		TRACE("Args to view mesh command: %s\n", meshArgs);
		OnDisplayMeshes(meshArgs);
    }
    else {
		delete [] meshArgs;
		return;	
	}

}

static
void
viewMesh(const FgArgs & args)
{
    FgSyntax            syntax(args,
        "[-c] [-r] (<mesh>.<ext> [<texImage> [<transparencyImage>]])+\n"
        "    -c    - Compare meshes rather than view all at once\n"
        "    -r    - Remove unused vertices for viewing\n"
        "    <ext> - " + fgLoadMeshFormatsDescription());
    bool                compare = false,
                        ru = false;
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
        string          fname = syntax.next(),
                        ext = fgToLower(fgPathToExt(fname));
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
    if (compare && (meshes.size() > 1))
        fgDisplayMeshes(meshes,true);
    else
        fgDisplayMeshes(meshes,false);
}

CFGViewDoc::~CFGViewDoc()
{
}

void CFGViewDoc::OnDisplayMeshes(char * meshArgs)
{
	TRACE("Args to view mesh command: %s\n", meshArgs);
	FgCmd(viewMesh,meshArgs);
}

BOOL CFGViewDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}




// CFGViewDoc serialization

void CFGViewDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

#ifdef SHARED_HANDLERS

// Support for thumbnails
void CFGViewDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// Modify this code to draw the document's data
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// Support for Search Handlers
void CFGViewDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// Set search contents from document's data. 
	// The content parts should be separated by ";"

	// For example:  strSearchContent = _T("point;rectangle;circle;ole object;");
	SetSearchContent(strSearchContent);
}

void CFGViewDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = NULL;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != NULL)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// CFGViewDoc diagnostics

#ifdef _DEBUG
void CFGViewDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CFGViewDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CFGViewDoc commands
