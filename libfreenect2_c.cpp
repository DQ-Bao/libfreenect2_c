#include "libfreenect2_c.h"
#include "libfreenect2/frame_listener_impl.h"
#include "libfreenect2/libfreenect2.hpp"
#include <cstring>

struct fn2_context {
    libfreenect2::Freenect2 impl;
};

struct fn2_device {
    libfreenect2::Freenect2Device* impl;
};

struct fn2_sync_multi_frame_listener {
    libfreenect2::SyncMultiFrameListener impl;
    libfreenect2::FrameMap frames;
};

fn2_context* fn2_create_context(void* usb_context) { return new fn2_context{ libfreenect2::Freenect2(usb_context) }; }

void fn2_destroy_context(fn2_context* ctx) { delete ctx; }

int fn2_enumerate_devices(fn2_context* ctx) { return ctx->impl.enumerateDevices(); }

fn2_device* fn2_open_device(fn2_context* ctx, int idx) {
    libfreenect2::Freenect2Device* dev = ctx->impl.openDevice(idx);
    if (!dev) return nullptr;
    return new fn2_device{ dev };
}

fn2_device* fn2_open_device_by_serial(fn2_context* ctx, const char* serial) {
    libfreenect2::Freenect2Device* dev = ctx->impl.openDevice(serial);
    if (!dev) return nullptr;
    return new fn2_device{ dev };
}

fn2_device* fn2_open_default_device(fn2_context* ctx) {
    libfreenect2::Freenect2Device* dev = ctx->impl.openDefaultDevice();
    if (!dev) return nullptr;
    return new fn2_device{ dev };
}

void fn2_device_set_color_frame_listener(fn2_device* dev, void* listener) {
    dev->impl->setColorFrameListener(static_cast<libfreenect2::FrameListener*>(listener));
}

bool fn2_device_start(fn2_device* dev, bool rgb, bool depth) { return dev->impl->startStreams(rgb, depth); }

bool fn2_device_stop(fn2_device* dev) { return dev->impl->stop(); }

bool fn2_device_close(fn2_device* dev) {
    if (!dev) return true;
    bool closed = dev->impl->close();
    delete dev;
    return closed;
}

static inline size_t fn2_memcpy(char* dst, size_t dst_cap, std::string& src) {
    if (src.size() <= 0 || dst_cap <= src.size()) return 0;
    std::memcpy(dst, src.c_str(), src.size() + 1);
    return src.size();
}

size_t fn2_get_default_device_serial_number(fn2_context* ctx, char* buf, size_t buf_size) {
    std::string serial = ctx->impl.getDefaultDeviceSerialNumber();
    return fn2_memcpy(buf, buf_size, serial);
}

size_t fn2_get_device_serial_number(fn2_context* ctx, int idx, char* buf, size_t buf_size) {
    std::string serial = ctx->impl.getDeviceSerialNumber(idx);
    return fn2_memcpy(buf, buf_size, serial);
}

size_t fn2_device_get_serial_number(fn2_device* dev, char* buf, size_t buf_size) {
    std::string serial = dev->impl->getSerialNumber();
    return fn2_memcpy(buf, buf_size, serial);
}

size_t fn2_device_get_firmware_version(fn2_device* dev, char* buf, size_t buf_size) {
    std::string version = dev->impl->getFirmwareVersion();
    return fn2_memcpy(buf, buf_size, version);
}

fn2_sync_multi_frame_listener* fn2_create_sync_multi_frame_listener(unsigned int frame_types) {
    return new fn2_sync_multi_frame_listener{
        libfreenect2::SyncMultiFrameListener(frame_types),
        libfreenect2::FrameMap(),
    };
}

void fn2_destroy_sync_multi_frame_listener(fn2_sync_multi_frame_listener* listener) { delete listener; }

bool fn2_has_new_frame(fn2_sync_multi_frame_listener* listener) { return listener->impl.hasNewFrame(); }

static inline void copy_frame(libfreenect2::Frame* src, fn2_frame* dst) {
    if (!src || !dst) return;
    dst->width = src->width;
    dst->height = src->height;
    dst->bytes_per_pixel = src->bytes_per_pixel;
    dst->data = src->data;
    dst->timestamp = src->timestamp;
    dst->sequence = src->sequence;
    dst->exposure = src->exposure;
    dst->gain = src->gain;
    dst->gamma = src->gamma;
    dst->status = src->status;
    dst->format = (fn2_frame_format)src->format;
}

bool fn2_wait_for_new_frame(fn2_sync_multi_frame_listener* listener, fn2_frame* color, fn2_frame* ir, fn2_frame* depth,
                            int timeout_ms) {
    if (!listener->impl.waitForNewFrame(listener->frames, timeout_ms)) return false;
    copy_frame(listener->frames[libfreenect2::Frame::Color], color);
    copy_frame(listener->frames[libfreenect2::Frame::Ir], ir);
    copy_frame(listener->frames[libfreenect2::Frame::Depth], depth);
    return true;
}

void fn2_wait_for_new_frame_forever(fn2_sync_multi_frame_listener* listener, fn2_frame* color, fn2_frame* ir,
                                    fn2_frame* depth) {
    listener->impl.waitForNewFrame(listener->frames);
    copy_frame(listener->frames[libfreenect2::Frame::Color], color);
    copy_frame(listener->frames[libfreenect2::Frame::Ir], ir);
    copy_frame(listener->frames[libfreenect2::Frame::Depth], depth);
}

void fn2_release_frames(fn2_sync_multi_frame_listener* listener, fn2_frame* color, fn2_frame* ir, fn2_frame* depth) {
    listener->impl.release(listener->frames);
    if (color) *color = {};
    if (ir) *ir = {};
    if (depth) *depth = {};
}
