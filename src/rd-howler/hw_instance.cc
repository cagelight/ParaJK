#include "hw_local.hh"
using namespace howler;

instance::instance() {	
	
}

void instance::initialize_renderer() {
	
	assert(window_initialized);
	if (renderer_initialized) return;
	
	//gl::initialize_defaults();
	
	width = glConfig.vidWidth;
	height = glConfig.vidHeight;
	
	glViewport(0, 0, glConfig.vidWidth, glConfig.vidHeight);
	glScissor(0, 0, glConfig.vidWidth, glConfig.vidHeight);
	
	//gl::blend(true);
	glEnable(GL_SCISSOR_TEST);
	//glEnable(GL_LINE_SMOOTH);
	//gl::depth_test(true);
	
	glClearColor(0.25, 0, 0, 1);
	
	main_sampler = make_q3sampler();
	
	q3mainprog.reset( new programs::q3main );
	q3lineprog.reset( new programs::q3line );
	q3skyboxstencilprog.reset( new programs::q3skyboxstencil );
	q3skyboxprog.reset( new programs::q3skybox );
	
	fullquad = q3mesh_basic::generate_fullquad();
	unitquad = q3mesh_basic::generate_unitquad();
	skybox = q3mesh_basic::generate_skybox_mesh();
	
	textures.generate_named_defaults();
	
	/*
	framebuffer = make_q3framebuffer(width * r_fboratio->value, height * r_fboratio->value);
	framebuffer->attach(q3framebuffer::attachment::color0, GL_RGBA8);
	framebuffer->attach(q3framebuffer::attachment::color1, GL_RGBA8);
	
	framebuffer->attach_depth_stencil_renderbuffer();
	//framebuffer->attach(q3framebuffer::attachment::depth, GL_DEPTH_COMPONENT32);
	
	if (!framebuffer->validate()) {
		Com_Error(ERR_FATAL, "rend::initialize: failed to initialize framebuffer");
	}
	
	scratch1 = make_q3texture(width, height, false);
	
	this->initialize_texture();
	this->initialize_shader();
	this->initialize_model();
	*/
	
	renderer_initialized = true;
	shaders.process_waiting();
	models.process_waiting();
	
	R_InitFonts();
}

instance::~instance() {
	R_ShutdownFonts();
}

void instance::load_world(char const * name) {
	m_world.reset( new q3world {} );
	m_world->load(name);
}

void instance::save_lightmap_atlas() {
	if (!m_world || !m_world->m_lightmap) return;
	m_world->m_lightmap->save(va("atlas/%s.png", m_world->m_basename.c_str()));
}

void instance::screenshot(char const * path) {
	std::vector<uint8_t> image;
	image.resize(width * height * 3);
	glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, image.data());
	RE_SavePNG(path, image.data(), width, height, 3);
}
