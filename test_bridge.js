// import { createRequire } from 'node:module';
// const require = createRequire(import.meta.url);
// const virtualTrackpad = require('./build/Release/virtual_trackpad.node');

// // Linux Kernel Input Constants
// const EV_SYN = 0x00, EV_KEY = 0x01, EV_ABS = 0x03;
// const BTN_TOUCH = 0x14a, BTN_TOOL_FINGER = 0x145, BTN_TOOL_DOUBLETAP = 0x14d;
// const ABS_X = 0x00, ABS_Y = 0x01;
// const ABS_MT_SLOT = 0x2f, ABS_MT_POSITION_X = 0x35, ABS_MT_POSITION_Y = 0x36, ABS_MT_TRACKING_ID = 0x39;

// const isStarted = virtualTrackpad.initDevice();

// if (isStarted) {
//     console.log("Device Online! Hover mouse over a browser NOW.");
    
//     setTimeout(async () => {
//         let yPos = 500; // Starting Y coordinate

//         // --- FRAME 1: FINGERS TOUCH DOWN ---
//         virtualTrackpad.emitEvent(EV_KEY, BTN_TOUCH, 1);
//         virtualTrackpad.emitEvent(EV_KEY, BTN_TOOL_DOUBLETAP, 1);

//         // Finger 0
//         virtualTrackpad.emitEvent(EV_ABS, ABS_MT_SLOT, 0);
//         virtualTrackpad.emitEvent(EV_ABS, ABS_MT_TRACKING_ID, 100);
//         virtualTrackpad.emitEvent(EV_ABS, ABS_MT_POSITION_X, 500);
//         virtualTrackpad.emitEvent(EV_ABS, ABS_MT_POSITION_Y, yPos);
        
//         // Finger 1
//         virtualTrackpad.emitEvent(EV_ABS, ABS_MT_SLOT, 1);
//         virtualTrackpad.emitEvent(EV_ABS, ABS_MT_TRACKING_ID, 101);
//         virtualTrackpad.emitEvent(EV_ABS, ABS_MT_POSITION_X, 550);
//         virtualTrackpad.emitEvent(EV_ABS, ABS_MT_POSITION_Y, yPos);

//         // Legacy Fallback (Required by Wayland)
//         virtualTrackpad.emitEvent(EV_ABS, ABS_X, 500);
//         virtualTrackpad.emitEvent(EV_ABS, ABS_Y, yPos);

//         virtualTrackpad.sync(); // End of Frame 1

//         // --- FRAMES 2 to 40: SMOOTH SCROLLING ---
//         for (let i = 0; i < 40; i++) {
//             await new Promise(r => setTimeout(r, 10)); 
//             yPos -= 5; // Move down 5 pixels

//             // Finger 0 Update
//             virtualTrackpad.emitEvent(EV_ABS, ABS_MT_SLOT, 0);
//             virtualTrackpad.emitEvent(EV_ABS, ABS_MT_POSITION_Y, yPos);
//             // Finger 1 Update
//             virtualTrackpad.emitEvent(EV_ABS, ABS_MT_SLOT, 1);
//             virtualTrackpad.emitEvent(EV_ABS, ABS_MT_POSITION_Y, yPos);
//             // Legacy Update
//             virtualTrackpad.emitEvent(EV_ABS, ABS_Y, yPos);

//             virtualTrackpad.sync(); // End of Frame
//         }
        
//         // --- FINAL FRAME: FINGERS LIFT OFF ---
//         virtualTrackpad.emitEvent(EV_KEY, BTN_TOUCH, 0);
//         virtualTrackpad.emitEvent(EV_KEY, BTN_TOOL_DOUBLETAP, 0);
        
//         virtualTrackpad.emitEvent(EV_ABS, ABS_MT_SLOT, 0);
//         virtualTrackpad.emitEvent(EV_ABS, ABS_MT_TRACKING_ID, -1);
        
