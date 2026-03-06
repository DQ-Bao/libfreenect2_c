#include "libfreenect2_c.h"
#include <math.h>
#include <raylib.h>
#include <stdio.h>

static inline void bgra_to_rgba(unsigned char* data, int count) {
    unsigned char* end = data + count * 4;
    while (data < end) {
        unsigned char tmp = data[0];
        data[0] = data[2];
        data[2] = tmp;
        data += 4;
    }
}

int main(void) {
    fn2_context* fn2 = fn2_create_context(NULL);
    if (fn2_enumerate_devices(fn2) == 0) {
        printf("No device connected\n");
        return 1;
    }
    char serial[256];
    fn2_get_default_device_serial_number(fn2, serial, sizeof(serial));
    fn2_device* dev = fn2_open_device_by_serial(fn2, serial);
    if (!dev) {
        printf("Failed to open device\n");
        return 1;
    }
    unsigned int types = 0;
    types |= FN2_FRAME_TYPE_COLOR;
    fn2_sync_multi_frame_listener* listener = fn2_create_sync_multi_frame_listener(types);
    fn2_device_set_color_frame_listener(dev, listener);

    if (!fn2_device_start(dev, true, false)) {
        printf("Failed to start device\n");
        return 1;
    }

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(1280, 800, "Kinect V2 Color");
    SetTargetFPS(60);
    Image image = GenImageColor(1920, 1080, BLACK);
    Texture2D texture = LoadTextureFromImage(image);
    UnloadImage(image);

    while (!WindowShouldClose()) {
        fn2_frame color = { 0 };
        fn2_wait_for_new_frame_forever(listener, &color, NULL, NULL);
        bgra_to_rgba(color.data, 1920 * 1080);
        UpdateTexture(texture, color.data);

        float window_w = (float)GetScreenWidth();
        float window_h = (float)GetScreenHeight();
        float image_w = 1920.0f;
        float image_h = 1080.0f;
        float scale = fminf(window_w / image_w, window_h / image_h);
        float draw_w = image_w * scale;
        float draw_h = image_h * scale;
        float offset_x = (window_w - draw_w) * 0.5f;
        float offset_y = (window_h - draw_h) * 0.5f;

        BeginDrawing();
        ClearBackground(BLACK);
        DrawTexturePro(texture, (Rectangle){ 0, 0, image_w, image_h },
                       (Rectangle){ offset_x, offset_y, draw_w, draw_h }, (Vector2){ 0, 0 }, 0.0f, WHITE);
        EndDrawing();
        fn2_release_frames(listener, &color, NULL, NULL);
    }

    fn2_device_stop(dev);
    UnloadTexture(texture);
    CloseWindow();
    fn2_destroy_sync_multi_frame_listener(listener);
    fn2_device_close(dev);
    fn2_destroy_context(fn2);
    return 0;
}
