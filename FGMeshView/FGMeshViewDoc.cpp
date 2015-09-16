
// FGMeshViewDoc.cpp : implementation of the CFGMeshViewDoc class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "FGMeshView.h"
#endif

#include "FGMeshViewDoc.h"

#include <propkey.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CFGMeshViewDoc

IMPLEMENT_DYNCREATE(CFGMeshViewDoc, CDocument)

BEGIN_MESSAGE_MAP(CFGMeshViewDoc, CDocument)
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CFGMeshViewDoc, CDocument)
END_DISPATCH_MAP()

// Note: we add support for IID_IFGMeshView to support typesafe binding
//  from VBA.  This IID must match the GUID that is attached to the 
//  dispinterface in the .IDL file.

// {A2C35A88-6AFB-4B85-99A8-2195E79AB8B8}
static const IID IID_IFGMeshView =
{ 0xA2C35A88, 0x6AFB, 0x4B85, { 0x99, 0xA8, 0x21, 0x95, 0xE7, 0x9A, 0xB8, 0xB8 } };

BEGIN_INTERFACE_MAP(CFGMeshViewDoc, CDocument)
	INTERFACE_PART(CFGMeshViewDoc, IID_IFGMeshView, Dispatch)
END_INTERFACE_MAP()


// CFGMeshViewDoc construction/destruction

CFGMeshViewDoc::CFGMeshViewDoc()
{
	// TODO: add one-time construction code here

	EnableAutomation();

	AfxOleLockApp();
}

CFGMeshViewDoc::~CFGMeshViewDoc()
{
	AfxOleUnlockApp();
}

BOOL CFGMeshViewDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;
	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)
	m_init = 0;
	return TRUE;
}




// CFGMeshViewDoc serialization

void CFGMeshViewDoc::Serialize(CArchive& ar)
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
void CFGMeshViewDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
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
void CFGMeshViewDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// Set search contents from document's data. 
	// The content parts should be separated by ";"

	// For example:  strSearchContent = _T("point;rectangle;circle;ole object;");
	SetSearchContent(strSearchContent);
}

void CFGMeshViewDoc::SetSearchContent(const CString& value)
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

// CFGMeshViewDoc diagnostics

#ifdef _DEBUG
void CFGMeshViewDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CFGMeshViewDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CFGMeshViewDoc commands
