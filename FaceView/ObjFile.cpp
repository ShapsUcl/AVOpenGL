#include "StdAfx.h"

#include <stdio.h>
#include <string.h>
#include "ObjFile.h"
#include "Morph.h"

#define _CRT_SECURE_NO_WARNINGS	1

CObjFile::CObjFile(void)
{
	numvertices=0;
	vertices=NULL;
	
	numnormals=0;
	normals=NULL;

	numtexcoords=0;
	texcoords=NULL;

	numfacetnorms=0;
	facetnorms=NULL;

	numtriangles=0;
	triangles=NULL;

	nummaterials=0;
	materials=NULL;

	numgroups=0;
	groups=0;

//	numtextures=0;
//	maxtextures=100;
//	textures=new GLMtexture[100];

	m_shift[0]=m_shift[1]=m_shift[2]=0;

	m_scale=1.0;
}

CObjFile::~CObjFile(void)
{
	GLMgroup* group;

	if (vertices) delete [] vertices;
	if (normals) delete [] normals;
	if (texcoords) delete [] texcoords;
	if (facetnorms) delete [] facetnorms;
	if (triangles) delete [] triangles;
	if (materials) delete [] materials;
//	if (textures) delete [] textures;
	while (groups) {
		group = groups;
		groups = groups->next;
		delete [] group->triangles;
		delete group;
	}
}

int CObjFile::Load(LPCTSTR filename,CTextureStore &texstore)
{
	FILE*     file;

	/* open the file */
	if ((file = fopen(filename, "r"))==NULL) return(0);
	pathname = filename;

	/* make a first pass through the file to get a count of the number
     of vertices, normals, texcoords & triangles */
	if (glmFirstPass(file,texstore)==0) {
		fclose(file);
		return 0;
	}

	TRACE("Pass 1: numvertices=%d\n",numvertices);
	TRACE("Pass 1: numtriangles=%d\n",numtriangles);
	TRACE("Pass 1: numnormals=%d\n",numnormals);
	TRACE("Pass 1: numtexcoords=%d\n",numtexcoords);

	/* allocate memory */
	vertices = new GLfloat[3 * (numvertices + 1)];
	triangles = new GLMtriangle[numtriangles];
	if (numnormals)
		normals = new GLfloat[3 * (numnormals + 1)];
	if (numtexcoords)
		texcoords = new GLfloat[2 * (numtexcoords + 1)];

	/* rewind to beginning of file and read in the data this pass */
	rewind(file);
	glmSecondPass(file);

	/* close the file */
	fclose(file);

	/* calculate position shift and scaling */
	glmFindPosition();

	return 1;
}

/* glmFindGroup: Find a group in the model
 */
GLMgroup* CObjFile::glmFindGroup(LPCTSTR name)
{
	GLMgroup* group;

	group = groups;
	while (group) {
		if (!strcmp(name, group->name)) break;
		group = group->next;
	}
	return group;
}

/* glmAddGroup: Add a group to the model
 */
GLMgroup* CObjFile::glmAddGroup(LPCTSTR name)
{
	GLMgroup *pgroup;
	GLMgroup* group;

	group = glmFindGroup(name);
	if (!group) {

		// EDIT: MAH June 2015 - preserve group order as that in OBJ file
		pgroup=groups;
		while (pgroup && pgroup->next) pgroup=pgroup->next;

		group = new GLMgroup;
	    group->name = name;
		group->material = 0;
		group->numtriangles = 0;
		group->triangles = NULL;
		group->next = NULL;
		if (pgroup==NULL)
			groups = group;
		else
			pgroup->next = group;
		numgroups++;
	}

	return group;
}

/* glmFirstPass: first pass at a Wavefront OBJ file that gets all the
 * statistics of the model (such as #vertices, #normals, etc)
 *
 * model - properly initialized GLMmodel structure
 * file  - (fopen'd) file descriptor 
 */
int	CObjFile::glmFirstPass(FILE* file,CTextureStore &texstore)
{
	GLMgroup* group;			/* current group */
	unsigned  v, n, t;
	char      buf[128];
	char      lastcommand=0;
	char		groupname[256];
	int			groupcnt=0;

	/* make a default group */
	group = glmAddGroup("default");
	numvertices = numnormals = numtexcoords = numtriangles = 0;

	while(fscanf(file, "%s", buf) != EOF) {
		switch(buf[0]) {
		case '#':				/* comment */
			/* eat up rest of line */
			fgets(buf, sizeof(buf), file);
			break;
		case 'v':				/* v, vn, vt */
			switch(buf[1]) {
				case '\0':			/* vertex */
					/* eat up rest of line */
					fgets(buf, sizeof(buf), file);
					numvertices++;
					break;
				case 'n':				/* normal */
					/* eat up rest of line */
					fgets(buf, sizeof(buf), file);
					numnormals++;
					break;
				case 't':				/* texcoord */
					/* eat up rest of line */
					fgets(buf, sizeof(buf), file);
					numtexcoords++;
					break;
				default:
					TRACE("glmFirstPass(): Unknown token \"%s\".\n", buf);
					break;
			}
			lastcommand='v';
			break;
		case 'm':
			fgets(buf, sizeof(buf), file);
			sscanf(buf, "%s %s", buf, buf);
			mtllibname = buf;
			if (glmReadMTL(buf,texstore)==0) return(0);
			lastcommand='m';
			break;
		case 'u':
			if (lastcommand!='g') {
				/* can't change material except at start of a group */
				sprintf(groupname,"GROUP%05d",++groupcnt);
				group=glmAddGroup(groupname);
			}
			/* eat up rest of line */
			fgets(buf, sizeof(buf), file);

			lastcommand='u';
			break;
		case 'g':				/* group */
			/* eat up rest of line */
			fgets(buf, sizeof(buf), file);
			sscanf(buf, "%s", buf);
			group = glmAddGroup(buf);
			lastcommand='g';
			break;
		case 'f':				/* face */
			v = n = t = 0;
			fscanf(file, "%s", buf);
			/* can be one of %d, %d//%d, %d/%d, %d/%d/%d %d//%d */
			if (strstr(buf, "//")) {
				/* v//n */
				sscanf(buf, "%d//%d", &v, &n);
				fscanf(file, "%d//%d", &v, &n);
				fscanf(file, "%d//%d", &v, &n);
				numtriangles++;
				group->numtriangles++;
				while(fscanf(file, "%d//%d", &v, &n) > 0) {
					numtriangles++;
					group->numtriangles++;
				}
			}
			else if (sscanf(buf, "%d/%d/%d", &v, &t, &n) == 3) {
				/* v/t/n */
				fscanf(file, "%d/%d/%d", &v, &t, &n);
				fscanf(file, "%d/%d/%d", &v, &t, &n);
				numtriangles++;
				group->numtriangles++;
				while(fscanf(file, "%d/%d/%d", &v, &t, &n) > 0) {
					numtriangles++;
					group->numtriangles++;
				}
			}
			else if (sscanf(buf, "%d/%d", &v, &t) == 2) {
				/* v/t */
				fscanf(file, "%d/%d", &v, &t);
				fscanf(file, "%d/%d", &v, &t);
				numtriangles++;
				group->numtriangles++;
				while(fscanf(file, "%d/%d", &v, &t) > 0) {
					numtriangles++;
					group->numtriangles++;
				}
			} 
			else {
				/* v */
				fscanf(file, "%d", &v);
				fscanf(file, "%d", &v);
				numtriangles++;
				group->numtriangles++;
				while(fscanf(file, "%d", &v) > 0) {
					numtriangles++;
					group->numtriangles++;
				}
			}
			lastcommand='f';
			break;
		default:
			/* eat up rest of line */
			fgets(buf, sizeof(buf), file);
			break;
	    }
	}

	/* allocate memory for the triangles in each group */
	group = groups;
	while (group) {
		group->triangles = new GLuint[group->numtriangles];
		group->numtriangles = 0;
		group = group->next;
	}

	return(1);
}

/* glmFindGroup: Find a material in the model
 */
GLuint CObjFile::glmFindMaterial(LPCTSTR name)
{
	GLuint i;

	/* XXX doing a linear search on a string key'd list is pretty lame,
     but it works and is fast enough for now. */
	for (i = 0; i <= nummaterials; i++) {
		if (materials[i].name.Compare(name)==0) {
			materials[i].used=1;
			return(i);
		}
	}

	/* didn't find the name, so print a warning and return the default
     material (0). */
	TRACE("glmFindMaterial():  can't find material \"%s\".\n", name);
	materials[0].used=1;
	return(0);
}

/* glmSecondPass: second pass at a Wavefront OBJ file that gets all
 * the data.
 *
 * model - properly initialized GLMmodel structure
 * file  - (fopen'd) file descriptor 
 */
#define T(x) (triangles[(x)])

