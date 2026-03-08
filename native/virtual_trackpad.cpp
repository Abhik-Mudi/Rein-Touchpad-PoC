#include <napi.h>

// ==========================================
// 🐧 LINUX IMPLEMENTATION (uinput)
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

    ioctl(uinput_fd, UI_SET_EVBIT, EV_KEY);
    ioctl(uinput_fd, UI_SET_KEYBIT, BTN_LEFT);
    ioctl(uinput_fd, UI_SET_KEYBIT, BTN_RIGHT);
    ioctl(uinput_fd, UI_SET_KEYBIT, BTN_TOUCH);
    ioctl(uinput_fd, UI_SET_KEYBIT, BTN_TOOL_FINGER);
    ioctl(uinput_fd, UI_SET_KEYBIT, BTN_TOOL_DOUBLETAP);

    ioctl(uinput_fd, UI_SET_EVBIT, EV_ABS);
    ioctl(uinput_fd, UI_SET_ABSBIT, ABS_X);
    ioctl(uinput_fd, UI_SET_ABSBIT, ABS_Y);
    ioctl(uinput_fd, UI_SET_ABSBIT, ABS_MT_SLOT);
    ioctl(uinput_fd, UI_SET_ABSBIT, ABS_MT_TRACKING_ID);
    ioctl(uinput_fd, UI_SET_ABSBIT, ABS_MT_POSITION_X);
    ioctl(uinput_fd, UI_SET_ABSBIT, ABS_MT_POSITION_Y);

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
    strcpy(usetup.name, "Rein Magic Trackpad"); 

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
        
        if (slot == 0) {
            emit(uinput_fd, EV_ABS, ABS_X, x);
            emit(uinput_fd, EV_ABS, ABS_Y, y);
        }
    }
}

void OS_Sync() {
    if (uinput_fd >= 0) {
        emit(uinput_fd, EV_SYN, SYN_REPORT, 0);
    }
}

void OS_DestroyDevice() {
    if (uinput_fd >= 0) {
        ioctl(uinput_fd, UI_DEV_DESTROY);
        close(uinput_fd);
        uinput_fd = -1;
    }
}

// ==========================================
// 🪟 WINDOWS IMPLEMENTATION (Touch Injection)
// ==========================================
#elif _WIN32
#include <windows.h>

bool OS_InitDevice() {
    return InitializeTouchInjection(5, TOUCH_FEEDBACK_NONE) != 0;
}

void OS_SendTouch(int slot, int tracking_id, int x, int y) {
    POINTER_TOUCH_INFO contact;
    memset(&contact, 0, sizeof(POINTER_TOUCH_INFO));

    contact.pointerInfo.pointerType = PT_TOUCH;
    contact.pointerInfo.pointerId = slot; 
    
    contact.pointerInfo.ptPixelLocation.x = x;
    contact.pointerInfo.ptPixelLocation.y = y;

    if (tracking_id == -1) {
        contact.pointerInfo.pointerFlags = POINTER_FLAG_UP;
    } else {
        contact.pointerInfo.pointerFlags = POINTER_FLAG_INCONTACT | POINTER_FLAG_UPDATE;
    }

    contact.touchFlags = TOUCH_FLAG_NONE;
    contact.touchMask = TOUCH_MASK_CONTACTAREA | TOUCH_MASK_ORIENTATION | TOUCH_MASK_PRESSURE;
    
    InjectTouchInput(1, &contact); 
}

void OS_Sync() {
    // Windows automatically syncs when InjectTouchInput is called. No manual sync needed!
}

void OS_DestroyDevice() {
    // Windows cleans up automatically when the process closes.
}

#endif

// ==========================================
// 🌉 THE JAVASCRIPT BRIDGE (Runs on ALL OSes)
// ==========================================
Napi::Value InitDeviceWrapper(const Napi::CallbackInfo& info) {
    bool success = OS_InitDevice();
    return Napi::Boolean::New(info.Env(), success);
}

Napi::Value SendTouchWrapper(const Napi::CallbackInfo& info) {
    if (info.Length() < 4) return Napi::Boolean::New(info.Env(), false);
    
    int slot = info[0].As<Napi::Number>().Int32Value();
    int tracking_id = info[1].As<Napi::Number>().Int32Value();
    int x = info[2].As<Napi::Number>().Int32Value();
    int y = info[3].As<Napi::Number>().Int32Value();

    OS_SendTouch(slot, tracking_id, x, y);
    
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

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set("initDevice", Napi::Function::New(env, InitDeviceWrapper));
    exports.Set("sendTouch", Napi::Function::New(env, SendTouchWrapper));
    exports.Set("sync", Napi::Function::New(env, SyncWrapper));
    exports.Set("destroyDevice", Napi::Function::New(env, DestroyDeviceWrapper));
    return exports;
}

NODE_API_MODULE(virtual_trackpad, Init)