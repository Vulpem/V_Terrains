#include "Globals.h"
#include "Application.h"
#include "ModuleRenderer3D.h"
#include "ModuleWindow.h"
#include "ModuleCamera3D.h"
#include "ModuleInput.h"

#include "GameObject.h"
#include "Transform.h"

#include "Camera.h"
#include "imGUI\imgui.h"

#include "OpenGL.h"

#include "Mesh_RenderInfo.h"
#include "ViewPort.h"

#include "R_Mesh.h"
#include "../V_Terrain/Code/Include.h"

#pragma comment (lib, "opengl32.lib") /* link Microsoft OpenGL lib   */
#pragma comment (lib, "glu32.lib")    /* link OpenGL Utility lib     */
#pragma comment (lib, "Glew/libx86/glew32.lib") /* link Microsoft OpenGL lib   */


ModuleRenderer3D::ModuleRenderer3D(Application* app, bool start_enabled) : Module(app, start_enabled)
{
}

// Destructor
ModuleRenderer3D::~ModuleRenderer3D()
{}

// Called before render is available
bool ModuleRenderer3D::Init()
{
	LOG("Creating 3D Renderer context");
	bool ret = true;

	//Create context
	m_GLcontext = SDL_GL_CreateContext(App->m_window->GetWindow());
	if (m_GLcontext == nullptr)
	{
		LOG("OpenGL context could not be created! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}

	GLenum err = glewInit();

	if (err != GLEW_OK)
	{
		LOG("Glew library could not init %s\n", glewGetErrorString(err));
		ret = false;
	}
	else
		LOG("Using Glew %s", glewGetString(GLEW_VERSION));

	if (ret == true)
	{
		// get version info
		LOG("Vendor: %s", glGetString(GL_VENDOR));
		LOG("Renderer: %s", glGetString(GL_RENDERER));
		LOG("OpenGL version supported %s", glGetString(GL_VERSION));
		LOG("GLSL: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

		if (VSYNC)
		{
			if (SDL_GL_SetSwapInterval(1) < 0)
			{
				LOG("Warning: Unable to set VSync! SDL Error: %s", SDL_GetError());
			}
		}
		else
		{
			if (SDL_GL_SetSwapInterval(0) < 0)
			{
				LOG("Warning: Unable to set VSync! SDL Error: %s", SDL_GetError());
			}
		}

		//Check for error
        GLenum error;
		while ((error = glGetError() )!= GL_NO_ERROR)
		{
			LOG("Error initializing OpenGL! %s\n", gluErrorString(error));
			ret = false;
		}

		glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
		glClearDepth(10.0f);

		//Initialize clear color
		glClearColor(m_clearColor.x, m_clearColor.y, m_clearColor.z, 1.f);

		//Check for error
		error = glGetError();
		if (error != GL_NO_ERROR)
		{
			LOG("Error initializing OpenGL! %s\n", gluErrorString(error));
			ret = false;
		}

		// Blend for transparency
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


		GLfloat LightModelAmbient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, LightModelAmbient);

		m_lights[0].m_ref = GL_LIGHT0;
		m_lights[1].m_ref = GL_LIGHT1;
		m_lights[2].m_ref = GL_LIGHT2;
		m_lights[3].m_ref = GL_LIGHT3;
		m_lights[4].m_ref = GL_LIGHT4;
		m_lights[5].m_ref = GL_LIGHT5;
		m_lights[6].m_ref = GL_LIGHT6;
		m_lights[7].m_ref = GL_LIGHT7;
		for (int n = 0; n < MAX_LIGHTS; n++)
		{
			m_lights[n].m_ambient.Set(0.25f, 0.25f, 0.25f, 1.0f);
			m_lights[n].m_diffuse.Set(0.75f, 0.75f, 0.75f, 1.0f);
			m_lights[n].SetPos(n * 5, 10, 0);
			m_lights[n].Init();
		}

		GLfloat MaterialAmbient[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, MaterialAmbient);

		GLfloat MaterialDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialDiffuse);

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glEnable(GL_LIGHTING);
		glEnable(GL_COLOR_MATERIAL);
		glEnable(GL_TEXTURE_2D);

		m_lights[0].Active(true);

		glShadeModel(GL_SMOOTH);		 // Enables Smooth Shading

		glLineWidth(1.0f);

		m_viewPorts.push_back(ViewPort(float2(0, 0), float2(SCREEN_WIDTH, SCREEN_HEIGHT), App->m_camera->GetDefaultCam(), m_viewPorts.size()));

	}

    //Initialize Projection Matrix
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    //Initialize Modelview Matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    //Check for error
    GLenum error;
    while ((error = glGetError()) != GL_NO_ERROR)
    {
        LOG("Error initializing OpenGL! %s\n", gluErrorString(error));
        ret = false;
    }

	return ret;
}

// PreUpdate: clear buffer
UpdateStatus ModuleRenderer3D::PreUpdate()
{
	glClearColor(m_clearColor.x, m_clearColor.y, m_clearColor.z, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	return UpdateStatus::Continue;
}

// PostUpdate present buffer to screen
UpdateStatus ModuleRenderer3D::PostUpdate()
{
	for (uint i = 0; i < MAX_LIGHTS; ++i)
	{
		m_lights[i].Render();
	}

	m_viewPorts.front().m_camera = App->m_camera->GetDefaultCam();
	TIMER_START("ViewPorts render");
	if (m_viewPorts.empty() == false)
	{
		TIMER_RESET_STORED("ViewPorts slowest");
		for (std::vector<ViewPort>::iterator port = m_viewPorts.begin(); port != m_viewPorts.end(); port++)
		{
			TIMER_START_PERF("ViewPorts slowest");
			if (port->m_active && port->m_autoRender)
			{
				SetViewPort(*port);
				App->Render(*port);
				RenderBlendObjects();
			}
			TIMER_READ_MS_MAX("ViewPorts slowest");
		}
	}
	else
	{
		LOG("Warning, there are no viewPorts!");
	}
	TIMER_READ_MS("ViewPorts render");

#if USE_EDITOR
	ImGui::Render();
#endif

	SDL_GL_SwapWindow(App->m_window->GetWindow());
	return UpdateStatus::Continue;
}

// Called before quitting
void ModuleRenderer3D::CleanUp()
{
	LOG("Destroying 3D Renderer");
	SDL_GL_DeleteContext(m_GLcontext);
}


void ModuleRenderer3D::OnScreenResize(int width, int heigth)
{
	m_viewPorts.front().m_size = float2(width, heigth);
}

void ModuleRenderer3D::UpdateProjectionMatrix(Camera* cam)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glLoadMatrixf(cam->GetProjectionMatrix().ptr());
	glMatrixMode(GL_MODELVIEW);
}

void ModuleRenderer3D::RenderBlendObjects()
{
	std::multimap<float, Mesh_RenderInfo>::reverse_iterator it = m_alphaObjects.rbegin();
	for (; it != m_alphaObjects.rend(); it++)
	{
		DrawMesh(it->second, true);
	}
	m_alphaObjects.clear();
}

void ModuleRenderer3D::DrawLine(float3 a, float3 b, float4 color)
{
	if (m_usingLights)
	{
		glDisable(GL_LIGHTING);
	}
	glColor4f(color.x, color.y, color.z, color.w);

	glBegin(GL_LINES);

	glVertex3fv(a.ptr()); glVertex3fv(b.ptr());

	glEnd();

	if (m_usingLights)
	{
		glEnable(GL_LIGHTING);
	}
}

void ModuleRenderer3D::DrawBox(float3* corners, float4 color)
{
	if (m_usingLights)
	{
		glDisable(GL_LIGHTING);
	}
	glColor4f(color.x, color.y, color.z, color.w);

	glBegin(GL_LINES);

	glVertex3fv(corners[0].ptr()); glVertex3fv(corners[1].ptr());
	glVertex3fv(corners[0].ptr()); glVertex3fv(corners[2].ptr());
	glVertex3fv(corners[0].ptr()); glVertex3fv(corners[4].ptr());
	glVertex3fv(corners[3].ptr()); glVertex3fv(corners[1].ptr());
	glVertex3fv(corners[3].ptr()); glVertex3fv(corners[2].ptr());
	glVertex3fv(corners[3].ptr()); glVertex3fv(corners[7].ptr());
	glVertex3fv(corners[5].ptr()); glVertex3fv(corners[1].ptr());
	glVertex3fv(corners[5].ptr()); glVertex3fv(corners[4].ptr());
	glVertex3fv(corners[5].ptr()); glVertex3fv(corners[7].ptr());
	glVertex3fv(corners[6].ptr()); glVertex3fv(corners[2].ptr());
	glVertex3fv(corners[6].ptr()); glVertex3fv(corners[4].ptr());
	glVertex3fv(corners[6].ptr()); glVertex3fv(corners[7].ptr());

	glEnd();

	if (m_usingLights)
	{
		glEnable(GL_LIGHTING);
	}
}

void ModuleRenderer3D::DrawLocator(float4x4 transform, float4 color)
{
	if (m_usingLights)
	{
		glDisable(GL_LIGHTING);
	}
	glPushMatrix();
	glMultMatrixf(transform.ptr());

	glColor4f(color.x, color.y, color.z, color.w);

	glBegin(GL_LINES);

	glVertex3f(1.0f, 0.0f, 0.0f); glVertex3f(-1.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 1.0f, 0.0f); glVertex3f(0.0f, -1.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 1.0f); glVertex3f(0.0f, 0.0f, -1.0f);
	//Arrow indicating forward
	glVertex3f(0.0f, 0.0f, 1.0f); glVertex3f(0.1f, 0.0f, 0.9f);
	glVertex3f(0.0f, 0.0f, 1.0f); glVertex3f(-0.1f, 0.0f, 0.9f);

	glEnd();

	if (m_usingLights)
	{
		glEnable(GL_LIGHTING);
	}
	glPopMatrix();
}

void ModuleRenderer3D::DrawLocator(float3 position, float4 color)
{
	App->m_renderer3D->DrawLocator((float4x4::FromTRS(position, float4x4::identity, float3(1, 1, 1))).Transposed(), color);
}

void ModuleRenderer3D::DrawMesh(Mesh_RenderInfo& meshInfo, bool renderBlends)
{
	glActiveTexture(0);
	if (meshInfo.m_alphaType == AlphaTestTypes::ALPHA_BLEND && renderBlends == false)
	{
		//TMP / TODO
		//This is pretty inaccurate and probably not optimized. But, hey, it works. Sometimes. Maybe.
		float3 objectPos = meshInfo.m_transform.Transposed().TranslatePart();
		float distanceToObject = m_currentViewPort->m_camera->object->GetTransform()->GetGlobalPos().Distance(objectPos);

		m_alphaObjects.insert(std::pair<float, Mesh_RenderInfo>(distanceToObject, meshInfo));
		return;
	}

	//Setting alpha&&blend
	switch (meshInfo.m_alphaType)
	{
	case (AlphaTestTypes::ALPHA_BLEND):
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, meshInfo.m_blendType);
	}
	case (AlphaTestTypes::ALPHA_DISCARD):
	{
		glEnable(GL_ALPHA_TEST);
		glAlphaFunc(GL_GREATER, meshInfo.m_alphaTest);
		break;
	}
	}

	glUseProgram(meshInfo.m_shader.m_program);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshInfo.m_indicesBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, meshInfo.m_dataBuffer);

	// ------ Setting uniforms -------------------------
	glUniformMatrix4fv(meshInfo.m_shader.m_modelMatrix, 1, GL_FALSE, meshInfo.m_transform.ptr());
	glUniformMatrix4fv(meshInfo.m_shader.m_viewMatrix, 1, GL_FALSE, m_currentViewPort->m_camera->GetViewMatrix().ptr());
	glUniformMatrix4fv(meshInfo.m_shader.m_projectionMatrix, 1, GL_FALSE, m_currentViewPort->m_camera->GetProjectionMatrix().ptr());

	glUniform1f(meshInfo.m_shader.m_time, (float)Time.AppRuntime);

	glUniform4fv(meshInfo.m_shader.m_ambientColor, 1, m_ambientLight.ptr());
	glUniform3fv(meshInfo.m_shader.m_globalLightDir, 1, m_sunDirection.ptr());
	glUniform1i(meshInfo.m_shader.m_fogDistance, RPGT::config.fogDistance);
	glUniform3fv(meshInfo.m_shader.m_fogColor, 1, RPGT::config.fogColor);
	glUniform1f(meshInfo.m_shader.m_maxHeight, RPGT::config.maxHeight);
	
	// ------ Setting data format -------------------------

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	if (meshInfo.m_drawNormals)
	{
		glUniform4fv(meshInfo.m_shader.m_materialColor, 1, float4(0.54f, 0.0f, 0.54f, 1.0f).ptr());
		RenderNormals(meshInfo);
	}

	if (meshInfo.m_drawWired)
	{
		glUniform4fv(meshInfo.m_shader.m_materialColor, 1, meshInfo.m_wiresColor.ptr());
		RenderMeshWired(meshInfo);
	}

	glUniform1i(meshInfo.m_shader.m_useLight, m_currentViewPort->m_useLighting);

	glUniform1i(meshInfo.m_shader.m_hasTexture, (meshInfo.m_textureBuffer != 0));
	if (meshInfo.m_textureBuffer > 0)
	{
		glBindTexture(GL_TEXTURE_2D, meshInfo.m_textureBuffer);
	}

	if (meshInfo.m_drawFilled)
	{
		glUniform4fv(meshInfo.m_shader.m_materialColor, 1, meshInfo.m_meshColor.ptr());
		RenderMeshFilled(meshInfo);
	}

	//Cleaning
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glDisable(GL_ALPHA_TEST);
	glDisable(GL_BLEND);

	glUseProgram(0);
}