GLvoid CObjFile::glmSecondPass(FILE* file) 
{
	GLMgroup* group;			/* current group pointer */
	GLuint    material;			/* current material */
	GLuint    v, n, t;
	char      buf[128];
	char      lastcommand=0;
	char		groupname[256];
	int			groupcnt=0;

	group        = groups;

	/* on the second pass through the file, read all the data into the
     allocated arrays */
	numvertices = numnormals = numtexcoords = 0;
	numtriangles = 0;
	material = 0;
	while(fscanf(file, "%s", buf) != EOF) {
	    switch(buf[0]) {
		case '#':				/* comment */
			/* eat up rest of line */
			fgets(buf, sizeof(buf), file);
			break;
		case 'v':				/* v, vn, vt */
			switch(buf[1]) {
			case '\0':			/* vertex */
				numvertices++;
				fscanf(file, "%f %f %f", &vertices[3 * numvertices + 0], &vertices[3 * numvertices + 1], &vertices[3 * numvertices + 2]);
				break;
			case 'n':				/* normal */
				numnormals++;
				fscanf(file, "%f %f %f", &normals[3 * numnormals + 0], &normals[3 * numnormals + 1], &normals[3 * numnormals + 2]);
				break;
			case 't':				/* texcoord */
				numtexcoords++;
				fscanf(file, "%f %f", &texcoords[2 * numtexcoords + 0],&texcoords[2 * numtexcoords + 1]);
				break;
			}
			lastcommand='v';
			break;
		case 'u':
			if (lastcommand!='g') {
				/* can't change material except at start of a group */
				sprintf(groupname,"GROUP%05d",++groupcnt);
				group=glmFindGroup(groupname);
			}
			fgets(buf, sizeof(buf), file);
			sscanf(buf, "%s %s", buf, buf);
			group->material = material = glmFindMaterial(buf);
			TRACE("usemtl %s => material %d\n",buf,material);
			break;
		case 'g':				/* group */
			/* eat up rest of line */
			fgets(buf, sizeof(buf), file);
			sscanf(buf, "%s", buf);
			group = glmFindGroup(buf);
			group->material = material;
			lastcommand='g';
			break;
		case 'f':				/* face */
			v = n = t = 0;
			fscanf(file, "%s", buf);
			/* can be one of %d, %d//%d, %d/%d, %d/%d/%d %d//%d */
			if (strstr(buf, "//")) {
				/* v//n */
				sscanf(buf, "%d//%d", &v, &n);
				T(numtriangles).vindices[0] = v;
				T(numtriangles).nindices[0] = n;
				fscanf(file, "%d//%d", &v, &n);
				T(numtriangles).vindices[1] = v;
				T(numtriangles).nindices[1] = n;
				fscanf(file, "%d//%d", &v, &n);
				T(numtriangles).vindices[2] = v;
				T(numtriangles).nindices[2] = n;
				group->triangles[group->numtriangles++] = numtriangles;
				numtriangles++;
				while (fscanf(file, "%d//%d", &v, &n) > 0) {
					T(numtriangles).vindices[0] = T(numtriangles-1).vindices[0];
					T(numtriangles).nindices[0] = T(numtriangles-1).nindices[0];
					T(numtriangles).vindices[1] = T(numtriangles-1).vindices[2];
					T(numtriangles).nindices[1] = T(numtriangles-1).nindices[2];
					T(numtriangles).vindices[2] = v;
					T(numtriangles).nindices[2] = n;
					group->triangles[group->numtriangles++] = numtriangles;
					numtriangles++;
				}
			}
			else if (sscanf(buf, "%d/%d/%d", &v, &t, &n) == 3) {
				/* v/t/n */
				T(numtriangles).vindices[0] = v;
				T(numtriangles).tindices[0] = t;
				T(numtriangles).nindices[0] = n;
				fscanf(file, "%d/%d/%d", &v, &t, &n);
				T(numtriangles).vindices[1] = v;
				T(numtriangles).tindices[1] = t;
				T(numtriangles).nindices[1] = n;
				fscanf(file, "%d/%d/%d", &v, &t, &n);
				T(numtriangles).vindices[2] = v;
				T(numtriangles).tindices[2] = t;
				T(numtriangles).nindices[2] = n;
				group->triangles[group->numtriangles++] = numtriangles;
				numtriangles++;
				while (fscanf(file, "%d/%d/%d", &v, &t, &n) > 0) {
					T(numtriangles).vindices[0] = T(numtriangles-1).vindices[0];
					T(numtriangles).tindices[0] = T(numtriangles-1).tindices[0];
					T(numtriangles).nindices[0] = T(numtriangles-1).nindices[0];
					T(numtriangles).vindices[1] = T(numtriangles-1).vindices[2];
					T(numtriangles).tindices[1] = T(numtriangles-1).tindices[2];
					T(numtriangles).nindices[1] = T(numtriangles-1).nindices[2];
					T(numtriangles).vindices[2] = v;
					T(numtriangles).tindices[2] = t;
					T(numtriangles).nindices[2] = n;
					group->triangles[group->numtriangles++] = numtriangles;
					numtriangles++;
				}
			}
			else if (sscanf(buf, "%d/%d", &v, &t) == 2) {
				/* v/t */
				T(numtriangles).vindices[0] = v;
				T(numtriangles).tindices[0] = t;
				fscanf(file, "%d/%d", &v, &t);
				T(numtriangles).vindices[1] = v;
				T(numtriangles).tindices[1] = t;
				fscanf(file, "%d/%d", &v, &t);
				T(numtriangles).vindices[2] = v;
				T(numtriangles).tindices[2] = t;
				group->triangles[group->numtriangles++] = numtriangles;
				numtriangles++;
				while (fscanf(file, "%d/%d", &v, &t) > 0) {
					T(numtriangles).vindices[0] = T(numtriangles-1).vindices[0];
					T(numtriangles).tindices[0] = T(numtriangles-1).tindices[0];
					T(numtriangles).vindices[1] = T(numtriangles-1).vindices[2];
					T(numtriangles).tindices[1] = T(numtriangles-1).tindices[2];
					T(numtriangles).vindices[2] = v;
					T(numtriangles).tindices[2] = t;
					group->triangles[group->numtriangles++] = numtriangles;
					numtriangles++;
				}
			}
			else {
				/* v */
				sscanf(buf, "%d", &v);
				T(numtriangles).vindices[0] = v;
				fscanf(file, "%d", &v);
				T(numtriangles).vindices[1] = v;
				fscanf(file, "%d", &v);
				T(numtriangles).vindices[2] = v;
				group->triangles[group->numtriangles++] = numtriangles;
				numtriangles++;
				while (fscanf(file, "%d", &v) > 0) {
					T(numtriangles).vindices[0] = T(numtriangles-1).vindices[0];
					T(numtriangles).vindices[1] = T(numtriangles-1).vindices[2];
					T(numtriangles).vindices[2] = v;
					group->triangles[group->numtriangles++] = numtriangles;
					numtriangles++;
				}
			}
			lastcommand='f';
			break;

		default:
			/* eat up rest of line */
			fgets(buf, sizeof(buf), file);
			break;
		}
	}

	TRACE("Loaded %s:\n",pathname);
	TRACE("\tnumvertices=%d\n",numvertices);
	TRACE("\tnumnormals=%d\n",numnormals);
	TRACE("\tnumtexcoords=%d\n",numtexcoords);
	TRACE("\tnumfacetnorms=%d\n",numfacetnorms);
	TRACE("\tnumtriangles=%d\n",numtriangles);
	TRACE("\tnummaterials=%d\n",nummaterials);
	TRACE("\tnumgroups=%d\n",numgroups);
//	TRACE("\tnumtextures=%d\n",numtextures);

  /* announce the memory requirements */
  TRACE("\tmemory=%d bytes\n",
	 numvertices  * 3*sizeof(GLfloat) +
	 numnormals   * 3*sizeof(GLfloat) * (numnormals ? 1 : 0) +
	 numtexcoords * 3*sizeof(GLfloat) * (numtexcoords ? 1 : 0) +
	 numtriangles * sizeof(GLMtriangle));

}

/* glmDirName: return the directory given a path
 *
 * path - filesystem path
 *
 * NOTE: the return value should be free'd.
 */
char* CObjFile::glmDirName(LPCTSTR path)
{
	char* dir;
	char* s;

	dir = _strdup(path);

	s = strrchr(dir, '/');
	if (s)
		s[1] = '\0';
	else {
		s = strrchr(dir, '\\');
		if (s)
			s[1]='\0';
		else
			dir[0] = '\0';
	}

	return dir;
}

/* glmReadMTL: read a wavefront material library file
 *
 * model - properly initialized GLMmodel structure
 * name  - name of the material library
 */
