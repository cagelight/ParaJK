#include "hw_local.hh"
using namespace howler;

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

instance::model_registry::model_registry() {
	model_t & mod =  models.emplace_back(make_q3basemodel())->base;
	strcpy(mod.name, "BAD MODEL");
	mod.type = MOD_BAD;
	mod.index = 0;
}

q3basemodel_ptr instance::model_registry::reg(char const * name, bool server) {
	auto m = lookup.find(name);
	if (m != lookup.end()) return m->second;
	
	q3basemodel_ptr mod = models.emplace_back(make_q3basemodel());
	mod->base.index = models.size() - 1;
	strcpy(mod->base.name, name);
	lookup[name] = mod;
	
	if (!Q_stricmp(name, "*default.gla")) {
		mod->buffer.resize(mod->base.dataSize = sizeof(FakeGLAFile));
		memcpy(mod->buffer.data(), FakeGLAFile, sizeof(FakeGLAFile));
		mod->load();
		assert(mod->base.type == MOD_MDXA);
		return mod;
	}
	
	FS_Reader fs {name};
	if (!fs.valid()) {
		return mod;
	}
	
	mod->buffer.resize(fs.size());
	memcpy(mod->buffer.data(), fs.data(), fs.size());
	mod->load();
	
	if (mod->base.type == MOD_BAD) return mod;
	if (mod->base.type == MOD_MDXA) return mod;
	
	if (!server) {
		if (!hw_inst->renderer_initialized)
			waiting_models.push_back(mod);
		else
			mod->setup_render();
	}
	
	return mod;
}

void instance::model_registry::reg(q3basemodel_ptr ptr) {
	ptr->base.index = models.size();
	models.push_back(ptr);
	lookup[ptr->base.name] = ptr;
}

q3basemodel_ptr instance::model_registry::get(qhandle_t h) {
	if (h < 0 || h >= static_cast<int32_t>(models.size())) return nullptr;
	return models[h];
}

void instance::model_registry::process_waiting() {
	for (q3basemodel_ptr & model : waiting_models)
		model->setup_render();
	waiting_models.clear();
}

void q3basemodel::load() {
	uint32_t ident = reinterpret_cast<uint32_t *>(buffer.data())[0];
	switch(ident) {
		case MDXA_IDENT: load_mdxa(); break;
		case MDXM_IDENT: load_mdxm(); break;
		case MD3_IDENT: load_md3(0); break;
		default:
			base.type = MOD_BAD;
	}
}

void q3basemodel::setup_render() {
	switch (base.type) {
		default: break;
		case MOD_MESH:
			setup_render_md3();
			break;
		case MOD_MDXM:
			setup_render_mdxm();
			break;
		case MOD_OBJ:
			setup_render_obj();
			break;
	}
}

//================================================================
// MD3
//================================================================

struct q3md3mesh : public q3mesh_basic {
	struct vertex_t {
		qm::vec3_t vert;
		qm::vec2_t uv;
		qm::vec3_t normal;
	};
	
	q3md3mesh(vertex_t const * data, size_t num) : q3mesh_basic(mode::triangles) {
		
		static constexpr uint_fast16_t offsetof_verts = 0;
		static constexpr uint_fast16_t sizeof_verts = sizeof(vertex_t::vert);
		static constexpr uint_fast16_t offsetof_uv = offsetof_verts + sizeof_verts;
		static constexpr uint_fast16_t sizeof_uv = sizeof(vertex_t::uv);
		static constexpr uint_fast16_t offsetof_normal = offsetof_uv + sizeof_uv;
		static constexpr uint_fast16_t sizeof_normal = sizeof(vertex_t::normal);
		static constexpr uint_fast16_t sizeof_all = offsetof_normal + sizeof_normal;
		static_assert(sizeof_all == sizeof(vertex_t));
		
		m_size = num;
		glCreateBuffers(1, &m_vbo);
		glNamedBufferData(m_vbo, num * sizeof_all, data, GL_STATIC_DRAW);
		
		glVertexArrayVertexBuffer(m_handle, 0, m_vbo, 0, sizeof_all);
		
		glEnableVertexArrayAttrib(m_handle, LAYOUT_VERTEX);
		glEnableVertexArrayAttrib(m_handle, LAYOUT_UV);
		glEnableVertexArrayAttrib(m_handle, LAYOUT_NORMAL);
		glVertexArrayAttribBinding(m_handle, LAYOUT_VERTEX, 0);
		glVertexArrayAttribBinding(m_handle, LAYOUT_UV, 0);
		glVertexArrayAttribBinding(m_handle, LAYOUT_NORMAL, 0);
		
		glVertexArrayAttribFormat(m_handle, LAYOUT_VERTEX, 3, GL_FLOAT, GL_FALSE, offsetof_verts);
		glVertexArrayAttribFormat(m_handle, LAYOUT_UV, 2, GL_FLOAT, GL_FALSE, offsetof_uv);
		glVertexArrayAttribFormat(m_handle, LAYOUT_NORMAL, 3, GL_FLOAT, GL_FALSE, offsetof_normal);
		
	}
	
