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

// combine two morphs into one: out = (1-mix)*p1 + mix*p2
int CMorph::CombineMorph(double w1,CMorph *pMorph1,double w2,CMorph *pMorph2)
{
	// clear any existing morph list
	m_vmorph.RemoveAll();
	m_nmorph.RemoveAll();
	m_tmorph.RemoveAll();
	m_numvertices=0;
	m_numnormals=0;
	m_numtextures=0;

	// create morph only if same # vertices in base models
	if (pMorph1->m_numvertices==pMorph2->m_numvertices) {
		// morph is possible
		m_numvertices=pMorph1->m_numvertices;
		// find all different vertices
		int i=0,j=0;
		while ((i<pMorph1->m_vmorph.GetCount())&&(j<pMorph2->m_vmorph.GetCount())) {
			morph_vertex v1 = pMorph1->m_vmorph[i];
			morph_vertex v2 = pMorph2->m_vmorph[j];
			if (v1.vidx < v2.vidx) {
				// vertex in morph1 not in morph2
				v1.diff[0] = w1*v1.diff[0];
				v1.diff[1] = w1*v1.diff[1];
				v1.diff[2] = w1*v1.diff[2];
				m_vmorph.Add(v1);
				i++;
			}
			else if (v2.vidx < v1.vidx) {
				// vertex in morph2 not in morph1
				v2.diff[0] = w2*v2.diff[0];
				v2.diff[1] = w2*v2.diff[1];
				v2.diff[2] = w2*v2.diff[2];
				m_vmorph.Add(v2);
				j++;
			}
			else {
				// vertex in both morph1 and morph2
				v1.diff[0] = w1*v1.diff[0] + w2*v2.diff[0];
				v1.diff[1] = w1*v1.diff[1] + w2*v2.diff[1];
				v1.diff[2] = w1*v1.diff[2] + w2*v2.diff[2];
				m_vmorph.Add(v1);
				i++;
				j++;
			}
		}
		while (i<pMorph1->m_vmorph.GetCount()) {
			morph_vertex v1 = pMorph1->m_vmorph[i];
			// vertex in morph1 not in morph2
			v1.diff[0] = w1*v1.diff[0];
			v1.diff[1] = w1*v1.diff[1];
			v1.diff[2] = w1*v1.diff[2];
			m_vmorph.Add(v1);
			i++;
		}
		while (j<pMorph2->m_vmorph.GetCount()) {
			morph_vertex v2 = pMorph2->m_vmorph[j];
			// vertex in morph2 not in morph1
			v2.diff[0] = w2*v2.diff[0];
			v2.diff[1] = w2*v2.diff[1];
			v2.diff[2] = w2*v2.diff[2];
			m_vmorph.Add(v2);
			j++;
		}
		TRACE("Combine morph found %d/%d morph vertices\n",m_vmorph.GetCount(),m_numvertices);
	}
	// create morph only if same # normals in base models
	if (pMorph1->m_numnormals==pMorph2->m_numnormals) {
		// morph is possible
		m_numnormals=pMorph1->m_numnormals;
		// find all different normals
		int i=0,j=0;
		while ((i<pMorph1->m_nmorph.GetCount())&&(j<pMorph2->m_nmorph.GetCount())) {
			morph_vertex v1 = pMorph1->m_nmorph[i];
			morph_vertex v2 = pMorph2->m_nmorph[j];
			if (v1.vidx < v2.vidx) {
				// vertex in morph1 not in morph2
				v1.diff[0] = w1*v1.diff[0];
				v1.diff[1] = w1*v1.diff[1];
				v1.diff[2] = w1*v1.diff[2];
				m_nmorph.Add(v1);
				i++;
			}
			else if (v2.vidx < v1.vidx) {
				// vertex in morph2 not in morph1
				v2.diff[0] = w2*v2.diff[0];
				v2.diff[1] = w2*v2.diff[1];
				v2.diff[2] = w2*v2.diff[2];
				m_nmorph.Add(v2);
				j++;
			}
			else {
				// vertex in both morph1 and morph2
				v1.diff[0] = w1*v1.diff[0] + w2*v2.diff[0];
				v1.diff[1] = w1*v1.diff[1] + w2*v2.diff[1];
				v1.diff[2] = w1*v1.diff[2] + w2*v2.diff[2];
				m_nmorph.Add(v1);
				i++;
				j++;
			}
		}
		while (i<pMorph1->m_nmorph.GetCount()) {
			morph_vertex v1 = pMorph1->m_nmorph[i];
			// vertex in morph1 not in morph2
			v1.diff[0] = w1*v1.diff[0];
			v1.diff[1] = w1*v1.diff[1];
			v1.diff[2] = w1*v1.diff[2];
			m_nmorph.Add(v1);
			i++;
		}
		while (j<pMorph2->m_nmorph.GetCount()) {
			morph_vertex v2 = pMorph2->m_nmorph[j];
			// vertex in morph2 not in morph1
			v2.diff[0] = w2*v2.diff[0];
			v2.diff[1] = w2*v2.diff[1];
			v2.diff[2] = w2*v2.diff[2];
			m_nmorph.Add(v2);
			j++;
		}
		TRACE("Combine morph found %d/%d morph normals\n",m_nmorph.GetCount(),m_numnormals);
	}
	// create morph only if same # textures
	if (pMorph1->m_numtextures==pMorph2->m_numtextures) {
		// morph is possible
		m_numtextures=pMorph1->m_numtextures;
		// find all different texture co-ordinates
		int i=0,j=0;
		while ((i<pMorph1->m_tmorph.GetCount())&&(j<pMorph2->m_tmorph.GetCount())) {
			morph_vertex v1 = pMorph1->m_tmorph[i];
			morph_vertex v2 = pMorph2->m_tmorph[j];
			if (v1.vidx < v2.vidx) {
				// vertex in morph1 not in morph2
				v1.diff[0] = w1*v1.diff[0];
				v1.diff[1] = w1*v1.diff[1];
				v1.diff[2] = w1*v1.diff[2];
				m_tmorph.Add(v1);
				i++;
			}
			else if (v2.vidx < v1.vidx) {
				// vertex in morph2 not in morph1
				v2.diff[0] = w2*v2.diff[0];
				v2.diff[1] = w2*v2.diff[1];
				v2.diff[2] = w2*v2.diff[2];
				m_tmorph.Add(v2);
				j++;
			}
			else {
				// vertex in both morph1 and morph2
				v1.diff[0] = w1*v1.diff[0] + w2*v2.diff[0];
				v1.diff[1] = w1*v1.diff[1] + w2*v2.diff[1];
				v1.diff[2] = w1*v1.diff[2] + w2*v2.diff[2];
				m_tmorph.Add(v1);
				i++;
				j++;
			}
		}
		while (i<pMorph1->m_tmorph.GetCount()) {
			morph_vertex v1 = pMorph1->m_tmorph[i];
			// vertex in morph1 not in morph2
			v1.diff[0] = w1*v1.diff[0];
			v1.diff[1] = w1*v1.diff[1];
			v1.diff[2] = w1*v1.diff[2];
			m_tmorph.Add(v1);
			i++;
		}
		while (j<pMorph2->m_tmorph.GetCount()) {
			morph_vertex v2 = pMorph2->m_tmorph[j];
			// vertex in morph2 not in morph1
			v2.diff[0] = w2*v2.diff[0];
			v2.diff[1] = w2*v2.diff[1];
			v2.diff[2] = w2*v2.diff[2];
			m_tmorph.Add(v2);
			j++;
		}
		TRACE("Combine morph found %d/%d morph texture coords\n",m_tmorph.GetCount(),m_numtextures);
	}

	return(m_vmorph.GetCount());

}

