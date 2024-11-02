#pragma once

#include <SDL2/SDL.h>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <array>
#include <vector>
#include <cstddef>
#include <unistd.h>
#include <memory>

float** matrix_dotproduct(float** m1, float** m2, size_t m1_rows, size_t m1_cols, size_t m2_rows, size_t m2_cols);

typedef enum {
    ROTATE_X,
    ROTATE_Y,
    ROTATE_Z
} ROT_AXIS;

class Model {
    public:
        std::vector<std::array<std::array<float*, 3>, 3>> m_vertices;
        Model();
        Model(const Model& m);
        Model& operator=(const Model& m);
        void rotate(float angle, ROT_AXIS axis);
        ~Model();
};
class Renderer {
    private:
        std::vector<std::shared_ptr<Model>> objects;
        float** proj_matrix;
        SDL_Window* win;
        SDL_Renderer* rend;
        SDL_Event events;
        uint16_t width;
        uint16_t height;
    public:
        Renderer(const char* title, uint16_t width, uint16_t height);
        void render_scene();
        void add_object(std::shared_ptr<Model> object);
        ~Renderer();
};