int CObjFile::glmReadMTL(LPCTSTR name,CTextureStore &texstore)
{
	FILE* file;
	char* dir;
	char* filename;
	char	tfilename[256];
	char  buf[256];
	GLuint i;

	dir = glmDirName(pathname);
	filename = (char*)malloc(sizeof(char) * (strlen(dir) + strlen(name) + 1));
	strcpy(filename, dir);
	strcat(filename, name);
	free(dir);

	file = fopen(filename, "r");
	if (!file) {
		TRACE("glmReadMTL() failed: can't open material file \"%s\".\n",filename);
		return(0);
	}
	TRACE("Loading materials from '%s'\n",filename);
	free(filename);

	/* count the number of materials in the file */
	nummaterials = 1;
	while (fscanf(file, "%s", buf) != EOF) {
	    switch(buf[0]) {
	    case '#':				/* comment */
			/* eat up rest of line */
			fgets(buf, sizeof(buf), file);
			break;
		case 'n':				/* newmtl */
			fgets(buf, sizeof(buf), file);
			nummaterials++;
			sscanf(buf, "%s %s", buf, buf);
			break;
		default:
			/* eat up rest of line */
			fgets(buf, sizeof(buf), file);
			break;
		}
	}

	rewind(file);

	if (materials) delete [] materials;
	materials = new GLMmaterial[nummaterials];

	/* set the default material */
	for (i = 0; i < nummaterials; i++) {
		materials[i].name = "";
		materials[i].shininess = 65.0f;
		materials[i].diffuse[0] = 0.8f;
		materials[i].diffuse[1] = 0.8f;
		materials[i].diffuse[2] = 0.8f;
		materials[i].diffuse[3] = 1.0f;
		materials[i].ambient[0] = 0.2f;
		materials[i].ambient[1] = 0.2f;
		materials[i].ambient[2] = 0.2f;
		materials[i].ambient[3] = 1.0f;
		materials[i].specular[0] = 0.0f;
		materials[i].specular[1] = 0.0f;
		materials[i].specular[2] = 0.0f;
		materials[i].specular[3] = 1.0f;
		materials[i].textureid=-1;
	}
	materials[0].name = "default";
	materials[0].used=0;

	/* now, read in the data */
	nummaterials = 0;
	while (fscanf(file, "%s", buf) != EOF) {
	    switch(buf[0]) {
		case '#':				/* comment */
			/* eat up rest of line */
			fgets(buf, sizeof(buf), file);
			break;
		case 'n':				/* newmtl */
			fgets(buf, sizeof(buf), file);
			sscanf(buf, "%s %s", buf, buf);
			nummaterials++;
			materials[nummaterials].name = buf;
			materials[nummaterials].used=0;
			TRACE("material[%d]='%s'\n",nummaterials,buf);
			break;
		case 'N':
			if (buf[1]=='s') {
				fscanf(file, "%f", &materials[nummaterials].shininess);
TRACE("Read shininess=%g\n",materials[nummaterials].shininess);
				/* wavefront shininess is from [0, 1000], so scale for OpenGL */
				materials[nummaterials].shininess /= 1000.0;
				materials[nummaterials].shininess *= 128.0;
			}
			break;
		case 'K':
			switch(buf[1]) {
			case 'd':
				fscanf(file, "%f %f %f",&materials[nummaterials].diffuse[0],&materials[nummaterials].diffuse[1],&materials[nummaterials].diffuse[2]);
				break;
			case 's':
				fscanf(file, "%f %f %f",&materials[nummaterials].specular[0],&materials[nummaterials].specular[1],&materials[nummaterials].specular[2]);
				break;
			case 'a':
				fscanf(file, "%f %f %f",&materials[nummaterials].ambient[0],&materials[nummaterials].ambient[1],&materials[nummaterials].ambient[2]);
				break;
			default:
				/* eat up rest of line */
				fgets(buf, sizeof(buf), file);
				break;
			}
			break;
		case 'm':		// texture
			fgets(tfilename,sizeof(tfilename),file);
			if (strncmp(buf,"map_Kd",6)==0) {
				materials[nummaterials].textureid=glmFindOrAddTexture(tfilename,texstore);
			}
			else {
				TRACE("map %s ignored\n",buf);
			}
			break;
	    default:
			/* eat up rest of line */
			fgets(buf, sizeof(buf), file);
			break;
		}
	}

	fclose(file);
	return(1);
}

/* glmDraw: Renders the model to the current OpenGL context using the
 * mode specified.
 *
 * model    - initialized GLMmodel structure
 * mode     - a bitwise OR of values describing what is to be rendered.
 *            GLM_NONE     -  render with only vertices
 *            GLM_FLAT     -  render with facet normals
 *            GLM_SMOOTH   -  render with vertex normals
 *            GLM_TEXTURE  -  render with texture coords
 *            GLM_COLOR    -  render with colors (color material)
 *            GLM_MATERIAL -  render with materials
 *            GLM_COLOR and GLM_MATERIAL should not both be specified.  
 *            GLM_FLAT and GLM_SMOOTH should not both be specified.  
 */
GLvoid CObjFile::glmDraw(GLuint mode)
{
	static GLuint i;
	static GLMgroup* group;
	static GLMtriangle* triangle;
	static GLMmaterial* material;

	if (!vertices) return;

	/* do a bit of warning */
	if (mode & GLM_FLAT && !facetnorms) {
		TRACE("glmDraw() warning: flat render mode requested with no facet normals defined.\n");
		mode &= ~GLM_FLAT;
	}
	if (mode & GLM_SMOOTH && !normals) {
		TRACE("glmDraw() warning: smooth render mode requested with no normals defined.\n");
		mode &= ~GLM_SMOOTH;
	}
	if (mode & GLM_TEXTURE && !texcoords) {
		TRACE("glmDraw() warning: texture render mode requested with no texture coordinates defined.\n");
		mode &= ~GLM_TEXTURE;
	}
	if (mode & GLM_FLAT && mode & GLM_SMOOTH) {
		TRACE("glmDraw() warning: flat render mode requested and smooth render mode requested (using smooth).\n");
		mode &= ~GLM_FLAT;
	}
	if (mode & GLM_COLOR && !materials) {
		TRACE("glmDraw() warning: color render mode requested with no materials defined.\n");
		mode &= ~GLM_COLOR;
	}
	if (mode & GLM_MATERIAL && !materials) {
		TRACE("glmDraw() warning: material render mode requested with no materials defined.\n");
		mode &= ~GLM_MATERIAL;
	}
	if (mode & GLM_COLOR && mode & GLM_MATERIAL) {
		TRACE("glmDraw() warning: color and material render mode requested using only material mode.\n");
		mode &= ~GLM_COLOR;
	}
	if (mode & GLM_COLOR)
	    glEnable(GL_COLOR_MATERIAL);
	else if (mode & GLM_MATERIAL)
		glDisable(GL_COLOR_MATERIAL);

	if (mode & GLM_TEXTURE) {
		glEnable(GL_TEXTURE_2D);
		glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
	}

	/* perhaps this loop should be unrolled into material, color, flat,
     smooth, etc. loops?  since most cpu's have good branch prediction
     schemes (and these branches will always go one way), probably
     wouldn't gain too much?  */
	int textureid=-1;
	group = groups;
	while (group) {


		material = &materials[group->material];
		if (material) textureid = material->textureid; else textureid=-1;
//		TRACE("Group texture=%d\n",textureid);

		if (mode & GLM_MATERIAL) {
			material = &materials[group->material];
			// TRACE("Choosing material %d = Kd1=%g\n",group->material,material->diffuse[0]);
			glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, material->ambient);
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, material->diffuse);
			glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, material->specular);
			if (material->shininess!=0.0) {
				// TRACE("shininess=%g\n",material->shininess);
				glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, material->shininess);
			}
		}

		if (mode & GLM_TEXTURE) {
			if (textureid<0)
				glBindTexture(GL_TEXTURE_2D,0);
			else {
//				glBindTexture(GL_TEXTURE_2D,textures[textureid].id);
				glBindTexture(GL_TEXTURE_2D,textureid);
//				TRACE("Bind texture id %d\n",textures[textureid].id);
			}
		}

		if (mode & GLM_COLOR) {
			material = &materials[group->material];
			// TRACE("Choosing material %d = Kd1=%g,%g,%g\n",group->material,material->diffuse[0],material->diffuse[1],material->diffuse[2]);
			glColor3fv(material->diffuse);
		}

		glBegin(GL_TRIANGLES);
		for (i = 0; i < group->numtriangles; i++) {
			triangle = &T(group->triangles[i]);

			if (mode & GLM_FLAT)
				glNormal3fv(&facetnorms[3 * triangle->findex]);
 			if (mode & GLM_SMOOTH)
				glNormal3fv(&normals[3 * triangle->nindices[0]]);
			if (mode & GLM_TEXTURE)
				glTexCoord2fv(&texcoords[2 * triangle->tindices[0]]);
			glVertex3fv(&vertices[3 * triangle->vindices[0]]);

			if (mode & GLM_SMOOTH)
				glNormal3fv(&normals[3 * triangle->nindices[1]]);
			if (mode & GLM_TEXTURE)
				glTexCoord2fv(&texcoords[2 * triangle->tindices[1]]);
			glVertex3fv(&vertices[3 * triangle->vindices[1]]);

			if (mode & GLM_SMOOTH)
				glNormal3fv(&normals[3 * triangle->nindices[2]]);
			if (mode & GLM_TEXTURE)
				glTexCoord2fv(&texcoords[2 * triangle->tindices[2]]);
			glVertex3fv(&vertices[3 * triangle->vindices[2]]);
		}
		glEnd();

		group = group->next;
	}
}

