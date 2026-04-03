#include<napi.h>

#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

// Include libevdev headers
#include <libevdev/libevdev.h>
#include <libevdev/libevdev-uinput.h>

struct libevdev *dev = NULL;
struct libevdev_uinput *uidev = NULL;

bool OS_InitDevice() {
    dev = libevdev_new();
    if (!dev) return false;

    libevdev_set_name(dev, "Rein Virtual Trackpad");
    libevdev_set_id_bustype(dev, BUS_USB);
    libevdev_set_id_vendor(dev, 0x1234);
    libevdev_set_id_product(dev, 0x5678);

    // Enable Relative Movement (Mouse)
    libevdev_enable_event_type(dev, EV_REL);
    libevdev_enable_event_code(dev, EV_REL, REL_X, NULL);
    libevdev_enable_event_code(dev, EV_REL, REL_Y, NULL);
    libevdev_enable_event_code(dev, EV_REL, REL_WHEEL, NULL); // Added for scroll

    // Enable Buttons (Left Click, Right Click)
    libevdev_enable_event_type(dev, EV_KEY);
    libevdev_enable_event_code(dev, EV_KEY, BTN_LEFT, NULL);
    libevdev_enable_event_code(dev, EV_KEY, BTN_RIGHT, NULL);

    int err = libevdev_uinput_create_from_device(dev, LIBEVDEV_UINPUT_OPEN_MANAGED, &uidev);
    if (err != 0) {
        libevdev_free(dev);
        return false;
    }
    return true;
}

// Virtual mouse
void OS_SendMouse(int action, int val1, int val2) {
    if (!uidev) return;

    if (action == 0) { // Move
        libevdev_uinput_write_event(uidev, EV_REL, REL_X, val1);
        libevdev_uinput_write_event(uidev, EV_REL, REL_Y, val2);
    } else if (action == 1) { // Click
        libevdev_uinput_write_event(uidev, EV_KEY, BTN_LEFT, val1);
    } else if (action == 2) { // Scroll
        libevdev_uinput_write_event(uidev, EV_REL, REL_WHEEL, val1);
    }
    
    // Send the frame
    libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0);
}

void OS_SendTouch(int slot, int tracking_id, int x, int y) {
    if (!uidev) return;
    // Multi-touch logic goes here
}

void OS_Sync() {
    if (!uidev) return;
    libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0);
}

void OS_EmitEvent(int type, int code, int val) {
    if (!uidev) return;
    libevdev_uinput_write_event(uidev, type, code, val);
}

void OS_DestroyDevice() {
    if (uidev) {
        libevdev_uinput_destroy(uidev);
        uidev = NULL;
    }
    if (dev) {
        libevdev_free(dev);
        dev = NULL;
    }
}


// THE JAVASCRIPT BRIDGE (Runs on ALL OSes)

Napi::Value InitDeviceWrapper(const Napi::CallbackInfo& info) {
    return Napi::Boolean::New(info.Env(), OS_InitDevice());
}

Napi::Value SendTouchWrapper(const Napi::CallbackInfo& info) {
    int slot = info[0].As<Napi::Number>().Int32Value();
    int tracking_id = info[1].As<Napi::Number>().Int32Value();
    int x = info[2].As<Napi::Number>().Int32Value();
    int y = info[3].As<Napi::Number>().Int32Value();
    OS_SendTouch(slot, tracking_id, x, y);
    return Napi::Boolean::New(info.Env(), true);
}

// NEW: Exposing the Mouse API to JavaScript
Napi::Value SendMouseWrapper(const Napi::CallbackInfo& info) {
    int action = info[0].As<Napi::Number>().Int32Value();
    int val1 = info[1].As<Napi::Number>().Int32Value();
    int val2 = info[2].As<Napi::Number>().Int32Value();
    OS_SendMouse(action, val1, val2);
    return Napi::Boolean::New(info.Env(), true);
}

Napi::Value SyncWrapper(const Napi::CallbackInfo& info) {
    OS_Sync();
    return Napi::Boolean::New(info.Env(), true);
}

Napi::Value DestroyDeviceWrapper(const Napi::CallbackInfo& info) {
    OS_DestroyDevice();
    return Napi::Boolean::New(info.Env(), true);
}

#ifdef __linux__
Napi::Value EmitEventWrapper(const Napi::CallbackInfo& info) {
    int type = info[0].As<Napi::Number>().Int32Value();
    int code = info[1].As<Napi::Number>().Int32Value();
    int val = info[2].As<Napi::Number>().Int32Value();
    OS_EmitEvent(type, code, val); 
    return Napi::Boolean::New(info.Env(), true);
}
#endif

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    #ifdef __linux__
    exports.Set("emitEvent", Napi::Function::New(env, EmitEventWrapper));
    #endif
    exports.Set("initDevice", Napi::Function::New(env, InitDeviceWrapper));
    exports.Set("sendTouch", Napi::Function::New(env, SendTouchWrapper));
    exports.Set("sendMouse", Napi::Function::New(env, SendMouseWrapper));
    exports.Set("sync", Napi::Function::New(env, SyncWrapper));
    exports.Set("destroyDevice", Napi::Function::New(env, DestroyDeviceWrapper));
    return exports;
}

NODE_API_MODULE(linux_libevdev, Init)