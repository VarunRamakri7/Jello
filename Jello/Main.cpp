#include <windows.h>

#include "..\imgui-master\imgui.h"
#include "..\imgui-master\backends\imgui_impl_glfw.h"
#include "..\imgui-master\backends\imgui_impl_opengl3.h"

#include "DebugCallback.h" // Functions for debugging glsl
#include "AttriblessRendering.h"

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
#include "trackball.h"
#include "BoundingBox.h"
#include "Physics.h"
#include "Plate.h"

#include <glm/gtx/string_cast.hpp> // for debug

#include "Cube.h"

const char* const window_title = "Jello";

int screen_width; // max for resize
int screen_height;

static const std::string vertex_shader("template_vs.glsl");
static const std::string fragment_shader("jello_fs.glsl");
static const std::string debug_vertex_shader("debug_vs.glsl");
static const std::string debug_fragment_shader("template_fs.glsl");

GLuint shader_program = -1;
GLuint debug_shader_program = -1;
float time_sec;

GLuint FBO; // Frame buffer object
GLuint fbo_tex = -1; // Color FBO texture
GLuint depth_tex = -1; // Depth FBO texture
GLenum buffers[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
float clear[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

GLuint attribless_vao = -1;

GLuint rectVAO;
GLuint rectVBO;

static const std::string mesh_name = "cube.obj";

MeshData mesh_data;

GLuint bg_vao = -1;
GLuint bg_vbo = -1;
const float bg_vertices[] =
{
    // Coords       // Tex coords
    1.0f, -1.0f,    1.0f, 0.0f,
    -1.0f, -1.0f,   0.0f, 0.0f,
    -1.0f, 1.0f,    0.0f, 1.0f,

    1.0f, 1.0f,     1.0f, 1.0f,
    1.0f, -1.0f,    1.0f, 0.0f,
    -1.0f, 1.0f,    0.0f, 1.0f
};

float cameraNear = 0.01f;
float cameraFar = 100.f;
struct CameraUniforms {
    glm::vec4 eye = glm::vec4(0.0f, 2.5f, 5.0f, 0.0f);
    glm::vec4 up = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
    glm::vec4 resolution = glm::vec4(800.0f, 800.0f, 1.0f, 0.0f); // Width, Height, Aspect Ratio
}CameraData;

struct LightUniforms {
    glm::vec4 light_w = glm::vec4(0.0f, 5.0f, 5.0f, 1.0f); // World-space light position
    glm::vec4 bg_color = glm::vec4(1.0f, 0.67f, 0.67f, 1.0f); // Background color
} LightData;

struct MaterialUniforms {
    glm::vec4 base_color = glm::vec4(0.75f, 0.75f, 0.75f, 1.0f); // base color
    glm::vec4 spec_color = glm::vec4(0.85f, 0.85f, 0.85f, 1.0f); // Specular Color
    float spec_factor = 0.1f; // Specular factor
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
    QUAD // Textured quad
};

float angle = 0.75f;
float scale = 0.35f;

bool recording = false;

// interactive
TrackBallC trackball;
bool mouseLeft, mouseMid, mouseRight;
GLdouble mouseX, mouseY; // current mouse positions
glm::vec2 mouseClickedPos; // stored mouse position on click
float mouseClickTime = 0.0f; // track time elapsed since clicked
glm::vec2 dragV = glm::vec2(0.0); // drag acceleration 
bool moveCam = false; // if user holds "c" on the keyboard, will make this true -> moves camera
bool movePlate = false; // if user holds "p" on the keyboard, will make this true -> moves plate
// if user drags mouse will apply acceleration force on jello

// display
bool showDiscrete = false;
bool showBB = true; // show bounding box
bool debugMode = false;

// physics
glm::vec3 gravity = glm::vec3(0.0, -9.8, 0.0); // acceleration 
bool addGravity = false;
enum integratorEnum {
    EULER, RK4
}; // euler = 0 , RK4 = 1
int integrator = integratorEnum::EULER;
// float values for it to be adjustable with ImGui
float fStiffness = 50.f;
float fDamping = 0.5f;
float fMass = 1.0f;
float fTimeStep = 0.005f;
int cubeResolution = 2;
bool cubeFixedFloor = true;
bool cubeStructuralSpring = true;
bool cubeShearSpring = true;
bool cubeBendSpring = true;

bool needReset = false;

// scene
Cube* myCube;
Plate* myPlate;
BoundingBox* boundingBox;
glm::dvec3 initPlatePos = glm::dvec3(0.5, 0.0, 0.5);


/*
 * Draws the GUI with ImGui
 */
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

   // display
   ImGui::Text("Display");
   ImGui::Checkbox("Debug Mode", &debugMode);
   ImGui::Checkbox("Show discrete", &showDiscrete);
   ImGui::Checkbox("Show bounding box", &showBB);
   ImGui::SliderInt("Particle Size", &myCube->pointSize, 1, 10);
   ImGui::Checkbox("Visualize Springs", &myCube->showSpring);

   // physics
   ImGui::Text("Physics");
   ImGui::SliderFloat("Stiffness", &fStiffness, 0.0f, 100.0f);
   ImGui::SliderFloat("Damping", &fDamping, 0.0, 1.0f);
   ImGui::SliderFloat("Mass", &fMass, 0.0f, 50.0f);
 
   ImGui::Text("Jello");
   // store and submit on reset 
   ImGui::SliderInt("Jello Resolution", &cubeResolution, 1, 8);
   ImGui::Checkbox("On Plate", &cubeFixedFloor);
   ImGui::Checkbox("Structural Spring", &cubeStructuralSpring);
   ImGui::Checkbox("Shear Spring", &cubeShearSpring);
   ImGui::Checkbox("Bend Spring", &cubeBendSpring);
   ImGui::Checkbox("Add Gravity", &addGravity);

   needReset = ImGui::Button("Reset");

   ImGui::Text("Integrators");
   ImGui::RadioButton("Euler", &integrator, integratorEnum::EULER);
   ImGui::RadioButton("RK4", &integrator, integratorEnum::RK4);
   ImGui::SliderFloat("TimeStep", &fTimeStep, 0.001, 0.01);

   ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
   ImGui::End();

   ImGui::Begin("Camera");

   //ImGui::SliderFloat3("Camera Eye", &CameraData.eye.x, -10.0f, 10.0f);

   ImGui::Image((void*)fbo_tex, ImVec2(128.0f, 128.0f), ImVec2(0.0, 1.0), ImVec2(1.0, 0.0)); ImGui::SameLine(); // Show FBO texture
   ImGui::Image((void*)depth_tex, ImVec2(128.0f, 128.0f), ImVec2(0.0, 1.0), ImVec2(1.0, 0.0)); // Show depth texture
   
   ImGui::End();

   ImGui::Begin("Shading");

   ImGui::SliderFloat3("Light Position", &LightData.light_w.x, -10.0f, 10.0f);

   ImGui::ColorEdit3("Base Color", &MaterialData.base_color.r, 0);
   ImGui::ColorEdit3("Specular Color", &MaterialData.spec_color.r, 0);
   ImGui::SliderFloat("Specular Factor", &MaterialData.spec_factor, 0.1f, 1.0f);
   ImGui::ColorEdit3("Background Color", &LightData.bg_color.r, 0);

   ImGui::End();

   //End ImGui Frame
   ImGui::Render();
   ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

/*
* Converts screen space mouse pixel coordinates to world space 
*/
void getWorld(glm::vec2 mouse, glm::vec4& world) {
    // get depth (should be constant)
    float depth;
    glReadPixels(mouse[0], mouse[1], 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);

    // scale to NDC: -1 <> 1
    float x_NDC = (mouse[0] * 2.0f / CameraData.resolution.x) - 1.0f;
    float y_NDC = (mouse[1] * 2.0f / CameraData.resolution.y) - 1.0f;
    float z_NDC = (depth - cameraNear) / (cameraFar - cameraNear);
    z_NDC = 2 * z_NDC - 1; 
    // into homogeneous coord
    glm::vec4 ndcCoord = glm::vec4(x_NDC, y_NDC, z_NDC, 1.0f);
    // clip space
    glm::vec4 clip = ndcCoord / ndcCoord.w;

    // camera always looking at the center 
    glm::mat4 V = glm::lookAt(glm::vec3(CameraData.eye), glm::vec3(0.0f), glm::vec3(CameraData.up));
    glm::mat4 P = glm::perspective(glm::pi<float>() / 4.0f, CameraData.resolution.z, cameraNear, cameraFar);
    glm::mat4 PV = P * V;// *trackball.Get3DViewCameraMatrix();
    // TODO get plate model's mat? but now its identity cos we change hte pixels 
    glm::mat4 invPV = glm::inverse(PV);

    world = invPV * clip;
}

//set the callbacks for the virtual trackball
//this is executed when the mouse is moving
void MouseCallback(GLFWwindow* window, double x, double y) {
    //do not forget to pass the events to ImGUI!
    ImGuiIO& io = ImGui::GetIO();
    io.AddMousePosEvent(x, y);
    if (io.WantCaptureMouse) return; //make sure you do not call this callback when over a menu
    //now process them
    mouseX = x;
    mouseY = y;

    //we need to perform an action only if a button is pressed
    if (mouseLeft)  trackball.Rotate(mouseX, mouseY);
    if (mouseMid)   trackball.Translate(mouseX, mouseY);
    if (mouseRight) trackball.Zoom(mouseX, mouseY);
}

//set the variables when the button is pressed or released
void MouseButtonCallback(GLFWwindow* window, int button, int state, int mods) {
    //do not forget to pass the events to ImGUI!

    ImGuiIO& io = ImGui::GetIO();
    io.AddMouseButtonEvent(button, state);
    if (io.WantCaptureMouse) return; //make sure you do not call this callback when over a menu

    //process them
    if (button == GLFW_MOUSE_BUTTON_LEFT && state == GLFW_PRESS)
    {
        if (moveCam) {
            trackball.Set(window, true, mouseX, mouseY);
        }

        mouseLeft = true;
        mouseClickTime = time_sec;
        mouseClickedPos = glm::vec2(mouseX, mouseY); // store clicked pos
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && state == GLFW_RELEASE)
    {
        if (moveCam) {
            trackball.Set(window, false, mouseX, mouseY);
        }
        mouseLeft = false;
        dragV = glm::vec2(0.0); //reset
    }
    if (button == GLFW_MOUSE_BUTTON_MIDDLE && state == GLFW_PRESS)
    {
        if (moveCam) {
            trackball.Set(window, true, mouseX, mouseY);
        }
        mouseMid = true;
    }
    if (button == GLFW_MOUSE_BUTTON_MIDDLE && state == GLFW_RELEASE)
    {
        if (moveCam) {
            trackball.Set(window, true, mouseX, mouseY);
        }
        mouseMid = false;
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && state == GLFW_PRESS)
    {
        if (moveCam) {
            trackball.Set(window, true, mouseX, mouseY);
        }
        mouseRight = true;
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && state == GLFW_RELEASE)
    {
        if (moveCam) {
            trackball.Set(window, true, mouseX, mouseY);
        }
        mouseRight = false;
    }
}

/// <summary>
/// Draw transparent scene without FBO
/// </summary>
void DrawHackScene(){
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
    const glm::mat4 M = glm::rotate(angle, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::scale(glm::vec3(scale * myCube->scale));
    const glm::mat4 V = glm::lookAt(glm::vec3(CameraData.eye), glm::vec3(0.0f), glm::vec3(CameraData.up));
    const glm::mat4 P = glm::perspective(glm::pi<float>() / 4.0f, CameraData.resolution.z, cameraNear, cameraFar);
    const glm::mat4 PV = P * V * trackball.Get3DViewCameraMatrix();

    // Get location for shader uniform variable

    glUniformMatrix4fv(UniformLocs::PV, 1, false, glm::value_ptr(PV));
    glUniformMatrix4fv(UniformLocs::M, 1, false, glm::value_ptr(M));

    // for debugging
    //glUniform1i(UniformLocs::pass, FRONT_FACES);
    //myCube->render(1, showDiscrete, drawType::DRAWTRI);
    //return;

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
    myCube->render(1, showDiscrete, drawType::DRAWTRI);

    // Pass 2: Draw cube front faces
    glUniform1i(UniformLocs::pass, FRONT_FACES);
    myCube->render(1, showDiscrete, drawType::DRAWTRI);

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
    glViewport(0, 0, screen_width, screen_height);
    draw_attribless_quad();

    glEnable(GL_DEPTH_TEST);

    if (debugMode) {
        glUseProgram(debug_shader_program);
        // draw onto viewport's regular size (window size) 
        // actually should draw to max size

        glViewport(0, 0, CameraData.resolution.x, CameraData.resolution.y);
        /*int PVM_loc = glGetUniformLocation(debug_shader_program, "PVM");
        glUniformMatrix4fv(PVM_loc, 1, false, glm::value_ptr(PVM));*/

        // draw points on top 
        glUniformMatrix4fv(UniformLocs::PV, 1, false, glm::value_ptr(PV));
        glUniformMatrix4fv(UniformLocs::M, 1, false, glm::value_ptr(M));

        myCube->render(1, showDiscrete, drawType::DRAWPOINT);
    }

}

// This function gets called every time the scene gets redisplayed
void display(GLFWwindow* window)
{
   //Clear the screen to the color previously specified in the glClearColor(...) call.
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   
   glUseProgram(shader_program);

   //if (showBB) {
   //    boundingBox->render(1);
   //}

   //myCube->render(1, showDiscrete);
   //if (myCube->fixedFloor) {
   //    // show plate
   //    myPlate->render(1);
   //}

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

   time_sec = static_cast<float>(glfwGetTime());

   // only if not moving camera
   if (mouseLeft && !moveCam) {
       // dragging mouse

       // calculate acceleration in world space
       glm::vec4 clickedW;
       glm::vec4 currentW;
       getWorld(glm::vec2(mouseX, mouseY), currentW);
       // move plate
       if (movePlate) {
           myPlate->setPosition(glm::vec3(currentW.x, 0.0f, 0.0f));
       }
       else {
           // acting as acceleration force
           getWorld(mouseClickedPos, clickedW);

           float timeDiff = time_sec - mouseClickTime;
           if (timeDiff > 0.01) {
               // drag : velocity = changeP/timeDiff -> acceleration /timeDiff again 
               dragV = (currentW - clickedW) / (timeDiff * timeDiff);
           }
       }
   }

   if (needReset) {
       // reset simulation
       std::cout << "RESETTING" << std::endl;

       // send cube values
       myCube->stiffness = (-1.0) * double(fStiffness); // negate
       myCube->damping = (-1.0) * double(fDamping); // negate
       myCube->mass = double(fMass);
       myCube->resolution = cubeResolution;
       myCube->structuralSpring = cubeStructuralSpring;
       myCube->shearSpring = cubeShearSpring;
       myCube->bendSpring = cubeBendSpring;
       myCube->fixedFloor = cubeFixedFloor;
       myCube->reset();
       myPlate->setPosition(initPlatePos);
       // need to reconstrain since new masspoints are created
       // TODO can we just reset mass point locs? 
       if (myCube->fixedFloor) {
           myPlate->setConstraintPoints(myCube->bottomFace);
       }
      
   }

    glUniform1f(UniformLocs::time, time_sec);

}

void reload_shader()
{
    GLuint new_shader = InitShader(vertex_shader.c_str(), fragment_shader.c_str());
    GLuint new_debug_shader = InitShader(debug_vertex_shader.c_str(), debug_fragment_shader.c_str());

    if (new_shader == -1 || new_debug_shader == -1) // loading failed
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
        if (debug_shader_program != -1)
        {
            glDeleteProgram(debug_shader_program);
        }
        shader_program = new_shader;
        debug_shader_program = new_debug_shader;
    }
}

//This function gets called when a key is pressed
void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{

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

         case GLFW_KEY_C:
             moveCam = true;
             break;

         case GLFW_KEY_P:
             movePlate = true;
             break;
      }
   }
   else if (action == GLFW_RELEASE) {
       switch (key)
       {
       case GLFW_KEY_C:
           moveCam = false;
           break;

       case GLFW_KEY_P:
           movePlate = false;
           break;
       }
   }
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
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    reload_shader();

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
    // why need the next 2 lines? 
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}


