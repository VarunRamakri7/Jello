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
#include <algorithm>

#include "InitShader.h"    //Functions for loading shaders from text files
#include "trackball.h"
#include "BoundingBox.h"
#include "Physics.h"
#include "Plate.h"
#include "Cube.h"

#include <glm/gtx/string_cast.hpp> // for debug


const char* const window_title = "Jello";

static const std::string vertex_shader("jello_vs.glsl");
static const std::string fragment_shader("jello_fs.glsl");
static const std::string debug_vertex_shader("debug_vs.glsl");
static const std::string debug_fragment_shader("debug_fs.glsl");

// SCENE
Cube* myCube;
Plate* myPlate;
BoundingBox* boundingBox;
glm::vec3 initPlatePos = glm::vec3(0.0f, 0.0f, 0.5f);
glm::vec3 initCubePos = glm::vec3(-0.5f, 0.0f, 0.5f);
glm::vec4 initCamPos = glm::vec4(0.0f, 2.5f, 5.0f, 1.0f); 

// RENDER
GLuint shader_program = -1; // to draw jello
GLuint debug_shader_program = -1; // to visualize masspoints and bounding box
GLuint FBO; // Frame buffer object
GLuint fbo_tex = -1; // Color FBO texture
GLuint depth_tex = -1; // Depth FBO texture
GLenum buffers[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
float clear[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

GLuint attribless_vao = -1;

GLuint rectVAO;
GLuint rectVBO;

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

// camera == window
// window width and height is the camera's resolution
float cameraNear = 0.01f;
float cameraFar = 100.f;
struct CameraUniforms {
    glm::vec4 eye = initCamPos;
    glm::vec4 up = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
    glm::vec4 resolution = glm::vec4(800.0f, 800.0f, 1.0f, 0.0f); // Width, Height, Aspect Ratio
    glm::ivec2 screen = glm::ivec2(0.0f, 0.0f);
}CameraData;

struct LightUniforms {
    glm::vec4 light_w = glm::vec4(0.0f, 5.0f, 5.0f, 1.0f); // World-space light position
    glm::vec4 bg_color = glm::vec4(1.0f, 0.9f, 0.9f, 1.0f); // Background color
} LightData;

struct MaterialUniforms {
    glm::vec4 base_color = glm::vec4(1.0f, 0.85f, 0.85f, 1.0f); // base color
    glm::vec4 spec_color = glm::vec4(0.85f, 0.85f, 0.85f, 1.0f); // Specular Color
    glm::vec4 absorption = glm::vec4(0.4f, 0.4f, 0.1f, 0.1f); // x,y,z are absorbption, z is specular factor
} MaterialData;

// Locations for the uniforms which are not in uniform blocks
namespace UniformLocs
{
    int M = 0;
    int PV = 1;
    int V = 2;
    int time = 3;
    int pass = 4;
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

// INTERACTIVE
float time_sec;

TrackBallC trackball;
bool mouseLeft, mouseMid, mouseRight;
double mouseX, mouseY; // current mouse positions
glm::vec2 mouseClickedPos; // stored mouse position on click
float mouseClickTime = 0.0f; // track time elapsed since clicked

glm::vec3 externalForce = glm::vec3(0.0); // total drag acceleration 

bool moveCam = false; // if user holds "c" on the keyboard, will make this true -> moves camera
bool movePlate = false; // if user holds "p" on the keyboard, will make this true -> moves plate

// if user drags mouse will apply acceleration force on jello
float forceDamping = 0.3f; 

// DISPLAY MODES
bool showDiscrete = false;
bool showSpring = false;
bool showBB = true; // show bounding box
bool debugMode = false;

// PHYSICS
glm::vec3 gravity = glm::vec3(0.0, -9.8, 0.0); // acceleration 
bool addGravity = false;

enum integratorEnum {
    EULER, RK4
}; // euler = 0 , RK4 = 1
int integrator = integratorEnum::EULER;

// float values for it to be adjustable with ImGui
float fTimeStep = 0.005f;
int cubeResolution = 2;
bool cubeFixedFloor = true;
bool cubeStructuralSpring = true;
bool cubeShearSpring = true;
bool cubeBendSpring = true;

bool needReset = false;
bool needCamReset = false;


// helper functions
void clamp(double min, double max, double& value) {
    value = std::min(value, max);
    value = std::max(value, min);
}
void clamp(float min, float max, float& value) {
    value = std::min(value, max);
    value = std::max(value, min);
}

/*
 * Draws the GUI with ImGui
 */
void draw_gui(GLFWwindow* window)
{
   // Begin ImGui Frame
   ImGui_ImplOpenGL3_NewFrame();
   ImGui_ImplGlfw_NewFrame();
   ImGui::NewFrame();

   // Draw Gui
   ImGui::Begin("Control Panel");

   // Instructions
   ImGui::Separator();
   ImGui::Text("Hold P + shake cursor to shake plate");
   ImGui::Text("Hold C + left mouse to rotate camera");
   ImGui::Text("Hold C + middle mouse to pan camera");
   ImGui::Text("Hold C + right mouse to zoom");

   // Shading
   ImGui::Separator();
   ImGui::Text("SHADING");
   ImGui::SliderFloat3("Light Position", &LightData.light_w.x, -10.0f, 10.0f);
   ImGui::ColorEdit3("Base Color", &MaterialData.base_color.r, 0);
   ImGui::ColorEdit4("Absorbption", &MaterialData.absorption.r, 0);
   ImGui::ColorEdit3("Specular Color", &MaterialData.spec_color.r, 0);
   ImGui::ColorEdit3("Background Color", &LightData.bg_color.r, 0);
 
   // Jello
   ImGui::Separator();
   ImGui::Text("JELLO");
   ImGui::SliderInt("Jello Resolution", &cubeResolution, 2, 8);
   ImGui::Checkbox("On Plate", &cubeFixedFloor);
   ImGui::Checkbox("Structural Spring", &cubeStructuralSpring);
   ImGui::Checkbox("Shear Spring", &cubeShearSpring);
   ImGui::Checkbox("Bend Spring", &cubeBendSpring);
   ImGui::Checkbox("Add Gravity", &addGravity);

   // Physics
   ImGui::Separator();
   ImGui::Text("PHYSICS");
   ImGui::SliderFloat("Stiffness", &myCube->stiffness, 0.0f, 2000.0f);
   ImGui::SliderFloat("Damping", &myCube->damping, 0.0, 10.0f);
   ImGui::SliderFloat("Mass", &myCube->mass, 1.0f, 50.0f); // cannot be 0
   needReset = ImGui::Button("Reset Simulation"); // reset simulation

   // Display
   ImGui::Separator();
   ImGui::Text("DISPLAY");
   ImGui::Checkbox("Debug Mode", &debugMode);
   if (debugMode) {
       // show other debug options
       ImGui::Checkbox("Show discrete", &showDiscrete);
       ImGui::Checkbox("Show bounding box", &showBB);
       ImGui::SliderInt("Particle Size", &myCube->pointSize, 1, 10);
       ImGui::Checkbox("Visualize Springs", &showSpring);
   }
   // Reset camera position
   needCamReset = ImGui::Button("Camera to Origin");

   // integrators
   ImGui::Separator();
   ImGui::Text("INTEGRATORS");
   ImGui::RadioButton("Euler", &integrator, integratorEnum::EULER);
   ImGui::RadioButton("RK4", &integrator, integratorEnum::RK4);
   ImGui::SliderFloat("TimeStep", &fTimeStep, 0.001f, 0.01f);

   ImGui::Separator();
   if (ImGui::Button("Quit"))
   {
       glfwSetWindowShouldClose(window, GLFW_TRUE);
   }

   ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
   ImGui::End();

   /* for Debugging camera
   ImGui::Begin("Camera");
   ImGui::Image((void*)fbo_tex, ImVec2(128.0f, 128.0f), ImVec2(0.0, 1.0), ImVec2(1.0, 0.0)); ImGui::SameLine(); // Show FBO texture
   ImGui::Image((void*)depth_tex, ImVec2(128.0f, 128.0f), ImVec2(0.0, 1.0), ImVec2(1.0, 0.0)); // Show depth texture
   ImGui::End();
   */

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
    glm::mat4 P = glm::perspective(glm::pi<float>() / 4.0f, float(CameraData.resolution.x)/ float(CameraData.resolution.y), cameraNear, cameraFar);
    glm::mat4 PV = P * V;
    glm::mat4 invPV = glm::inverse(PV);

    world = invPV * clip;
}

// set the callbacks for the virtual trackball
// this is executed when the mouse is moving
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

// set the variables when the button is pressed or released
void MouseButtonCallback(GLFWwindow* window, int button, int state, int mods) {
    //do not forget to pass the events to ImGUI!

    ImGuiIO& io = ImGui::GetIO();
    io.AddMouseButtonEvent(button, state);
    if (io.WantCaptureMouse) return; // make sure you do not call this callback when over a menu

    // process them
    if (button == GLFW_MOUSE_BUTTON_LEFT && state == GLFW_PRESS)
    {
        if (moveCam) {
            trackball.Set(window, true, mouseX, mouseY);
        }

        if (movePlate == false) {
            // store clicked position and time 
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

        // get position offset
        // vector from clicked to current in world space
        clamp(0.0, double(CameraData.resolution.y), mouseY);
        clamp(0.0, double(CameraData.resolution.x), mouseX);
        
        glm::vec2 posOffset = glm::vec2(mouseX, mouseY) - mouseClickedPos;

        if (movePlate == false){
            // accumulate external acceleration force

            // get time offset
            float timeDiff = time_sec - mouseClickTime;

            // acting as acceleration force
            if (timeDiff > 0.01) {
                // drag : velocity = changeP/timeDiff -> acceleration /timeDiff again 
                glm::vec2 dragV = posOffset / (timeDiff * timeDiff);
                posOffset *= 0.1; // damp
                clamp(-50.0f, 50.0f, posOffset.x); // clamp force so it wont flip the jelly 
                clamp(-50.0f, 50.0f, posOffset.y); 

                float magnitude = glm::length(posOffset) / (timeDiff * timeDiff);
                // add force depending on camera's position 
                glm::vec4 y = glm::vec4(posOffset, 0,0) * glm::vec4(1, 1, 0, 0) * trackball.Get3DViewCameraMatrix();
                glm::vec4 x = posOffset.y * glm::vec4(1, 0, 0, 0) * trackball.Get3DViewCameraMatrix();
                y = y / (timeDiff * timeDiff);
                x = x / (timeDiff * timeDiff);
                externalForce += glm::vec3(y); 
            }
        }

        mouseLeft = false;
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
   /* glUniform1i(UniformLocs::pass, DEFAULT);
    glBindVertexArray(mesh_data.mVao);
    glDrawElements(GL_TRIANGLES, mesh_data.mSubmesh[0].mNumIndices, GL_UNSIGNED_INT, 0);*/
}

/// <summary>
/// Draw transparent scene with FBO
/// </summary>
void DrawScene()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear FBO texture

    const glm::mat4 V = glm::lookAt(glm::vec3(CameraData.eye), glm::vec3(0.0f), glm::vec3(CameraData.up));
    const glm::mat4 P = glm::perspective(glm::pi<float>() / 4.0f, float(CameraData.resolution.x) / float(CameraData.resolution.y), cameraNear, cameraFar);
    const glm::mat4 PV = P * V * trackball.Get3DViewCameraMatrix();

    glUseProgram(shader_program);
    glViewport(0, 0, CameraData.screen.x, CameraData.screen.y); // Change viewport size

    // Get location for shader uniform variable
    glUniformMatrix4fv(UniformLocs::PV, 1, false, glm::value_ptr(PV));
    glUniformMatrix4fv(UniformLocs::V, 1, false, glm::value_ptr(V));
    // cube passes its own M matrix

    // Pass 0: Draw background
    glUniform1i(UniformLocs::pass, BACKGROUND);

    glBindFramebuffer(GL_FRAMEBUFFER, FBO); // Render to FBO
    glDrawBuffers(2, buffers); // Draw to color attachment 0 and 1
    
    glViewport(0, 0, CameraData.screen.x, CameraData.screen.y); // Change viewport size
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear FBO texture
    
    // Draw background quad
    glBindVertexArray(bg_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // draw plate with debug line shader
    if (myCube->fixedFloor) {
        // show plate
        glUseProgram(debug_shader_program);
        glUniformMatrix4fv(UniformLocs::PV, 1, false, glm::value_ptr(PV));
        myPlate->render(UniformLocs::M);
    }

    glUseProgram(shader_program);
    // Pass 1: Draw cube back faces and store eye-space depth
    glUniform1i(UniformLocs::pass, BACK_FACES);
    myCube->render(UniformLocs::M, showDiscrete, showSpring, debugMode);

    // Pass 2: Draw cube front faces
    glUniform1i(UniformLocs::pass, FRONT_FACES);
    myCube->render(UniformLocs::M, showDiscrete, showSpring, debugMode);

    // Render textured quad to back buffer
    glUniform1i(UniformLocs::pass, QUAD);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDrawBuffer(GL_BACK);
    
    glViewport(0, 0, CameraData.screen.x, CameraData.screen.y);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear FBO texture

    glBindTextureUnit(0, fbo_tex); // Bind color texture
    glBindTextureUnit(1, depth_tex); // Bind depth texture

    glDisable(GL_DEPTH_TEST);

    glBindVertexArray(attribless_vao);
    glViewport(0, 0, CameraData.screen.x, CameraData.screen.y);
    draw_attribless_quad();

    if (debugMode) {

        glUseProgram(debug_shader_program);
        glUniformMatrix4fv(UniformLocs::PV, 1, false, glm::value_ptr(PV));

        glViewport(0, 0, CameraData.resolution.x, CameraData.resolution.y);

        // draw points on top 
        myCube->render(UniformLocs::M, showDiscrete, showSpring, debugMode);

        if (showBB) {
            // draw bounding box
            boundingBox->render(UniformLocs::M);
        }
    }

}

// This function gets called every time the scene gets redisplayed
void display(GLFWwindow* window)
{
    //Clear the screen to the color previously specified in the glClearColor(...) call.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   
    glUseProgram(shader_program); 

    glBindBuffer(GL_UNIFORM_BUFFER, material_ubo); //Bind the OpenGL UBO before we update the data.
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(MaterialUniforms), &MaterialData); //Upload the new uniform values.

    glBindBuffer(GL_UNIFORM_BUFFER, light_ubo); //Bind the OpenGL UBO before we update the data.
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(LightUniforms), &LightData); //Upload the new uniform values.

    glBindBuffer(GL_UNIFORM_BUFFER, camera_ubo); //Bind the OpenGL UBO before we update the data.
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(CameraUniforms), &CameraData); //Upload the new uniform values.

    //DrawHackScene(); // Draw hack transparency scene
    DrawScene(); // Draw proper transparency scene

    draw_gui(window);

    /* Swap front and back buffers */
    glfwSwapBuffers(window);
}

void idle()
{

   time_sec = static_cast<float>(glfwGetTime());

   // only if not moving camera
   if (mouseLeft && movePlate) {
       // dragging mouse = drag plate
       clamp(0.0, double(CameraData.resolution.y), mouseY);
       clamp(0.0, double(CameraData.resolution.x), mouseX);
       glm::vec4 currentW;
       getWorld(glm::vec2(mouseX, mouseY), currentW);
       myPlate->setPosition(glm::vec3(currentW.x, 0.0f, 0.0f), fTimeStep);
   }

   // reset button pressed or if values changed and needs to be resetted
   if (needReset || myCube->resolution != cubeResolution || myCube->structuralSpring != cubeStructuralSpring ||
       myCube->shearSpring != cubeShearSpring || myCube->bendSpring != cubeBendSpring || myCube->fixedFloor != cubeFixedFloor) {
       // reset simulation
       std::cout << "RESETTING" << std::endl;

       // send cube values
       myCube->resolution = cubeResolution;
       myCube->structuralSpring = cubeStructuralSpring;
       myCube->shearSpring = cubeShearSpring;
       myCube->bendSpring = cubeBendSpring;
       myCube->fixedFloor = cubeFixedFloor;
       myCube->reset();
       myPlate->setPosition(initPlatePos, fTimeStep);

       // need to reconstrain since new masspoints are created
       if (myCube->fixedFloor) {
           myPlate->setConstraintPoints(myCube->bottomFace);
       }
   }

    // camera
    if (needCamReset) {
        CameraData.eye = initCamPos;
        trackball = TrackBallC();
    }
   
    // physics
    myCube->setExternalForce(addGravity ? externalForce + gravity : externalForce);
    externalForce *= forceDamping;
    glUniform1f(UniformLocs::time, time_sec);

}

void reload_shader()
{
    GLuint new_shader = InitShader(vertex_shader.c_str(), fragment_shader.c_str());
    GLuint new_debug_shader = InitShader(debug_vertex_shader.c_str(), debug_fragment_shader.c_str());

    if (new_shader == -1 || new_debug_shader == -1) // loading failed
    {
        glClearColor(1.0f, 0.0f, 1.0f, 0.0f); // change clear color if shader can't be compiled
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

// This function gets called when a key is pressed
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
    CameraData.screen.x = mode->width;
    CameraData.screen.y = mode->height;
}

void resize(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height); // Set viewport to cover entire framebuffer
    CameraData.resolution = glm::vec4(width, height, float(width) / float(height), 0.0f);
}

// Initialize OpenGL state. This function only gets called once.
void initOpenGL()
{
    glewInit();

    // Print out information about the OpenGL version supported by the graphics driver.	
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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, CameraData.screen.x, CameraData.screen.y, 0, GL_RGB, GL_UNSIGNED_BYTE, 0); // Use screen size
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Create depth texture
    glGenTextures(1, &depth_tex);
    glBindTexture(GL_TEXTURE_2D, depth_tex); // Bind depth texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, CameraData.screen.x, CameraData.screen.y, 0, GL_RED, GL_FLOAT, 0); // Use screen size
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
    myCube = new Cube(2, initCubePos, shader_program, debug_shader_program); // initial cube resolution = 2 
    myCube->setSpringMode(true, true, true);
    boundingBox = new BoundingBox(6, 6, 6, glm::vec3(-3.0f, 5.5f, 3.0f), debug_shader_program);
    myPlate = new Plate(initPlatePos, 2.0, debug_shader_program);
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
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
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