ViewPort* ModuleRenderer3D::HoveringViewPort()
{
	for (std::vector<ViewPort>::reverse_iterator it = m_viewPorts.rbegin(); it != m_viewPorts.rend(); it++)
	{
		if (it->m_active)
		{
			if (App->m_input->GetMouseX() > it->m_pos.x && App->m_input->GetMouseX() < it->m_pos.x + it->m_size.x &&
				App->m_input->GetMouseY() > it->m_pos.y && App->m_input->GetMouseY() < it->m_pos.y + it->m_size.y)
			{
				return &*it;
			}
		}
	}
	return nullptr;
}

float2 ModuleRenderer3D::ViewPortToScreen(const float2 & pos_in_ViewPort, ViewPort** OUT_port)
{
	*OUT_port = HoveringViewPort();
	if (*OUT_port != nullptr)
	{
		return float2((*OUT_port)->m_pos + pos_in_ViewPort);
	}
	return float2(-1,-1);
}

float2 ModuleRenderer3D::ScreenToViewPort(const float2 & pos_in_screen, ViewPort** OUT_port)
{
	*OUT_port = HoveringViewPort();
	if (*OUT_port != nullptr)
	{
		return float2(pos_in_screen - (*OUT_port)->m_pos);
	}
	return float2(-1,-1);
}

