#include "StdAfx.h"
#include "ObjStore.h"


CObjStore::CObjStore(void)
{
}


CObjStore::~CObjStore(void)
{
	for (int i=0;i<m_pObjs.GetCount();i++) delete m_pObjs[i];
	m_pObjs.RemoveAll();
}


int CObjStore::Add(LPCTSTR lpszPathName)
{
	CObjFile *pObj = new CObjFile();

	// load in the OBJ format file
	if (pObj->Load(lpszPathName,m_Textures)==0) {
		CString msg;
		msg.Format("Could not open '%s'",lpszPathName);
		AfxMessageBox((LPCTSTR)msg,MB_ICONEXCLAMATION|MB_OK);
		return -1;
	}

	GLfloat	dim[3];
	pObj->glmDimensions(dim);
	TRACE("Dimensions = %g %g %g\n",dim[0],dim[1],dim[2]);

	// calculate the normals in case not specified in model
	if (!pObj->facetnorms) pObj->glmFacetNormals();
	if (!pObj->normals) pObj->glmVertexNormals(90.0f);

	TRACE("Processed %s:\n",lpszPathName);
	TRACE("\tnumvertices=%d\n",pObj->numvertices);
	TRACE("\tnumnormals=%d\n",pObj->numnormals);
	TRACE("\tnumtexcoords=%d\n",pObj->numtexcoords);
	TRACE("\tnumfacetnorms=%d\n",pObj->numfacetnorms);
	TRACE("\tnumtriangles=%d\n",pObj->numtriangles);
	TRACE("\tnummaterials=%d\n",pObj->nummaterials);
	TRACE("\tnumgroups=%d\n",pObj->numgroups);

	m_pObjs.Add(pObj);
	TRACE("m_pObjs contains %d objs\n",m_pObjs.GetCount());

	return 0;
}


void CObjStore::RemoveAll(void)
{
	for (int i=0;i<m_pObjs.GetCount();i++) delete m_pObjs[i];
	m_pObjs.RemoveAll();
}


CObjFile * CObjStore::GetAt(int index)
{
	if ((0<=index)&&(index<m_pObjs.GetCount())) {
		return(m_pObjs[index]);
	}
	return NULL;
}

CObjFile * CObjStore::operator[](int index)
{
	return GetAt(index);
}


int CObjStore::GetCount(void)
{
	return m_pObjs.GetCount();
}

void CObjStore::ReloadTextures(void)
{
	m_Textures.Reload();
}
