#include <windows.h>

#include "..\imgui-master\imgui.h"
#include "..\imgui-master\backends\imgui_impl_glfw.h"
#include "..\imgui-master\backends\imgui_impl_opengl3.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

#include "InitShader.h"    //Functions for loading shaders from text files
#include "LoadMesh.h"      //Functions for creating OpenGL buffers from mesh files
#include "LoadTexture.h"   //Functions for creating OpenGL textures from image files
#include "VideoMux.h"      //Functions for saving videos
#include "DebugCallback.h" // Functions for debugging glsl
#include "AttriblessRendering.h"

const char* const window_title = "Jello";

int screen_width;
int screen_height;
static const std::string vertex_shader("template_vs.glsl");
static const std::string fragment_shader("jello_fs.glsl");
GLuint shader_program = -1;

GLuint FBO; // Frame buffer object
GLuint fbo_tex = -1; // Color FBO texture
GLuint depth_tex = -1; // Depth FBO texture
GLenum buffers[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
float clear[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

GLuint attribless_vao = -1;

GLuint rectVAO;
GLuint rectVBO;

static const std::string mesh_name = "RubiksCube_01.obj";

MeshData mesh_data;

GLuint bg_vao = -1;
GLuint bg_vbo = -1;
float bg_vertices[] =
{
    // Coords       // Tex coords
    1.0f, -1.0f,    1.0f, 0.0f,
    -1.0f, -1.0f,   0.0f, 0.0f,
    -1.0f, 1.0f,    0.0f, 1.0f,

    1.0f, 1.0f,     1.0f, 1.0f,
    1.0f, -1.0f,    1.0f, 0.0f,
    -1.0f, 1.0f,    0.0f, 1.0f
};

struct CameraUniforms {
    glm::vec4 eye = glm::vec4(0.0f, 2.5f, 3.0f, 0.0f);
    glm::vec4 up = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
    glm::vec4 resolution = glm::vec4(800.0f, 800.0f, 1.0f, 0.0f);
    //float aspect = resolution.x / resolution.y;
}CameraData;

struct LightUniforms {
    glm::vec4 light_w = glm::vec4(-10.0f, 10.0f, 5.0f, 1.0f); // World-space light position
    glm::vec4 bg_color = glm::vec4(0.35f, 0.35f, 0.35f, 1.0f); // Background color
} LightData;

struct MaterialUniforms {
    glm::vec4 base_color = glm::vec4(0.35f, 0.35f, 0.35f, 1.0f); // base color
    glm::vec4 spec_color = glm::vec4(0.85f, 0.85f, 0.85f, 1.0f); // Specular Color
    float spec_factor = 2.0f; // Specular factor
} MaterialData;

// Locations for the uniforms which are not in uniform blocks
namespace UniformLocs
{
    int M = 0;
    int PV = 1;
    int time = 2;
    int pass = 3;
}

GLuint light_ubo = -1;
GLuint material_ubo = -1;
GLuint camera_ubo = -1;
namespace UboBinding
{
    int light = 2;
    int material = 3;
    int camera = 4;
}

enum PASS
{
    BACKGROUND, // Render background
    BACK_FACES, // Render mesh back faces and store eye-space depth
    FRONT_FACES, // Render front faces, compute eye-space depth
    DEFAULT,
    QUAD
};

float angle = 0.75f;
float scale = 0.5f;
bool recording = false;

void draw_gui(GLFWwindow* window)
{
   //Begin ImGui Frame
   ImGui_ImplOpenGL3_NewFrame();
   ImGui_ImplGlfw_NewFrame();
   ImGui::NewFrame();

   //Draw Gui
   ImGui::Begin("Debug window");                       
   if (ImGui::Button("Quit"))                          
   {
      glfwSetWindowShouldClose(window, GLFW_TRUE);
   }    

   const int filename_len = 256;
   static char video_filename[filename_len] = "capture.mp4";

   ImGui::InputText("Video filename", video_filename, filename_len);
   ImGui::SameLine();
   if (recording == false)
   {
      if (ImGui::Button("Start Recording"))
      {
         int w, h;
         glfwGetFramebufferSize(window, &w, &h);
         recording = true;
         start_encoding(video_filename, w, h); //Uses ffmpeg
      }
      
   }
   else
   {
      if (ImGui::Button("Stop Recording"))
      {
         recording = false;
         finish_encoding(); //Uses ffmpeg
      }
   }

   ImGui::SliderFloat("View angle", &angle, -glm::pi<float>(), +glm::pi<float>());
   ImGui::SliderFloat("Scale", &scale, -10.0f, +10.0f);

   ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
   ImGui::End();

   ImGui::Begin("Camera");

   ImGui::SliderFloat3("Camera Eye", &CameraData.eye.x, -10.0f, 10.0f);

   //Show fbo_tex for debugging purposes. This is highly recommended for multipass rendering.
   ImGui::Image((void*)fbo_tex, ImVec2(128.0f, 128.0f), ImVec2(0.0, 1.0), ImVec2(1.0, 0.0)); ImGui::SameLine();
   ImGui::Image((void*)depth_tex, ImVec2(128.0f, 128.0f), ImVec2(0.0, 1.0), ImVec2(1.0, 0.0)); // Show depth texture
   
   ImGui::End();

   ImGui::Begin("Shading");

   ImGui::SliderFloat3("Light Position", &LightData.light_w.x, -10.0f, 10.0f);

   ImGui::ColorEdit3("Base Color", &MaterialData.base_color.r, 0);
   ImGui::ColorEdit3("Specular Color", &MaterialData.spec_color.r, 0);
   ImGui::SliderFloat("Specular Factor", &MaterialData.spec_factor, 1.0f, 10.0f);
   ImGui::ColorEdit3("Background Color", &LightData.bg_color.r, 0);

   ImGui::End();

   //End ImGui Frame
   ImGui::Render();
   ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

/// <summary>
/// Draw transparent scene without FBO
/// </summary>
void DrawHackScene()
{
    glDisable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Draw background
    glUniform1i(UniformLocs::pass, BACKGROUND);
    glBindVertexArray(bg_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Draw cube
    glUniform1i(UniformLocs::pass, DEFAULT);
    glBindVertexArray(mesh_data.mVao);
    glDrawElements(GL_TRIANGLES, mesh_data.mSubmesh[0].mNumIndices, GL_UNSIGNED_INT, 0);
}

/// <summary>
/// Draw transparent scene with FBO
/// </summary>
void DrawScene()
{
    // Pass 0: Draw background
    glUniform1i(UniformLocs::pass, BACKGROUND);

    glBindFramebuffer(GL_FRAMEBUFFER, FBO); // Render to FBO
    glDrawBuffers(2, buffers); // Draw to color attachment 0 and 1
    
    glViewport(0, 0, screen_width, screen_height); // Change viewport size
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear FBO texture
    
    // Draw background quad
    glBindVertexArray(bg_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Pass 1: Draw cube back faces and store eye-space depth
    glUniform1i(UniformLocs::pass, BACK_FACES);
    glBindVertexArray(mesh_data.mVao);
    glDrawElements(GL_TRIANGLES, mesh_data.mSubmesh[0].mNumIndices, GL_UNSIGNED_INT, 0);

    // Pass 2: Draw cube front faces
    glUniform1i(UniformLocs::pass, FRONT_FACES);
    glDrawElements(GL_TRIANGLES, mesh_data.mSubmesh[0].mNumIndices, GL_UNSIGNED_INT, 0);

    // Render textured quad to back buffer
    glUniform1i(UniformLocs::pass, QUAD);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDrawBuffer(GL_BACK);
    
    glViewport(0, 0, screen_width, screen_height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear FBO texture

    glBindTextureUnit(0, fbo_tex); // Bind color texture
    glBindTextureUnit(1, depth_tex); // Bind depth texture

    glDisable(GL_DEPTH_TEST);

    glBindVertexArray(attribless_vao);
    draw_attribless_quad();
}

// This function gets called every time the scene gets redisplayed
void display(GLFWwindow* window)
{
    glm::mat4 M = glm::rotate(angle, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::scale(glm::vec3(scale * mesh_data.mScaleFactor));
    glm::mat4 V = glm::lookAt(glm::vec3(CameraData.eye), glm::vec3(0.0f), glm::vec3(CameraData.up));
    glm::mat4 P = glm::perspective(glm::pi<float>() / 4.0f, CameraData.resolution.z, 0.1f, 100.0f);

    glUseProgram(shader_program);

    //glBindTexture(2, texture_id);

    // Get location for shader uniform variable
    glm::mat4 PV = P * V;
    glUniformMatrix4fv(UniformLocs::PV, 1, false, glm::value_ptr(PV));
        
    glUniformMatrix4fv(UniformLocs::M, 1, false, glm::value_ptr(M));

    glBindBuffer(GL_UNIFORM_BUFFER, material_ubo); //Bind the OpenGL UBO before we update the data.
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(MaterialUniforms), &MaterialData); //Upload the new uniform values.

    glBindBuffer(GL_UNIFORM_BUFFER, light_ubo); //Bind the OpenGL UBO before we update the data.
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(LightUniforms), &LightData); //Upload the new uniform values.

    glBindBuffer(GL_UNIFORM_BUFFER, camera_ubo); //Bind the OpenGL UBO before we update the data.
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(CameraUniforms), &CameraData); //Upload the new uniform values.

    //DrawHackScene(); // Draw hack transparency scene
    DrawScene(); // Draw proper transparency scene

    if (recording == true)
    {
        glFinish();
        glReadBuffer(GL_BACK);
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        read_frame_to_encode(&rgb, &pixels, w, h);
        encode_frame(rgb);
    }

    draw_gui(window);

    /* Swap front and back buffers */
    glfwSwapBuffers(window);
}

void idle()
{
    float time_sec = static_cast<float>(glfwGetTime());

    //Pass time_sec value to the shaders
    //int time_loc = glGetUniformLocation(shader_program, "time");
    glUniform1f(UniformLocs::time, time_sec);
}

void reload_shader()
{
    GLuint new_shader = InitShader(vertex_shader.c_str(), fragment_shader.c_str());

    if (new_shader == -1) // loading failed
    {
        glClearColor(1.0f, 0.0f, 1.0f, 0.0f); //change clear color if shader can't be compiled
    }
    else
    {
        glClearColor(0.35f, 0.35f, 0.35f, 0.0f);

        if (shader_program != -1)
        {
            glDeleteProgram(shader_program);
        }
        shader_program = new_shader;
    }
}

//This function gets called when a key is pressed
void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    //std::cout << "key : " << key << ", " << char(key) << ", scancode: " << scancode << ", action: " << action << ", mods: " << mods << std::endl;

    if (action == GLFW_PRESS)
    {
        switch (key)
        {
        case 'r':
        case 'R':
            reload_shader();
            break;

        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            break;
        }
    }
}

//This function gets called when the mouse moves over the window.
void mouse_cursor(GLFWwindow* window, double x, double y)
{
    //std::cout << "cursor pos: " << x << ", " << y << std::endl;
}

//This function gets called when a mouse button is pressed.
void mouse_button(GLFWwindow* window, int button, int action, int mods)
{
    //std::cout << "button : "<< button << ", action: " << action << ", mods: " << mods << std::endl;
}

/// <summary>
/// Get the total screen size
/// </summary>
void GetScreenSize()
{
    // Get screen dimensions
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    screen_width = mode->width;
    screen_height = mode->height;
}

void resize(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height); // Set viewport to cover entire framebuffer
    CameraData.resolution = glm::vec4(width, height, width / height, 0.0f);
    //CameraData.aspect = float(width) / float(height); // Set aspect ratio
}

//Initialize OpenGL state. This function only gets called once.
void initOpenGL()
{
    glewInit();

#ifdef _DEBUG
    RegisterCallback();
#endif

    //Print out information about the OpenGL version supported by the graphics driver.	
    std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    reload_shader();
    mesh_data = LoadMesh(mesh_name);
    //texture_id = LoadTexture(texture_name);

    glGenVertexArrays(1, &attribless_vao);

    // For Color Attachment
    glGenTextures(1, &fbo_tex);
    glBindTexture(GL_TEXTURE_2D, fbo_tex); // Bind color texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screen_width, screen_height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0); // Use screen size
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Create depth texture
    glGenTextures(1, &depth_tex);
    glBindTexture(GL_TEXTURE_2D, depth_tex); // Bind depth texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, screen_width, screen_height, 0, GL_RED, GL_FLOAT, 0); // Use screen size
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture

    //Create the framebuffer object
    glGenFramebuffers(1, &FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo_tex, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, depth_tex, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    }
    else
    {
        std::cout << "Framebuffer complete!" << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0); // Unbind framebuffer

    // for MaterialUniforms
    glGenBuffers(1, &material_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, material_ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(MaterialUniforms), &MaterialData, GL_STREAM_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, UboBinding::material, material_ubo);

    // for LightUniforms
    glGenBuffers(1, &light_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, light_ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(LightUniforms), &LightData, GL_STREAM_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, UboBinding::light, light_ubo);

    // for CameraUniforms
    glGenBuffers(1, &camera_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, camera_ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(CameraUniforms), &CameraData, GL_STREAM_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, UboBinding::camera, camera_ubo);

    // For background
    glGenVertexArrays(1, &bg_vao);
    glGenBuffers(1, &bg_vbo);
    glBindVertexArray(bg_vao);

    glBindBuffer(GL_ARRAY_BUFFER, bg_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(bg_vertices), bg_vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

int main(int argc, char** argv)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
    {
        return -1;
    }

#ifdef _DEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(CameraData.resolution.x, CameraData.resolution.y, window_title, NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    //Register callback functions with glfw. 
    glfwSetKeyCallback(window, keyboard);
    glfwSetCursorPosCallback(window, mouse_cursor);
    glfwSetMouseButtonCallback(window, mouse_button);
    glfwSetFramebufferSizeCallback(window, resize);

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    GetScreenSize();

    initOpenGL();

    //Init ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 150");

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        idle();
        display(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    // Cleanup ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}