GLvoid CObjFile::glmDrawMorph(CMorph *pMorph,double wt,GLuint mode)
{
	static GLuint i;
	static GLMgroup* group;
	static GLMtriangle* triangle;
	static GLMmaterial* material;

	if (!vertices) return;

//	TRACE("glmDrawMorph wt=%g\n",wt);

	/* do a bit of warning */
	if (mode & GLM_FLAT && !facetnorms) {
		TRACE("glmDraw() warning: flat render mode requested with no facet normals defined.\n");
		mode &= ~GLM_FLAT;
	}
	if (mode & GLM_SMOOTH && !normals) {
		TRACE("glmDraw() warning: smooth render mode requested with no normals defined.\n");
		mode &= ~GLM_SMOOTH;
	}
	if (mode & GLM_TEXTURE && !texcoords) {
		TRACE("glmDraw() warning: texture render mode requested with no texture coordinates defined.\n");
		mode &= ~GLM_TEXTURE;
	}
	if (mode & GLM_FLAT && mode & GLM_SMOOTH) {
		TRACE("glmDraw() warning: flat render mode requested and smooth render mode requested (using smooth).\n");
		mode &= ~GLM_FLAT;
	}
	if (mode & GLM_COLOR && !materials) {
		TRACE("glmDraw() warning: color render mode requested with no materials defined.\n");
		mode &= ~GLM_COLOR;
	}
	if (mode & GLM_MATERIAL && !materials) {
		TRACE("glmDraw() warning: material render mode requested with no materials defined.\n");
		mode &= ~GLM_MATERIAL;
	}
	if (mode & GLM_COLOR && mode & GLM_MATERIAL) {
		TRACE("glmDraw() warning: color and material render mode requested using only material mode.\n");
		mode &= ~GLM_COLOR;
	}
	if (mode & GLM_COLOR)
	    glEnable(GL_COLOR_MATERIAL);
	else if (mode & GLM_MATERIAL)
		glDisable(GL_COLOR_MATERIAL);

	if (mode & GLM_TEXTURE) {
		glEnable(GL_TEXTURE_2D);
		glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
	}

	/* generate list of morphed vertices */
	GLfloat*	mvertices=(GLfloat *)calloc(numvertices+1,3*sizeof(GLfloat));
	for (i=1;i<=numvertices;i++) {
		mvertices[3*i+0] = vertices[3*i+0];
		mvertices[3*i+1] = vertices[3*i+1];
		mvertices[3*i+2] = vertices[3*i+2];
	}
	for (int j=0;j<pMorph->m_vmorph.GetCount();j++) {
		morph_vertex v = pMorph->m_vmorph[j];
		i = v.vidx;
		mvertices[3*i+0] += (GLfloat)(wt*v.diff[0]);
		mvertices[3*i+1] += (GLfloat)(wt*v.diff[1]);
		mvertices[3*i+2] += (GLfloat)(wt*v.diff[2]);
	}

	/* generate list of morphed vertex normals */
	GLfloat*	mnormals=(GLfloat *)calloc(numnormals+1,3*sizeof(GLfloat));
	for (i=1;i<=numnormals;i++) {
		mnormals[3*i+0] = normals[3*i+0];
		mnormals[3*i+1] = normals[3*i+1];
		mnormals[3*i+2] = normals[3*i+2];
	}
	for (int j=0;j<pMorph->m_nmorph.GetCount();j++) {
		morph_vertex v = pMorph->m_nmorph[j];
		i = v.vidx;
		mnormals[3*i+0] += (GLfloat)(wt*v.diff[0]);
		mnormals[3*i+1] += (GLfloat)(wt*v.diff[1]);
		mnormals[3*i+2] += (GLfloat)(wt*v.diff[2]);
	}

	/* generate list of morphed texture co-ordinates */
	GLfloat*	mtexcoords=(GLfloat *)calloc(numtexcoords+1,2*sizeof(GLfloat));
	for (i=1;i<=numtexcoords;i++) {
		mtexcoords[2*i+0] = texcoords[2*i+0];
		mtexcoords[2*i+1] = texcoords[2*i+1];
	}
	for (int j=0;j<pMorph->m_tmorph.GetCount();j++) {
		morph_vertex v = pMorph->m_tmorph[j];
		i = v.vidx;
		mtexcoords[2*i+0] += (GLfloat)(wt*v.diff[0]);
		mtexcoords[2*i+1] += (GLfloat)(wt*v.diff[1]);
	}

	/* perhaps this loop should be unrolled into material, color, flat,
     smooth, etc. loops?  since most cpu's have good branch prediction
     schemes (and these branches will always go one way), probably
     wouldn't gain too much?  */
	int textureid=-1;
	group = groups;
	while (group) {


		material = &materials[group->material];
		if (material) textureid = material->textureid; else textureid=-1;
//		TRACE("Group texture=%d\n",textureid);

		if (mode & GLM_MATERIAL) {
			material = &materials[group->material];
			// TRACE("Choosing material %d = Kd1=%g\n",group->material,material->diffuse[0]);
			glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, material->ambient);
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, material->diffuse);
			glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, material->specular);
			if (material->shininess!=0.0) {
				// TRACE("shininess=%g\n",material->shininess);
				glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, material->shininess);
			}
		}

		if (mode & GLM_TEXTURE) {
			if (textureid<0)
				glBindTexture(GL_TEXTURE_2D,0);
			else {
//				glBindTexture(GL_TEXTURE_2D,textures[textureid].id);
				glBindTexture(GL_TEXTURE_2D,textureid);
//				TRACE("Bind texture id %d\n",textures[textureid].id);
			}
		}

		if (mode & GLM_COLOR) {
			material = &materials[group->material];
			// TRACE("Choosing material %d = Kd1=%g,%g,%g\n",group->material,material->diffuse[0],material->diffuse[1],material->diffuse[2]);
			glColor3fv(material->diffuse);
		}

		glBegin(GL_TRIANGLES);
		for (i = 0; i < group->numtriangles; i++) {
			triangle = &T(group->triangles[i]);

			if (mode & GLM_FLAT)
				glNormal3fv(&facetnorms[3 * triangle->findex]);
 			if (mode & GLM_SMOOTH)
				glNormal3fv(&mnormals[3 * triangle->nindices[0]]);
			if (mode & GLM_TEXTURE)
				glTexCoord2fv(&mtexcoords[2 * triangle->tindices[0]]);
			glVertex3fv(&mvertices[3 * triangle->vindices[0]]);

			if (mode & GLM_SMOOTH)
				glNormal3fv(&mnormals[3 * triangle->nindices[1]]);
			if (mode & GLM_TEXTURE)
				glTexCoord2fv(&mtexcoords[2 * triangle->tindices[1]]);
			glVertex3fv(&mvertices[3 * triangle->vindices[1]]);

			if (mode & GLM_SMOOTH)
				glNormal3fv(&mnormals[3 * triangle->nindices[2]]);
			if (mode & GLM_TEXTURE)
				glTexCoord2fv(&mtexcoords[2 * triangle->tindices[2]]);
			glVertex3fv(&mvertices[3 * triangle->vindices[2]]);
		}
		glEnd();

		group = group->next;
	}

	free(mvertices);
	free(mnormals);
	free(mtexcoords);
}

/* glmList: Generates and returns a display list for the model using
 * the mode specified.
 *
 * model    - initialized GLMmodel structure
 * mode     - a bitwise OR of values describing what is to be rendered.
 *            GLM_NONE     -  render with only vertices
 *            GLM_FLAT     -  render with facet normals
 *            GLM_SMOOTH   -  render with vertex normals
 *            GLM_TEXTURE  -  render with texture coords
 *            GLM_COLOR    -  render with colors (color material)
 *            GLM_MATERIAL -  render with materials
 *            GLM_COLOR and GLM_MATERIAL should not both be specified.  
 * GLM_FLAT and GLM_SMOOTH should not both be specified.  */
