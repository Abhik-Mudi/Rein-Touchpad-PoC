import { createRequire } from 'node:module';
const require = createRequire(import.meta.url);
const virtualTrackpad = require('./build/Release/virtual_trackpad.node');

// Linux Kernel Input Constants
const EV_SYN = 0x00, EV_KEY = 0x01, EV_ABS = 0x03;
const BTN_TOUCH = 0x14a, BTN_TOOL_FINGER = 0x145, BTN_TOOL_DOUBLETAP = 0x14d;
const ABS_X = 0x00, ABS_Y = 0x01;
const ABS_MT_SLOT = 0x2f, ABS_MT_POSITION_X = 0x35, ABS_MT_POSITION_Y = 0x36, ABS_MT_TRACKING_ID = 0x39;

const isStarted = virtualTrackpad.initDevice();

if (isStarted) {
    console.log("✅ Device Online! Hover mouse over a browser NOW.");
    
    setTimeout(async () => {
        let yPos = 500; // Starting Y coordinate

        // --- FRAME 1: FINGERS TOUCH DOWN ---
        virtualTrackpad.emitEvent(EV_KEY, BTN_TOUCH, 1);
        virtualTrackpad.emitEvent(EV_KEY, BTN_TOOL_DOUBLETAP, 1);

        // Finger 0
        virtualTrackpad.emitEvent(EV_ABS, ABS_MT_SLOT, 0);
        virtualTrackpad.emitEvent(EV_ABS, ABS_MT_TRACKING_ID, 100);
        virtualTrackpad.emitEvent(EV_ABS, ABS_MT_POSITION_X, 500);
        virtualTrackpad.emitEvent(EV_ABS, ABS_MT_POSITION_Y, yPos);
        
        // Finger 1
        virtualTrackpad.emitEvent(EV_ABS, ABS_MT_SLOT, 1);
        virtualTrackpad.emitEvent(EV_ABS, ABS_MT_TRACKING_ID, 101);
        virtualTrackpad.emitEvent(EV_ABS, ABS_MT_POSITION_X, 550);
        virtualTrackpad.emitEvent(EV_ABS, ABS_MT_POSITION_Y, yPos);

        // Legacy Fallback (Required by Wayland)
        virtualTrackpad.emitEvent(EV_ABS, ABS_X, 500);
        virtualTrackpad.emitEvent(EV_ABS, ABS_Y, yPos);

        virtualTrackpad.sync(); // End of Frame 1

        // --- FRAMES 2 to 40: SMOOTH SCROLLING ---
        for (let i = 0; i < 40; i++) {
            await new Promise(r => setTimeout(r, 10)); 
            yPos -= 5; // Move down 5 pixels

            // Finger 0 Update
            virtualTrackpad.emitEvent(EV_ABS, ABS_MT_SLOT, 0);
            virtualTrackpad.emitEvent(EV_ABS, ABS_MT_POSITION_Y, yPos);
            // Finger 1 Update
            virtualTrackpad.emitEvent(EV_ABS, ABS_MT_SLOT, 1);
            virtualTrackpad.emitEvent(EV_ABS, ABS_MT_POSITION_Y, yPos);
            // Legacy Update
            virtualTrackpad.emitEvent(EV_ABS, ABS_Y, yPos);

            virtualTrackpad.sync(); // End of Frame
        }
        
        // --- FINAL FRAME: FINGERS LIFT OFF ---
        virtualTrackpad.emitEvent(EV_KEY, BTN_TOUCH, 0);
        virtualTrackpad.emitEvent(EV_KEY, BTN_TOOL_DOUBLETAP, 0);
        
        virtualTrackpad.emitEvent(EV_ABS, ABS_MT_SLOT, 0);
        virtualTrackpad.emitEvent(EV_ABS, ABS_MT_TRACKING_ID, -1);
        
        virtualTrackpad.emitEvent(EV_ABS, ABS_MT_SLOT, 1);
        virtualTrackpad.emitEvent(EV_ABS, ABS_MT_TRACKING_ID, -1);
        
        virtualTrackpad.sync(); // End of Final Frame

        console.log("📜 Perfect Multi-Touch Scroll Executed!");
        setTimeout(() => virtualTrackpad.destroyDevice(), 100);
        
    }, 3000);
}