uint ModuleRenderer3D::AddViewPort(float2 m_pos, float2 m_size, Camera * cam)
{
	m_viewPorts.push_back(ViewPort(m_pos, m_size, cam, m_viewPorts.size()));
	return m_viewPorts.back().m_ID;
}

ViewPort * ModuleRenderer3D::FindViewPort(uint m_ID)
{
	for (std::vector<ViewPort>::iterator it = m_viewPorts.begin(); it != m_viewPorts.end(); it++)
	{
		if (it->m_ID == m_ID)
		{
			return it._Ptr;
		}
	}
	return nullptr;
}

bool ModuleRenderer3D::DeleteViewPort(uint m_ID)
{
	for (std::vector<ViewPort>::iterator it = m_viewPorts.begin(); it != m_viewPorts.end(); it++)
	{
		if (it->m_ID == m_ID)
		{
			m_viewPorts.erase(it);
			return true;
		}
	}
	return false;
}

void ModuleRenderer3D::SetViewPort(ViewPort& port)
{
	m_currentViewPort = &port;
	port.SetCameraMatrix();
	glViewport(port.m_pos.x, App->m_window->GetWindowSize().y - (port.m_size.y + port.m_pos.y), port.m_size.x, port.m_size.y);
	UpdateProjectionMatrix(port.m_camera);

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(port.m_camera->GetViewMatrix().ptr());

	if (port.m_useSingleSidedFaces) { glEnable(GL_CULL_FACE); m_usingSingleSidedFaces = true; }
	else { glDisable(GL_CULL_FACE); m_usingSingleSidedFaces = false; }

	if (port.m_useLighting) { glEnable(GL_LIGHTING); m_usingLights = true; }
	else { glDisable(GL_LIGHTING); m_usingLights = false;}

	if (!port.m_renderHeightMap) { glEnable(GL_TEXTURE_2D); m_usingTextures = true; }
	else { glDisable(GL_TEXTURE_2D); m_usingTextures = false;}
}