// implement critically-damped driver
// this is a single step of a fourth-order Runge-Kutta simulation of the
// second-order ODE for a damped simple harmonic oscillator
// freq = 2.pi.f where f = approx 1/time-to-target
void CMorph::DampedDrive(double &pos, double &vel, double targ, double freq, double h)
{
	float	y1,y2;
	float	yt1,yt2;
	float	k11,k12,k21,k22,k31,k32,k41,k42;

	y1=targ - pos;
	y2=vel;

	yt1=y1;
	yt2=y2;
	k11=yt2;
	k12=-2*freq*yt2-freq*freq*yt1;

	yt1=y1+(h/2)*k11;
	yt2=y2+(h/2)*k12;
	k21=yt2;
	k22=-2*freq*yt2-freq*freq*yt1;

	yt1=y1+(h/2)*k21;
	yt2=y2+(h/2)*k22;
	k31=yt2;
	k32=-2*freq*yt2-freq*freq*yt1;

	yt1=y1+h*k31;
	yt2=y2+h*k32;
	k41=yt2;
	k42=-2*freq*yt2-freq*freq*yt1;

	y1=y1+(h/6)*(k11+2*k21+2*k31+k41);
	y2=y2+(h/6)*(k12+2*k22+2*k32+k42);

	pos = targ - y1;
	vel = y2;
}