GLuint CObjFile::glmList(GLuint mode)
{
	GLuint list;

	list = glGenLists(1);
	glNewList(list, GL_COMPILE);
	glmDraw(mode);
	glEndList();

	return list;
}

/* glmMax: returns the maximum of two floats */
GLfloat CObjFile::glmMax(GLfloat a, GLfloat b) 
{
	if (b > a) return b;
	return a;
}

/* glmAbs: returns the absolute value of a float */
GLfloat CObjFile::glmAbs(GLfloat f)
{
	if (f < 0) return -f;
	return f;
}

/* glmDot: compute the dot product of two vectors
 *
 * u - array of 3 GLfloats (GLfloat u[3])
 * v - array of 3 GLfloats (GLfloat v[3])
 */
GLfloat CObjFile::glmDot(GLfloat* u, GLfloat* v)
{
  return u[0]*v[0] + u[1]*v[1] + u[2]*v[2];
}

/* glmCross: compute the cross product of two vectors
 *
 * u - array of 3 GLfloats (GLfloat u[3])
 * v - array of 3 GLfloats (GLfloat v[3])
 * n - array of 3 GLfloats (GLfloat n[3]) to return the cross product in
 */
GLvoid CObjFile::glmCross(GLfloat* u, GLfloat* v, GLfloat* n)
{
  n[0] = u[1]*v[2] - u[2]*v[1];
  n[1] = u[2]*v[0] - u[0]*v[2];
  n[2] = u[0]*v[1] - u[1]*v[0];
}

/* glmNormalize: normalize a vector
 *
 * v - array of 3 GLfloats (GLfloat v[3]) to be normalized
 */
GLvoid CObjFile::glmNormalize(GLfloat* v)
{
	GLfloat l;

	l = (GLfloat)sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
	v[0] /= l;
	v[1] /= l;
	v[2] /= l;
}

/* glmUnitize: "unitize" a model by translating it to the origin and
 * scaling it to fit in a unit cube around the origin (-1 to 1 in all
 * dimensions).  
 * Returns the scalefactor used.
 *
 * model - properly initialized GLMmodel structure 
 */
GLfloat CObjFile::glmUnitize()
{
	GLuint  i;
	GLfloat maxx, minx, maxy, miny, maxz, minz;
	GLfloat cx, cy, cz, w, h, d;
	GLfloat scale;

	if (!vertices) return(0.0);

	/* get the max/mins */
	maxx = minx = vertices[3 + 0];
	maxy = miny = vertices[3 + 1];
	maxz = minz = vertices[3 + 2];
	for (i = 1; i <= numvertices; i++) {
		if (maxx < vertices[3 * i + 0]) maxx = vertices[3 * i + 0];
		if (minx > vertices[3 * i + 0]) minx = vertices[3 * i + 0];
		if (maxy < vertices[3 * i + 1]) maxy = vertices[3 * i + 1];
		if (miny > vertices[3 * i + 1]) miny = vertices[3 * i + 1];
		if (maxz < vertices[3 * i + 2]) maxz = vertices[3 * i + 2];
		if (minz > vertices[3 * i + 2]) minz = vertices[3 * i + 2];
	}
	
	/* calculate model width, height, and depth */
	w = maxx - minx;
	h = maxy - miny;
	d = maxz - minz;

	/* calculate center of the model */
	cx = (GLfloat)((maxx + minx) / 2.0);
	cy = (GLfloat)((maxy + miny) / 2.0);
	cz = (GLfloat)((maxz + minz) / 2.0);

	/* calculate unitizing scale factor */
	scale = (GLfloat)(2.0 / glmMax(glmMax(w, h), d));

	/* translate around center then scale */
	for (i = 1; i <= numvertices; i++) {
		vertices[3 * i + 0] -= cx;
	    vertices[3 * i + 1] -= cy;
		vertices[3 * i + 2] -= cz;
		vertices[3 * i + 0] *= scale;
		vertices[3 * i + 1] *= scale;
		vertices[3 * i + 2] *= scale;
	}

	return scale;
}

/* glmFacetNormals: Generates facet normals for a model (by taking the
 * cross product of the two vectors derived from the sides of each
 * triangle).  Assumes a counter-clockwise winding.
 *
 * model - initialized GLMmodel structure
 */
GLvoid CObjFile::glmFacetNormals()
{
	GLuint  i;
	GLfloat u[3];
	GLfloat v[3];

	if (!vertices) return;

	/* clobber any old facetnormals */
	if (facetnorms) delete [] facetnorms;

	/* allocate memory for the new facet normals */
	numfacetnorms = numtriangles;
	facetnorms = new GLfloat[3 * (numfacetnorms + 1)];

	for (i = 0; i < numtriangles; i++) {
		triangles[i].findex = i+1;

		u[0] = vertices[3 * T(i).vindices[1] + 0] - vertices[3 * T(i).vindices[0] + 0];
		u[1] = vertices[3 * T(i).vindices[1] + 1] - vertices[3 * T(i).vindices[0] + 1];
		u[2] = vertices[3 * T(i).vindices[1] + 2] - vertices[3 * T(i).vindices[0] + 2];

		v[0] = vertices[3 * T(i).vindices[2] + 0] - vertices[3 * T(i).vindices[0] + 0];
		v[1] = vertices[3 * T(i).vindices[2] + 1] - vertices[3 * T(i).vindices[0] + 1];
	    v[2] = vertices[3 * T(i).vindices[2] + 2] - vertices[3 * T(i).vindices[0] + 2];

		glmCross(u, v, &facetnorms[3 * (i+1)]);
		glmNormalize(&facetnorms[3 * (i+1)]);
	}
}

/* _GLMnode: general purpose node
 */
typedef struct _GLMnode {
  GLuint           index;
  GLboolean        averaged;
  struct _GLMnode* next;
} GLMnode;

/* glmVertexNormals: Generates smooth vertex normals for a model.
 * First builds a list of all the triangles each vertex is in.  Then
 * loops through each vertex in the the list averaging all the facet
 * normals of the triangles each vertex is in.  Finally, sets the
 * normal index in the triangle for the vertex to the generated smooth
 * normal.  If the dot product of a facet normal and the facet normal
 * associated with the first triangle in the list of triangles the
 * current vertex is in is greater than the cosine of the angle
 * parameter to the function, that facet normal is not added into the
 * average normal calculation and the corresponding vertex is given
 * the facet normal.  This tends to preserve hard edges.  The angle to
 * use depends on the model, but 90 degrees is usually a good start.
 *
 * model - initialized GLMmodel structure
 * angle - maximum angle (in degrees) to smooth across
 */