void ModuleRenderer3D::RenderMeshWired(const Mesh_RenderInfo& data)
{
	if (m_usingSingleSidedFaces)
	{
		glDisable(GL_CULL_FACE);
	}
	if (m_usingLights)
	{
		glDisable(GL_LIGHTING);
	}

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glColor4fv(data.m_wiresColor.ptr());

	glDrawElements(GL_TRIANGLES, data.m_nIndices, GL_UNSIGNED_INT, NULL);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	if (m_usingSingleSidedFaces)
	{
		glEnable(GL_CULL_FACE);
	}
	if (m_usingLights)
	{
		glEnable(GL_LIGHTING);
	}

}

void ModuleRenderer3D::RenderMeshFilled(const Mesh_RenderInfo& data)
{
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glColor4fv(data.m_meshColor.ptr());
	glDrawElements(GL_TRIANGLES, data.m_nIndices, GL_UNSIGNED_INT, NULL);
}

void ModuleRenderer3D::RenderNormals(const Mesh_RenderInfo & data)
{
	if (data.m_origin->m_hasNormals)
	{
		for (uint n = 0; n < data.m_nVertices; n++)
		{
			DrawLine(
				float3(data.m_origin->m_vertices[n]),
				float3(data.m_origin->m_vertices[n] + data.m_origin->m_normals[n]),
				float4(0.54f, 0.0f, 0.54f, 1.0f));
		}
	}
}
