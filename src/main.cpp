#include <iostream>
#include <fstream>
#include <sstream>
#include <glm.hpp>
#include "gtx/string_cast.hpp"
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

bool loadOBJ(const std::string& path, std::vector<glm::vec3>& out_vertices, std::vector<Face>& out_faces, float scaleFactor = 1.0f) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cout << "Error opening file: " << path << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string type;
        iss >> type;

        if (type == "v") {
            glm::vec3 vertex;
            iss >> vertex.x >> vertex.y >> vertex.z;
            vertex *= scaleFactor;  // Escalar el vértice
            out_vertices.push_back(vertex);
        }
        else if (type == "f") {
            Face face;
            std::string indexStr;
            while (iss >> indexStr) {
                std::array<int, 3> indices;
                std::replace(indexStr.begin(), indexStr.end(), '/', ' ');
                std::istringstream indexIss(indexStr);
                indexIss >> indices[0] >> indices[1] >> indices[2];
                face.vertexIndices.push_back(indices);
            }
            out_faces.push_back(face);
        }
    }

    file.close();
    return true;
}

std::ostream &operator<< (std::ostream &out, const glm::vec3 &vec) {
    out << "{" 
        << vec.x << " " << vec.y << " "<< vec.z 
        << "}";

    return out;
}

void drawObject(const std::vector<glm::vec3>& vertices, const std::vector<Face>& faces) {
    clear();  // Clear the framebuffer

    for (const auto& face : faces) {
        int index1 = face.vertexIndices[0][0] - 1;
        int index2 = face.vertexIndices[1][0] - 1;
        int index3 = face.vertexIndices[2][0] - 1;


        // Get the corresponding vertex positions
        glm::vec3 vertex1 = vertices[index1];
        glm::vec3 vertex2 = vertices[index2];
        glm::vec3 vertex3 = vertices[index3];

        // Draw the triangle using the triangle function
        triangle(vertex1, vertex2, vertex3);
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

    if (loadOBJ("/Users/estebandonis/Documents/SextoSemestre/GraficasPorComputadora/Laboratorios/SR1/src/RealNave.obj", vertices, faces, 10.0f)) {
        // Successfully loaded OBJ file

        glm::vec3 translationOffset(FRAMEBUFFER_HEIGHT/2, FRAMEBUFFER_WIDTH/2, 0.0f); // Adjust x and y as needed

        // Apply the translation to each vertex in the combinedVertexArray
        for (auto& vertex : vertices) {
            vertex += translationOffset;
        }

        drawObject(vertices, faces);
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