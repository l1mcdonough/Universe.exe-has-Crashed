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

using namespace Game::RayExtend;
namespace compute = boost::compute;
int main(int argc, char** args)
{
    // <compute> Demonstrate that boost compute is working
    compute::device gpu = compute::system::default_device();
    compute::context cl_context(gpu);
    compute::command_queue queue(cl_context, gpu);
    const size_t sample_count = 10000000;
    std::vector<float> host_vector(sample_count);
    std::vector<float> return_vector(sample_count);
    for(size_t ii = 0; ii < sample_count; ++ii)
        host_vector[ii] = GetRandomValue(0, 100);
    compute::vector<float> device_vector(sample_count, cl_context);
    compute::copy(host_vector.begin(), host_vector.end(), device_vector.begin(), queue);
    compute::transform(
        device_vector.begin(),
        device_vector.end(),
        device_vector.begin(),
        compute::sqrt<float>(),
        queue
    );
    compute::copy(device_vector.begin(), device_vector.end(), return_vector.begin(), queue);
    for(size_t ii = 0; ii < 10; ++ii)
        std::cout << host_vector[ii] << " ";
    std::cout << "\n";
    for(size_t ii = 0; ii < 10; ++ii)
        std::cout << return_vector[ii] << " ";
    std::cout << "\n";
    // </compute>

    Game::Application application;

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


        // Get shader locations
    shader.locs[SHADER_LOC_MATRIX_MVP] = GetShaderLocation(shader, "mvp");
    shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");

    // Set shader value: ambient light level
    int ambientLoc = GetShaderLocation(shader, "ambient");
    float ambientLight[4] = {0.2f, 0.2f, 0.2f, 1.0f};
    SetShaderValue(shader, ambientLoc, ambientLight, SHADER_UNIFORM_VEC4);

    // Create one light
    CreateLight(LIGHT_DIRECTIONAL, Vector3{ 50.0f, 50.0f, 0.0f }, Vector3Zero(), WHITE, shader);

    // NOTE: We are assigning the intancing shader to material.shader
    // to be used on mesh drawing with DrawMeshInstanced()
    Material matInstances = LoadMaterialDefault();
    matInstances.shader = shader;
    matInstances.maps[MATERIAL_MAP_DIFFUSE].color = RED;

    // Load default material (using raylib intenral default shader) for non-instanced mesh drawing
    // WARNING: Default shader enables vertex color attribute BUT GenMeshCube() does not generate vertex colors, so,
    // when drawing the color attribute is disabled and a default color value is provided as input for thevertex attribute
    Material matDefault = LoadMaterialDefault();
    matDefault.maps[MATERIAL_MAP_DIFFUSE].color = BLUE;
    Mesh cube = GenMeshCube(1.0f, 1.0f, 1.0f);

    while (!WindowShouldClose()) {
        UpdateCamera(&camera, CAMERA_FREE);
        //std::cout << camera.position.x << " " << camera.position.y << " " << camera.position.z << "\n";
        // Update the light shader with the camera view position
        float cameraPos[3] = { camera.position.x, camera.position.y, camera.position.z };
        SetShaderValue(shader, shader.locs[SHADER_LOC_VECTOR_VIEW], cameraPos, SHADER_UNIFORM_VEC3);

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
        transforms.clear();
        for (size_t z = 0; z < depth; ++z) {
            for (size_t y = 0; y < height; ++y) {
                for (size_t x = 0; x < width; ++x) {
                    size_t idx = z * width * height + y * width + x;
                    if (host_grid[idx]) {
                        Matrix translation = MatrixTranslate(x, y, z);
                        transforms.push_back(translation);
                    }
                }
            }
        }

        BeginDrawing();
            ClearBackground(BLACK);

            BeginMode3D(camera);
                DrawMeshInstanced(cube, matInstances, transforms.data(), transforms.size());
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
