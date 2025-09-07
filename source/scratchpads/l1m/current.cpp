#include <engine/conway.hpp>
#include <engine/visual_debugging.hpp>
using namespace Engine;
namespace compute = boost::compute;

void initialize_host_grid(std::vector<char>& host_grid, size_t width, size_t height);

int main()
{

    OpenCLContext cl;
    Engine::ConwayLayer conway(cl);
    initialize_host_grid(conway.host_grid, conway.width, conway.height);


    compute::program program = compute::program::build_with_source(
        conway3d_kernel_src,
        cl.context
    );
    compute::kernel kernel(program, "conway3d_step");
    kernel.set_arg(0, conway.d_current);
    kernel.set_arg(1, conway.d_next);
    kernel.set_arg(2, (cl_uint)conway.width);
    kernel.set_arg(3, (cl_uint)conway.height);
    kernel.set_arg(4, (cl_uint)conway.depth);

    size_t global_size[3] = { conway.width, conway.height, conway.depth };

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

    const std::string lighting_instanced_vs_path = (Engine::shader_path() / "lighting_instancing.vs").string();
    const std::string lighting_fs_path = (Engine::shader_path() / "lighting.fs").string();
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
            for (int i = 0; i < conway.grid_size; i++) {
                if (conway.host_grid[i]) continue;
                int x = i % conway.width;
                int y = (i / conway.width) % conway.height;
                int z = i / (conway.width * conway.height);
                BoundingBox box = {
                    { (float)x - 0.5f, (float)y - 0.5f, (float)z - 0.5f },
                    { (float)x + 0.5f, (float)y + 0.5f, (float)z + 0.5f }
                };
                RayCollision collision = GetRayCollisionBox(ray, box);
                if (collision.hit == true) {
                    conway.host_grid[i] = 1;
                    break;
                }
            }
            compute::copy(
                conway.host_grid.begin(), 
                conway.host_grid.end(), 
                conway.d_current.begin(), 
                cl.queue
            );
        }

        if (!paused) {
            cl.queue.enqueue_nd_range_kernel(kernel, 3, nullptr, global_size, nullptr);
            cl.queue.finish();
            std::swap(conway.d_current, conway.d_next);
            kernel.set_arg(0, conway.d_current);
            kernel.set_arg(1, conway.d_next);
        }

        compute::copy(
            conway.d_current.begin(), 
            conway.d_current.end(), 
            conway.host_grid.begin(), 
            cl.queue
        );
        conway.transforms.clear();
        for (size_t z = 0; z < conway.depth; ++z) {
            for (size_t y = 0; y < conway.height; ++y) {
                for (size_t x = 0; x < conway.width; ++x) {
                    size_t idx = z * conway.width * conway.height + y * conway.width + x;
                    if (conway.host_grid[idx]) {
                        Matrix translation = MatrixTranslate(x, y, z);
                        conway.transforms.push_back(translation);
                    }
                }
            }
        }

        BeginDrawing();
            ClearBackground(BLACK);

            BeginMode3D(camera);
                DrawMeshInstanced(
                    cube, 
                    matInstances, 
                    conway.transforms.data(), conway.transforms.size()
                );
                Engine::draw_gizmo(camera, { 0.01f, 0.05f, 0.f });
            EndMode3D();

            Engine::camera_debug_display(camera);
            DrawText(paused ? "\n\n\n[PAUSED] Press SPACE to resume" : "Press SPACE to pause", 10, 10, 20, LIGHTGRAY);
            DrawFPS(10, 40);
        EndDrawing();
    }

    UnloadModel(cubeModel);
    CloseWindow();
    return 0;
}

void initialize_host_grid(std::vector<char>& host_grid, size_t width, size_t height)
{
    host_grid[(1 * width * height) + (1 * width) + 2] = 1;
    host_grid[(2 * width * height) + (2 * width) + 3] = 1;
    host_grid[(3 * width * height) + (3 * width) + 1] = 1;
    host_grid[(3 * width * height) + (3 * width) + 2] = 1;
    host_grid[(3 * width * height) + (3 * width) + 3] = 1;
}