	~q3md3mesh() {
		glDeleteBuffers(1, &m_vbo);
	}
private:
	GLuint m_vbo;
};

void q3basemodel::setup_render_md3() {
	model = make_q3model();
	
	md3Header_t * header = base.md3[0];
	
	md3Surface_t * surf = (md3Surface_t *)( (byte *)header + header->ofsSurfaces );
	for (int32_t s = 0; s < header->numSurfaces; s++) {
		
		md3XyzNormal_t * verts = (md3XyzNormal_t *) ((byte *)surf + surf->ofsXyzNormals);
		md3St_t * uvs = (md3St_t *) ((byte *)surf + surf->ofsSt);
		md3Triangle_t * triangles = (md3Triangle_t *) ((byte *)surf + surf->ofsTriangles);
		
		std::vector<q3md3mesh::vertex_t> vert_data;
		
		for (int32_t i = 0; i < surf->numTriangles; i++) {
			for (size_t j = 0; j < 3; j++) {
				auto const & v = verts[triangles[i].indexes[j]];
				auto const & u = uvs[triangles[i].indexes[j]];
				
				float lat = ((v.normal >> 8) & 0xFF) * (2 * qm::pi) / 255.0f;
				float lng = (v.normal & 0xFF) * (2 * qm::pi) / 255.0f;
				
				vert_data.emplace_back( q3md3mesh::vertex_t {
					qm::vec3_t { (float)v.xyz[1], (float)v.xyz[2], (float)v.xyz[0] } / 64.0,
					qm::vec2_t { u.st[0], u.st[1] },
					qm::vec3_t { -std::sin(lat) * std::sin(lng), -std::cos(lng), -std::cos(lat) * std::sin(lng) }.normalized(),
				});
			}
		}
		
		std::shared_ptr<q3md3mesh> mesh = std::make_shared<q3md3mesh>(vert_data.data(), vert_data.size());
		
		assert(surf->numShaders == 1); // how can there ever be more than 1? that makes no sense...
		md3Shader_t * shader = (md3Shader_t *) ( (byte *)surf + surf->ofsShaders );
		model->meshes.emplace_back( hw_inst->shaders.get(shader->shaderIndex), mesh );
		
		surf = (md3Surface_t *)( (byte *)surf + surf->ofsEnd );
	}
}

//================================================================
// MDXM
//================================================================

struct mdxm_animated_mesh : public q3mesh_basic {
	
	struct vertex_t {
		qm::vec3_t vert;
		qm::vec2_t uv;
		qm::vec3_t normal;
		std::array<byte, 4> bone_groups;
		qm::vec4_t bone_weights;
	};
	
