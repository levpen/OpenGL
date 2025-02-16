#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "Shader.h"
#include "stb_image.h"
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>
#include <glm/glm/gtc/type_ptr.hpp>
#include "Constants.h"
#include "Camera.h"
#include "Model.h"
#include <filesystem>
#include <map>


// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

glm::mat4 model = glm::mat4(1.0f);
glm::mat4 view = glm::mat4(1.0f);
glm::mat4 projection;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f);

void render_with_border(Model& object, Shader& modelShader, Shader& borderShader);

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
unsigned int texturePreparation(std::string img_source, bool rgb, const int GL_TEXTURE_NUM, bool has_alpha = false);
void setShaderMatrices(Shader& shader, glm::mat4& model, glm::mat4& view, glm::mat4& projection);
glm::mat3 computeNormalMat(glm::mat4& model);

void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

glm::mat4 cameraVectors();

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    //Callbacks
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_STENCIL_TEST);
    //glStencilMask(0x00);
    //glStencilFunc(GL_EQUAL, 1, 0xFF);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
    glEnable(GL_CULL_FACE);
    //glCullFace(GL_FRONT);
    //glFrontFace(GL_CCW);

    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

    unsigned int diffuseMap = texturePreparation("container2.png", false, GL_TEXTURE0);
    unsigned int grassTexture = texturePreparation("blending_transparent_window.png", false, GL_TEXTURE0, true);
    unsigned int specularMap = texturePreparation("container2_specular.png", false, GL_TEXTURE0);
    vector<glm::vec3> vegetation
    {
        glm::vec3(-1.5f, 0.0f, -0.48f),
        glm::vec3(1.5f, 0.0f, 0.51f),
        glm::vec3(0.0f, 0.0f, 0.7f),
        glm::vec3(-0.3f, 0.0f, -2.3f),
        glm::vec3(0.5f, 0.0f, -0.6f)
    };
    //unsigned int texture = texturePreparation("container.jpg", true, GL_TEXTURE0);
    //unsigned int texture1 = texturePreparation("awesomeface.png", false, GL_TEXTURE1);

    Shader ourShader("./VertexShader.vert", "./FragmentShader.frag");
    Shader lightCubeShader("./LightSource.vert", "./LightSource.frag");
    Shader alphaShader("./VertexShader.vert", "./BasicFragmentShader.frag");

    Model backpack("./backpack/backpack.obj");

    unsigned int VBO, EBO, VAO;
    //glGenBuffers(1, &EBO);
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    // 1. bind Vertex Array Object
    glBindVertexArray(VAO);
    // 2. copy our vertices array in a buffer for OpenGL to use
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    // 3. copy our index array in a element buffer for OpenGL to use
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    // 3. then set our vertex attributes pointers
    short stride = 8 * sizeof(float);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    //glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    //glEnableVertexAttribArray(1);
    //glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
    //glEnableVertexAttribArray(2);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    //LightSource
    unsigned int lightVAO;
    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);
    // we only need to bind to the VBO, the container's VBO's data already contains the data.
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // set the vertex attribute 
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);

    //GrassSource
    unsigned vegetationVAO, vegetationVBO;
    glGenVertexArrays(1, &vegetationVAO);
    glGenBuffers(1, &vegetationVBO);

    glBindVertexArray(vegetationVAO);
    glBindBuffer(GL_ARRAY_BUFFER, vegetationVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
    stride = 5 * sizeof(float);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);


    ourShader.use();

    //unsigned int transformLoc = glGetUniformLocation(ourShader.ID, "transform");
    ourShader.setVec3("objectColor", 1.0f, 1.0f, 1.0f);

    ourShader.setInt("material.specular", 1);
    ourShader.setFloat("material.shininess", 64.0f);
    ourShader.setInt("material.diffuse", 0);

    ourShader.setVec3("dirLight.ambient", 0.2f, 0.2f, 0.2f);
    ourShader.setVec3("dirLight.diffuse", 0.5f, 1.0f, 0.5f); // darken diffuse light a bit
    ourShader.setVec3("dirLight.specular", 1.0f, 1.0f, 1.0f);
    ourShader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
    
    for (int i = 0; i < 4; ++i)
    {
        auto str = "pointLights[" + std::to_string(i) + "].";
        ourShader.setFloat(str + "constant", 1.0f);
        ourShader.setFloat(str +"linear", 0.09f);
        ourShader.setFloat(str + "quadratic", 0.032f);

        ourShader.setVec3(str + "ambient", 0.2f, 0.2f, 0.2f);
        ourShader.setVec3(str + "diffuse", 1.0f, 0.5f, 0.5f);
        ourShader.setVec3(str + "specular", 1.0f, 1.0f, 1.0f);

        ourShader.setVec3(str + "position", pointLightPositions[i]);

    }


    //ourShader.setFloat("light.cutOff", glm::cos(glm::radians(12.5f)));
    //ourShader.setFloat("light.outerCutOff", glm::cos(glm::radians(18.5f)));

    



    //ourShader.setInt("texture1", 0);
    //ourShader.setInt("texture2", 1);

    //MOUSE HIDE
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        // input
        processInput(window);
        ourShader.setVec3("viewPos", camera.Position);

        // rendering commands here
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        //Disable stencil rewrite for border
        glStencilMask(0x00);

        model = glm::mat4(1.0f);
        view = camera.GetViewMatrix();
        projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

        ourShader.use();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, specularMap);
        for (unsigned int i = 0; i < 10; i++)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i]);
            float angle = 20.0f * i;
            model = glm::rotate(model, (float)glfwGetTime() * glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            setShaderMatrices(ourShader, model, view, projection);
            //ourShader.setVec3("light.position", lightPos);
            ourShader.setMat3("normalMat", computeNormalMat(model));
            
            glBindVertexArray(VAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        //Light cubes render
        lightCubeShader.use();
        lightCubeShader.setVec3("lightColor", 1.0f, 0.5f, 0.5f);

        for (short i = 0; i < 4; ++i)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, pointLightPositions[i]);
            model = glm::scale(model, glm::vec3(0.2f));
            setShaderMatrices(lightCubeShader, model, view, projection);
            glBindVertexArray(lightVAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        //Backpack render
        render_with_border(backpack, ourShader, lightCubeShader);
        
        //Opaque objects render (dont forget to sort)
        glDisable(GL_CULL_FACE);

        std::map<float, glm::vec3> sorted;
        for (unsigned int i = 0; i < vegetation.size(); i++)
        {
            float distance = glm::length(camera.Position - vegetation[i]);
            sorted[distance] = vegetation[i];
        }

        alphaShader.use();
        alphaShader.setInt("texture1", 0);
        glBindVertexArray(vegetationVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, grassTexture);
        for (std::map<float, glm::vec3>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, it->second);
            setShaderMatrices(alphaShader, model, view, projection);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
        glEnable(GL_CULL_FACE);


        // check and call events and swap the buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glDeleteVertexArrays(1, &VAO);
    glDeleteVertexArrays(1, &lightVAO);
    glDeleteVertexArrays(1, &vegetationVAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &vegetationVBO);
    glDeleteBuffers(1, &EBO);

    glfwTerminate();

    return 0;
}


void render_with_border(Model& object, Shader& modelShader, Shader& borderShader)
{
    modelShader.use();

    model = glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, 5.0f, 1.0f));

    setShaderMatrices(modelShader, model, view, projection);

    modelShader.setMat3("normalMat", computeNormalMat(model));

    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilMask(0xFF);

    object.Draw(modelShader);

    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilMask(0x00); // disable writing to the stencil buffer
    glDisable(GL_DEPTH_TEST);
    borderShader.use();
    model = glm::scale(model, glm::vec3(1.1f));
    modelShader.setMat4("model", model);

    object.Draw(borderShader);

    glStencilMask(0xFF);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glEnable(GL_DEPTH_TEST);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}


