#include <game/application.hpp>
#include <vector>
#include <algorithm>
#ifndef BOOST_COMPUTE_USE_CPP11
    #define BOOST_COMPUTE_USE_CPP11
#endif
#ifndef CL_TARGET_OPENCL_VERSION
    #define CL_TARGET_OPENCL_VERSION 300
#endif
#include <boost/compute.hpp>

// Conway3D_Raylib_Instanced.cpp
// Optimized OpenCL 3D Conway's Game of Life with raylib instanced rendering

#include <iostream>
#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"

namespace compute = boost::compute;

const char* conway3d_kernel_src = BOOST_COMPUTE_STRINGIZE_SOURCE(
    __kernel void conway3d_step(__global const char* current,
        __global char* next,
        const uint width,
        const uint height,
        const uint depth) {
    int x = get_global_id(0);
    int y = get_global_id(1);
    int z = get_global_id(2);
    int index = z * width * height + y * width + x;

    int count = 0;
    for (int dz = -1; dz <= 1; dz++) {
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                if (dx == 0 && dy == 0 && dz == 0) continue;
                int nx = x + dx;
                int ny = y + dy;
                int nz = z + dz;
                if (nx >= 0 && ny >= 0 && nz >= 0 && nx < width && ny < height && nz < depth) {
                    int n_index = nz * width * height + ny * width + nx;
                    count += current[n_index];
                }
            }
        }
    }

    char state = current[index];
    if (state == 1 && (count == 2 || count == 3)) {
        next[index] = 1;
    }
    else if (state == 0 && count == 3) {
        next[index] = 1;
    }
    else {
        next[index] = 0;
    }
}
);

int main() {
    const size_t width = 100, height = 100, depth = 100;
    const size_t grid_size = width * height * depth;
    std::vector<char> host_grid(grid_size, 0);
    std::vector<Vector3> positions;

    host_grid[(1 * width * height) + (1 * width) + 2] = 1;
    host_grid[(2 * width * height) + (2 * width) + 3] = 1;
    host_grid[(3 * width * height) + (3 * width) + 1] = 1;
    host_grid[(3 * width * height) + (3 * width) + 2] = 1;
    host_grid[(3 * width * height) + (3 * width) + 3] = 1;

    compute::device device = compute::system::default_device();
    compute::context context(device);
    compute::command_queue queue(context, device);
    std::cout << "Using OpenCL device: " << device.name() << std::endl;

    compute::vector<char> d_current(host_grid.begin(), host_grid.end(), queue);
    compute::vector<char> d_next(grid_size, context);

    compute::program program = compute::program::build_with_source(conway3d_kernel_src, context);
    compute::kernel kernel(program, "conway3d_step");
    kernel.set_arg(0, d_current);
    kernel.set_arg(1, d_next);
    kernel.set_arg(2, (cl_uint)width);
    kernel.set_arg(3, (cl_uint)height);
    kernel.set_arg(4, (cl_uint)depth);

    size_t global_size[3] = { width, height, depth };

    InitWindow(800, 600, "Conway 3D - Raylib Instanced + OpenCL");
    Camera3D camera = { 0 };
    camera.position = { 0.0f, 0.0f, 0.0f };
    camera.target = { 16.0f, 16.0f, 16.0f };
    camera.up = { 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    SetTargetFPS(60);

    Mesh cubeMesh = GenMeshCube(1.0f, 1.0f, 1.0f);
    Model cubeModel = LoadModelFromMesh(cubeMesh);

    bool paused = false;
    DisableCursor();

    const std::string lighting_instanced_vs_path = (Game::shader_path() / "lighting_instancing.vs").string();
    const std::string lighting_fs_path = (Game::shader_path() / "lighting.fs").string();
    Shader shader = LoadShader(lighting_instanced_vs_path.c_str(), lighting_fs_path.c_str());

    while (!WindowShouldClose()) {
        UpdateCamera(&camera, CAMERA_FREE);
        //std::cout << camera.position.x << " " << camera.position.y << " " << camera.position.z << "\n";

        if (IsKeyPressed(KEY_SPACE)) paused = !paused;

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 mouse = GetMousePosition();
            Ray ray = GetMouseRay(mouse, camera);
            for (int i = 0; i < width * height * depth; i++) {
                if (host_grid[i]) continue;
                int x = i % width;
                int y = (i / width) % height;
                int z = i / (width * height);
                BoundingBox box = {
                    { (float)x - 0.5f, (float)y - 0.5f, (float)z - 0.5f },
                    { (float)x + 0.5f, (float)y + 0.5f, (float)z + 0.5f }
                };
                RayCollision collision = GetRayCollisionBox(ray, box);
                if (collision.hit == true) {
                    host_grid[i] = 1;
                    break;
                }
            }
            compute::copy(host_grid.begin(), host_grid.end(), d_current.begin(), queue);
        }

        if (!paused) {
            queue.enqueue_nd_range_kernel(kernel, 3, nullptr, global_size, nullptr);
            queue.finish();
            std::swap(d_current, d_next);
            kernel.set_arg(0, d_current);
            kernel.set_arg(1, d_next);
        }

        compute::copy(d_current.begin(), d_current.end(), host_grid.begin(), queue);
        positions.clear();
        for (size_t z = 0; z < depth; ++z) {
            for (size_t y = 0; y < height; ++y) {
                for (size_t x = 0; x < width; ++x) {
                    size_t idx = z * width * height + y * width + x;
                    if (host_grid[idx]) {
                        positions.push_back({ (float)x, (float)y, (float)z });
                    }
                }
            }
        }

        BeginDrawing();
        ClearBackground(BLACK);

        BeginMode3D(camera);
        for (auto& pos : positions) {
            DrawModel(cubeModel, pos, 0.9f, WHITE);
        }
        Game::draw_gizmo(camera, { 0.01f, 0.05f, 0.f });
        EndMode3D();

        Game::camera_debug_display(camera);
        DrawText(paused ? "\n\n\n[PAUSED] Press SPACE to resume" : "Press SPACE to pause", 10, 10, 20, LIGHTGRAY);
        DrawFPS(10, 40);
        EndDrawing();
    }

    UnloadModel(cubeModel);
    CloseWindow();
    return 0;
}