void CMorph::DriveMorph(CMorph *pMorph,double &position,double &velocity,double frequency,double timestep)
{
	double opos=position,npos;

	// move the position along using critically-damped driver
	DampedDrive(position,velocity,1.0,frequency,timestep);
	npos=position;

	// create morph only if same # vertices in base models
	if (m_numvertices==pMorph->m_numvertices) {
		// find all different vertices
		int i=0,j=0;
		while ((i<m_vmorph.GetCount())&&(j<pMorph->m_vmorph.GetCount())) {
			morph_vertex v1 = m_vmorph[i];
			morph_vertex v2 = pMorph->m_vmorph[j];
			if (v1.vidx < v2.vidx) {
				// vertex in morph1 not in morph2 - move to zero/cull
				v1.diff[0] = (1-npos)*(v1.diff[0])/(1-opos);
				v1.diff[1] = (1-npos)*(v1.diff[1])/(1-opos);
				v1.diff[2] = (1-npos)*(v1.diff[2])/(1-opos);
				if ((fabs(v1.diff[0])<0.01) && (fabs(v1.diff[1])<0.01) && (fabs(v1.diff[2])<0.01)) {
					m_vmorph.RemoveAt(i);
				}
				else {
					m_vmorph.SetAt(i,v1);
					i++;
				}
			}
			else if (v2.vidx < v1.vidx) {
				// vertex in morph2 not in morph1
				v2.diff[0] = npos*v2.diff[0];
				v2.diff[1] = npos*v2.diff[1];
				v2.diff[2] = npos*v2.diff[2];
				m_vmorph.InsertAt(i,v2);
				i++;
				j++;
			}
			else {
				// vertex in both morph1 and morph2
				v1.diff[0] = (1-npos)*(v1.diff[0]-opos*v2.diff[0])/(1-opos)+npos*v2.diff[0];
				v1.diff[1] = (1-npos)*(v1.diff[1]-opos*v2.diff[1])/(1-opos)+npos*v2.diff[1];
				v1.diff[2] = (1-npos)*(v1.diff[2]-opos*v2.diff[2])/(1-opos)+npos*v2.diff[2];
				m_vmorph.SetAt(i,v1);
				i++;
				j++;
			}
		}
		while (i<m_vmorph.GetCount()) {
			morph_vertex v1 = m_vmorph[i];
			// vertex in morph1 not in morph2 - move to zero/cull
			v1.diff[0] = (1-npos)*(v1.diff[0])/(1-opos);
			v1.diff[1] = (1-npos)*(v1.diff[1])/(1-opos);
			v1.diff[2] = (1-npos)*(v1.diff[2])/(1-opos);
			if ((fabs(v1.diff[0])<0.01) && (fabs(v1.diff[1])<0.01) && (fabs(v1.diff[2])<0.01)) {
				m_vmorph.RemoveAt(i);
			}
			else {
				m_vmorph.SetAt(i,v1);
				i++;
			}
		}
		while (j<pMorph->m_vmorph.GetCount()) {
			morph_vertex v2 = pMorph->m_vmorph[j];
			// vertex in morph2 not in morph1
			v2.diff[0] = npos*v2.diff[0];
			v2.diff[1] = npos*v2.diff[1];
			v2.diff[2] = npos*v2.diff[2];
			m_vmorph.InsertAt(i,v2);
			i++;
			j++;
		}
		TRACE("Drive morph found %d/%d morph vertices\n",m_vmorph.GetCount(),m_numvertices);
	}
	// create morph only if same # normals in base models
	if (m_numnormals==pMorph->m_numnormals) {
		// find all different normals
		int i=0,j=0;
		while ((i<m_nmorph.GetCount())&&(j<pMorph->m_nmorph.GetCount())) {
			morph_vertex v1 = m_nmorph[i];
			morph_vertex v2 = pMorph->m_nmorph[j];
			if (v1.vidx < v2.vidx) {
				// vertex in morph1 not in morph2 - move to zero/cull
				v1.diff[0] = (1-npos)*(v1.diff[0])/(1-opos);
				v1.diff[1] = (1-npos)*(v1.diff[1])/(1-opos);
				v1.diff[2] = (1-npos)*(v1.diff[2])/(1-opos);
				if ((fabs(v1.diff[0])<0.01) && (fabs(v1.diff[1])<0.01) && (fabs(v1.diff[2])<0.01)) {
					m_nmorph.RemoveAt(i);
				}
				else {
					m_nmorph.SetAt(i,v1);
					i++;
				}
			}
			else if (v2.vidx < v1.vidx) {
				// vertex in morph2 not in morph1
				v2.diff[0] = npos*v2.diff[0];
				v2.diff[1] = npos*v2.diff[1];
				v2.diff[2] = npos*v2.diff[2];
				m_nmorph.InsertAt(i,v2);
				i++;
				j++;
			}
			else {
				// vertex in both morph1 and morph2
				v1.diff[0] = (1-npos)*(v1.diff[0]-opos*v2.diff[0])/(1-opos)+npos*v2.diff[0];
				v1.diff[1] = (1-npos)*(v1.diff[1]-opos*v2.diff[1])/(1-opos)+npos*v2.diff[1];
				v1.diff[2] = (1-npos)*(v1.diff[2]-opos*v2.diff[2])/(1-opos)+npos*v2.diff[2];
				m_nmorph.SetAt(i,v1);
				i++;
				j++;
			}
		}
		while (i<m_nmorph.GetCount()) {
			morph_vertex v1 = m_nmorph[i];
			// vertex in morph1 not in morph2 - move to zero/cull
			v1.diff[0] = (1-npos)*(v1.diff[0])/(1-opos);
			v1.diff[1] = (1-npos)*(v1.diff[1])/(1-opos);
			v1.diff[2] = (1-npos)*(v1.diff[2])/(1-opos);
			if ((fabs(v1.diff[0])<0.01) && (fabs(v1.diff[1])<0.01) && (fabs(v1.diff[2])<0.01)) {
				m_nmorph.RemoveAt(i);
			}
			else {
				m_nmorph.SetAt(i,v1);
				i++;
			}
		}
		while (j<pMorph->m_nmorph.GetCount()) {
			morph_vertex v2 = pMorph->m_nmorph[j];
			// vertex in morph2 not in morph1
			v2.diff[0] = npos*v2.diff[0];
			v2.diff[1] = npos*v2.diff[1];
			v2.diff[2] = npos*v2.diff[2];
			m_nmorph.InsertAt(i,v2);
			i++;
			j++;
		}
		TRACE("Drive morph found %d/%d morph normals\n",m_nmorph.GetCount(),m_numnormals);
	}
	// create morph only if same # textures
	if (m_numtextures==pMorph->m_numtextures) {
		// find all different texture co-ordinates
		int i=0,j=0;
		while ((i<m_tmorph.GetCount())&&(j<pMorph->m_tmorph.GetCount())) {
			morph_vertex v1 = m_tmorph[i];
			morph_vertex v2 = pMorph->m_tmorph[j];
			if (v1.vidx < v2.vidx) {
				// vertex in morph1 not in morph2 - move to zero/cull
				v1.diff[0] = (1-npos)*(v1.diff[0])/(1-opos);
				v1.diff[1] = (1-npos)*(v1.diff[1])/(1-opos);
				v1.diff[2] = (1-npos)*(v1.diff[2])/(1-opos);
				if ((fabs(v1.diff[0])<0.01) && (fabs(v1.diff[1])<0.01) && (fabs(v1.diff[2])<0.01)) {
					m_tmorph.RemoveAt(i);
				}
				else {
					m_tmorph.SetAt(i,v1);
					i++;
				}
			}
			else if (v2.vidx < v1.vidx) {
				// vertex in morph2 not in morph1
				v2.diff[0] = npos*v2.diff[0];
				v2.diff[1] = npos*v2.diff[1];
				v2.diff[2] = npos*v2.diff[2];
				m_tmorph.InsertAt(i,v2);
				i++;
				j++;
			}
			else {
				// vertex in both morph1 and morph2
				v1.diff[0] = (1-npos)*(v1.diff[0]-opos*v2.diff[0])/(1-opos)+npos*v2.diff[0];
				v1.diff[1] = (1-npos)*(v1.diff[1]-opos*v2.diff[1])/(1-opos)+npos*v2.diff[1];
				v1.diff[2] = (1-npos)*(v1.diff[2]-opos*v2.diff[2])/(1-opos)+npos*v2.diff[2];
				m_tmorph.SetAt(i,v1);
				i++;
				j++;
			}
		}
		while (i<m_tmorph.GetCount()) {
			morph_vertex v1 = m_tmorph[i];
			// vertex in morph1 not in morph2 - move to zero/cull
			v1.diff[0] = (1-npos)*(v1.diff[0])/(1-opos);
			v1.diff[1] = (1-npos)*(v1.diff[1])/(1-opos);
			v1.diff[2] = (1-npos)*(v1.diff[2])/(1-opos);
			if ((fabs(v1.diff[0])<0.01) && (fabs(v1.diff[1])<0.01) && (fabs(v1.diff[2])<0.01)) {
				m_tmorph.RemoveAt(i);
			}
			else {
				m_tmorph.SetAt(i,v1);
				i++;
			}
		}
		while (j<pMorph->m_tmorph.GetCount()) {
			morph_vertex v2 = pMorph->m_tmorph[j];
			// vertex in morph2 not in morph1
			v2.diff[0] = npos*v2.diff[0];
			v2.diff[1] = npos*v2.diff[1];
			v2.diff[2] = npos*v2.diff[2];
			m_tmorph.InsertAt(i,v2);
			i++;
			j++;
		}
		TRACE("Drive morph found %d/%d morph texture coords\n",m_tmorph.GetCount(),m_numtextures);
	}

}