void buildScene() {
    // build scene
    myCube = new Cube(4); // initial cube resolution = 2 
    myCube->setSpringMode(true, true, true);
    boundingBox = new BoundingBox(5, 5, 5, glm::vec3(-2, 2, 2.0f));
    myPlate = new Plate(initPlatePos, 2.0);
    if (myCube->fixedFloor) {
        myPlate->setConstraintPoints(myCube->bottomFace);
    }
}


//C++ programs start executing in the main() function.
int main(int argc, char **argv){

    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
    {
        return -1;
    }
    #ifdef _DEBUG
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    #endif

    // negotiate with the OpenGL
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(CameraData.resolution.x, CameraData.resolution.y, window_title, NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);
    //Register callback functions with glfw. 
    glfwSetKeyCallback(window, keyboard);
    glfwSetCursorPosCallback(window, MouseCallback);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    glfwSetFramebufferSizeCallback(window, resize);

    GetScreenSize();
    initOpenGL();
    buildScene();

    //Init ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 150");

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        idle();

        // physics
        if (movePlate == false) {
            // acting as external acceleration force
            myCube->setExternalForce(addGravity ? glm::vec3(dragV, 0.0) + gravity : glm::vec3(dragV, 0.0));
        }

        if (integrator == integratorEnum::EULER) {
            integrateEuler(myCube, double(fTimeStep));
        }
        else if (integrator == integratorEnum::RK4) {
            integrateRK4(myCube, double(fTimeStep));
        }

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