// replace the existing code with one that assigns normals to every triangle
#if 0
GLvoid CObjFile::glmVertexNormals(GLfloat angle)
{
	GLMnode*  node;
	GLMnode*  tail;
	GLMnode** members;
	GLfloat*  tnormals;
	GLfloat   average[3];
	GLfloat   dot, cos_angle;
	GLuint    i, avg;

	if (!facetnorms) return;

	/* calculate the cosine of the angle (in degrees) */
	cos_angle = (GLfloat)(cos(angle * M_PI / 180.0));

	/* nuke any previous normals */
	if (normals) delete [] normals;

	/* allocate space for new normals */
	numnormals = numtriangles * 3; /* 3 normals per triangle */
	tnormals = new GLfloat[3* (numnormals+1)];

	/* allocate a structure that will hold a linked list of triangle indices for each vertex */
	members = (GLMnode**)malloc(sizeof(GLMnode*) * (numvertices + 1));
	for (i = 1; i <= numvertices; i++) members[i] = NULL;
  
	/* for every triangle, create a node for each vertex in it */
	for (i = 0; i < numtriangles; i++) {
		node = (GLMnode*)malloc(sizeof(GLMnode));
		node->index = i;
		node->next  = members[T(i).vindices[0]];
		members[T(i).vindices[0]] = node;

	    node = (GLMnode*)malloc(sizeof(GLMnode));
	    node->index = i;
	    node->next  = members[T(i).vindices[1]];
	    members[T(i).vindices[1]] = node;

	    node = (GLMnode*)malloc(sizeof(GLMnode));
	    node->index = i;
	    node->next  = members[T(i).vindices[2]];
	    members[T(i).vindices[2]] = node;
	}

	/* calculate the average normal for each vertex */
	numnormals = 1;
	for (i = 1; i <= numvertices; i++) {
	    /* calculate an average normal for this vertex by averaging the
	       facet normal of every triangle this vertex is in */
		node = members[i];
		if (!node) TRACE("glmVertexNormals(): vertex w/o a triangle\n");
	    average[0] = 0.0; average[1] = 0.0; average[2] = 0.0;
	    avg = 0;
	    while (node) {
			/* only average if the dot product of the angle between the two
		       facet normals is greater than the cosine of the threshold
		       angle -- or, said another way, the angle between the two
		       facet normals is less than (or equal to) the threshold angle */
			dot = glmDot(&facetnorms[3 * T(node->index).findex],&facetnorms[3 * T(members[i]->index).findex]);
			if (dot > cos_angle) {
				node->averaged = GL_TRUE;
				average[0] += facetnorms[3 * T(node->index).findex + 0];
				average[1] += facetnorms[3 * T(node->index).findex + 1];
				average[2] += facetnorms[3 * T(node->index).findex + 2];
				avg = 1;			/* we averaged at least one normal! */
			}
			else {
				node->averaged = GL_FALSE;
			}
			node = node->next;
		}

		if (avg) {
			/* normalize the averaged normal */
			glmNormalize(average);

			/* add the normal to the vertex normals list */
			tnormals[3 * numnormals + 0] = average[0];
			tnormals[3 * numnormals + 1] = average[1];
			tnormals[3 * numnormals + 2] = average[2];
			avg = numnormals;
			numnormals++;
		}

		/* set the normal of this vertex in each triangle it is in */
		node = members[i];
		while (node) {
			if (node->averaged) {
				/* if this node was averaged, use the average normal */
				if (T(node->index).vindices[0] == i) 
					T(node->index).nindices[0] = avg;
				else if (T(node->index).vindices[1] == i)
					T(node->index).nindices[1] = avg;
				else if (T(node->index).vindices[2] == i)
					T(node->index).nindices[2] = avg;
			}
			else {
				/* if this node wasn't averaged, use the facet normal */
				tnormals[3 * numnormals + 0] = facetnorms[3 * T(node->index).findex + 0];
				tnormals[3 * numnormals + 1] = facetnorms[3 * T(node->index).findex + 1];
				tnormals[3 * numnormals + 2] = facetnorms[3 * T(node->index).findex + 2];
				if (T(node->index).vindices[0] == i)
					T(node->index).nindices[0] = numnormals;
				else if (T(node->index).vindices[1] == i)
					T(node->index).nindices[1] = numnormals;
				else if (T(node->index).vindices[2] == i)
					T(node->index).nindices[2] = numnormals;
				numnormals++;
			}
			node = node->next;
		}
	}

	numnormals = numnormals - 1;

	/* free the member information */
	for (i = 1; i <= numvertices; i++) {
		node = members[i];
		while (node) {
			tail = node;
			node = node->next;
			free(tail);
		}
	}
	free(members);

	/* pack the normals array (we previously allocated the maximum
		number of normals that could possibly be created (numtriangles *
		3), so get rid of some of them (usually alot unless none of the
		facet normals were averaged)) */
	normals = new GLfloat[3* (numnormals+1)];
	for (i = 1; i <= numnormals; i++) {
		normals[3 * i + 0] = tnormals[3 * i + 0];
		normals[3 * i + 1] = tnormals[3 * i + 1];
		normals[3 * i + 2] = tnormals[3 * i + 2];
	}
	delete [] tnormals;
}
#else
GLvoid CObjFile::glmVertexNormals(GLfloat angle)
{
	GLMnode*  node;
	GLMnode*  tail;
	GLMnode** members;
	GLfloat   average[3];
	GLfloat   dot, cos_angle;
	GLuint    i, avg;

	if (!facetnorms) return;

	/* calculate the cosine of the angle (in degrees) */
	cos_angle = (GLfloat)(cos(angle * M_PI / 180.0));

	/* nuke any previous normals */
	if (normals) delete [] normals;

	/* allocate space for new normals */
	numnormals = numtriangles; /* 3 normals per triangle */
	normals = new GLfloat[3* (numnormals+1)];

	/* allocate a structure that will hold a linked list of triangle indices for each vertex */
	members = (GLMnode**)malloc(sizeof(GLMnode*) * (numvertices + 1));
	for (i = 1; i <= numvertices; i++) members[i] = NULL;
  
	/* for every triangle, create a node for each vertex in it */
	for (i = 0; i < numtriangles; i++) {
		node = (GLMnode*)malloc(sizeof(GLMnode));
		node->index = i;
		node->next  = members[T(i).vindices[0]];
		members[T(i).vindices[0]] = node;

	    node = (GLMnode*)malloc(sizeof(GLMnode));
	    node->index = i;
	    node->next  = members[T(i).vindices[1]];
	    members[T(i).vindices[1]] = node;

	    node = (GLMnode*)malloc(sizeof(GLMnode));
	    node->index = i;
	    node->next  = members[T(i).vindices[2]];
	    members[T(i).vindices[2]] = node;
	}

	/* calculate the average normal for each vertex */
	for (i = 1; i <= numvertices; i++) {
	    /* calculate an average normal for this vertex by averaging the
	       facet normal of every triangle this vertex is in */
		node = members[i];
		if (!node) TRACE("glmVertexNormals(): vertex w/o a triangle\n");
	    average[0] = 0.0; average[1] = 0.0; average[2] = 0.0;
	    avg = 0;
	    while (node) {
			/* only average if the dot product of the angle between the two
		       facet normals is greater than the cosine of the threshold
		       angle -- or, said another way, the angle between the two
		       facet normals is less than (or equal to) the threshold angle */
			dot = glmDot(&facetnorms[3 * T(node->index).findex],&facetnorms[3 * T(members[i]->index).findex]);
			if (dot > cos_angle) {
				node->averaged = GL_TRUE;
				average[0] += facetnorms[3 * T(node->index).findex + 0];
				average[1] += facetnorms[3 * T(node->index).findex + 1];
				average[2] += facetnorms[3 * T(node->index).findex + 2];
				avg = 1;			/* we averaged at least one normal! */
			}
			else {
				node->averaged = GL_FALSE;
			}
			node = node->next;
		}

		if (avg) {
			/* normalize the averaged normal */
			glmNormalize(average);

			/* add the normal to the vertex normals list */
			normals[3 * i + 0] = average[0];
			normals[3 * i + 1] = average[1];
			normals[3 * i + 2] = average[2];
		}
		else {
			/* if this node wasn't averaged, use the facet normal */
			node = members[i];
			normals[3 * i + 0] = facetnorms[3 * T(node->index).findex + 0];
			normals[3 * i + 1] = facetnorms[3 * T(node->index).findex + 1];
			normals[3 * i + 2] = facetnorms[3 * T(node->index).findex + 2];
		}

		/* set the normal of this vertex in each triangle it is in */
		node = members[i];
		while (node) {
			if (T(node->index).vindices[0] == i) 
				T(node->index).nindices[0] = i;
			else if (T(node->index).vindices[1] == i)
				T(node->index).nindices[1] = i;
			else if (T(node->index).vindices[2] == i)
				T(node->index).nindices[2] = i;
			node = node->next;
		}
	}

	/* free the member information */
	for (i = 1; i <= numvertices; i++) {
		node = members[i];
		while (node) {
			tail = node;
			node = node->next;
			free(tail);
		}
	}
	free(members);

}
#endif

/* glmWriteMTL: write a wavefront material library file
 *
 * model      - properly initialized GLMmodel structure
 * modelpath  - pathname of the model being written
 * mtllibname - name of the material library to be written
 */
int	CObjFile::glmWriteMTL(LPCTSTR modelpath, LPCTSTR mtllibname)
{
	FILE* file;
	char* dir;
	char* filename;
	GLMmaterial* material;
	GLuint i;

	dir = glmDirName(modelpath);
	filename = (char*)malloc(sizeof(char) * (strlen(dir)+strlen(mtllibname)+1));
	strcpy(filename, dir);
	strcat(filename, mtllibname);
	free(dir);

	/* open the file */
	if ((file = fopen(filename, "w"))==NULL) {
		TRACE("Failed to open '%s'\n",filename);
		free(filename);
		return(0);
	}
	free(filename);

	/* spit out a header */
	fprintf(file, "#  \n");
	fprintf(file, "#  Wavefront MTL generated by GLM library\n");
	fprintf(file, "#  \n");
	fprintf(file, "#  GLM library\n");
	fprintf(file, "#  Nate Robins\n");
	fprintf(file, "#  ndr@pobox.com\n");
	fprintf(file, "#  http://www.pobox.com/~ndr\n");
	fprintf(file, "#  \n\n");

	for (i = 0; i <= nummaterials; i++) if (materials[i].used) {
		material = &materials[i];
		fprintf(file, "newmtl %s\n", material->name);
	    fprintf(file, "Ka %f %f %f\n", material->ambient[0], material->ambient[1], material->ambient[2]);
		fprintf(file, "Kd %f %f %f\n", material->diffuse[0], material->diffuse[1], material->diffuse[2]);
		fprintf(file, "Ks %f %f %f\n", material->specular[0],material->specular[1],material->specular[2]);
		fprintf(file, "Ns %f\n", material->shininess / 128.0 * 1000.0);
		fprintf(file, "\n");
	}

	fclose(file);
	return(1);
}

