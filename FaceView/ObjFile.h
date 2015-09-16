#pragma once

#include "TextureStore.h"
class CMorph;

#ifndef M_PI
#define M_PI 3.14159265
#endif

#define GLM_NONE     (0)		/* render with only vertices */
#define GLM_FLAT     (1 << 0)		/* render with facet normals */
#define GLM_SMOOTH   (1 << 1)		/* render with vertex normals */
#define GLM_TEXTURE  (1 << 2)		/* render with texture coords */
#define GLM_COLOR    (1 << 3)		/* render with colors */
#define GLM_MATERIAL (1 << 4)		/* render with materials */


/* GLMmaterial: Structure that defines a material in a model. 
 */
typedef struct _GLMmaterial
{
	CString		name;				/* name of material */
	GLfloat		diffuse[4];			/* diffuse component */
	GLfloat		ambient[4];			/* ambient component */
	GLfloat		specular[4];			/* specular component */
	GLfloat		emmissive[4];			/* emmissive component */
	GLfloat		shininess;			/* specular exponent */
	GLuint		textureid;			/* ID for used texture */
	int			used;				/* used by any group ? */
} GLMmaterial;

/* GLMtriangle: Structure that defines a triangle in a model.
 */
typedef struct _GLMtriangle {
	GLuint vindices[3];			/* array of triangle vertex indices */
	GLuint nindices[3];			/* array of triangle normal indices */
	GLuint tindices[3];			/* array of triangle texcoord indices*/
	GLuint findex;			/* index of triangle facet normal */
} GLMtriangle;

/* GLMtexture: Support for textures */
typedef struct _GLMtexture {
	CString	name;
	GLuint	id;
//	GLfloat	width;
//	GLfloat	height;
} GLMtexture;

/* GLMgroup: Structure that defines a group in a model.
 */
typedef struct _GLMgroup {
	CString		name;		/* name of this group */
	GLuint		numtriangles;	/* number of triangles in this group */
	GLuint*		triangles;		/* array of triangle indices */
	GLuint		material;           /* index to material for group */
	struct _GLMgroup* next;		/* pointer to next group in model */
} GLMgroup;



class CObjFile
{
public:
	CObjFile(void);
	~CObjFile(void);
public:
	CString		pathname;			/* path to this model */
	CString		mtllibname;			/* name of the material library */

	GLuint		numvertices;			/* number of vertices in model */
	GLfloat*	vertices;			/* array of vertices  */

	GLuint		numnormals;			/* number of normals in model */
	GLfloat*	normals;			/* array of normals */

	GLuint		numtexcoords;		/* number of texcoords in model */
	GLfloat*	texcoords;			/* array of texture coordinates */

	GLuint		numfacetnorms;		/* number of facetnorms in model */
	GLfloat*	facetnorms;			/* array of facetnorms */

	GLuint		numtriangles;		/* number of triangles in model */
	GLMtriangle*	triangles;		/* array of triangles */

	GLuint		nummaterials;		/* number of materials in model */
	GLMmaterial*	materials;		/* array of materials */

	GLuint		numgroups;		/* number of groups in model */
	GLMgroup*	groups;			/* linked list of groups */

//	GLuint		numtextures;		/* number of texture ids */
//	GLuint		maxtextures;		/* maximum number of texture ids */
//	GLuint		*textures;			/* array of texture ids */

	GLfloat		m_shift[3];			/* shift required to centre model on origin */
	GLfloat		m_scale;			/* scaling required to fit in -1 .. +1 box */

	int Load(LPCTSTR filename,CTextureStore &texstore);
	GLMgroup* glmFindGroup(LPCTSTR name);
	GLMgroup* glmAddGroup(LPCTSTR name);
	char*	glmDirName(LPCTSTR path);
	GLuint	glmFindOrAddTexture(LPCTSTR name,CTextureStore &texstore);
	GLuint	glmFindMaterial(LPCTSTR name);
	int		glmReadMTL(LPCTSTR name,CTextureStore &texstore);
	int		glmFirstPass(FILE* file,CTextureStore &texstore);
	void	glmSecondPass(FILE* file);
	GLvoid	glmDraw(GLuint mode);
	GLuint	glmList(GLuint mode);
	GLfloat glmMax(GLfloat a, GLfloat b);
	GLfloat glmAbs(GLfloat f);
	GLfloat	glmDot(GLfloat* u, GLfloat* v);
	GLvoid	glmCross(GLfloat* u, GLfloat* v, GLfloat* n);
	GLvoid	glmNormalize(GLfloat* v);
	GLfloat glmUnitize();
	GLvoid	glmFacetNormals();
	GLvoid	glmVertexNormals(GLfloat angle);
	int		glmWriteMTL(LPCTSTR modelpath, LPCTSTR mtllibname);
	int		glmWriteOBJ(LPCTSTR filename, GLuint mode);
	GLvoid	glmScale(GLfloat scale);
	GLvoid	glmRotate(GLfloat rotx,GLfloat roty,GLfloat rotz);
	GLvoid	glmDimensions(GLfloat *dimensions);
	GLvoid	glmLinearTexture(void);
//	GLuint	glmLoadTexture(LPCTSTR filename,GLboolean alpha,GLboolean repeat, GLboolean filtering,GLboolean mipmaps, GLfloat *texcoordwidth,GLfloat *texcoordheight);
	GLvoid	glmDrawMorph(CMorph *pMorph,double wt,GLuint mode);
	GLvoid	glmFindPosition();

};
