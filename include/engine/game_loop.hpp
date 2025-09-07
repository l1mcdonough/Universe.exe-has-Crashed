bool paused = false;
void gameLoop(Camera3D camera) {
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
        Engine::draw_gizmo(camera, { 0.01f, 0.05f, 0.f });
        EndMode3D();

        Engine::camera_debug_display(camera);
        DrawText(paused ? "\n\n\n[PAUSED] Press SPACE to resume" : "Press SPACE to pause", 10, 10, 20, LIGHTGRAY);
        DrawFPS(10, 40);
        EndDrawing();
    }
}