/* glmWriteOBJ: Writes a model description in Wavefront .OBJ format to
 * a file.
 *
 * model    - initialized GLMmodel structure
 * filename - name of the file to write the Wavefront .OBJ format data to
 * mode     - a bitwise or of values describing what is written to the file
 *            GLM_NONE     -  render with only vertices
 *            GLM_FLAT     -  render with facet normals
 *            GLM_SMOOTH   -  render with vertex normals
 *            GLM_TEXTURE  -  render with texture coords
 *            GLM_COLOR    -  render with colors (color material)
 *            GLM_MATERIAL -  render with materials
 *            GLM_COLOR and GLM_MATERIAL should not both be specified.  
 *            GLM_FLAT and GLM_SMOOTH should not both be specified.  
 */
int	CObjFile::glmWriteOBJ(LPCTSTR filename, GLuint mode)
{
	GLuint    i;
	FILE*     file;
	GLMgroup* group;

	/* do a bit of warning */
	if (mode & GLM_FLAT && !facetnorms) {
		TRACE("glmWriteOBJ() warning: flat normal output requested with no facet normals defined.\n");
		mode &= ~GLM_FLAT;
	}
	if (mode & GLM_SMOOTH && !normals) {
		TRACE("glmWriteOBJ() warning: smooth normal output requested with no normals defined.\n");
		mode &= ~GLM_SMOOTH;
	}
	if (mode & GLM_TEXTURE && !texcoords) {
		TRACE("glmWriteOBJ() warning: texture coordinate output requested with no texture coordinates defined.\n");
		mode &= ~GLM_TEXTURE;
	}
	if (mode & GLM_FLAT && mode & GLM_SMOOTH) {
		TRACE("glmWriteOBJ() warning: flat normal output requested and smooth normal output requested (using smooth).\n");
		mode &= ~GLM_FLAT;
	}
	if (mode & GLM_COLOR && !materials) {
		TRACE("glmWriteOBJ() warning: color output requested with no colors (materials) defined.\n");
		mode &= ~GLM_COLOR;
	}
	if (mode & GLM_MATERIAL && !materials) {
		TRACE("glmWriteOBJ() warning: material output requested with no materials defined.\n");
		mode &= ~GLM_MATERIAL;
	}
	if (mode & GLM_COLOR && mode & GLM_MATERIAL) {
		TRACE("glmDraw() warning: color and material output requested outputting only materials.\n");
		mode &= ~GLM_COLOR;
	}

	/* open the file */
	if ((file = fopen(filename, "w"))==NULL) return(0);

	/* spit out a header */
	fprintf(file, "#  \n");
	fprintf(file, "#  Wavefront OBJ generated by GLM library\n");
	fprintf(file, "#  \n");
	fprintf(file, "#  GLM library\n");
	fprintf(file, "#  Nate Robins\n");
	fprintf(file, "#  ndr@pobox.com\n");
	fprintf(file, "#  http://www.pobox.com/~ndr\n");
	fprintf(file, "#  \n");

	if (mode & GLM_MATERIAL && mtllibname) {
		fprintf(file, "\nmtllib %s\n\n", mtllibname);
		if (glmWriteMTL(filename, mtllibname)==0) {
			TRACE("Failed to write material file\n");
		}
	}

	/* spit out the vertices */
	fprintf(file, "\n");
	fprintf(file, "# %d vertices\n", numvertices);
	for (i = 1; i <= numvertices; i++) {
		fprintf(file, "v %f %f %f\n", vertices[3 * i + 0], vertices[3 * i + 1], vertices[3 * i + 2]);
	}

	/* spit out the smooth/flat normals */
	if (mode & GLM_SMOOTH) {
		fprintf(file, "\n");
		fprintf(file, "# %d normals\n", numnormals);
		for (i = 1; i <= numnormals; i++) {
			fprintf(file, "vn %f %f %f\n", normals[3 * i + 0], normals[3 * i + 1], normals[3 * i + 2]);
		}
	}
	else if (mode & GLM_FLAT) {
		fprintf(file, "\n");
		fprintf(file, "# %d normals\n", numfacetnorms);
		for (i = 1; i <= numnormals; i++) {
			fprintf(file, "vn %f %f %f\n", facetnorms[3 * i + 0], facetnorms[3 * i + 1], facetnorms[3 * i + 2]);
		}
	}

	/* spit out the texture coordinates */
	if (mode & GLM_TEXTURE) {
		fprintf(file, "\n");
		fprintf(file, "# %d texcoords\n", texcoords);
		for (i = 1; i <= numtexcoords; i++) {
			fprintf(file, "vt %f %f\n", texcoords[2 * i + 0], texcoords[2 * i + 1]);
		}
	}

	fprintf(file, "\n");
	fprintf(file, "# %d groups\n", numgroups);
	fprintf(file, "# Total %d faces (triangles)\n", numtriangles);
	fprintf(file, "\n");

	group = groups;
	while (group) {
		if (group->numtriangles > 0) {
			fprintf(file, "g %s\n", group->name);
			fprintf(file, "# %d faces (triangles)\n", group->numtriangles);
			if (mode & GLM_MATERIAL)
				fprintf(file, "usemtl %s\n", materials[group->material].name);
			for (i = 0; i < group->numtriangles; i++) {
				if (mode & GLM_SMOOTH && mode & GLM_TEXTURE) {
					fprintf(file, "f %d/%d/%d %d/%d/%d %d/%d/%d\n",
						T(group->triangles[i]).vindices[0], 
						T(group->triangles[i]).nindices[0], 
						T(group->triangles[i]).tindices[0],
						T(group->triangles[i]).vindices[1],
						T(group->triangles[i]).nindices[1],
						T(group->triangles[i]).tindices[1],
						T(group->triangles[i]).vindices[2],
						T(group->triangles[i]).nindices[2],
						T(group->triangles[i]).tindices[2]);
				}
				else if (mode & GLM_FLAT && mode & GLM_TEXTURE) {
					fprintf(file, "f %d/%d %d/%d %d/%d\n",
						T(group->triangles[i]).vindices[0],
						T(group->triangles[i]).findex,
						T(group->triangles[i]).vindices[1],
						T(group->triangles[i]).findex,
						T(group->triangles[i]).vindices[2],
						T(group->triangles[i]).findex);
				}
				else if (mode & GLM_TEXTURE) {
					fprintf(file, "f %d/%d %d/%d %d/%d\n",
						T(group->triangles[i]).vindices[0],
						T(group->triangles[i]).tindices[0],
						T(group->triangles[i]).vindices[1],
						T(group->triangles[i]).tindices[1],
						T(group->triangles[i]).vindices[2],
						T(group->triangles[i]).tindices[2]);
				}
				else if (mode & GLM_SMOOTH) {
					fprintf(file, "f %d//%d %d//%d %d//%d\n",
						T(group->triangles[i]).vindices[0],
						T(group->triangles[i]).nindices[0],
						T(group->triangles[i]).vindices[1],
						T(group->triangles[i]).nindices[1],
						T(group->triangles[i]).vindices[2], 
						T(group->triangles[i]).nindices[2]);
				}
				else if (mode & GLM_FLAT) {
					fprintf(file, "f %d//%d %d//%d %d//%d\n",
						T(group->triangles[i]).vindices[0], 
						T(group->triangles[i]).findex,
						T(group->triangles[i]).vindices[1],
						T(group->triangles[i]).findex,
						T(group->triangles[i]).vindices[2],
						T(group->triangles[i]).findex);
				}
				else {
					fprintf(file, "f %d %d %d\n",
						T(group->triangles[i]).vindices[0],
						T(group->triangles[i]).vindices[1],
						T(group->triangles[i]).vindices[2]);
				}
			}
			fprintf(file, "\n");
		}
		group = group->next;
	}

	fclose(file);

	return(1);
}

/* glmScale: Scales a model by a given amount.
 * 
 * model - properly initialized GLMmodel structure
 * scale - scalefactor (0.5 = half as large, 2.0 = twice as large)
 */
GLvoid CObjFile::glmScale(GLfloat scale)
{
  GLuint i;

  for (i = 1; i <= numvertices; i++) {
    vertices[3 * i + 0] *= scale;
    vertices[3 * i + 1] *= scale;
    vertices[3 * i + 2] *= scale;
  }
}

/* vertex rotate about an axis */
void vrotate(GLfloat &x,GLfloat &y,GLfloat ang)
{
	double	r=sqrt(x*x+y*y);
	double	theta=atan2(y,x);
	x=(GLfloat)(r*cos(theta+ang));
	y=(GLfloat)(r*sin(theta+ang));
}

