
#include "prismatic_renderer.hpp"

#include <SDL2/SDL.h>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <array>
#include <vector>
#include <cstddef>
#include <unistd.h>
#include <memory>

///// EXTERNAL LIB FUNCTION
float** matrix_dotproduct(float** m1, float** m2, size_t m1_rows, size_t m1_cols, size_t m2_rows, size_t m2_cols) {
    if (m1_cols != m2_rows) {
        std::cout << "err";
        return NULL;
    }

    float** res = (float**)calloc(m1_rows, sizeof(float*));
    for (size_t i=0;i<m1_rows;++i) {
        res[i] = (float*)calloc(m2_cols, sizeof(float));
    }

    for (size_t i=0;i<m1_rows;++i) {
        for (size_t j=0;j<m2_cols;++j) {
            for (size_t k=0;k<m2_rows;++k) {
                res[i][j] += m1[i][k] * m2[k][j];
            }
        }

    }
    return res;
}
////

Model::Model() {}
Model::Model(const Model& m) {
    for (size_t i=0;i<this->m_vertices.size();++i) {
        for (size_t j=0;j<this->m_vertices[i].size();++j) {
            for (size_t k=0;k<this->m_vertices[i][j].size();++k) {
                delete this->m_vertices[i][j][k];
                this->m_vertices[i][j][k] = new float[1];
                *this->m_vertices[i][j][k] = *m.m_vertices[i][j][k];
            }
        }
    }
}
Model& Model::operator=(const Model& m) {
    for (size_t i=0;i<this->m_vertices.size();++i) {
        for (size_t j=0;j<this->m_vertices[i].size();++j) {
            for (size_t k=0;k<this->m_vertices[i][j].size();++k) {
                delete this->m_vertices[i][j][k];
                this->m_vertices[i][j][k] = new float[1];
                *this->m_vertices[i][j][k] = *m.m_vertices[i][j][k];
            }
        }
    }
    return *this;
}
Model::~Model() {
    for (size_t i=0;i<this->m_vertices.size();++i) {
        for (size_t j=0;j<this->m_vertices[i].size();++j) {
            for (size_t k=0;k<this->m_vertices[i][j].size();++k) {
                delete this->m_vertices[i][j][k];
            }
        }
    }
}

void Model::rotate(float angle, ROT_AXIS axis) {
    float** rotation_matrix = new float*[3];
    for (size_t i=0;i<3;++i) {
        rotation_matrix[i] = new float[3];
    }
    switch (axis) {
        case ROTATE_X: {
            rotation_matrix[0][0] = 1.f; rotation_matrix[0][1] = 0.f; rotation_matrix[0][2] = 0.f;
            rotation_matrix[1][0] = 0.f; rotation_matrix[1][1] = cos(angle); rotation_matrix[1][2] = -(sin(angle));
            rotation_matrix[2][0] = 0.f; rotation_matrix[2][1] = sin(angle); rotation_matrix[2][2] = cos(angle);
            break;
        }
        case ROTATE_Y: {
            rotation_matrix[0][0] = cos(angle); rotation_matrix[0][1] = 0.f; rotation_matrix[0][2] = sin(angle);
            rotation_matrix[1][0] = 0.f; rotation_matrix[1][1] = 1.f; rotation_matrix[1][2] = 0.f;
            rotation_matrix[2][0] = -(sin(angle)); rotation_matrix[2][1] = 0.f; rotation_matrix[2][2] = cos(angle);
            break;
        }
        case ROTATE_Z: {
            rotation_matrix[0][0] = cos(angle); rotation_matrix[0][1] = -(sin(angle)); rotation_matrix[0][2] = 0.f;
            rotation_matrix[1][0] = sin(angle); rotation_matrix[1][1] = cos(angle); rotation_matrix[1][2] = 0.f;
            rotation_matrix[2][0] = 0.f; rotation_matrix[2][1] = 0.f; rotation_matrix[2][2] = 1.f;
            break;
        }
    }
    for (size_t i=0;i<this->m_vertices.size();++i) {
        for (size_t j=0;j<this->m_vertices[i].size();++j) {
            float** ret_matrix = matrix_dotproduct(rotation_matrix, this->m_vertices[i][j].data(), 3, 3, this->m_vertices[j][i].size(), 1);
            for (size_t k=0;k<this->m_vertices[i][j].size();++k) {
                delete this->m_vertices[i][j][k];
            }
            std::copy(ret_matrix, ret_matrix+3, this->m_vertices[i][j].begin());
        }
    }
    for (size_t i=0;i<3;++i) {
        delete rotation_matrix[i];
    }
    delete[] rotation_matrix;
}

Renderer::Renderer(const char* title, uint16_t width, uint16_t height) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::exit(-1);
    }
    this->win = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, 0);
    if (this->win == nullptr) {
        std::exit(-1);
    }
    this->rend = SDL_CreateRenderer(this->win, -1, SDL_RENDERER_ACCELERATED);
    if (this->rend == nullptr) {
        std::exit(-1);
    }

    this->width = width;
    this->height = height;
    SDL_SetRenderDrawColor(this->rend, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(this->rend);

    this->proj_matrix = new float*[2];
    this->proj_matrix[0] = new float[3](0.f);
    this->proj_matrix[0][0] = 1.f;
    this->proj_matrix[1] = new float[3](0.f);
    this->proj_matrix[1][1] = 1.f;
}
void Renderer::render_scene() {
    std::array<SDL_Vertex, 3> vertices;
    int indices[3] = {0, 1, 2};
    float** point;
    for (size_t i=0;i<this->objects.size();++i) {
        for (size_t j=0;j<this->objects[i]->m_vertices.size();++j) {
            for (size_t k=0;k<this->objects[i]->m_vertices[j].size();++k) {
                point = matrix_dotproduct(this->proj_matrix, this->objects[i]->m_vertices[j][k].data(), 2, 3, this->objects[i]->m_vertices[j][k].size(), 1);
                vertices[k].position.x = point[0][0];
                vertices[k].position.y = point[1][0];
                vertices[k].color.r = 255;
                vertices[k].color.g = 255;
                vertices[k].color.b = 255;
                vertices[k].color.a = 255;
                for (size_t l=0;l<2;++l) {
                    delete point[l];
                }
                delete point;
            }
            SDL_RenderGeometry(this->rend, NULL, vertices.data(), 3, indices, 3);
        }
    }
    SDL_RenderPresent(this->rend);
}
void Renderer::add_object(std::shared_ptr<Model> object) {
    this->objects.push_back(object);
}
Renderer::~Renderer() {
    for (size_t i=0;i<2;++i) {
        delete this->proj_matrix[i];
    }
    delete this->proj_matrix;
    this->proj_matrix = nullptr;
    SDL_DestroyRenderer(this->rend);
    SDL_DestroyWindow(this->win);
    SDL_Quit();
}
