#include "StdAfx.h"
#include "ObjFile.h"
#include "Morph.h"


CMorph::CMorph(void)
{
	m_numvertices=0;
	m_numnormals=0;
	m_numtextures=0;
}


CMorph::~CMorph(void)
{
}

int CMorph::CreateMorph(CObjFile *pObj1,CObjFile *pObj2)
{
	// clear any existing morph list
	m_vmorph.RemoveAll();
	m_nmorph.RemoveAll();
	m_tmorph.RemoveAll();
	m_numvertices=0;
	m_numnormals=0;
	m_numtextures=0;

	// create morph only if same # vertices
	if (pObj1->numvertices==pObj2->numvertices) {
		// morph is possible
		m_numvertices=pObj1->numvertices;
		// find all different vertices
		for (int i=1;i<=m_numvertices;i++) {
			if ((fabs(pObj2->vertices[3*i+0]-pObj1->vertices[3*i+0])>1.0E-3)||
				(fabs(pObj2->vertices[3*i+1]-pObj1->vertices[3*i+1])>1.0E-3)||
				(fabs(pObj2->vertices[3*i+2]-pObj1->vertices[3*i+2])>1.0E-3)) {
				morph_vertex v;
				v.vidx=i;
				v.diff[0]=pObj2->vertices[3*i+0]-pObj1->vertices[3*i+0];
				v.diff[1]=pObj2->vertices[3*i+1]-pObj1->vertices[3*i+1];
				v.diff[2]=pObj2->vertices[3*i+2]-pObj1->vertices[3*i+2];
				m_vmorph.Add(v);
			}
		}
		TRACE("Create morph found %d/%d morph vertices\n",m_vmorph.GetCount(),m_numvertices);
	}
	// create morph only if same # normals
	if (pObj1->numnormals==pObj2->numnormals) {
		// morph is possible
		m_numnormals=pObj1->numnormals;
		// find all different normals
		for (int i=1;i<=m_numnormals;i++) {
			if ((fabs(pObj2->normals[3*i+0]-pObj1->normals[3*i+0])>1.0E-3)||
				(fabs(pObj2->normals[3*i+1]-pObj1->normals[3*i+1])>1.0E-3)||
				(fabs(pObj2->normals[3*i+2]-pObj1->normals[3*i+2])>1.0E-3)) {
				morph_vertex v;
				v.vidx=i;
				v.diff[0]=pObj2->normals[3*i+0]-pObj1->normals[3*i+0];
				v.diff[1]=pObj2->normals[3*i+1]-pObj1->normals[3*i+1];
				v.diff[2]=pObj2->normals[3*i+2]-pObj1->normals[3*i+2];
				m_nmorph.Add(v);
			}
		}
		TRACE("Create morph found %d/%d morph normals\n",m_nmorph.GetCount(),m_numnormals);
	}
	// create morph only if same # textures
	if (pObj1->numtexcoords==pObj2->numtexcoords) {
		// morph is possible
		m_numtextures=pObj1->numtexcoords;
		// find all different texture co-ordinates
		for (int i=1;i<=m_numtextures;i++) {
			if ((fabs(pObj2->texcoords[2*i+0]-pObj1->texcoords[2*i+0])>1.0E-3)||
				(fabs(pObj2->texcoords[2*i+1]-pObj1->texcoords[2*i+1])>1.0E-3)) {
				morph_vertex v;
				v.vidx=i;
				v.diff[0]=pObj2->texcoords[2*i+0]-pObj1->texcoords[2*i+0];
				v.diff[1]=pObj2->texcoords[2*i+1]-pObj1->texcoords[2*i+1];
				v.diff[2]=0;
				m_tmorph.Add(v);
			}
		}
		TRACE("Create morph found %d/%d morph texture coords\n",m_tmorph.GetCount(),m_numtextures);
	}

	return(m_vmorph.GetCount());
}
