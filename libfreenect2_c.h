#ifndef LIBFREENECT2_C_H
#define LIBFREENECT2_C_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct fn2_context fn2_context;
typedef struct fn2_device fn2_device;

// Color camera calibration parameters.
// Kinect v2 includes factory preset values for these parameters. They are used in Registration.
typedef struct fn2_color_camera_params {
    // Intrinsic parameters
    float fx; // Focal length x (pixel)
    float fy; // Focal length y (pixel)
    float cx; // Principal point x (pixel)
    float cy; // Principal point y (pixel)

    // Extrinsic parameters
    //
    // These parameters are used in [a
    // formula](https://github.com/OpenKinect/libfreenect2/issues/41#issuecomment-72022111) to map coordinates in the
    // depth camera to the color camera.
    //
    // They cannot be used for matrix transformation.
    float shift_d, shift_m;

    float mx_x3y0; // xxx
    float mx_x0y3; // yyy
    float mx_x2y1; // xxy
    float mx_x1y2; // yyx
    float mx_x2y0; // xx
    float mx_x0y2; // yy
    float mx_x1y1; // xy
    float mx_x1y0; // x
    float mx_x0y1; // y
    float mx_x0y0; // 1

    float my_x3y0; // xxx
    float my_x0y3; // yyy
    float my_x2y1; // xxy
    float my_x1y2; // yyx
    float my_x2y0; // xx
    float my_x0y2; // yy
    float my_x1y1; // xy
    float my_x1y0; // x
    float my_x0y1; // y
    float my_x0y0; // 1
} fn2_color_camera_params;

// IR camera intrinsic calibration parameters.
// Kinect v2 includes factory preset values for these parameters. They are used in depth image decoding, and
// Registration.
typedef struct fn2_ir_camera_params {
    float fx; // Focal length x (pixel)
    float fy; // Focal length y (pixel)
    float cx; // Principal point x (pixel)
    float cy; // Principal point y (pixel)
    float k1; // Radial distortion coefficient, 1st-order
    float k2; // Radial distortion coefficient, 2nd-order
    float k3; // Radial distortion coefficient, 3rd-order
    float p1; // Tangential distortion coefficient
    float p2; // Tangential distortion coefficient
} fn2_ir_camera_params;

// Configuration of depth processing.
typedef struct fn2_config {
    float min_depth; // Clip at this minimum distance (meter).
    float max_depth; // Clip at this maximum distance (meter).

    bool enable_bilateral_filter;  // Remove some "flying pixels".
    bool enable_edge_aware_filter; // Remove pixels on edges because ToF cameras produce noisy edges.
} fn2_config;

typedef enum {
    FN2_FRAME_TYPE_COLOR = 1, // 1920x1080. BGRX or RGBX.
    FN2_FRAME_TYPE_IR = 2,    // 512x424 float. Range is [0.0, 65535.0].
    FN2_FRAME_TYPE_DEPTH = 4, // 512x424 float, unit: mm. Non-positive, NaN, and infinity are invalid or missing data.
} fn2_frame_type;

typedef enum {
    FN2_FRAME_FORMAT_INVALID = 0, // Invalid format.
    FN2_FRAME_FORMAT_RAW = 1,     // Raw bitstream. 'bytes_per_pixel' defines the number of bytes
    FN2_FRAME_FORMAT_FLOAT = 2,   // A 4-byte float per pixel
    FN2_FRAME_FORMAT_BGRX = 4,    // 4 bytes of B, G, R, and unused per pixel
    FN2_FRAME_FORMAT_RGBX = 5,    // 4 bytes of R, G, B, and unused per pixel
    FN2_FRAME_FORMAT_GRAY = 6,    // 1 byte of gray per pixel
} fn2_frame_format;

