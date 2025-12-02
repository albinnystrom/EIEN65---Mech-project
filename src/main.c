#include "raylib.h"

// Only define this in ONE source file:
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

int main(void)
{
    InitWindow(800, 450, "raylib + raygui example");
    SetTargetFPS(60);

    bool showWindow = true;
    Rectangle windowRect = { 200, 100, 400, 250 };
    float sliderValue = 0.5f;

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawText("Hello from raylib + raygui!", 20, 20, 20, DARKGRAY);

        if (GuiButton((Rectangle){ 20, 60, 140, 30 }, "Toggle Window"))
        {
            showWindow = !showWindow;
        }

        if (showWindow)
        {
            if (GuiWindowBox(windowRect, "Demo Window"))
            {
                showWindow = false;
            }

            GuiLabel(
                (Rectangle){ windowRect.x + 20, windowRect.y + 50, 200, 20 },
                "This is a label"
            );

            GuiButton(
                (Rectangle){ windowRect.x + 20, windowRect.y + 80, 120, 30 },
                "A Button"
            );

            sliderValue = GuiSlider(
                (Rectangle){ windowRect.x + 20, windowRect.y + 130, 200, 20 },
                "Min",
                "Max",
                &sliderValue,
                0.0f,
                1.0f
            );

        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
