#ifndef ODIN_OPENGL_RENDERER_CPP
#define ODIN_OPENGL_RENDERER_CPP

namespace Odin {

	inline OpenGLRenderer::OpenGLRenderer(void) {
		p_window = NULL;
		p_screenSurface = NULL;
		m_enabledAttributes = NULL;
	}
	inline OpenGLRenderer::~OpenGLRenderer(void) {
		p_window = NULL;
		if (p_screenSurface != NULL) SDL_FreeSurface(p_screenSurface);
		if (m_enabledAttributes != NULL) delete []m_enabledAttributes;
	}

	inline void OpenGLRenderer::m_InitGL(void) {
		m_precision = "highp";
		m_extensions.Clear();

		m_maxAnisotropy = 0;
		m_maxTextures = 0;
		m_maxVertexTextures = 0;
		m_maxTextureSize = 0;
		m_maxCubeTextureSize = 0;
		m_maxRenderBufferSize = 0;

		m_maxUniforms = 0;
		m_maxVaryings = 0;
		m_maxAttributes = 0;
		
		if (m_enabledAttributes != NULL) delete []m_enabledAttributes;
		m_enabledAttributes = NULL;
		
		m_program = -1;
		m_programForce = true;
		m_textureIndex = 0;
		m_activeTexture = -1;
		m_viewportX = 0.0f;
		m_viewportY = 0.0f;
		m_viewportWidth = 1.0f;
		m_viewportHeight = 1.0f;
		m_depthTest = true;
		m_depthWrite = true;
		m_blendingDisabled = true;
		m_blending = Blending::None;
		m_clearColor.Zero();
		m_clearAlpha = 1.0f;
		m_clearDepth = 1.0f;
		m_clearStencil = 0;
		m_frontFace = FrontFace::CounterClockWise;
		m_cullFaceDisabled = true;
		m_cullFace = CullFace::Back;
		m_lineWidth = 1.0f;
		m_stencilFunction = StencilFunction::Always;
		m_stencilReferenceValue = 0;
		m_stencilMask = 1;
		
		SetViewport(0.0f, 0.0f, 1.0f, 1.0f);
		SetDepthTest();
		SetDepthWrite();
		SetBlending();
		SetClearColor();
		SetClearDepth();
		SetClearStencil();
		SetFrontFace();
		SetCullFace();
		SetLineWidth();
		SetStencilFunction(CullFace::Back);
	}
	