	mdxm_animated_mesh(vertex_t const * data, size_t num) : q3mesh_basic(mode::triangles) {
		
		static constexpr uint_fast16_t offsetof_verts = 0;
		static constexpr uint_fast16_t sizeof_verts = sizeof(vertex_t::vert);
		static constexpr uint_fast16_t offsetof_uv = offsetof_verts + sizeof_verts;
		static constexpr uint_fast16_t sizeof_uv = sizeof(vertex_t::uv);
		static constexpr uint_fast16_t offsetof_normal = offsetof_uv + sizeof_uv;
		static constexpr uint_fast16_t sizeof_normal = sizeof(vertex_t::normal);
		static constexpr uint_fast16_t offsetof_boneg = offsetof_normal + sizeof_normal;
		static constexpr uint_fast16_t sizeof_boneg = sizeof(vertex_t::bone_groups);
		static constexpr uint_fast16_t offsetof_bonew = offsetof_boneg + sizeof_boneg;
		static constexpr uint_fast16_t sizeof_bonew = sizeof(vertex_t::bone_weights);
		static constexpr uint_fast16_t sizeof_all = offsetof_bonew + sizeof_bonew;
		static_assert(sizeof_all == sizeof(vertex_t));
		
		m_size = num;
		glCreateBuffers(1, &m_vbo);
		glNamedBufferData(m_vbo, num * sizeof_all, data, GL_STATIC_DRAW);
		
		glVertexArrayVertexBuffer(m_handle, 0, m_vbo, 0, sizeof_all);
		
		glGetError();
		
		glEnableVertexArrayAttrib(m_handle, LAYOUT_VERTEX);
		glEnableVertexArrayAttrib(m_handle, LAYOUT_UV);
		glEnableVertexArrayAttrib(m_handle, LAYOUT_NORMAL);
		glEnableVertexArrayAttrib(m_handle, LAYOUT_BONE_GROUPS);
		glEnableVertexArrayAttrib(m_handle, LAYOUT_BONE_WEIGHT);
		
		glVertexArrayAttribBinding(m_handle, LAYOUT_VERTEX, 0);
		glVertexArrayAttribBinding(m_handle, LAYOUT_UV, 0);
		glVertexArrayAttribBinding(m_handle, LAYOUT_NORMAL, 0);
		glVertexArrayAttribBinding(m_handle, LAYOUT_BONE_GROUPS, 0);
		glVertexArrayAttribBinding(m_handle, LAYOUT_BONE_WEIGHT, 0);
		
		glVertexArrayAttribFormat(m_handle, LAYOUT_VERTEX, 3, GL_FLOAT, GL_FALSE, offsetof_verts);
		glVertexArrayAttribFormat(m_handle, LAYOUT_UV, 2, GL_FLOAT, GL_FALSE, offsetof_uv);
		glVertexArrayAttribFormat(m_handle, LAYOUT_NORMAL, 3, GL_FLOAT, GL_FALSE, offsetof_normal);
		glVertexArrayAttribIFormat(m_handle, LAYOUT_BONE_GROUPS, 4, GL_UNSIGNED_BYTE, offsetof_boneg);
		glVertexArrayAttribFormat(m_handle, LAYOUT_BONE_WEIGHT, 4, GL_FLOAT, GL_FALSE, offsetof_bonew);
		
		if (glGetError() != GL_NO_ERROR) {
			Com_Error(ERR_FATAL, "OpenGL error in mdxm_animated_mesh");
		}
	}
	
	~mdxm_animated_mesh() {
		glDeleteBuffers(1, &m_vbo);
	}
private:
	GLuint m_vbo;
};

