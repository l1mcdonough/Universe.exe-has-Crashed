/*******************************************************************************************
*
*   raylib [shapes] example - Draw basic shapes 2d (rectangle, circle, line...)
*
*   Example originally created with raylib 1.0, last time updated with raylib 4.2
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2014-2024 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 1300;
    const int screenHeight = 1300;

    InitWindow(screenWidth, screenHeight, "raylib [shapes] example - basic shapes drawing");

    float rotation = 0.0f;

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    int rectSize = 5;
    int grid[256][256] = { 0 };
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        Vector2 mousePosition = GetMousePosition();

        // Check if the left mouse button is clicked
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            // Calculate the column and row of the rectangle clicked
            int clickedColumn = mousePosition.x / rectSize;
            int clickedRow = mousePosition.y / rectSize;
            grid[clickedRow][clickedColumn] = 255;
        }
        BeginDrawing();

        ClearBackground(RAYWHITE);

        DrawText("some basic shapes available on raylib", 20, 20, 20, DARKGRAY);


        // Rectangle shapes and lines
        for (int y = 0; y < 256; y++) {
            for (int x = 0; x < 256; x++) {
                int greenValue = grid[y][x];
                Color rectColor = { 0, greenValue, 0, 255 };
                DrawRectangle(x * rectSize, y * rectSize, rectSize, rectSize, rectColor);
            }
        }
        
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

int getAdjacentLifeTotal(int grid[256][256], int x, int y) {
    int x1 = x - 1;
    if (x1 < 0)
        x1 = 0;
    int y1 = y - 1;
    if (y1 < 0)
        y1 = 0;
    int x2 = x + 2;
    if (x2 > 256)
        x2 = 256;
    int y2 = y + 2;
    if (y2 > 256)
        y2 = 256;

}