void CMorph::InterpolateMorph(CMorph *pMorph,double fraction)
{
	// create morph only if same # vertices in base models
	if (m_numvertices==pMorph->m_numvertices) {
		// find all different vertices
		int i=0,j=0;
		while ((i<m_vmorph.GetCount())&&(j<pMorph->m_vmorph.GetCount())) {
			morph_vertex v1 = m_vmorph[i];
			morph_vertex v2 = pMorph->m_vmorph[j];
			if (v1.vidx < v2.vidx) {
				// vertex in morph1 not in morph2 - move to zero/cull
				v1.diff[0] = (1-fraction)*v1.diff[0];
				v1.diff[1] = (1-fraction)*v1.diff[1];
				v1.diff[2] = (1-fraction)*v1.diff[2];
				if ((fabs(v1.diff[0])<0.01) && (fabs(v1.diff[1])<0.01) && (fabs(v1.diff[2])<0.01)) {
					m_vmorph.RemoveAt(i);
				}
				else {
					m_vmorph.SetAt(i,v1);
					i++;
				}
			}
			else if (v2.vidx < v1.vidx) {
				// vertex in morph2 not in morph1
				v2.diff[0] = fraction*v2.diff[0];
				v2.diff[1] = fraction*v2.diff[1];
				v2.diff[2] = fraction*v2.diff[2];
				m_vmorph.InsertAt(i,v2);
				i++;
				j++;
			}
			else {
				// vertex in both morph1 and morph2
				v1.diff[0] = (1-fraction)*v1.diff[0]+fraction*v2.diff[0];
				v1.diff[1] = (1-fraction)*v1.diff[1]+fraction*v2.diff[1];
				v1.diff[2] = (1-fraction)*v1.diff[2]+fraction*v2.diff[2];
				m_vmorph.SetAt(i,v1);
				i++;
				j++;
			}
		}
		while (i<m_vmorph.GetCount()) {
			morph_vertex v1 = m_vmorph[i];
			// vertex in morph1 not in morph2 - move to zero/cull
			v1.diff[0] = (1-fraction)*v1.diff[0];
			v1.diff[1] = (1-fraction)*v1.diff[1];
			v1.diff[2] = (1-fraction)*v1.diff[2];
			if ((fabs(v1.diff[0])<0.01) && (fabs(v1.diff[1])<0.01) && (fabs(v1.diff[2])<0.01)) {
				m_vmorph.RemoveAt(i);
			}
			else {
				m_vmorph.SetAt(i,v1);
				i++;
			}
		}
		while (j<pMorph->m_vmorph.GetCount()) {
			morph_vertex v2 = pMorph->m_vmorph[j];
			// vertex in morph2 not in morph1
			v2.diff[0] = fraction*v2.diff[0];
			v2.diff[1] = fraction*v2.diff[1];
			v2.diff[2] = fraction*v2.diff[2];
			m_vmorph.InsertAt(i,v2);
			i++;
			j++;
		}
		TRACE("Interp morph found %d/%d morph vertices\n",m_vmorph.GetCount(),m_numvertices);
	}
	// create morph only if same # normals in base models
	if (m_numnormals==pMorph->m_numnormals) {
		// find all different normals
		int i=0,j=0;
		while ((i<m_nmorph.GetCount())&&(j<pMorph->m_nmorph.GetCount())) {
			morph_vertex v1 = m_nmorph[i];
			morph_vertex v2 = pMorph->m_nmorph[j];
			if (v1.vidx < v2.vidx) {
				// vertex in morph1 not in morph2 - move to zero/cull
				v1.diff[0] = (1-fraction)*v1.diff[0];
				v1.diff[1] = (1-fraction)*v1.diff[1];
				v1.diff[2] = (1-fraction)*v1.diff[2];
				if ((fabs(v1.diff[0])<0.01) && (fabs(v1.diff[1])<0.01) && (fabs(v1.diff[2])<0.01)) {
					m_nmorph.RemoveAt(i);
				}
				else {
					m_nmorph.SetAt(i,v1);
					i++;
				}
			}
			else if (v2.vidx < v1.vidx) {
				// vertex in morph2 not in morph1
				v2.diff[0] = fraction*v2.diff[0];
				v2.diff[1] = fraction*v2.diff[1];
				v2.diff[2] = fraction*v2.diff[2];
				m_nmorph.InsertAt(i,v2);
				i++;
				j++;
			}
			else {
				// vertex in both morph1 and morph2
				v1.diff[0] = (1-fraction)*v1.diff[0]+fraction*v2.diff[0];
				v1.diff[1] = (1-fraction)*v1.diff[1]+fraction*v2.diff[1];
				v1.diff[2] = (1-fraction)*v1.diff[2]+fraction*v2.diff[2];
				m_nmorph.SetAt(i,v1);
				i++;
				j++;
			}
		}
		while (i<m_nmorph.GetCount()) {
			morph_vertex v1 = m_nmorph[i];
			// vertex in morph1 not in morph2 - move to zero/cull
			v1.diff[0] = (1-fraction)*v1.diff[0];
			v1.diff[1] = (1-fraction)*v1.diff[1];
			v1.diff[2] = (1-fraction)*v1.diff[2];
			if ((fabs(v1.diff[0])<0.01) && (fabs(v1.diff[1])<0.01) && (fabs(v1.diff[2])<0.01)) {
				m_nmorph.RemoveAt(i);
			}
			else {
				m_nmorph.SetAt(i,v1);
				i++;
			}
		}
		while (j<pMorph->m_nmorph.GetCount()) {
			morph_vertex v2 = pMorph->m_nmorph[j];
			// vertex in morph2 not in morph1
			v2.diff[0] = fraction*v2.diff[0];
			v2.diff[1] = fraction*v2.diff[1];
			v2.diff[2] = fraction*v2.diff[2];
			m_nmorph.InsertAt(i,v2);
			i++;
			j++;
		}
		TRACE("Interp morph found %d/%d morph normals\n",m_nmorph.GetCount(),m_numnormals);
	}
	// create morph only if same # textures
	if (m_numtextures==pMorph->m_numtextures) {
		// find all different texture co-ordinates
		int i=0,j=0;
		while ((i<m_tmorph.GetCount())&&(j<pMorph->m_tmorph.GetCount())) {
			morph_vertex v1 = m_tmorph[i];
			morph_vertex v2 = pMorph->m_tmorph[j];
			if (v1.vidx < v2.vidx) {
				// vertex in morph1 not in morph2 - move to zero/cull
				v1.diff[0] = (1-fraction)*v1.diff[0];
				v1.diff[1] = (1-fraction)*v1.diff[1];
				v1.diff[2] = (1-fraction)*v1.diff[2];
				if ((fabs(v1.diff[0])<0.01) && (fabs(v1.diff[1])<0.01) && (fabs(v1.diff[2])<0.01)) {
					m_tmorph.RemoveAt(i);
				}
				else {
					m_tmorph.SetAt(i,v1);
					i++;
				}
			}
			else if (v2.vidx < v1.vidx) {
				// vertex in morph2 not in morph1
				v2.diff[0] = fraction*v2.diff[0];
				v2.diff[1] = fraction*v2.diff[1];
				v2.diff[2] = fraction*v2.diff[2];
				m_tmorph.InsertAt(i,v2);
				i++;
				j++;
			}
			else {
				// vertex in both morph1 and morph2
				v1.diff[0] = (1-fraction)*v1.diff[0]+fraction*v2.diff[0];
				v1.diff[1] = (1-fraction)*v1.diff[1]+fraction*v2.diff[1];
				v1.diff[2] = (1-fraction)*v1.diff[2]+fraction*v2.diff[2];
				m_tmorph.SetAt(i,v1);
				i++;
				j++;
			}
		}
		while (i<m_tmorph.GetCount()) {
			morph_vertex v1 = m_tmorph[i];
			// vertex in morph1 not in morph2 - move to zero/cull
			v1.diff[0] = (1-fraction)*v1.diff[0];
			v1.diff[1] = (1-fraction)*v1.diff[1];
			v1.diff[2] = (1-fraction)*v1.diff[2];
			if ((fabs(v1.diff[0])<0.01) && (fabs(v1.diff[1])<0.01) && (fabs(v1.diff[2])<0.01)) {
				m_tmorph.RemoveAt(i);
			}
			else {
				m_tmorph.SetAt(i,v1);
				i++;
			}
		}
		while (j<pMorph->m_tmorph.GetCount()) {
			morph_vertex v2 = pMorph->m_tmorph[j];
			// vertex in morph2 not in morph1
			v2.diff[0] = fraction*v2.diff[0];
			v2.diff[1] = fraction*v2.diff[1];
			v2.diff[2] = fraction*v2.diff[2];
			m_tmorph.InsertAt(i,v2);
			i++;
			j++;
		}
		TRACE("Interp morph found %d/%d morph texture coords\n",m_tmorph.GetCount(),m_numtextures);
	}

}

