#pragma once

// morph definition
typedef struct morph_vertex_t {
	unsigned int	vidx;			// vertex index
	GLfloat			diff[3];		// difference between models
} morph_vertex;

class CMorph
{
public:
	CArray<morph_vertex,morph_vertex> m_vmorph;		// vertex morphs
	CArray<morph_vertex,morph_vertex> m_nmorph;		// vertex normal morphs
	CArray<morph_vertex,morph_vertex> m_tmorph;		// texture co-ordinate morphs
	int			m_numvertices;		// total # vertices in model
	int			m_numnormals;		// total # normals in model
	int			m_numtextures;		// total # texture co-ordinates in model
public:
	CMorph(void);
	~CMorph(void);
	int CreateMorph(CObjFile *pObj1,CObjFile *pObj2);
	int CombineMorph(double w1,CMorph *pMorph1,double w2,CMorph *pMorph2);
	void DampedDrive(double &pos, double &vel, double targ, double freq, double h);
	void DriveMorph(CMorph *pMorph,double &position,double &velocity,double frequency,double timestep);
	void InterpolateMorph(CMorph *pMorph,double fraction);
};

