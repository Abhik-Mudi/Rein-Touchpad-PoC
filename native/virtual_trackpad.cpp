#include <napi.h>

// ==========================================
// LINUX IMPLEMENTATION (uinput Trackpad + Mouse)
// ==========================================
#ifdef __linux__
#include <linux/uinput.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int uinput_fd = -1;

void emit(int fd, int type, int code, int val) {
    struct input_event ie;
    ie.type = type;
    ie.code = code;
    ie.value = val;
    ie.time.tv_sec = 0;
    ie.time.tv_usec = 0;
    write(fd, &ie, sizeof(ie));
}

void setup_abs(int fd, int code, int min, int max) {
    struct uinput_abs_setup abs_setup;
    memset(&abs_setup, 0, sizeof(abs_setup));
    abs_setup.code = code;
    abs_setup.absinfo.minimum = min;
    abs_setup.absinfo.maximum = max;
    abs_setup.absinfo.resolution = 12; 
    ioctl(fd, UI_ABS_SETUP, &abs_setup);
}

bool OS_InitDevice() {
    uinput_fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (uinput_fd < 0) return false;

    // Enable Touch & Buttons
    ioctl(uinput_fd, UI_SET_EVBIT, EV_KEY);
    ioctl(uinput_fd, UI_SET_KEYBIT, BTN_LEFT);
    ioctl(uinput_fd, UI_SET_KEYBIT, BTN_RIGHT);
    ioctl(uinput_fd, UI_SET_KEYBIT, BTN_TOUCH);
    ioctl(uinput_fd, UI_SET_KEYBIT, BTN_TOOL_FINGER);
    
    // ✅ ADDED THIS: Tells Linux we are allowed to use Double-Tap (0x14d)
    ioctl(uinput_fd, UI_SET_KEYBIT, 0x14d); 

    // Enable Absolute Multi-Touch (For sendTouch)
    ioctl(uinput_fd, UI_SET_EVBIT, EV_ABS);
    ioctl(uinput_fd, UI_SET_ABSBIT, ABS_X);
    ioctl(uinput_fd, UI_SET_ABSBIT, ABS_Y);
    ioctl(uinput_fd, UI_SET_ABSBIT, ABS_MT_SLOT);
    ioctl(uinput_fd, UI_SET_ABSBIT, ABS_MT_TRACKING_ID);
    ioctl(uinput_fd, UI_SET_ABSBIT, ABS_MT_POSITION_X);
    ioctl(uinput_fd, UI_SET_ABSBIT, ABS_MT_POSITION_Y);

    // Enable Relative Movement & Scroll (For sendMouse)
    ioctl(uinput_fd, UI_SET_EVBIT, EV_REL);
    ioctl(uinput_fd, UI_SET_RELBIT, REL_X);
    ioctl(uinput_fd, UI_SET_RELBIT, REL_Y);
    ioctl(uinput_fd, UI_SET_RELBIT, REL_WHEEL);

    ioctl(uinput_fd, UI_SET_PROPBIT, INPUT_PROP_POINTER);
    ioctl(uinput_fd, UI_SET_PROPBIT, INPUT_PROP_BUTTONPAD);

    setup_abs(uinput_fd, ABS_X, 0, 2000);
    setup_abs(uinput_fd, ABS_Y, 0, 2000);
    setup_abs(uinput_fd, ABS_MT_POSITION_X, 0, 2000);
    setup_abs(uinput_fd, ABS_MT_POSITION_Y, 0, 2000);
    setup_abs(uinput_fd, ABS_MT_SLOT, 0, 2);

    struct uinput_setup usetup;
    memset(&usetup, 0, sizeof(usetup));
    usetup.id.bustype = BUS_USB;
    usetup.id.vendor = 0x1234;
    usetup.id.product = 0x5678;
    strcpy(usetup.name, "Rein God Mode Input"); 

    ioctl(uinput_fd, UI_DEV_SETUP, &usetup);
    ioctl(uinput_fd, UI_DEV_CREATE);

    return true;
}