void q3basemodel::setup_render_mdxm() {
	model = make_q3model();
	
	for (int32_t i = 0; i < base.mdxm->numSurfaces; i++) {
		mdxmSurface_t * surf = (mdxmSurface_t *)ri.G2_FindSurface(&base, i, 0);
		mdxmHierarchyOffsets_t * surfI = (mdxmHierarchyOffsets_t *)((byte *)base.mdxm + sizeof(mdxmHeader_t));
		mdxmSurfHierarchy_t * surfH = (mdxmSurfHierarchy_t *)((byte *)surfI + surfI->offsets[surf->thisSurfaceIndex]);
		if (surfH->name[0] == '*') {
			model->meshes.emplace_back(nullptr, nullptr);
			continue;
		}
		
		mdxmVertex_t * verts = (mdxmVertex_t *) ((byte *)surf + surf->ofsVerts);
		mdxmVertexTexCoord_t * uvs = (mdxmVertexTexCoord_t *) &verts[surf->numVerts];
		mdxmTriangle_t * triangles = (mdxmTriangle_t *) ((byte *)surf + surf->ofsTriangles);
		
		std::vector<mdxm_animated_mesh::vertex_t> verticies;
		for (int32_t i = 0; i < surf->numTriangles; i++) {
			for (size_t j = 0; j < 3; j++) {
				
				mdxmVertex_t const & vtx = verts[triangles[i].indexes[j]];
				mdxmVertexTexCoord_t const & uv = uvs[triangles[i].indexes[j]];
				
				uint32_t pack = vtx.uiNmWeightsAndBoneIndexes;
				uint32_t num_groups = (pack >> 30) + 1; 
				assert(num_groups <= 4 && num_groups >= 1);
				
				std::array<byte, 4> bone_idx;
				qm::vec4_t bone_wgt;
				
				for (uint32_t i = 0; i < 4; i++)  {
					if (i < num_groups) {
						bone_idx[i] = (pack >> (iG2_BITS_PER_BONEREF * i)) & ((1 << iG2_BITS_PER_BONEREF) - 1);
						assert(bone_idx[i] < 72);
						bone_wgt[i] = (vtx.BoneWeightings[i] | ((pack >> (iG2_BONEWEIGHT_TOPBITS_SHIFT + i * 2)) & iG2_BONEWEIGHT_TOPBITS_AND)) * fG2_BONEWEIGHT_RECIPROCAL_MULT;
					} else {
						bone_idx[i] = 0xFF;
						bone_wgt[i] = 0;
					}
				}
				
				float wgt_sum = 0;
				for (uint32_t i = 0; i < num_groups; i++)
					wgt_sum += bone_wgt[i];
				if (wgt_sum)
					bone_wgt /= wgt_sum;
				else
					bone_wgt = {1, 1, 1, 1};
				
				verticies.emplace_back( mdxm_animated_mesh::vertex_t {
					qm::vec3_t { vtx.vertCoords[1], -vtx.vertCoords[2], vtx.vertCoords[0], },
					qm::vec2_t { uv.texCoords[0], uv.texCoords[1] },
					qm::vec3_t { vtx.normal[1], -vtx.normal[2], vtx.normal[0], } .normalized(),
					bone_idx,
					bone_wgt
				});
			}
		}
		model->meshes.emplace_back( hw_inst->shaders.get(surfH->shaderIndex), std::make_shared<mdxm_animated_mesh>(verticies.data(), verticies.size()) );
	}
}

//================================================================
// OBJ
//================================================================

struct q3objmesh : public q3mesh_basic {
	struct vertex_t {
		qm::vec3_t vert;
		qm::vec2_t uv;
		qm::vec3_t normal;
	};
	
	q3objmesh(vertex_t const * data, size_t num) : q3mesh_basic(mode::triangles) {
		
		static constexpr uint_fast16_t offsetof_verts = 0;
		static constexpr uint_fast16_t sizeof_verts = sizeof(vertex_t::vert);
		static constexpr uint_fast16_t offsetof_uv = offsetof_verts + sizeof_verts;
		static constexpr uint_fast16_t sizeof_uv = sizeof(vertex_t::uv);
		static constexpr uint_fast16_t offsetof_normal = offsetof_uv + sizeof_uv;
		static constexpr uint_fast16_t sizeof_normal = sizeof(vertex_t::normal);
		static constexpr uint_fast16_t sizeof_all = offsetof_normal + sizeof_normal;
		static_assert(sizeof_all == sizeof(vertex_t));
		
		m_size = num;
		glCreateBuffers(1, &m_vbo);
		glNamedBufferData(m_vbo, num * sizeof_all, data, GL_STATIC_DRAW);
		
		glVertexArrayVertexBuffer(m_handle, 0, m_vbo, 0, sizeof_all);
		
		glEnableVertexArrayAttrib(m_handle, LAYOUT_VERTEX);
		glEnableVertexArrayAttrib(m_handle, LAYOUT_UV);
		glEnableVertexArrayAttrib(m_handle, LAYOUT_NORMAL);
		glVertexArrayAttribBinding(m_handle, LAYOUT_VERTEX, 0);
		glVertexArrayAttribBinding(m_handle, LAYOUT_UV, 0);
		glVertexArrayAttribBinding(m_handle, LAYOUT_NORMAL, 0);
		
		glVertexArrayAttribFormat(m_handle, LAYOUT_VERTEX, 3, GL_FLOAT, GL_FALSE, offsetof_verts);
		glVertexArrayAttribFormat(m_handle, LAYOUT_UV, 2, GL_FLOAT, GL_FALSE, offsetof_uv);
		glVertexArrayAttribFormat(m_handle, LAYOUT_NORMAL, 3, GL_FLOAT, GL_FALSE, offsetof_normal);
		
	}
	