void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

unsigned int texturePreparation(std::string img_source, bool rgb, const int GL_TEXTURE_NUM, bool has_alpha)
{
    unsigned int texture;
    glGenTextures(1, &texture);
    glActiveTexture(GL_TEXTURE_NUM);
    glBindTexture(GL_TEXTURE_2D, texture);
    // set the texture wrapping/filtering options (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, has_alpha ? GL_CLAMP_TO_EDGE : GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, has_alpha ? GL_CLAMP_TO_EDGE : GL_REPEAT);
    /*glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);*/
    // load and generate the texture
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(!has_alpha);
    unsigned char* data = stbi_load(img_source.c_str(), &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, has_alpha ? GL_RGBA : GL_RGB, width, height, 0, rgb ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);
    return texture;
}

void setShaderMatrices(Shader& ourShader, glm::mat4& model, glm::mat4& view, glm::mat4& projection) {
    ourShader.setMat4("projection", projection);
    ourShader.setMat4("view", view);
    ourShader.setMat4("model", model);
}

bool firstMouse = true;
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

glm::mat4 cameraVectors() {
    const float radius = 10.0f;
    float camX = sin(glfwGetTime()) * radius;
    float camZ = cos(glfwGetTime()) * radius;
    glm::mat4 view;
    view = glm::lookAt(glm::vec3(camX, 0.0, camZ), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));

    return view;
}

glm::mat3 computeNormalMat(glm::mat4& model) {
    return glm::transpose(glm::inverse(model));
}
