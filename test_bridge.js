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

const isStarted = virtualTrackpad.initDevice();

if (isStarted) {
    console.log("Device Online! (Dual Touch + Mouse Architecture)");
    console.log("Hover your physical mouse over the Windows Desktop or File Explorer...");
    
    setTimeout(async () => {
        // ==========================================
        // TEST 1: VIRTUAL MOUSE (Relative Movement)
        // ==========================================
        console.log("[TEST 1] Moving the cursor diagonally...");
        for (let i = 0; i < 40; i++) {
            await new Promise(r => setTimeout(r, 10)); 
            // sendMouse(action, val1, val2)
            // action 0 = Move. Moving +5 pixels X (Right) and +5 pixels Y (Down)
            virtualTrackpad.sendMouse(0, 5, 5); 
        }
        
        // ==========================================
        // TEST 2: VIRTUAL MOUSE (Legacy Scroll)
        // ==========================================
        console.log("[TEST 2] Scrolling the page...");
        for(let i = 0; i < 5; i++) {
             await new Promise(r => setTimeout(r, 100)); 
             // action 2 = Scroll. val1 = -120 (One standard scroll wheel tick down)
             virtualTrackpad.sendMouse(2, -120, 0);
        }

        // ==========================================
        // TEST 3: VIRTUAL TOUCHSCREEN (Absolute Tap)
        // ==========================================
        console.log("[TEST 3] Tapping screen at exact coordinates (500, 500)...");
        // sendTouch(slot, tracking_id, x, y)
        virtualTrackpad.sendTouch(0, 100, 500, 500); // Finger touches glass
        virtualTrackpad.sync();
        
        await new Promise(r => setTimeout(r, 50)); // Hold for 50ms
        
        virtualTrackpad.sendTouch(0, -1, 500, 500);  // Finger lifts off
        virtualTrackpad.sync();

        console.log("All commands executed");
        setTimeout(() => virtualTrackpad.destroyDevice(), 100);
        
    }, 3000);
} else {
    console.log("❌ Failed to initialize device.");
}