void OS_SendTouch(int slot, int tracking_id, int x, int y) {
    if (uinput_fd < 0) return;
    emit(uinput_fd, EV_ABS, ABS_MT_SLOT, slot);

    if (tracking_id == -1) {
        emit(uinput_fd, EV_ABS, ABS_MT_TRACKING_ID, -1);
    } else {
        emit(uinput_fd, EV_ABS, ABS_MT_TRACKING_ID, tracking_id);
        emit(uinput_fd, EV_ABS, ABS_MT_POSITION_X, x);
        emit(uinput_fd, EV_ABS, ABS_MT_POSITION_Y, y);
    }
}

void OS_SendMouse(int action, int val1, int val2) {
    if (uinput_fd < 0) return;
    if (action == 0) { 
        emit(uinput_fd, EV_REL, REL_X, val1);
        emit(uinput_fd, EV_REL, REL_Y, val2);
    } else if (action == 1) { 
        emit(uinput_fd, EV_KEY, BTN_LEFT, val1);
    } else if (action == 2) { 
        emit(uinput_fd, EV_REL, REL_WHEEL, val1);
    }
    emit(uinput_fd, EV_SYN, SYN_REPORT, 0); 
}

void OS_EmitEvent(int type, int code, int val) {
    if (uinput_fd >= 0) emit(uinput_fd, type, code, val);
}

void OS_Sync() {
    if (uinput_fd >= 0) emit(uinput_fd, EV_SYN, SYN_REPORT, 0);
}

void OS_DestroyDevice() {
    if (uinput_fd >= 0) {
        ioctl(uinput_fd, UI_DEV_DESTROY);
        close(uinput_fd);
        uinput_fd = -1;
    }
}

// ==========================================
// WINDOWS IMPLEMENTATION (Hybrid Touch + Mouse)
// ==========================================
#elif _WIN32
#include <windows.h>
#include <iostream>

POINTER_TOUCH_INFO contacts[2];
bool is_contact_active[2] = {false, false};
bool pending_update = false;

bool OS_InitDevice() {
    // Initialize Touch Injection (SendInput doesn't need init)
    return InitializeTouchInjection(2, TOUCH_FEEDBACK_DEFAULT) != 0;
}

// 1. THE TOUCH API (For modern gestures & absolute positioning)
void OS_SendTouch(int slot, int tracking_id, int x, int y) {
    if (slot < 0 || slot >= 2) return;

    memset(&contacts[slot], 0, sizeof(POINTER_TOUCH_INFO));
    contacts[slot].pointerInfo.pointerType = PT_TOUCH;
    contacts[slot].pointerInfo.pointerId = slot; 
    contacts[slot].pointerInfo.ptPixelLocation.x = x;
    contacts[slot].pointerInfo.ptPixelLocation.y = y;
    contacts[slot].touchFlags = TOUCH_FLAG_NONE;
    contacts[slot].touchMask = TOUCH_MASK_NONE; 

    if (tracking_id == -1) {
        contacts[slot].pointerInfo.pointerFlags = POINTER_FLAG_UP;
        is_contact_active[slot] = false;
    } else {
        if (!is_contact_active[slot]) {
            contacts[slot].pointerInfo.pointerFlags = POINTER_FLAG_DOWN | POINTER_FLAG_INRANGE | POINTER_FLAG_INCONTACT;
            is_contact_active[slot] = true;
        } else {
            contacts[slot].pointerInfo.pointerFlags = POINTER_FLAG_UPDATE | POINTER_FLAG_INRANGE | POINTER_FLAG_INCONTACT;
        }
    }
    pending_update = true;
}

