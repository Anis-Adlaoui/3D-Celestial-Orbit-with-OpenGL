#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <vector>
#include <iostream>
#include <shader.hpp>

#define M_PI 3.14

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace glm;



// Fonction pour gérer les entrées clavier
void moveCamera(GLFWwindow* window, vec3& cameraPos, vec3& cameraFront, vec3& cameraUp, float deltaTime) {
    float cameraSpeed = 2.5f * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= normalize(cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += normalize(cross(cameraFront, cameraUp)) * cameraSpeed;
}

void moveCameraTarget(GLFWwindow* window, glm::vec3& targetPosition, float deltaTime) {
    static double lastX, lastY;
    static bool firstMouse = true;
    static float sensitivity = 0.1f; // Sensibilité de la souris

    // Obtenez la position actuelle de la souris
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    // Calculez le déplacement de la souris
    float xOffset = xpos - lastX;
    float yOffset = lastY - ypos; // Inverser Y car les coordonnées de la souris augmentent vers le bas
    lastX = xpos;
    lastY = ypos;

    // Appliquez la sensibilité
    xOffset *= sensitivity;
    yOffset *= sensitivity;

    // Modifiez la position cible en fonction du mouvement de la souris
    targetPosition.x += xOffset * deltaTime;
    targetPosition.y += yOffset * deltaTime;
}


// Fonction pour créer une sphère (génération des sommets et indices uniquement)
void createSphere(std::vector<float>& vertices, std::vector<unsigned int>& indices, float radius, unsigned int numSlices, unsigned int numStacks, unsigned int& numVertices) {
    vertices.clear();
    indices.clear();

    for (unsigned int stack = 0; stack <= numStacks; ++stack) {
        float phi = M_PI * float(stack) / float(numStacks);
        for (unsigned int slice = 0; slice <= numSlices; ++slice) {
            float theta = 2.0f * M_PI * float(slice) / float(numSlices);
            float x = cosf(theta) * sinf(phi);
            float y = cosf(phi);
            float z = sinf(theta) * sinf(phi);

            vertices.push_back(radius * x);
            vertices.push_back(radius * y);
            vertices.push_back(radius * z);

            float u = (float)slice / numSlices;
            float v = (float)stack / numStacks;
            vertices.push_back(u);
            vertices.push_back(v);
        }
    }

    for (unsigned int stack = 0; stack < numStacks; ++stack) {
        for (unsigned int slice = 0; slice < numSlices; ++slice) {
            indices.push_back((stack + 0) * (numSlices + 1) + slice);
            indices.push_back((stack + 1) * (numSlices + 1) + slice);
            indices.push_back((stack + 0) * (numSlices + 1) + slice + 1);

            indices.push_back((stack + 0) * (numSlices + 1) + slice + 1);
            indices.push_back((stack + 1) * (numSlices + 1) + slice);
            indices.push_back((stack + 1) * (numSlices + 1) + slice + 1);
        }
    }

    numVertices = vertices.size() / 5;
}

// Générer une position aléatoire pour une étoile
vec3 getRandomPosition(float range) {
    float x = ((rand() % 2000) / 1000.0f - 1.0f) * range;
    float y = ((rand() % 2000) / 1000.0f - 1.0f) * range;
    float z = ((rand() % 2000) / 1000.0f - 1.0f) * range;
    return vec3(x, y, z);
}

// Fonction pour faire tourner les étoiles autour de la Terre
void updateStarPositions(std::vector<vec3>& starPositions, float rotationSpeed, float deltaTime) {
    mat4 rotationMatrix = rotate(mat4(1.0f), rotationSpeed * deltaTime, vec3(0.0f, 1.0f, 0.0f));
    for (auto& position : starPositions) {
        vec4 rotatedPosition = rotationMatrix * vec4(position, 1.0f);
        position = vec3(rotatedPosition);
    }
}

int main() {
    GLFWwindow* window;
    int widthWindow = 800, heightWindow = 800;

    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(widthWindow, heightWindow, "Terre avec Étoiles", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


    glClearColor(0.0196f, 0.0196f, 0.1372f, 1.0f);

    // Génération des sommets et indices pour la Terre
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    unsigned int numVertices;
    createSphere(vertices, indices, 1.0f, 72, 36, numVertices);

    GLuint earthVAO, earthVBO, earthEBO;
    glGenVertexArrays(1, &earthVAO);
    glGenBuffers(1, &earthVBO);
    glGenBuffers(1, &earthEBO);

    glBindVertexArray(earthVAO);

    glBindBuffer(GL_ARRAY_BUFFER, earthVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, earthEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);



    // Génération des étoiles
    std::vector<float> starVertices;
    std::vector<unsigned int> starIndices;
    unsigned int starNumVertices;
    createSphere(starVertices, starIndices, 0.01f, 36, 18, starNumVertices);

    GLuint starVAO, starVBO, starEBO;

    glGenVertexArrays(1, &starVAO);

    glGenBuffers(1, &starVBO);

    glGenBuffers(1, &starEBO);


    glBindVertexArray(starVAO);

    glBindBuffer(GL_ARRAY_BUFFER, starVBO);
    glBufferData(GL_ARRAY_BUFFER, starVertices.size() * sizeof(float), starVertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, starEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, starIndices.size() * sizeof(unsigned int), starIndices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    // Générer des positions pour les étoiles
    std::vector<vec3> starPositions;
    int numStars = 1000;
    for (int i = 0; i < numStars; i++) {
        starPositions.push_back(getRandomPosition(20.0f));
    }

    // Charger la texture de la Terre
    GLuint earthTexture;
    glGenTextures(1, &earthTexture);
    glBindTexture(GL_TEXTURE_2D, earthTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int texWidth, texHeight, texChannels;
    unsigned char* earth_texture = stbi_load("C:/Users/Msi_Katana B12V/Desktop/IV/Synthèse d'images/Texture/earth.jpg", &texWidth, &texHeight, &texChannels, 0);

    if (earth_texture) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texWidth, texHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, earth_texture);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cerr << "Failed to load texture" << std::endl;
    }
    stbi_image_free(earth_texture);

    // Load second earth texture (earth2.jpg)
    GLuint earthTexture2;
    glGenTextures(1, &earthTexture2);
    glBindTexture(GL_TEXTURE_2D, earthTexture2);
    int texWidth2, texHeight2, texChannels2;
    unsigned char* earth_texture2 = stbi_load("C:/Users/Msi_Katana B12V/Desktop/IV/Synthèse d'images/Texture/earth2.jpg", &texWidth2, &texHeight2, &texChannels2, 0);
    if (earth_texture2) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texWidth2, texHeight2, 0, GL_RGB, GL_UNSIGNED_BYTE, earth_texture2);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cerr << "Failed to load texture2" << std::endl;
    }
    stbi_image_free(earth_texture2);

    GLuint ShaderProgram = LoadShaders("C:/Users/Msi_Katana B12V/Desktop/IV/Synthèse d'images/shader/shader/SimpleVertexShader.vertexshader",
        "C:/Users/Msi_Katana B12V/Desktop/IV/Synthèse d'images/shader/shader/SimpleFragmentShader.fragmentshader");

    float cameraRotationSpeed = 0.5f; // Vitesse de rotation de la caméra
    float cameraDistance = 5.0f;      // Distance initiale de la caméra

    vec3 cameraPos = vec3(0.0f, 0.0f, 5.0f);
    vec3 cameraFront = vec3(0.0f, 0.0f, -1.0f);
    vec3 cameraUp = vec3(0.0f, 1.0f, 0.0f);

    mat4 Projection = perspective(radians(60.0f), (float)widthWindow / heightWindow, 0.1f, 500.0f);

    GLuint MatrixID = glGetUniformLocation(ShaderProgram, "MVP");
    GLuint IsStarID = glGetUniformLocation(ShaderProgram, "isStar");
    GLuint StarColorID = glGetUniformLocation(ShaderProgram, "starColor");
    GLuint TextureID = glGetUniformLocation(ShaderProgram, "texture1"); // Get texture uniform ID




    glEnable(GL_DEPTH_TEST);

    float rotationAngle = 0.0f;
    float starRotationSpeed = 0.5f;
    float lastFrame = 0.0f;

    float starRotationAngle = 0.0f; // Angle de rotation des étoiles
    GLuint StarRotationID = glGetUniformLocation(ShaderProgram, "starRotation"); // Uniforme pour la rotation des étoiles

    glm::vec3 targetPosition(0.0f, 0.0f, 0.0f); // Position cible initiale

    bool useTexture2 = false; // Flag to track which texture to use

    // Main rendering loop
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(ShaderProgram);

        // Check for 'L' key press to toggle texture
        if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
            useTexture2 = !useTexture2; // Toggle the texture flag
            // Small delay to prevent repeated toggles on a single key press
            glfwSetTime(glfwGetTime() + 0.1); // Adjust delay as needed.
        }


        // Déplacement de la caméra avec les touches (position de la caméra)
        moveCamera(window, cameraPos, cameraFront, cameraUp, deltaTime);

        // Mise à jour de la position cible de la caméra avec la souris
        moveCameraTarget(window, targetPosition, deltaTime);

        // Création de la matrice View avec la nouvelle position cible
        glm::mat4 View = glm::lookAt(cameraPos, targetPosition, cameraUp);
        // Render the Earth (main sphere)
        rotationAngle += 0.001f;
        glm::mat4 Model = glm::rotate(glm::mat4(1.0f), rotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 MVP = Projection * View * Model;
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);


        // Zoom avec les touches Z et S
        if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
            cameraDistance -= 0.1f; // Zoom avant
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            cameraDistance += 0.1f; // Zoom arrière
        }
        cameraDistance = glm::clamp(cameraDistance, 2.0f, 10.0f); // Limiter le zoom

        glUniform1i(IsStarID, 0);

        glActiveTexture(GL_TEXTURE0);
        if (useTexture2) {
            glBindTexture(GL_TEXTURE_2D, earthTexture2); // Use the second texture
        }
        else {
            glBindTexture(GL_TEXTURE_2D, earthTexture);  // Use the first texture
        }

        glUniform1i(TextureID, 0); // Set the texture uniform (important!)

        glBindVertexArray(earthVAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);



        // Render the stars (small spheres rotating around the Earth)
        starRotationAngle += 0.0005f; // Adjust speed of rotation
        glUniform1f(StarRotationID, starRotationAngle); // Update the rotation uniform
        glUniform1i(IsStarID, 1);
        glUniform3f(StarColorID, 254 / 255.0f, 255 / 255.0f, 166 / 255.0f); // Yellowish star color
        

        for (auto& position : starPositions) {
            glm::mat4 starModel = glm::mat4(1.0f);
            starModel = glm::rotate(starModel, starRotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));
            starModel = glm::translate(starModel, position);
            MVP = Projection * View * starModel;
            glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

            glBindVertexArray(starVAO);
            glDrawElements(GL_TRIANGLES, starIndices.size(), GL_UNSIGNED_INT, 0);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }



    // Cleanup resources
    glDeleteVertexArrays(1, &earthVAO);
    glDeleteBuffers(1, &earthVBO);
    glDeleteBuffers(1, &earthEBO);

    glDeleteVertexArrays(1, &starVAO);
    glDeleteBuffers(1, &starVBO);
    glDeleteBuffers(1, &starEBO);

    glDeleteTextures(1, &earthTexture); // Delete both textures
    glDeleteTextures(1, &earthTexture2);

    glfwTerminate();
    return 0;
}