/* glmRotate: Rotates a model (around origin) by a given amount.
 * 
 * model - properly initialized GLMmodel structure
 * rotx,roty,rotz - rotation angle in degrees
 */
GLvoid CObjFile::glmRotate(GLfloat rotx,GLfloat roty,GLfloat rotz)
{
	GLuint i;
	GLfloat	angx,angy,angz;

	while (rotx > 180) rotx-=360;
	while (roty > 180) roty-=360;
	while (rotz > 180) rotz-=360;
	while (rotx <= -180) rotx+=360;
	while (roty <= -180) roty+=360;
	while (rotz <= -180) rotz+=360;

	angx=(GLfloat)(M_PI*rotx/180);
	angy=(GLfloat)(M_PI*roty/180);
	angz=(GLfloat)(M_PI*rotz/180);

	// rotate about x axis
	for (i = 1; i <= numvertices; i++) {
		vrotate(vertices[3 * i + 1],vertices[3 * i + 2],angx);
	}

	// rotate about y axis
	for (i = 1; i <= numvertices; i++) {
		vrotate(vertices[3 * i + 2],vertices[3 * i + 0],angy);
	}

	// rotate about z axis
	for (i = 1; i <= numvertices; i++) {
		vrotate(vertices[3 * i + 0],vertices[3 * i + 1],angz);
	}


}

// find and add texture
GLuint CObjFile::glmFindOrAddTexture(LPCTSTR name,CTextureStore &texstore)
{
	CString fname=name;
	char	*dir;
	CString	filename;

	// trim blanks
	fname.Trim();

	// add it
	dir = glmDirName(pathname);
	filename.Format("%s%s",dir,fname);
	TRACE("Loading texture from '%s'\n",(LPCTSTR)filename);
	free(dir);

	GLuint id=texstore.FindOrAdd(filename);

	return(id);
}

GLvoid CObjFile::glmDimensions(GLfloat *dimensions)
{
	GLuint	i;
	GLfloat	maxx,minx,maxy,miny,maxz,minz;

	maxx=minx=vertices[3+0];
	maxy=miny=vertices[3+1];
	maxz=minz=vertices[3+2];
	for (i=2;i<=numvertices;i++) {
		if (vertices[3*i+0] > maxx) maxx=vertices[3*i+0];
		if (vertices[3*i+1] > maxy) maxy=vertices[3*i+1];
		if (vertices[3*i+2] > maxz) maxz=vertices[3*i+2];
		if (vertices[3*i+0] < minx) minx=vertices[3*i+0];
		if (vertices[3*i+1] < miny) miny=vertices[3*i+1];
		if (vertices[3*i+2] < minz) minz=vertices[3*i+2];
	}
	dimensions[0] = maxx-minx;
	dimensions[1] = maxy-miny;
	dimensions[2] = maxz-minz;
}

GLvoid CObjFile::glmFindPosition()
{
	GLuint	i;
	GLfloat	maxx,minx,maxy,miny,maxz,minz;

	maxx=minx=vertices[3+0];
	maxy=miny=vertices[3+1];
	maxz=minz=vertices[3+2];
	for (i=2;i<=numvertices;i++) {
		if (vertices[3*i+0] > maxx) maxx=vertices[3*i+0];
		if (vertices[3*i+1] > maxy) maxy=vertices[3*i+1];
		if (vertices[3*i+2] > maxz) maxz=vertices[3*i+2];
		if (vertices[3*i+0] < minx) minx=vertices[3*i+0];
		if (vertices[3*i+1] < miny) miny=vertices[3*i+1];
		if (vertices[3*i+2] < minz) minz=vertices[3*i+2];
	}
	// calculate shift required to centre on origin
	m_shift[0] = -(maxx+minx)/2;
	m_shift[1] = -(maxy+miny)/2;
	m_shift[2] = -(maxz+minz)/2;
	TRACE("Model shift x=%g y=%g z=%g\n",m_shift[0],m_shift[1],m_shift[2]);
	// calculate scaling required to fit in two-unit box
	maxx -= minx;
	maxy -= miny;
	maxz -= minz;
	if (maxx>maxy) {
		if (maxx>maxz) 
			m_scale=2.0f/maxx;
		else
			m_scale=2.0f/maxz;
	}
	else {
		if (maxy>maxz)
			m_scale=2.0f/maxy;
		else
			m_scale=2.0f/maxz;
	}
	TRACE("Model scale factor=%g\n",m_scale);
}

GLvoid CObjFile::glmLinearTexture(void)
{
	GLMgroup	*group;
	GLfloat	dimensions[3];
	GLfloat	x,y,scalefactor;
	GLuint	i;

	if (texcoords) delete [] texcoords;
	numtexcoords=numvertices;
	texcoords = new GLfloat[2*(numtexcoords+1)];

	glmDimensions(dimensions);

	scalefactor=(float)(2.0/glmMax(glmMax(dimensions[0],dimensions[1]),dimensions[2]));

	// calculate texture coordinates
	for (i=1;i<=numvertices;i++) {
		x = vertices[3*i+0]*scalefactor;
		y = vertices[3*i+2]*scalefactor;			// why "2" ?????
		texcoords[2*i+0] = (float)((x+1.0)/2.0);
		texcoords[2*i+1] = (float)((y+1.0)/2.0);
	}

	// put texture indices on all the triangles
	group=groups;
	while (group) {
		for (i=0;i<group->numtriangles;i++) {
			triangles[group->triangles[i]].tindices[0]=triangles[group->triangles[i]].vindices[0];
			triangles[group->triangles[i]].tindices[1]=triangles[group->triangles[i]].vindices[1];
			triangles[group->triangles[i]].tindices[2]=triangles[group->triangles[i]].vindices[2];
		}
		group=group->next;
	}
	return;
}

#if 0
GLuint CObjFile::glmLoadTexture(LPCTSTR filename,GLboolean alpha,GLboolean repeat, GLboolean filtering,GLboolean mipmaps, GLfloat *texcoordwidth,GLfloat *texcoordheight)
{
	GLint	gl_max_texture_size;
	GLuint	tex;
	int		width,height,pixelsize;
	int		type;
	int	filter_min, filter_mag;
	GLubyte	*data;
	int		xSize2,ySize2;

	glGetIntegerv(GL_MAX_TEXTURE_SIZE,&gl_max_texture_size);
	TRACE("max texture size=%d\n",gl_max_texture_size);

	CObjTexture	ttt;
	if (!ttt.LoadTGA(filename)) {
		return(0);
	}

	data = ttt.imageData;
	width = ttt.width;
	height = ttt.height;
	type = ttt.type;

	switch (type) {
	case GL_LUMINANCE:
		pixelsize=1;
		break;
	case GL_RGB:
	case GL_BGR_EXT:
		pixelsize=3;
		break;
	case GL_RGBA:
	case GL_BGRA_EXT:
		pixelsize=4;
		break;
	default:
		TRACE("Unknown pixel type %d\n",type);
		pixelsize=0;
	}

	if (((pixelsize*width)%4)==0)
		glPixelStorei(GL_UNPACK_ALIGNMENT,4);
	else
		glPixelStorei(GL_UNPACK_ALIGNMENT,1);

	xSize2 = width;
	ySize2 = height;

#if 0
	/* scale image to power of 2 in height and width */
	xsize2 = 2;
	while (xSize2 < width) xSize2 *= 2;
	ySize2 = 2;
	while (ySize2 < height) ySize2 *= 2;

	if ((width!=xSize2)||(height!=ySize2)) {
		// scale image
		rdata = new GLubyte[xSize2*ySize2*pixelsize];
		retval = gluScaleImage(type,width,height,GL_UNSIGNED_BYTE,data,xSize2,ySize2,GL_UNSIGNED_BYTE,rdata);
		delete [] data;
		data = rdata;
	}
#endif

	// generate the textures
	glGenTextures(1,&tex);
	glBindTexture(GL_TEXTURE_2D,tex);

	if (filtering) {
		filter_min = (mipmaps) ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
		filter_mag = GL_LINEAR;
	}
	else {
		filter_min = (mipmaps) ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST;
		filter_mag = GL_NEAREST;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter_min);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter_mag);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (repeat) ? GL_REPEAT : GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (repeat) ? GL_REPEAT : GL_CLAMP);

	if (mipmaps)
        gluBuild2DMipmaps(GL_TEXTURE_2D, type, xSize2, ySize2, type, GL_UNSIGNED_BYTE, data);
    else
		glTexImage2D(GL_TEXTURE_2D, 0, type, xSize2, ySize2, 0, type, GL_UNSIGNED_BYTE, data);

    *texcoordwidth = (GLfloat)xSize2;        // size of texture coords
    *texcoordheight = (GLfloat)ySize2;

    return tex;

}
#endif
