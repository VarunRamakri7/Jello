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

const char* const window_title = "Jello";

static const std::string vertex_shader("template_vs.glsl");
static const std::string fragment_shader("jello_fs.glsl");
GLuint shader_program = -1;

static const std::string mesh_name = "RubiksCube_01.obj";

MeshData mesh_data;

GLuint background_vao = -1;
GLuint background_vbo = -1;
const glm::vec3 background_vertices[4] =
{
    glm::vec3(1.0f, 1.0f, 0.0f),
    glm::vec3(1.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 1.0f, 0.0f)
};

struct CameraUniforms {
    glm::vec3 eye = glm::vec3(0.0f, 2.5f, 3.0f);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::ivec2 resolution = glm::ivec2(800, 800);
    float aspect = resolution.x / resolution.y;
}CameraData;

struct LightUniforms {
    glm::vec4 light_w = glm::vec4(-10.0f, 10.0f, 5.0f, 1.0f); // World-space light position
} LightData;

struct MaterialUniforms {
    glm::vec4 base_color = glm::vec4(0.35f, 0.35f, 0.35f, 1.0f); // base color
    glm::vec4 spec_color = glm::vec4(0.85f, 0.85f, 0.85f, 1.0f); // Specular Color
    float spec_factor = 2.0f; // Specular factor
} MaterialData;

//Locations for the uniforms which are not in uniform blocks
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
    int light = 0;
    int material = 1;
    int camera = 2;
}

enum PASS
{
    BACKGROUND, // Render background
    BACK_FACES, // Render mesh back faces and store eye-space depth
    FRONT_FACES, // Render front faces, compute eye-space depth
    DEFAULT
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

   ImGui::End();


   ImGui::Begin("Shading");

   ImGui::SliderFloat3("Light Position", &LightData.light_w.x, -10.0f, 10.0f);

   ImGui::ColorEdit3("Base Color", &MaterialData.base_color.r, 0);
   ImGui::ColorEdit3("Specular Color", &MaterialData.spec_color.r, 0);
   ImGui::SliderFloat("Specular Factor", &MaterialData.spec_factor, 1.0f, 10.0f);

   ImGui::End();

   //End ImGui Frame
   ImGui::Render();
   ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

// This function gets called every time the scene gets redisplayed
void display(GLFWwindow* window)
{
    //Clear the screen to the color previously specified in the glClearColor(...) call.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 M = glm::rotate(angle, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::scale(glm::vec3(scale * mesh_data.mScaleFactor));
    glm::mat4 V = glm::lookAt(CameraData.eye, glm::vec3(0.0f), CameraData.up);
    glm::mat4 P = glm::perspective(glm::pi<float>() / 4.0f, CameraData.aspect, 0.1f, 100.0f);

    glUseProgram(shader_program);

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

    glUniform1i(UniformLocs::pass, BACKGROUND);
    glBindVertexArray(background_vao);
    glDrawArrays(GL_POINTS, 0, 4);

    glUniform1i(UniformLocs::pass, DEFAULT);
    glBindVertexArray(mesh_data.mVao);
    glDrawElements(GL_TRIANGLES, mesh_data.mSubmesh[0].mNumIndices, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);

    draw_gui(window);

    if (recording == true)
    {
        glFinish();
        glReadBuffer(GL_BACK);
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        read_frame_to_encode(&rgb, &pixels, w, h);
        encode_frame(rgb);
    }

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
    std::cout << "key : " << key << ", " << char(key) << ", scancode: " << scancode << ", action: " << action << ", mods: " << mods << std::endl;

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

void resize(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height); // Set viewport to cover entire framebuffer
    CameraData.resolution = glm::ivec2(width, height);
    CameraData.aspect = float(width) / float(height); // Set aspect ratio
}

//Initialize OpenGL state. This function only gets called once.
void initOpenGL()
{
    glewInit();

    //Print out information about the OpenGL version supported by the graphics driver.	
    std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    //glEnable(GL_DEPTH_TEST);
    //glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    //glBlendFunc(GL_ONE, GL_SRC_COLOR);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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
    glGenBuffers(1, &background_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, background_vbo);
    glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(glm::vec3), background_vertices, GL_STREAM_DRAW);
    glBindBufferBase(GL_ARRAY_BUFFER, 0, background_vbo);

    glGenVertexArrays(1, &background_vao);
    glBindVertexArray(background_vao);

    glBindBuffer(GL_ARRAY_BUFFER, background_vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);
    
    // Unbing VAO, and VBO
    //glEnableVertexAttribArray(0);
    //glBindBuffer(GL_ARRAY_BUFFER, 0);

    reload_shader();

    mesh_data = LoadMesh(mesh_name);
}

int main(int argc, char** argv)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
    {
        return -1;
    }

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