typedef struct fn2_frame {
    size_t width;            // Length of a line (in pixels).
    size_t height;           // Number of lines in the frame.
    size_t bytes_per_pixel;  // Number of bytes in a pixel. If frame format is 'Raw' this is the buffer size.
    unsigned char* data;     // Data of the frame (aligned). @see See Frame::Type for pixel format.
    unsigned int timestamp;  // Unit: 0.125 millisecond. Usually incrementing by 266 (30Hz) or 533 (15Hz).
    unsigned int sequence;   // Increasing frame sequence number
    float exposure;          // From 0.5 (very bright) to ~60.0 (fully covered)
    float gain;              // From 1.0 (bright) to 1.5 (covered)
    float gamma;             // From 1.0 (bright) to 6.4 (covered)
    unsigned int status;     // zero if ok; non-zero for errors.
    fn2_frame_format format; // Byte format. Informative only, doesn't indicate errors.
} fn2_frame;

typedef bool (*fn2_on_new_frame_callback)(fn2_frame_type type, fn2_frame* frame, void* user_data);

typedef struct fn2_sync_multi_frame_listener fn2_sync_multi_frame_listener;

// Default is min_depth = 0.5, max_depth = 4.5, enable_bilateral_filter = true, enable_edge_aware_filter = true
static inline void fn2_config_default(fn2_config* cfg) {
    cfg->min_depth = 0.5f;
    cfg->max_depth = 4.5f;
    cfg->enable_bilateral_filter = true;
    cfg->enable_edge_aware_filter = false;
}

fn2_context* fn2_create_context(void* usb_context);

void fn2_destroy_context(fn2_context* ctx);

// Must be called before doing anything else.
// Return number of devices, 0 if none.
int fn2_enumerate_devices(fn2_context* ctx);

// Return string length of the serial number, 0 if failed
size_t fn2_get_device_serial_number(fn2_context* ctx, int idx, char* buf, size_t buf_size);

// Return string length of the serial number, 0 if failed
size_t fn2_device_get_serial_number(fn2_device* dev, char* buf, size_t buf_size);

// Return string length of the version, 0 if failed
size_t fn2_device_get_firmware_version(fn2_device* dev, char* buf, size_t buf_size);

// Return string length of the serial number, 0 if failed
size_t fn2_get_default_device_serial_number(fn2_context* ctx, char* buf, size_t buf_size);

// Open device by index with default pipeline.
// Index numbers are not determinstic during enumeration.
// Return New device object, or NULL on failure.
fn2_device* fn2_open_device(fn2_context* ctx, int idx);

// Open device by serial number with default pipeline.
// Return new device object, or NULL on failure.
fn2_device* fn2_open_device_by_serial(fn2_context* ctx, const char* serial);

// Open the first device with default pipeline.
// Return new device object, or NULL on failure.
fn2_device* fn2_open_default_device(fn2_context* ctx);

void fn2_device_set_color_frame_listener(fn2_device* dev, void* listener);

// Start data processing with or without some streams.
// All configurations must only be called before start() or after stop().
// FrameListener will receive frames when the device is running.
// Return true if ok, false if error.
bool fn2_device_start(fn2_device* dev, bool rgb, bool depth);

// Stop data processing.
// Return true if ok, false if error.
bool fn2_device_stop(fn2_device* dev);

// Shut down the device.
// Return true if ok, false if error.
bool fn2_device_close(fn2_device* dev);

fn2_sync_multi_frame_listener* fn2_create_sync_multi_frame_listener(unsigned int frame_types);

void fn2_destroy_sync_multi_frame_listener(fn2_sync_multi_frame_listener* listener);

bool fn2_has_new_frame(fn2_sync_multi_frame_listener* listener);

bool fn2_wait_for_new_frame(fn2_sync_multi_frame_listener* listener, fn2_frame* color, fn2_frame* ir, fn2_frame* depth,
                            int timeout_ms);

void fn2_wait_for_new_frame_forever(fn2_sync_multi_frame_listener* listener, fn2_frame* color, fn2_frame* ir,
                                    fn2_frame* depth);

void fn2_release_frames(fn2_sync_multi_frame_listener* listener, fn2_frame* color, fn2_frame* ir, fn2_frame* depth);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !LIBFREENECT2_C_H
