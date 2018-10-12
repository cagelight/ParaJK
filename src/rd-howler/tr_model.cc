#include "tr_local.hh"

static GLfloat fullscreen_quad_verts [] = {
	-1, -1,  0,
	-1,  1,  0,
	 1, -1,  0,
	 1,  1,  0,
};

static GLfloat unit_quad_verts [] = {
	 0,  0,  0,
	 0,  1,  0,
	 1,  0,  0,
	 1,  1,  0,
};

static GLfloat quad_uvs [] = {
	 0,  0,
	 0,  1,
	 1,  0,
	 1,  1,
};

static qboolean R_LoadMDXM( model_t *mod, void *buffer, const char *mod_name, bool server );
static qboolean R_LoadMDXA( model_t *mod, void *buffer, const char *mod_name );
static qboolean R_LoadMD3 (model_t *mod, int lod, void *buffer, const char *mod_name );

static constexpr byte const FakeGLAFile[] = {
	0x32, 0x4C, 0x47, 0x41, 0x06, 0x00, 0x00, 0x00, 0x2A, 0x64, 0x65, 0x66, 0x61, 0x75, 0x6C, 0x74,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3F, 0x01, 0x00, 0x00, 0x00,
	0x14, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x18, 0x01, 0x00, 0x00, 0x68, 0x00, 0x00, 0x00,
	0x26, 0x01, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x4D, 0x6F, 0x64, 0x56, 0x69, 0x65, 0x77, 0x20,
	0x69, 0x6E, 0x74, 0x65, 0x72, 0x6E, 0x61, 0x6C, 0x20, 0x64, 0x65, 0x66, 0x61, 0x75, 0x6C, 0x74,
	0x00, 0xCD, 0xCD, 0xCD, 0xCD, 0xCD, 0xCD, 0xCD, 0xCD, 0xCD, 0xCD, 0xCD, 0xCD, 0xCD, 0xCD, 0xCD,
	0xCD, 0xCD, 0xCD, 0xCD, 0xCD, 0xCD, 0xCD, 0xCD, 0xCD, 0xCD, 0xCD, 0xCD, 0xCD, 0xCD, 0xCD, 0xCD,
	0xCD, 0xCD, 0xCD, 0xCD, 0xCD, 0xCD, 0xCD, 0xCD, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
	0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFD, 0xBF, 0xFE, 0x7F, 0xFE, 0x7F, 0xFE, 0x7F,
	0x00, 0x80, 0x00, 0x80, 0x00, 0x80
};

modelbank::modelbank() {
	model_t & mod =  models.emplace_back( std::make_shared<basemodel>() )->mod;
	strcpy(mod.name, "BAD MODEL");
	mod.type = MOD_BAD;
	mod.index = 0;
}


modelbank::~modelbank() {
	
}

qhandle_t modelbank::register_model(char const * name, bool server) {
	auto m = model_lookup.find(name);
	if (m != model_lookup.end()) return m->second;
	
	qhandle_t handle = -1;
	
	for (size_t i = 0; i < models.size(); i++) {
		if (!models[i]) {
			handle = i;
		}
	}
	
	if (handle == -1) {
		handle = models.size();
		models.emplace_back(std::make_shared<basemodel>());
	}
	
	basemodel_ptr rmod = models[handle];
	
	if (!Q_stricmp(name, "*default.gla")) {
		strcpy(rmod->mod.name, name);
		rmod->mod.index = handle;
		rmod->mod.dataSize = sizeof(FakeGLAFile);
		R_LoadMDXA(&rmod->mod, (void *)FakeGLAFile, name);
		model_lookup[name] = handle;
		return handle;
	}
	
	fileHandle_t f;
	long len = ri.FS_FOpenFileRead(name, &f, qfalse);
	if (len <= 0) {
		Com_Printf(S_COLOR_RED "ERROR: Failed to open model '%s' for reading!\n", name);
		strcpy(rmod->mod.name, name);
		rmod->mod.index = handle;
		rmod->mod.type = MOD_BAD;
		model_lookup[name] = handle;
		return handle;
	}
	
	rmod->buffer.resize(len);
	
	ri.FS_Read(rmod->buffer.data(), len, f);
	ri.FS_FCloseFile(f);
	
	int ident = *reinterpret_cast<int *>(rmod->buffer.data());
	
	strcpy(rmod->mod.name, name);
	rmod->mod.index = handle;
	rmod->mod.dataSize = len;
	
	switch (ident) {
		case MDXA_IDENT:
			R_LoadMDXA(&rmod->mod, rmod->buffer.data(), name);
			break;
		case MDXM_IDENT:
			R_LoadMDXM(&rmod->mod, rmod->buffer.data(), name, server);
			break;
		case MD3_IDENT:
			R_LoadMD3(&rmod->mod, 0, rmod->buffer.data(), name);
			rmod->mod.type = MOD_MESH;
			break;
		default:
			rmod->mod.type = MOD_BAD;
			break;
	}
	
	if (r && !server) r->model_load(handle);
	model_lookup[name] = handle;
	return handle;
}

