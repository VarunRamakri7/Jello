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
#include "trackball.h"
#include "BoundingBox.h"
#include "Camera.h"
#include "Physics.h"

#include <glm/gtx/string_cast.hpp> // for debug

#include "Cube.h"

const int init_window_width = 800;
const int init_window_height = 800;
const char* const window_title = "Jello";

static const std::string vertex_shader("template_vs.glsl");
static const std::string fragment_shader("template_fs.glsl");
GLuint shader_program = -1;
float time_sec;
bool recording = false;

// interactive
TrackBallC trackball;
bool mouseLeft, mouseMid, mouseRight;
GLdouble mouseX, mouseY;
glm::vec2 mousePosA;
float mouseClickTime = 0.0f;
float mouseReleaseTime = 0.0f;
glm::vec2 dragV = glm::vec2(0.0); // from mouse interaction
glm::vec3 gravity = glm::vec3(0.0, -9.8, 0.0);
bool moveCam = false;

// display
bool showDiscrete = false;
bool showBB = true;

// physics
bool addGravity = false;
enum integratorEnum {
    EULER, RK4
}; // euler = 0 , RK4 = 1
int integrator = integratorEnum::EULER;
float fStiffness = -50.f;
float fDamping = -0.5f;
float fMass = 1.0f;
float fTimeStep = 0.005f;
bool needReset = false;

// scene
Cube* myCube;
BoundingBox* boundingBox;
Plane* plate;
Camera* myCamera;
const glm::vec3 cameraInitPos = glm::vec3(0.0f, 0.0f, 5.0f);
std::vector<BoundingBox*> sceneObjs;

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

   ImGui::Checkbox("Show discrete", &showDiscrete);
   ImGui::Checkbox("Show bounding box", &showBB);
   ImGui::SliderInt("Particle Size", &myCube->pointSize, 1, 10);
   ImGui::SliderInt("Jello Resolution", &myCube->resolution, 1, 8);
   ImGui::SliderFloat("Stiffness", &fStiffness, 0.0f, 100.0f);
   ImGui::SliderFloat("Damping", &fDamping, 0.0, 1.0f);
   ImGui::SliderFloat("Mass", &fMass, 0.0f, 50.0f);
   ImGui::SliderFloat("TimeStep", &fTimeStep, 0.001, 0.01);
   needReset = ImGui::Button("Reset");
   //add reset and resolution
   // then debug the 4 squares  
   ImGui::Checkbox("On Plate", &myCube->fixedFloor);
   ImGui::Checkbox("Structural Spring", &myCube->structuralSpring);
   ImGui::Checkbox("Shear Spring", &myCube->shearSpring);
   ImGui::Checkbox("Bend Spring", &myCube->bendSpring);
   ImGui::Checkbox("Visualize Springs", &myCube->showSpring);
   ImGui::Checkbox("Add Gravity", &addGravity);
   ImGui::RadioButton("Euler", &integrator, integratorEnum::EULER);
   ImGui::RadioButton("RK4", &integrator, integratorEnum::RK4);

   ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
   ImGui::End();

   //End ImGui Frame
   ImGui::Render();
   ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
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
        mousePosA = glm::vec2(mouseX, mouseY); // store clicked pos
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

// This function gets called every time the scene gets redisplayed
void display(GLFWwindow* window)
{
   //Clear the screen to the color previously specified in the glClearColor(...) call.
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   glm::mat4 M = myCube->getModelMatrix();
   // move cam and no drag when C is pressed
   glm::mat4 PV = myCamera->getPV() * trackball.Set3DViewCameraMatrix();

   //std::cout << glm::to_string(M) << std::endl;
   glUseProgram(shader_program);

   if (showBB) {
       boundingBox->render(1);
   }
   myCube->render(1, showDiscrete);
   //plate->render(1);

   //Get location for shader uniform variable
   glm::mat4 PVM = PV *M;
   int PVM_loc = glGetUniformLocation(shader_program, "PVM");
   glUniformMatrix4fv(PVM_loc, 1, false, glm::value_ptr(PVM));

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
   time_sec = static_cast<float>(glfwGetTime());

   // only if not moving camera
   if (mouseLeft && !moveCam) {
       // hold

       // calculate acceleration 
       glm::vec2 changeP = glm::vec2(mouseX, mouseY) - mousePosA;
       float moved = glm::length(changeP);
       float timeDiff = time_sec - mouseClickTime;
       if (timeDiff > 0.01) {
           // drag
           dragV = changeP / (timeDiff * timeDiff) * 0.001f;
       }

   }

   if (needReset) {
       // reset simulation

       // send cube values
       myCube->stiffness = (-1.0) * double(fStiffness); // negate
       myCube->damping = (-1.0) * double(fDamping); // negate
       myCube->mass = double(fMass);
       myCube->timeStep = double(fTimeStep);

       myCube->reset();
       // TODO Add the spring modes

   }

   //Pass time_sec value to the shaders
   int time_loc = glGetUniformLocation(shader_program, "time");
   glUniform1f(time_loc, time_sec);
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

   if(action == GLFW_PRESS)
   {
      switch(key)
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
      }
   }
   else if (action == GLFW_RELEASE) {
       switch (key)
       {
       case GLFW_KEY_C:
           moveCam = false;
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

//Initialize OpenGL state. This function only gets called once.
void initOpenGL()
{
   glewInit();

   //Print out information about the OpenGL version supported by the graphics driver.	
   std::cout << "Vendor: "       << glGetString(GL_VENDOR)                    << std::endl;
   std::cout << "Renderer: "     << glGetString(GL_RENDERER)                  << std::endl;
   std::cout << "Version: "      << glGetString(GL_VERSION)                   << std::endl;
   std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION)  << std::endl;
   glEnable(GL_DEPTH_TEST);

   reload_shader();
}



//C++ programs start executing in the main() function.
int main(int argc, char **argv)
{
   GLFWwindow* window;

   /* Initialize the library */
   if (!glfwInit())
   {
      return -1;
   }

   // negotiate with the OpenGL
   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
   glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

   /* Create a windowed mode window and its OpenGL context */
   window = glfwCreateWindow(init_window_width, init_window_height, window_title, NULL, NULL);
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

   initOpenGL();
   myCamera = new Camera();
   myCamera->setPosition(cameraInitPos);
   myCube = new Cube(2);
   myCube->setSpringMode(true, true, true);
   boundingBox = new BoundingBox(5,5,5, glm::vec3(-2, 2, 2.0f));
   sceneObjs.push_back(boundingBox);
   
   //Init ImGui
   IMGUI_CHECKVERSION();
   ImGui::CreateContext();
   ImGui_ImplGlfw_InitForOpenGL(window, true);
   ImGui_ImplOpenGL3_Init("#version 150");

   /* Loop until the user closes the window */
   while (!glfwWindowShouldClose(window))
   {
      idle();
      // TO DO add constant external force
      
      myCube->setExternalForce(addGravity? glm::vec3(dragV, 0.0) + gravity : glm::vec3(dragV, 0.0));
      
       // + gravity// TODO why gravity + plate makes it go through? 
      //myCube->setExternalForce(glm::vec3(sin(time_sec), 0.0, 0.0));
      // TO DO add to reset simulation 
      if (integrator == integratorEnum::EULER) {
          integrateEuler(myCube);
      }
      else if (integrator == integratorEnum::RK4) {
          integrateRK4(myCube);
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