	~q3objmesh() {
		glDeleteBuffers(1, &m_vbo);
	}
private:
	GLuint m_vbo;
};

void q3basemodel::setup_render_obj() {
	model = make_q3model();
	
	for (int32_t i = 0; i < base.obj->numSurfaces; i++) {
		objSurface_t const & surf = base.obj->surfaces[i];
	}
}

/*
void q3basemodel::setup_render_md3() {
	model = make_q3model();
	
	md3Header_t * header = base.md3[0];
	
	md3Surface_t * surf = (md3Surface_t *)( (byte *)header + header->ofsSurfaces );
	for (int32_t s = 0; s < header->numSurfaces; s++) {
		
		md3XyzNormal_t * verts = (md3XyzNormal_t *) ((byte *)surf + surf->ofsXyzNormals);
		md3St_t * uvs = (md3St_t *) ((byte *)surf + surf->ofsSt);
		md3Triangle_t * triangles = (md3Triangle_t *) ((byte *)surf + surf->ofsTriangles);
		
		std::vector<q3md3mesh::vertex_t> vert_data;
		
		for (int32_t i = 0; i < surf->numTriangles; i++) {
			for (size_t j = 0; j < 3; j++) {
				auto const & v = verts[triangles[i].indexes[j]];
				auto const & u = uvs[triangles[i].indexes[j]];
				
				float lat = ((v.normal >> 8) & 0xFF) * (2 * qm::pi) / 255.0f;
				float lng = (v.normal & 0xFF) * (2 * qm::pi) / 255.0f;
				
				vert_data.emplace_back( q3md3mesh::vertex_t {
					qm::vec3_t { (float)v.xyz[1], (float)v.xyz[2], (float)v.xyz[0] } / 64.0,
					qm::vec2_t { u.st[0], u.st[1] },
					qm::vec3_t { -std::sin(lat) * std::sin(lng), -std::cos(lng), -std::cos(lat) * std::sin(lng) }.normalized(),
				});
			}
		}
		
		std::shared_ptr<q3md3mesh> mesh = std::make_shared<q3md3mesh>(vert_data.data(), vert_data.size());
		
		assert(surf->numShaders == 1); // how can there ever be more than 1? that makes no sense...
		md3Shader_t * shader = (md3Shader_t *) ( (byte *)surf + surf->ofsShaders );
		model->meshes.emplace_back( hw_inst->shaders.get(shader->shaderIndex), mesh );
		
		surf = (md3Surface_t *)( (byte *)surf + surf->ofsEnd );
	}
}
*/

//================================================================

void q3basemodel::load_mdxa() {
	mdxaHeader_t * header = (mdxaHeader_t *)buffer.data();
	if (header->version != MDXA_VERSION)
		Com_Error(ERR_FATAL, "q3basemodel::load_mdxa: \"%s\" is wrong MDXA version", base.name);
	
	base.type = MOD_MDXA;
	base.mdxa = header;
	
	if (base.mdxa->numFrames < 1)
		ri.Printf( PRINT_ALL, S_COLOR_YELLOW  "q3basemodel::load_mdxa: WARNING: \"%s\" has no frames\n", base.name );
}