void OS_Sync() {
    if (!pending_update) return;

    POINTER_TOUCH_INFO frame_contacts[2];
    int count = 0;
    
    for (int i = 0; i < 2; i++) {
        if (is_contact_active[i] || contacts[i].pointerInfo.pointerFlags == POINTER_FLAG_UP) {
            frame_contacts[count] = contacts[i];
            count++;
            if (contacts[i].pointerInfo.pointerFlags == POINTER_FLAG_UP) {
                contacts[i].pointerInfo.pointerFlags = 0; 
            }
        }
    }
    if (count > 0) InjectTouchInput(count, frame_contacts);
    pending_update = false;
}

// 2. THE MOUSE API (For legacy scrolling and global relative movement)
void OS_SendMouse(int action, int val1, int val2) {
    INPUT input = {0};
    input.type = INPUT_MOUSE;

    if (action == 0) { // Move (val1 = dx, val2 = dy)
        input.mi.dx = val1;
        input.mi.dy = val2;
        input.mi.dwFlags = MOUSEEVENTF_MOVE;
    } else if (action == 1) { // Click (val1 = 1 down, 0 up)
        if (val1 == 1) input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
        else if (val1 == 0) input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
    } else if (action == 2) { // Scroll (val1 = wheel data)
        input.mi.mouseData = val1; 
        input.mi.dwFlags = MOUSEEVENTF_WHEEL;
    }
    SendInput(1, &input, sizeof(INPUT));
}

void OS_DestroyDevice() {}


// ==========================================
// MACOS IMPLEMENTATION (CoreGraphics)
// ==========================================
#elif __APPLE__
#include <ApplicationServices/ApplicationServices.h>

bool OS_InitDevice() {
    return true; // CoreGraphics handles its own session initialization
}

// 1. THE TOUCH API (The macOS Trap)
void OS_SendTouch(int slot, int tracking_id, int x, int y) {
    // WARNING: macOS does NOT have a public user-space Touch API!
    // We leave this empty because Apple strictly bans faking a trackpad 
    // without a Kernel/System Extension.
}

// 2. THE MOUSE API (The Global Fallback)
void OS_SendMouse(int action, int val1, int val2) {
    // Create an event source
    CGEventSourceRef source = CGEventSourceCreate(kCGEventSourceStateHIDSystemState);

    if (action == 0) { 
        // MOVE: Calculate relative movement
        CGEventRef getPos = CGEventCreate(NULL);
        CGPoint currentPos = CGEventGetLocation(getPos);
        CFRelease(getPos);

        currentPos.x += val1; // Add dx
        currentPos.y += val2; // Add dy

        CGEventRef move = CGEventCreateMouseEvent(source, kCGEventMouseMoved, currentPos, kCGMouseButtonLeft);
        CGEventPost(kCGHIDEventTap, move);
        CFRelease(move);
    } 
    else if (action == 1) { 
        // CLICK: Left Down or Left Up
        CGEventRef getPos = CGEventCreate(NULL);
        CGPoint currentPos = CGEventGetLocation(getPos);
        CFRelease(getPos);

        CGEventType type = (val1 == 1) ? kCGEventLeftMouseDown : kCGEventLeftMouseUp;
        CGEventRef click = CGEventCreateMouseEvent(source, type, currentPos, kCGMouseButtonLeft);
        CGEventPost(kCGHIDEventTap, click);
        CFRelease(click);
    } 
    else if (action == 2) { 
        // SCROLL: Line-based scrolling
        // macOS handles scroll values differently than Windows (usually 1 or -1 lines)
        int lines = (val1 > 0) ? 1 : -1; 
        CGEventRef scroll = CGEventCreateScrollWheelEvent(source, kCGScrollEventUnitLine, 1, lines);
        CGEventPost(kCGHIDEventTap, scroll);
        CFRelease(scroll);
    }

    CFRelease(source);
}

void OS_Sync() {}
void OS_DestroyDevice() {}

#endif

// ==========================================
// THE JAVASCRIPT BRIDGE (Runs on ALL OSes)
// ==========================================
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

NODE_API_MODULE(virtual_trackpad, Init)