	inline void OpenGLRenderer::m_GetGPUInfo(void) {
		int32 tmp1, tmp2;
		#ifdef Mobile
		bool isMobile = true;
		#else
		bool isMobile = false;
		#endif
		
		int32 vsHighpRange, vsHighpPrecision,
			  vsMediumpRange, vsMediumpPrecision,
			  fsHighpRange, fsHighpPrecision,
			  fsMediumpRange, fsMediumpPrecision;

		bool highpAvailable, mediumpAvailable;
		std::string precision = "highp";
		
		glGetShaderPrecisionFormat(GL_VERTEX_SHADER, GL_HIGH_FLOAT, &vsHighpRange, &vsHighpPrecision);
		glGetShaderPrecisionFormat(GL_VERTEX_SHADER, GL_MEDIUM_FLOAT, &vsMediumpRange, &vsMediumpPrecision);
		
		glGetShaderPrecisionFormat(GL_FRAGMENT_SHADER, GL_HIGH_FLOAT, &fsHighpRange, &fsHighpPrecision);
		glGetShaderPrecisionFormat(GL_FRAGMENT_SHADER, GL_MEDIUM_FLOAT, &fsMediumpRange, &fsMediumpPrecision);
		
		highpAvailable = vsHighpPrecision > 0 && vsHighpPrecision > 0;
        mediumpAvailable = fsHighpPrecision > 0 && fsHighpPrecision > 0;
		
		if (!highpAvailable || isMobile) {
			if (mediumpAvailable) {
				precision = "mediump";
			} else {
				precision = "lowp";
			}
		}
		
		m_precision = precision;
		if (GLEW_EXT_texture_filter_anisotropic) {
			glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &m_maxAnisotropy);
		}
		glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &m_maxTextures);
		glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &m_maxVertexTextures);
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &m_maxTextureSize);
		glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE, &m_maxCubeTextureSize);
		glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &m_maxRenderBufferSize);
		
		glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_VECTORS, &tmp1);
		glGetIntegerv(GL_MAX_VERTEX_UNIFORM_VECTORS, &tmp2);
		m_maxUniforms = Mathf.Min(tmp1, tmp2) * 4;
		
		glGetIntegerv(GL_MAX_VARYING_VECTORS, &m_maxVaryings);
		m_maxVaryings *= 4;
		
		glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &m_maxAttributes);
		
		if (m_enabledAttributes != NULL) delete []m_enabledAttributes;
		m_enabledAttributes = new uint32[m_maxAttributes];
		for (int32 i = 0; i < m_maxAttributes; i++) m_enabledAttributes[i] = 0;
	}
	
	inline void OpenGLRenderer::SetWindow(Window* window) {
		if (p_screenSurface != NULL) SDL_FreeSurface(p_screenSurface);
		
		p_window = window;
		p_screenSurface = SDL_GetWindowSurface(p_window->GetSDLWindow());
		
		m_InitGL();
		m_GetGPUInfo();
	}
	inline Window* OpenGLRenderer::GetWindow(Window* window) {
		return p_window;
	}
	
	inline uint32 OpenGLRenderer::CreateShader(uint32 shaderType, const std::string &shaderString) {
		uint32 shader = glCreateShader(shaderType);
		const char *shaderSource = shaderString.c_str();
		glShaderSource(shader, 1, &shaderSource, NULL);
		
		glCompileShader(shader);
		
		int32 status;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
		if (status == false) {
			int32 infoLogLength;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
			
			char8 *strInfoLog = new char8[infoLogLength + 1];
			glGetShaderInfoLog(shader, infoLogLength, NULL, strInfoLog);
			
			const char *strShaderType = NULL;
			switch(shaderType) {
				case GL_VERTEX_SHADER:
					strShaderType = "vertex";
					break;
				case GL_GEOMETRY_SHADER:
					strShaderType = "geometry";
					break;
				case GL_FRAGMENT_SHADER:
					strShaderType = "fragment";
					break;
			}
			
			fprintf(stderr, "Compile failure in %s shader:\n%s\n", strShaderType, strInfoLog);
			delete[] strInfoLog;
		}
		
		return shader;
	}
	inline uint32 OpenGLRenderer::CreateProgram(uint32 vertexShader, uint32 fragmentShader) {
		uint32 program = glCreateProgram();
    
		glAttachShader(program, vertexShader);
		glAttachShader(program, fragmentShader);
		
		glLinkProgram(program);
		
		int32 status;
		glGetProgramiv(program, GL_LINK_STATUS, &status);
		if (status == false) {
			int32 infoLogLength;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
			
			char8 *strInfoLog = new char8[infoLogLength + 1];
			glGetProgramInfoLog(program, infoLogLength, NULL, strInfoLog);
			fprintf(stderr, "Linker failure: %s\n", strInfoLog);
			delete[] strInfoLog;
		}
		
		glDetachShader(program, vertexShader);
		glDetachShader(program, fragmentShader);
	
		return program;
	}
	inline uint32 OpenGLRenderer::CreateProgram(std::string vertexShaderSource, std::string fragmentShaderSource) {
		uint32 program = glCreateProgram();
    
		uint32 vertexShader = CreateShader(GL_VERTEX_SHADER, vertexShaderSource);
		uint32 fragmentShader = CreateShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
	
		glAttachShader(program, vertexShader);
		glAttachShader(program, fragmentShader);
		
		glLinkProgram(program);
		
		int32 status;
		glGetProgramiv(program, GL_LINK_STATUS, &status);
		if (status == false) {
			int32 infoLogLength;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
			
			char8 *strInfoLog = new char8[infoLogLength + 1];
			glGetProgramInfoLog(program, infoLogLength, NULL, strInfoLog);
			fprintf(stderr, "Linker failure: %s\n", strInfoLog);
			delete[] strInfoLog;
		}
		
		glDetachShader(program, vertexShader);
		glDetachShader(program, fragmentShader);
	
		return program;
	}
	inline uint32 OpenGLRenderer::CreateProgram(Array<uint32>& shaderList) {
		uint32 program = glCreateProgram();
    
		for(uint32 i = 0, il = shaderList.Length(); i < il; i++) {
			glAttachShader(program, shaderList[i]);
		}
		
		glLinkProgram(program);
		
		int32 status;
		glGetProgramiv(program, GL_LINK_STATUS, &status);
		if (status == false) {
			int32 infoLogLength;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
			
			char8 *strInfoLog = new char8[infoLogLength + 1];
			glGetProgramInfoLog(program, infoLogLength, NULL, strInfoLog);
			fprintf(stderr, "Linker failure: %s\n", strInfoLog);
			delete[] strInfoLog;
		}
		
		for(uint32 i = 0, il = shaderList.Length(); i < il; i++) {
			glDetachShader(program, shaderList[i]);
		}
	
		return program;
	}
	
	template <typename Type> inline uint32 OpenGLRenderer::CreateVertexBuffer(Array<Type>& items, uint32 type, uint32 draw) {
		uint32 vertexBuffer;
		
		glGenBuffers(1, &vertexBuffer);
		glBindBuffer(type, vertexBuffer);
		glBufferData(type, sizeof(items), items, draw);
		
		return vertexBuffer;
	}
	
	inline uint32 OpenGLRenderer::CreateTexture(Texture* texture) {
		SDL_Surface* image = texture->m_image;
		if (image == NULL) {
			printf("texture %s image is NULL", texture->p_name.c_str());
			return 0;
		}
		image = SDL_ConvertSurface(image, p_screenSurface->format, 0);
		bool isPOT = Mathf.IsPowerOfTwo(image->w) && Mathf.IsPowerOfTwo(image->h);
		
		bool mipmap = texture->m_generateMipmap;
		int32 anisotropy = texture->m_anisotropy;
		TextureFilter filter = texture->m_filter;
		TextureFormat format = texture->m_format;
		TextureWrap wrap = texture->m_wrap;
		TextureType type = texture->m_type;
		
		uint32& textureID = texture->m_textureID;
		int32 minFilter, magFilter;

		if (wrap == TextureWrap::MirrorRepeat) {
			wrap = isPOT ? TextureWrap::MirrorRepeat : TextureWrap::Clamp;
		} else if (wrap == TextureWrap::Repeat) {
			wrap = isPOT ? TextureWrap::Repeat : TextureWrap::Clamp;
		}
		if (filter == TextureFilter::None) {
			magFilter = GL_NEAREST;
			if (mipmap && isPOT) {
				minFilter = GL_LINEAR_MIPMAP_NEAREST;
			} else {
				minFilter = GL_NEAREST;
			}
		} else if (filter == TextureFilter::Linear) {
			magFilter = GL_LINEAR;
			if (mipmap && isPOT) {
				minFilter = GL_LINEAR_MIPMAP_LINEAR;
			} else {
				minFilter = GL_LINEAR;
			}
		}
		
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, static_cast<uint32>(format), image->w, image->h, 0, static_cast<uint32>(format), static_cast<uint32>(type), image->pixels);
		
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, minFilter);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, magFilter);
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, static_cast<int32>(wrap));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, static_cast<int32>(wrap));
		
		if (anisotropy > 0 && GLEW_EXT_texture_filter_anisotropic) {
			glTexParameterf(GL_TEXTURE_2D, GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, Mathf.Clamp(anisotropy, 1, m_maxAnisotropy));
		}
		if (mipmap && isPOT) {
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		glBindTexture(GL_TEXTURE_2D, 0);
		
		return textureID;
	}
	
	inline void OpenGLRenderer::ClearCanvas(bool color, bool depth, bool stencil) {
		uint32 bits = 0;

		if (color) bits |= GL_COLOR_BUFFER_BIT;
		if (depth) bits |= GL_DEPTH_BUFFER_BIT;
		if (stencil) bits |= GL_STENCIL_BUFFER_BIT;

		glClear(bits);
	}
	inline void OpenGLRenderer::ClearColor(void) {
		glClear(GL_COLOR_BUFFER_BIT);
	}
	inline void OpenGLRenderer::ClearDepth(void) {
		glClear(GL_DEPTH_BUFFER_BIT);
	}
	inline void OpenGLRenderer::ClearStencil(void) {
		glClear(GL_STENCIL_BUFFER_BIT);
	}

	inline void OpenGLRenderer::EnableAttribute(uint32 attribute) {
		if (attribute >= static_cast<uint32>(m_maxAttributes)) return;
		
		if (m_enabledAttributes[attribute] == 0) {
			glEnableVertexAttribArray(attribute);
			m_enabledAttributes[attribute] = 1;
		}
	}
	inline void OpenGLRenderer::DisableAttribute(uint32 attribute) {
		if (attribute >= static_cast<uint32>(m_maxAttributes)) return;

		if (m_enabledAttributes[attribute] == 1) {
			glDisableVertexAttribArray(attribute);
			m_enabledAttributes[attribute] = 0;
		}
	}
	inline void OpenGLRenderer::DisableAttributes(void) {
		uint32 i = m_maxAttributes;

		while (i--) {
			if (m_enabledAttributes[i] == 1) {
				glDisableVertexAttribArray(i);
				m_enabledAttributes[i] = 0;
			}
		}
	}
	
	inline void OpenGLRenderer::SetProgram(uint32 program) {
		
		if (m_program != program) {
			m_program = program;
			m_programForce = true;

			glUseProgram(program);
		} else {
			m_programForce = false;
		}

		m_textureIndex = 0;
		m_activeTexture = -1;
	}
	
	inline void OpenGLRenderer::SetViewport(float32 x, float32 y, float32 width, float32 height) {
		if (m_viewportX != x || m_viewportY != y || m_viewportWidth != width || m_viewportHeight != height) {
			m_viewportX = x;
			m_viewportY = y;
			m_viewportWidth = width;
			m_viewportHeight = height;
			glViewport(x, y, width, height);
		}
	}
	inline void OpenGLRenderer::SetDepthTest(bool depthTest) {
		if (m_depthTest != depthTest) {
			m_depthTest = depthTest;
			
			if (depthTest) {
				glEnable(GL_DEPTH_TEST);
			} else {
				glDisable(GL_DEPTH_TEST);
			}
		}
	}
	inline void OpenGLRenderer::SetDepthWrite(bool depthWrite) {
		if (m_depthWrite != depthWrite) {
			m_depthWrite = depthWrite;
			glDepthMask(depthWrite);
		}
	}
	inline void OpenGLRenderer::SetBlending(Blending blending) {
		if (m_blending != blending) {
			m_blending = blending;
			
			switch (blending) {
				case Blending::Additive:
					if (m_blendingDisabled) glEnable(GL_BLEND);
					glBlendFunc(GL_SRC_ALPHA, GL_ONE);
					break;
				
				case Blending::Subtractive:
					if (m_blendingDisabled) glEnable(GL_BLEND);
					glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
					break;
				
				case Blending::Multiply:
					if (m_blendingDisabled) glEnable(GL_BLEND);
					glBlendFunc(GL_ZERO, GL_SRC_COLOR);
					break;
				
				case Blending::Screen:
					if (m_blendingDisabled) glEnable(GL_BLEND);
					glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
					break;
				
				case Blending::Default:
					if (m_blendingDisabled) glEnable(GL_BLEND);
					glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
					//glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
					break;
				
				case Blending::None:
				default:
					glDisable(GL_BLEND);
					m_blendingDisabled = true;
					m_blending = Blending::None;
					return;
			}
			
			m_blendingDisabled = false;
		}
	}
	inline void OpenGLRenderer::SetClearColor(const Colorf& color, float32 alpha) {
		if (m_clearColor != color || m_clearAlpha != alpha) {
			m_clearColor = color;
			m_clearAlpha = alpha;
			glClearColor(color.r, color.g, color.b, alpha);
		}
	}
	inline void OpenGLRenderer::SetClearDepth(float32 depth) {
		if (m_clearDepth != depth) {
			m_clearDepth = depth;
			glClearDepth(depth);
		}
	}
	inline void OpenGLRenderer::SetClearStencil(int32 stencil) {
		if (m_clearStencil != stencil) {
			m_clearStencil = stencil;
			glClearStencil(stencil);
		}
	}
	inline void OpenGLRenderer::SetFrontFace(FrontFace mode) {
		if (m_frontFace != mode) {
			m_frontFace = mode;
			glFrontFace(static_cast<GLenum>(mode));
		}
	}
	inline void OpenGLRenderer::SetCullFace(CullFace mode) {
		if (m_cullFace != mode) {
			m_cullFace = mode;
			if (m_cullFaceDisabled) {
				m_cullFaceDisabled = false;
				glEnable(GL_CULL_FACE);
			}
			glCullFace(static_cast<GLenum>(mode));
		}
	}
	inline void OpenGLRenderer::SetLineWidth(float32 width) {
		if (m_lineWidth != width) {
			m_lineWidth = width;
			glLineWidth(width);
		}
	}
	inline void OpenGLRenderer::SetScissor(float32 x, float32 y, float32 width, float32 height) {
		glScissor(x, y, width, height);
	}
	inline void OpenGLRenderer::SetStencilFunction(CullFace cullFace, StencilFunction function, int32 referenceValue, uint32 mask) {
		if (m_stencilFunction != function || m_stencilReferenceValue != referenceValue || m_stencilMask != mask) {
			m_stencilFunction = function;
			m_stencilReferenceValue = referenceValue;
			m_stencilMask = mask;
			glStencilFuncSeparate(static_cast<GLenum>(cullFace), static_cast<GLenum>(function), referenceValue, mask);
		}
	}
	inline void OpenGLRenderer::SetStencilFunction(StencilFunction function, int32 referenceValue, uint32 mask) {
		if (m_stencilFunction != function || m_stencilReferenceValue != referenceValue || m_stencilMask != mask) {
			m_stencilFunction = function;
			m_stencilReferenceValue = referenceValue;
			m_stencilMask = mask;
			glStencilFunc(static_cast<GLenum>(function), referenceValue, mask);
		}
	}
}

#endif