static constexpr int OldToNewRemapTable[72] = {
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

void q3basemodel::load_mdxm(bool server) {
	mdxmHeader_t * header = (mdxmHeader_t *)buffer.data();
	if (header->version != MDXM_VERSION)
		Com_Error(ERR_FATAL, "q3basemodel::load_mdxm: \"%s\" is wrong MDXM version", base.name);
	
	base.type = MOD_MDXM;
	base.mdxm = header;
	
	char const * anim_name = va("%s.gla",header->animName);
	header->animIndex = hw_inst->models.reg(anim_name)->base.index;
	if (!header->animIndex)
		Com_Error(ERR_FATAL, "q3basemodel::load_mdxm: could not find animation \"%s\" for model \"%s\"", anim_name, base.name);
	
	base.numLods = header->numLODs - 1;
	
	mdxmSurfHierarchy_t * surfi = (mdxmSurfHierarchy_t *)(buffer.data() + header->ofsSurfHierarchy);
	for (int32_t i = 0; i < header->numSurfaces; i++) {
		Q_strlwr(surfi->name);
		if ( !strcmp( &surfi->name[strlen(surfi->name) - 4], "_off") ) 
			surfi->name[strlen(surfi->name) - 4] = 0; //remove "_off" from name
		if (surfi->shader[0]) {
			surfi->shaderIndex = hw_inst->shaders.reg(surfi->shader, true, default_shader_mode::diffuse)->index;
		}
		surfi = (mdxmSurfHierarchy_t *)( (byte *)surfi + (size_t)( &((mdxmSurfHierarchy_t *)0)->childIndexes[ surfi->numChildren ] ));
	}
	
	bool needs_bone_conversion = header->numBones == 72 && strstr(header->animName, "_humanoid");
	mdxmLOD_t * lod = (mdxmLOD_t *)(buffer.data() + header->ofsLODs);
	for (int32_t l = 0; l < header->numLODs; l++) {
		mdxmSurface_t * surf = (mdxmSurface_t *) ( (byte *)lod + sizeof (mdxmLOD_t) + (header->numSurfaces * sizeof(mdxmLODSurfOffset_t)) );
		
		for (int32_t i = 0; i < header->numSurfaces; i++) {
			surf->ident = SF_MDX;
			if (needs_bone_conversion) {
				int *boneRef = (int *) ( (byte *)surf + surf->ofsBoneReferences );
				for (int32_t j = 0 ; j < surf->numBoneReferences ; j++) {
					assert(boneRef[j] >= 0 && boneRef[j] < 72);
					if (boneRef[j] >= 0 && boneRef[j] < 72)
						boneRef[j]=OldToNewRemapTable[boneRef[j]];
					else
						boneRef[j]=0;
				}
			}
			surf = (mdxmSurface_t *)( (byte *)surf + surf->ofsEnd );
		}
		lod = (mdxmLOD_t *)( (byte *)lod + lod->ofsEnd );
	}
}

void q3basemodel::load_md3(int32_t lod) {
	md3Header_t * header = (md3Header_t *)buffer.data();
	if (header->version != MD3_VERSION)
		Com_Error(ERR_FATAL, "q3basemodel::load_md3: \"%s\" is wrong MD3 version", base.name);
	
	base.type = MOD_MESH;
	base.md3[lod] = header;
	
	md3Surface_t * surf = (md3Surface_t *) ( buffer.data() + header->ofsSurfaces );
	for (int32_t i = 0 ; i < header->numSurfaces ; i++) {
		
		surf->ident = SF_MD3;
		
		Q_strlwr(surf->name);
		int32_t j = strlen(surf->name);
		if ( j > 2 && surf->name[j-2] == '_' ) {
			surf->name[j-2] = 0;
		}
		
		md3Shader_t * shader = (md3Shader_t *) ((byte *)surf + surf->ofsShaders);
		for (j = 0; j < surf->numShaders; j++) {
			shader->shaderIndex = hw_inst->shaders.reg(shader->name, true, default_shader_mode::diffuse)->index;
		}
		
		surf = (md3Surface_t *)( (byte *)surf + surf->ofsEnd );
	}
}
