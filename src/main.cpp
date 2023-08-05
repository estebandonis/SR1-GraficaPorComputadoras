#include <iostream>
#include <fstream>
#include <sstream>
#include <glm.hpp>
#include <SDL2/SDL.h>
#include <vector>
#include <string>

// Define the size of the framebuffer
const int FRAMEBUFFER_WIDTH = 500;
const int FRAMEBUFFER_HEIGHT = 500;
const int FRAMEBUFFER_SIZE = FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT;
const int WINDOW_WIDTH = 500;
const int WINDOW_HEIGHT = 500;

// Define a Color struct to hold the RGB values of a pixel
struct Color {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
};

// Declare the framebuffer as a global variable
Color framebuffer[FRAMEBUFFER_SIZE];

// Declare a global clearColor of type Color
Color clearColor = {0, 0, 0, 255}; // Initially set to black

// Declare a global currentColor of type Color
Color currentColor = {255, 255, 255, 255}; // Initially set to white

int x = 50;
int y = 50;

void clear() {
    for (int i = 0; i < FRAMEBUFFER_SIZE; i++) {
        framebuffer[i] = clearColor;
    }
}

// Function to set a specific pixel in the framebuffer to the currentColor
void point(int x, int y) {
    if (x >= 0 && x < FRAMEBUFFER_WIDTH && y >= 0 && y < FRAMEBUFFER_HEIGHT) {
        framebuffer[y * FRAMEBUFFER_WIDTH + x] = currentColor;
    }
}

void renderBuffer(SDL_Renderer* renderer) {
    // Create a texture
    SDL_Texture* texture = SDL_CreateTexture(
        renderer, 
        SDL_PIXELFORMAT_ABGR8888,
        SDL_TEXTUREACCESS_STREAMING,
        FRAMEBUFFER_WIDTH, 
        FRAMEBUFFER_HEIGHT
    );

    // Update the texture with the pixel data from the framebuffer
    SDL_UpdateTexture(
        texture, 
        NULL, 
        framebuffer, 
        FRAMEBUFFER_WIDTH * sizeof(Color)
    );

    // Copy the texture to the renderer
    SDL_RenderCopy(renderer, texture, NULL, NULL);

    // Destroy the texture
    SDL_DestroyTexture(texture);
}

void line(glm::vec3 start, glm::vec3 end) {
    int x1 = round(start.x), y1 = round(start.y);
    int x2 = round(end.x), y2 = round(end.y);

    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);

    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;

    int err = dx - dy;

    while (true) {
        point(x1, y1);

        if (x1 == x2 && y1 == y2) {
            break;
        }

        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}

void triangle(const glm::vec3& A, const glm::vec3& B, const glm::vec3& C) {
    line(A, B);
    line(B, C);
    line(C, A);
}

struct Face {
    std::vector<std::array<int, 3>> vertexIndices;
};

bool loadOBJ(const std::string& path, std::vector<glm::vec3>& out_vertices, std::vector<Face>& out_faces, float scale) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << path << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string token;
        iss >> token;

        if (token == "v") {
            glm::vec3 vertex;
            iss >> vertex.x >> vertex.y >> vertex.z;

            // Apply scaling to the vertex coordinates
            vertex *= scale;

            out_vertices.push_back(vertex);
        } else if (token == "f") {
            Face face;
            int v1, v2, v3;
            char slash;
            while (iss >> v1 >> slash >> v2 >> slash >> v3) {
                face.vertexIndices.push_back({v1 - 1, v2 - 1, v3 - 1}); // OBJ indices are 1-based
            }
            out_faces.push_back(face);
        }
    }

    file.close();
    return true;
}

std::vector<glm::vec3> setupVertexArray(const std::vector<glm::vec3>& vertices, const std::vector<Face>& faces) {
    std::vector<glm::vec3> vertexArray;

    // For each face
    for (const auto& face : faces) {
        // For each vertex in the face
        for (const auto& vertexIndices : face.vertexIndices) {
            // Get the vertex position from the input array using the index from the face
            glm::vec3 vertexPosition = vertices[vertexIndices[0]];

            // Add the vertex position to the vertex array
            vertexArray.push_back(vertexPosition);
        }
    }

    return vertexArray;
}

void drawObject(const std::vector<glm::vec3>& combinedVertexArray, const std::vector<Face>& faces) {
    for (const auto& face : faces) {
        for (const auto& vertexIndices : face.vertexIndices) {
            glm::vec3 vertexA = combinedVertexArray[vertexIndices[0]];
            glm::vec3 vertexB = combinedVertexArray[vertexIndices[1]];
            glm::vec3 vertexC = combinedVertexArray[vertexIndices[2]];

            triangle(vertexA, vertexB, vertexC);
        }
    }
}

int main() {
    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_Window* window = SDL_CreateWindow("SR", 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0);

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    bool running = true;
    SDL_Event event;

    std::vector<glm::vec3> vertices;
    std::vector<Face> faces;

    if (loadOBJ("/Users/estebandonis/Documents/SextoSemestre/GraficasPorComputadora/Ejemplos/Blender/cube.obj", vertices, faces, 80.0f)) {
        // Successfully loaded OBJ file
        // Now you can use the vertices and faces vectors
        // ...
        std::cout << "Leido" << std::endl;

        std::vector<glm::vec3> combinedVertexArray = setupVertexArray(vertices, faces);

        glm::vec3 translationOffset(250, 250, 0.0f); // Adjust x and y as needed

        // Apply the translation to each vertex in the combinedVertexArray
        for (auto& vertex : combinedVertexArray) {
            vertex += translationOffset;
        }

        drawObject(combinedVertexArray, faces);
    }


    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        // Call our render function
        renderBuffer(renderer);

        // Present the frame buffer to the screen
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}