//         virtualTrackpad.emitEvent(EV_ABS, ABS_MT_SLOT, 1);
//         virtualTrackpad.emitEvent(EV_ABS, ABS_MT_TRACKING_ID, -1);
        
//         virtualTrackpad.sync(); // End of Final Frame

//         console.log("Perfect Multi-Touch Scroll Executed!");
//         setTimeout(() => virtualTrackpad.destroyDevice(), 100);
        
//     }, 3000);
// }

import { createRequire } from 'node:module';
const require = createRequire(import.meta.url);
const virtualTrackpad = require('./build/Release/virtual_trackpad.node');

// Let's detect the OS in JS just so we can log helpful messages!
const isMac = process.platform === 'darwin';

const isStarted = virtualTrackpad.initDevice();

if (isStarted) {
    console.log(`🚀 God Mode Device Online! (Detected OS: ${process.platform})`);
    console.log("⚠️ Hands off the mouse! Automated test starting in 3 seconds...");
    
    setTimeout(async () => {
        // ==========================================
        // TEST 1: VIRTUAL MOUSE (Move) - WORKS ON ALL OS
        // ==========================================
        console.log("👉 [TEST 1] Moving the cursor diagonally...");
        for (let i = 0; i < 40; i++) {
            await new Promise(r => setTimeout(r, 10)); 
            // sendMouse(action, val1, val2) -> action 0 is Move
            virtualTrackpad.sendMouse(0, 5, 5); 
        }
        
        // ==========================================
        // TEST 2: VIRTUAL MOUSE (Click) - WORKS ON ALL OS
        // ==========================================
        console.log("👆 [TEST 2] Executing Left Click...");
        // action 1 is Click. val1 = 1 (Down), val1 = 0 (Up)
        virtualTrackpad.sendMouse(1, 1, 0); // Mouse Down
        await new Promise(r => setTimeout(r, 50)); // Hold for 50ms
        virtualTrackpad.sendMouse(1, 0, 0); // Mouse Up

        await new Promise(r => setTimeout(r, 500)); // Pause

        // ==========================================
        // TEST 3: VIRTUAL MOUSE (Scroll) - WORKS ON ALL OS
        // ==========================================
        console.log("📜 [TEST 3] Scrolling the page down...");
        for(let i = 0; i < 10; i++) {
             await new Promise(r => setTimeout(r, 50)); 
             // action 2 is Scroll. 
             // Windows uses -120 for down. macOS C++ block converts any negative number to -1 line.
             virtualTrackpad.sendMouse(2, -120, 0); 
        }

        // ==========================================
        // TEST 4: VIRTUAL TOUCHSCREEN - OS DEPENDENT
        // ==========================================
        if (isMac) {
            console.log("🛑 [TEST 4] Skipping Touchscreen tests. macOS blocks user-space touch APIs.");
        } else {
            console.log("🔍 [TEST 4] Executing Pinch-to-Zoom (Windows/Linux only)...");
            let f1_x = 450, f1_y = 500; 
            let f2_x = 550, f2_y = 500; 

            virtualTrackpad.sendTouch(0, 100, f1_x, f1_y);
            virtualTrackpad.sendTouch(1, 101, f2_x, f2_y);
            virtualTrackpad.sync();

            for (let i = 0; i < 30; i++) {
                await new Promise(r => setTimeout(r, 15)); 
                f1_x -= 5; 
                f2_x += 5; 
                virtualTrackpad.sendTouch(0, 100, f1_x, f1_y);
                virtualTrackpad.sendTouch(1, 101, f2_x, f2_y);
                virtualTrackpad.sync();
            }

            virtualTrackpad.sendTouch(0, -1, f1_x, f1_y);
            virtualTrackpad.sendTouch(1, -1, f2_x, f2_y);
            virtualTrackpad.sync();
        }

        console.log("✅ Universal test sequence complete!");
        setTimeout(() => virtualTrackpad.destroyDevice(), 100);
        
    }, 3000);
} else {
    console.log("❌ Failed to initialize device.");
}