basemodel_ptr modelbank::get_model(qhandle_t h) {
	if (h < 0 || h >= models.size()) return nullptr;
	return models[h];
}

void rend::initialize_model() {	
	glCreateVertexArrays(1, &unitquad.vao);
	glCreateBuffers(2, unitquad.vbo);
	glNamedBufferData(unitquad.vbo[0], sizeof(unit_quad_verts), unit_quad_verts, GL_STATIC_DRAW);
	glVertexArrayAttribBinding(unitquad.vao, 0, 0);
	glVertexArrayVertexBuffer(unitquad.vao, 0, unitquad.vbo[0], 0, 12);
	glEnableVertexArrayAttrib(unitquad.vao, 0);
	glVertexArrayAttribFormat(unitquad.vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
	glNamedBufferData(unitquad.vbo[1], sizeof(quad_uvs), quad_uvs, GL_STATIC_DRAW);
	glVertexArrayAttribBinding(unitquad.vao, 1, 1);
	glVertexArrayVertexBuffer(unitquad.vao, 1, unitquad.vbo[1], 0, 8);
	glEnableVertexArrayAttrib(unitquad.vao, 1);
	glVertexArrayAttribFormat(unitquad.vao, 1, 2, GL_FLOAT, GL_FALSE, 0);
	unitquad.size = 4;
	unitquad.mode = GL_TRIANGLE_STRIP;
	
	glCreateVertexArrays(1, &fullquad.vao);
	glCreateBuffers(2, fullquad.vbo);
	glNamedBufferData(fullquad.vbo[0], sizeof(fullscreen_quad_verts), fullscreen_quad_verts, GL_STATIC_DRAW);
	glVertexArrayAttribBinding(fullquad.vao, 0, 0);
	glVertexArrayVertexBuffer(fullquad.vao, 0, fullquad.vbo[0], 0, 12);
	glEnableVertexArrayAttrib(fullquad.vao, 0);
	glVertexArrayAttribFormat(fullquad.vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
	glNamedBufferData(fullquad.vbo[1], sizeof(quad_uvs), quad_uvs, GL_STATIC_DRAW);
	glVertexArrayAttribBinding(fullquad.vao, 1, 1);
	glVertexArrayVertexBuffer(fullquad.vao, 1, fullquad.vbo[1], 0, 8);
	glEnableVertexArrayAttrib(fullquad.vao, 1);
	glVertexArrayAttribFormat(fullquad.vao, 1, 2, GL_FLOAT, GL_FALSE, 0);
	fullquad.size = 4;
	fullquad.mode = GL_TRIANGLE_STRIP;
}

void rend::model_load(qhandle_t h) {
	
	basemodel_ptr mod = mbank->get_model(h);
	if (!mod) return;
	
	if (h >= models.size()) {
		models.resize(h+1);
	}
	if (!models[h]) models[h] = std::make_shared<q3model>();
	q3model & rmod = *models[h];
	rmod.name = mod->mod.name;
	rmod.base = mod;
	
	switch (mod->mod.type) {
		default:
			return;
		case MOD_MDXM: {
			for (int si = 0; si < mod->mod.mdxm->numSurfaces; si++) {
				mdxmSurface_t * surf = (mdxmSurface_t *)ri.G2_FindSurface(&mod->mod, si, 0);
				mdxmHierarchyOffsets_t * surfI = (mdxmHierarchyOffsets_t *)((byte *)mod->mod.mdxm + sizeof(mdxmHeader_t));
				mdxmSurfHierarchy_t * surfH = (mdxmSurfHierarchy_t *)((byte *)surfI + surfI->offsets[surf->thisSurfaceIndex]);
				
				if (surfH->name[0] == '*') continue;
				
				std::vector<float> vert_data;
				std::vector<float> uv_data;
				mdxmVertex_t * verts = (mdxmVertex_t *) ((byte *)surf + surf->ofsVerts);
				mdxmVertexTexCoord_t * uvs = (mdxmVertexTexCoord_t *) &verts[surf->numVerts];
				mdxmTriangle_t * triangles = (mdxmTriangle_t *) ((byte *)surf + surf->ofsTriangles);
				
				for (size_t i = 0; i < surf->numTriangles; i++) {
					for (size_t j = 0; j < 3; j++) {
						auto const & v = verts[triangles[i].indexes[j]];
						vert_data.push_back(v.vertCoords[0]);
						vert_data.push_back(v.vertCoords[2]);
						vert_data.push_back(-v.vertCoords[1]);
						auto const & u = uvs[triangles[i].indexes[j]];
						uv_data.push_back(u.texCoords[0]);
						uv_data.push_back(u.texCoords[1]);
					}
				}
				
				q3mesh & mesh = rmod.meshes.emplace_back();
				mesh.size = vert_data.size() / 3;
				mesh.shader = shader_register(surfH->shader);
				glCreateVertexArrays(1, &mesh.vao);
				glCreateBuffers(2, mesh.vbo);
				glNamedBufferData(mesh.vbo[0], vert_data.size() * 4, vert_data.data(), GL_STATIC_DRAW);
				glVertexArrayAttribBinding(mesh.vao, 0, 0);
				glVertexArrayVertexBuffer(mesh.vao, 0, mesh.vbo[0], 0, 12);
				glEnableVertexArrayAttrib(mesh.vao, 0);
				glVertexArrayAttribFormat(mesh.vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
				glNamedBufferData(mesh.vbo[1], uv_data.size() * 4, uv_data.data(), GL_STATIC_DRAW);
				glVertexArrayAttribBinding(mesh.vao, 1, 1);
				glVertexArrayVertexBuffer(mesh.vao, 1, mesh.vbo[1], 0, 8);
				glEnableVertexArrayAttrib(mesh.vao, 1);
				glVertexArrayAttribFormat(mesh.vao, 1, 2, GL_FLOAT, GL_FALSE, 0);
			}
		} break;
		case MOD_MESH: {
			md3Header_t * header = mod->mod.md3[0];
			md3Surface_t * surf = (md3Surface_t *)( (byte *)header + header->ofsSurfaces );
			for (int s = 0 ; s < header->numSurfaces ; s++) {
				
				std::vector<float> vert_data;
				std::vector<float> uv_data;
				q3mesh & mesh = rmod.meshes.emplace_back();
				
				md3XyzNormal_t * verts = (md3XyzNormal_t *) ((byte *)surf + surf->ofsXyzNormals);
				md3St_t * uvs = (md3St_t *) ((byte *)surf + surf->ofsSt);
				md3Triangle_t * triangles = (md3Triangle_t *) ((byte *)surf + surf->ofsTriangles);
				
				for (size_t i = 0; i < surf->numTriangles; i++) {
					for (size_t j = 0; j < 3; j++) {
						auto const & v = verts[triangles[i].indexes[j]];
						vert_data.push_back(v.xyz[1] / 64.0);
						vert_data.push_back(v.xyz[2] / 64.0);
						vert_data.push_back(v.xyz[0] / 64.0);
						auto const & u = uvs[triangles[i].indexes[j]];
						uv_data.push_back(u.st[0]);
						uv_data.push_back(u.st[1]);
					}
				}
				
				mesh.size = vert_data.size() / 3;
				glCreateVertexArrays(1, &mesh.vao);
				glCreateBuffers(2, mesh.vbo);
				glNamedBufferData(mesh.vbo[0], vert_data.size() * 4, vert_data.data(), GL_STATIC_DRAW);
				glVertexArrayAttribBinding(mesh.vao, 0, 0);
				glVertexArrayVertexBuffer(mesh.vao, 0, mesh.vbo[0], 0, 12);
				glEnableVertexArrayAttrib(mesh.vao, 0);
				glVertexArrayAttribFormat(mesh.vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
				glNamedBufferData(mesh.vbo[1], uv_data.size() * 4, uv_data.data(), GL_STATIC_DRAW);
				glVertexArrayAttribBinding(mesh.vao, 1, 1);
				glVertexArrayVertexBuffer(mesh.vao, 1, mesh.vbo[1], 0, 8);
				glEnableVertexArrayAttrib(mesh.vao, 1);
				glVertexArrayAttribFormat(mesh.vao, 1, 2, GL_FLOAT, GL_FALSE, 0);
				
				md3Shader_t * shader = (md3Shader_t *) ( (byte *)surf + surf->ofsShaders );
				mesh.shader = shader_register(shader->name);
				
				surf = (md3Surface_t *)( (byte *)surf + surf->ofsEnd );
			}
		} break;
	}
}

q3mesh::q3mesh(q3mesh && other) {
	vao = other.vao;
	other.vao = 0;
	memcpy(vbo, other.vbo, sizeof(vbo));
	memset(other.vbo, 0, sizeof(vbo));
	shader = other.shader;
	size = other.size;
	mode = other.mode;
}

q3mesh::~q3mesh() {
	if (vbo[0]) glDeleteBuffers(3, vbo);
	if (vao) glDeleteVertexArrays(1, &vao);
}

static int OldToNewRemapTable[72] = {
0,// Bone 0:   "model_root":           Parent: ""  (index -1)
1,// Bone 1:   "pelvis":               Parent: "model_root"  (index 0)
2,// Bone 2:   "Motion":               Parent: "pelvis"  (index 1)
3,// Bone 3:   "lfemurYZ":             Parent: "pelvis"  (index 1)
4,// Bone 4:   "lfemurX":              Parent: "pelvis"  (index 1)
5,// Bone 5:   "ltibia":               Parent: "pelvis"  (index 1)
6,// Bone 6:   "ltalus":               Parent: "pelvis"  (index 1)
6,// Bone 7:   "ltarsal":              Parent: "pelvis"  (index 1)
7,// Bone 8:   "rfemurYZ":             Parent: "pelvis"  (index 1)
8,// Bone 9:   "rfemurX":	            Parent: "pelvis"  (index 1)
9,// Bone10:   "rtibia":	            Parent: "pelvis"  (index 1)
10,// Bone11:   "rtalus":	            Parent: "pelvis"  (index 1)
10,// Bone12:   "rtarsal":              Parent: "pelvis"  (index 1)
11,// Bone13:   "lower_lumbar":         Parent: "pelvis"  (index 1)
12,// Bone14:   "upper_lumbar":	        Parent: "lower_lumbar"  (index 13)
13,// Bone15:   "thoracic":	            Parent: "upper_lumbar"  (index 14)
14,// Bone16:   "cervical":	            Parent: "thoracic"  (index 15)
15,// Bone17:   "cranium":              Parent: "cervical"  (index 16)
16,// Bone18:   "ceyebrow":	            Parent: "face_always_"  (index 71)
17,// Bone19:   "jaw":                  Parent: "face_always_"  (index 71)
18,// Bone20:   "lblip2":	            Parent: "face_always_"  (index 71)
19,// Bone21:   "leye":		            Parent: "face_always_"  (index 71)
20,// Bone22:   "rblip2":	            Parent: "face_always_"  (index 71)
21,// Bone23:   "ltlip2":               Parent: "face_always_"  (index 71)
22,// Bone24:   "rtlip2":	            Parent: "face_always_"  (index 71)
23,// Bone25:   "reye":		            Parent: "face_always_"  (index 71)
24,// Bone26:   "rclavical":            Parent: "thoracic"  (index 15)
25,// Bone27:   "rhumerus":             Parent: "thoracic"  (index 15)
26,// Bone28:   "rhumerusX":            Parent: "thoracic"  (index 15)
27,// Bone29:   "rradius":              Parent: "thoracic"  (index 15)
28,// Bone30:   "rradiusX":             Parent: "thoracic"  (index 15)
29,// Bone31:   "rhand":                Parent: "thoracic"  (index 15)
29,// Bone32:   "mc7":                  Parent: "thoracic"  (index 15)
34,// Bone33:   "r_d5_j1":              Parent: "thoracic"  (index 15)
35,// Bone34:   "r_d5_j2":              Parent: "thoracic"  (index 15)
35,// Bone35:   "r_d5_j3":              Parent: "thoracic"  (index 15)
30,// Bone36:   "r_d1_j1":              Parent: "thoracic"  (index 15)
31,// Bone37:   "r_d1_j2":              Parent: "thoracic"  (index 15)
31,// Bone38:   "r_d1_j3":              Parent: "thoracic"  (index 15)
32,// Bone39:   "r_d2_j1":              Parent: "thoracic"  (index 15)
33,// Bone40:   "r_d2_j2":              Parent: "thoracic"  (index 15)
33,// Bone41:   "r_d2_j3":              Parent: "thoracic"  (index 15)
32,// Bone42:   "r_d3_j1":			    Parent: "thoracic"  (index 15)
33,// Bone43:   "r_d3_j2":		        Parent: "thoracic"  (index 15)
33,// Bone44:   "r_d3_j3":              Parent: "thoracic"  (index 15)
34,// Bone45:   "r_d4_j1":              Parent: "thoracic"  (index 15)
35,// Bone46:   "r_d4_j2":	            Parent: "thoracic"  (index 15)
35,// Bone47:   "r_d4_j3":		        Parent: "thoracic"  (index 15)
36,// Bone48:   "rhang_tag_bone":	    Parent: "thoracic"  (index 15)
37,// Bone49:   "lclavical":            Parent: "thoracic"  (index 15)
38,// Bone50:   "lhumerus":	            Parent: "thoracic"  (index 15)
39,// Bone51:   "lhumerusX":	        Parent: "thoracic"  (index 15)
40,// Bone52:   "lradius":	            Parent: "thoracic"  (index 15)
41,// Bone53:   "lradiusX":	            Parent: "thoracic"  (index 15)
42,// Bone54:   "lhand":	            Parent: "thoracic"  (index 15)
42,// Bone55:   "mc5":		            Parent: "thoracic"  (index 15)
43,// Bone56:   "l_d5_j1":	            Parent: "thoracic"  (index 15)
44,// Bone57:   "l_d5_j2":	            Parent: "thoracic"  (index 15)
44,// Bone58:   "l_d5_j3":	            Parent: "thoracic"  (index 15)
43,// Bone59:   "l_d4_j1":	            Parent: "thoracic"  (index 15)
44,// Bone60:   "l_d4_j2":	            Parent: "thoracic"  (index 15)
44,// Bone61:   "l_d4_j3":	            Parent: "thoracic"  (index 15)
45,// Bone62:   "l_d3_j1":	            Parent: "thoracic"  (index 15)
46,// Bone63:   "l_d3_j2":	            Parent: "thoracic"  (index 15)
46,// Bone64:   "l_d3_j3":	            Parent: "thoracic"  (index 15)
45,// Bone65:   "l_d2_j1":	            Parent: "thoracic"  (index 15)
46,// Bone66:   "l_d2_j2":	            Parent: "thoracic"  (index 15)
46,// Bone67:   "l_d2_j3":	            Parent: "thoracic"  (index 15)
47,// Bone68:   "l_d1_j1":				Parent: "thoracic"  (index 15)
48,// Bone69:   "l_d1_j2":	            Parent: "thoracic"  (index 15)
48,// Bone70:   "l_d1_j3":				Parent: "thoracic"  (index 15)
52// Bone71:   "face_always_":			Parent: "cranium"  (index 17)
};

static qboolean R_LoadMDXM( model_t *mod, void *buffer, const char *mod_name, bool server ) {
	int					i,l, j;
	mdxmHeader_t		*pinmodel, *mdxm;
	mdxmLOD_t			*lod;
	mdxmSurface_t		*surf;
	int					version;
	int					size;
	mdxmSurfHierarchy_t	*surfInfo;

	pinmodel= (mdxmHeader_t *)buffer;
	//
	// read some fields from the binary, but only LittleLong() them when we know this wasn't an already-cached model...
	//
	version = (pinmodel->version);
	size	= (pinmodel->ofsEnd);

	if (version != MDXM_VERSION) {
		ri.Printf( PRINT_ALL, S_COLOR_YELLOW  "R_LoadMDXM: %s has wrong version (%i should be %i)\n",
				 mod_name, version, MDXM_VERSION);
		return qfalse;
	}

	mod->type	   = MOD_MDXM;
	mod->dataSize += size;

	mdxm = mod->mdxm = (mdxmHeader_t*) pinmodel;

	// first up, go load in the animation file we need that has the skeletal animation info for this model
	mdxm->animIndex = mbank->register_model(va ("%s.gla",mdxm->animName));

	if (!mdxm->animIndex)
	{
		ri.Printf( PRINT_ALL, S_COLOR_YELLOW  "R_LoadMDXM: missing animation file %s for mesh %s\n", mdxm->animName, mdxm->name);
		return qfalse;
	}

	mod->numLods = mdxm->numLODs -1 ;	//copy this up to the model for ease of use - it wil get inced after this.

	bool isAnOldModelFile = false;
	if (mdxm->numBones == 72 && strstr(mdxm->animName,"_humanoid") )
	{
		isAnOldModelFile = true;
	}

	surfInfo = (mdxmSurfHierarchy_t *)( (byte *)mdxm + mdxm->ofsSurfHierarchy);
 	for ( i = 0 ; i < mdxm->numSurfaces ; i++)
	{
		Q_strlwr(surfInfo->name);	//just in case
		if ( !strcmp( &surfInfo->name[strlen(surfInfo->name)-4],"_off") )
		{
			surfInfo->name[strlen(surfInfo->name)-4]=0;	//remove "_off" from name
		}
		
		if (!server && surfInfo->shader[0]) surfInfo->shaderIndex = r->shader_register(surfInfo->shader)->index;

		/*
		shader_t	*sh;
		// get the shader name
		sh = R_FindShader( surfInfo->shader, lightmapsNone, stylesDefault, qtrue );
		// insert it in the surface list
		if ( sh->defaultShader )
		{
			surfInfo->shaderIndex = 0;
		}
		else
		{
			surfInfo->shaderIndex = sh->index;
		}

		RE_RegisterModels_StoreShaderRequest(mod_name, &surfInfo->shader[0], &surfInfo->shaderIndex);
		*/

		// find the next surface
		surfInfo = (mdxmSurfHierarchy_t *)( (byte *)surfInfo + (size_t)( &((mdxmSurfHierarchy_t *)0)->childIndexes[ surfInfo->numChildren ] ));
  	}

	// swap all the LOD's	(we need to do the middle part of this even for intel, because of shader reg and err-check)
	lod = (mdxmLOD_t *) ( (byte *)mdxm + mdxm->ofsLODs );
	for ( l = 0 ; l < mdxm->numLODs ; l++)
	{
		int	triCount = 0;
		// swap all the surfaces
		surf = (mdxmSurface_t *) ( (byte *)lod + sizeof (mdxmLOD_t) + (mdxm->numSurfaces * sizeof(mdxmLODSurfOffset_t)) );
		for ( i = 0 ; i < mdxm->numSurfaces ; i++)
		{

			triCount += surf->numTriangles;

			if ( surf->numVerts > SHADER_MAX_VERTEXES ) {
				Com_Error (ERR_DROP, "R_LoadMDXM: %s has more than %i verts on a surface (%i)",
					mod_name, SHADER_MAX_VERTEXES, surf->numVerts );
			}
			if ( surf->numTriangles*3 > SHADER_MAX_INDEXES ) {
				Com_Error (ERR_DROP, "R_LoadMDXM: %s has more than %i triangles on a surface (%i)",
					mod_name, SHADER_MAX_INDEXES / 3, surf->numTriangles );
			}

			// change to surface identifier
			surf->ident = SF_MDX;

			if (isAnOldModelFile)
			{
				int *boneRef = (int *) ( (byte *)surf + surf->ofsBoneReferences );
				for ( j = 0 ; j < surf->numBoneReferences ; j++ )
				{
					assert(boneRef[j] >= 0 && boneRef[j] < 72);
					if (boneRef[j] >= 0 && boneRef[j] < 72)
					{
						boneRef[j]=OldToNewRemapTable[boneRef[j]];
					}
					else
					{
			surf->ident = SF_MDX;
						boneRef[j]=0;
					}
				}
			}
			// find the next surface
			surf = (mdxmSurface_t *)( (byte *)surf + surf->ofsEnd );
		}
		// find the next LOD
		lod = (mdxmLOD_t *)( (byte *)lod + lod->ofsEnd );
	}
	return qtrue;
}

static qboolean R_LoadMDXA( model_t *mod, void *buffer, const char *mod_name ) {

	mdxaHeader_t		*pinmodel, *mdxa;
	int					version;
	int					size;
	
 	pinmodel = (mdxaHeader_t *)buffer;
	//
	// read some fields from the binary, but only LittleLong() them when we know this wasn't an already-cached model...
	//
	version = (pinmodel->version);
	size	= (pinmodel->ofsEnd);

	if (version != MDXA_VERSION) {
		ri.Printf( PRINT_ALL, S_COLOR_YELLOW  "R_LoadMDXA: %s has wrong version (%i should be %i)\n",
				 mod_name, version, MDXA_VERSION);
		return qfalse;
	}

	mod->type = MOD_MDXA;
	mod->dataSize  += size;

	mdxa = mod->mdxa = pinmodel;

 	if ( mdxa->numFrames < 1 ) {
		ri.Printf( PRINT_ALL, S_COLOR_YELLOW  "R_LoadMDXA: %s has no frames\n", mod_name );
		return qfalse;
	}
	
	return qtrue;
}

static qboolean R_LoadMD3 (model_t *mod, int lod, void *buffer, const char *mod_name ) {
	int					i, j;
	md3Header_t			*pinmodel;
	md3Surface_t		*surf;
	int					version;
	int					size;

	pinmodel= (md3Header_t *)buffer;
	version = pinmodel->version;
	size	= pinmodel->ofsEnd;

	if (version != MD3_VERSION) {
		ri.Printf( PRINT_ALL, S_COLOR_YELLOW  "R_LoadMD3: %s has wrong version (%i should be %i)\n",
				 mod_name, version, MD3_VERSION);
		return qfalse;
	}

	mod->type      = MOD_MESH;
	mod->dataSize += size;
	
	mod->md3[lod] = (md3Header_t *)pinmodel;

	if ( mod->md3[lod]->numFrames < 1 ) {
		ri.Printf( PRINT_ALL, S_COLOR_YELLOW  "R_LoadMD3: %s has no frames\n", mod_name );
		return qfalse;
	}

	// swap all the surfaces
	surf = (md3Surface_t *) ( (byte *)mod->md3[lod] + mod->md3[lod]->ofsSurfaces );
	for ( i = 0 ; i < mod->md3[lod]->numSurfaces ; i++) {

		if ( surf->numVerts >= SHADER_MAX_VERTEXES ) {
			Com_Error (ERR_DROP, "R_LoadMD3: %s has more than %i verts on %s (%i)",
				mod_name, SHADER_MAX_VERTEXES - 1, surf->name[0] ? surf->name : "a surface", surf->numVerts );
		}
		if ( surf->numTriangles*3 >= SHADER_MAX_INDEXES ) {
			Com_Error (ERR_DROP, "R_LoadMD3: %s has more than %i triangles on %s (%i)",
				mod_name, ( SHADER_MAX_INDEXES / 3 ) - 1, surf->name[0] ? surf->name : "a surface", surf->numTriangles );
		}

		// change to surface identifier
		surf->ident = SF_MD3;

		// lowercase the surface name so skin compares are faster
		Q_strlwr( surf->name );

		// strip off a trailing _1 or _2
		// this is a crutch for q3data being a mess
		j = strlen( surf->name );
		if ( j > 2 && surf->name[j-2] == '_' ) {
			surf->name[j-2] = 0;
		}
        // register the shaders
		md3Shader_t		*shader;
        shader = (md3Shader_t *) ( (byte *)surf + surf->ofsShaders );
        for ( j = 0 ; j < surf->numShaders ; j++, shader++ ) {
			shader->shaderIndex = r->shader_register(shader->name, true)->index;
			/*
            shader_t	*sh;

            sh = R_FindShader( shader->name, lightmapsNone, stylesDefault, qtrue );
			if ( sh->defaultShader ) {
				shader->shaderIndex = 0;
			} else {
				shader->shaderIndex = sh->index;
			}
			RE_RegisterModels_StoreShaderRequest(mod_name, &shader->name[0], &shader->shaderIndex);
			*/
        }

		// find the next surface
		surf = (md3Surface_t *)( (byte *)surf + surf->ofsEnd );
	}

	return qtrue;
}
