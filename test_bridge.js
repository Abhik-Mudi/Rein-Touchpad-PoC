import { createRequire } from 'node:module';
const require = createRequire(import.meta.url);
const virtualTrackpad = require('./build/Release/virtual_trackpad.node');

// OS Detection
const isLinux = process.platform === 'linux';
const isMac = process.platform === 'darwin';

const isStarted = virtualTrackpad.initDevice();

if (isStarted) {
    console.log(`Device Online! (Detected OS: ${process.platform})`);
    
    setTimeout(async () => {
        

        // LINUX WAYLAND EXECUTION (Raw emitEvent)

        if (isLinux) {
            console.log("Running Linux-specific Wayland sequence...");

            const EV_SYN = 0x00, EV_KEY = 0x01, EV_ABS = 0x03;
            const BTN_TOUCH = 0x14a, BTN_TOOL_DOUBLETAP = 0x14d;
            const ABS_X = 0x00, ABS_Y = 0x01;
            const ABS_MT_SLOT = 0x2f, ABS_MT_POSITION_X = 0x35, ABS_MT_POSITION_Y = 0x36, ABS_MT_TRACKING_ID = 0x39;

            let yPos = 500;
            
            // Frame 1: Fingers touch down
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
            virtualTrackpad.sync();

            // Scrolling Frames
            for (let i = 0; i < 40; i++) {
                await new Promise(r => setTimeout(r, 10)); 
                yPos -= 5;

                // Finger 0 update
                virtualTrackpad.emitEvent(EV_ABS, ABS_MT_SLOT, 0);
                virtualTrackpad.emitEvent(EV_ABS, ABS_MT_POSITION_Y, yPos);

                // Finger 1 update
                virtualTrackpad.emitEvent(EV_ABS, ABS_MT_SLOT, 1);
                virtualTrackpad.emitEvent(EV_ABS, ABS_MT_POSITION_Y, yPos);

                // Legacy update
                virtualTrackpad.emitEvent(EV_ABS, ABS_Y, yPos);

                virtualTrackpad.sync();
            }

            // Fingers Lift Off
            virtualTrackpad.emitEvent(EV_KEY, BTN_TOUCH, 0);
            virtualTrackpad.emitEvent(EV_KEY, BTN_TOOL_DOUBLETAP, 0);

            virtualTrackpad.emitEvent(EV_ABS, ABS_MT_SLOT, 0);
            virtualTrackpad.emitEvent(EV_ABS, ABS_MT_TRACKING_ID, -1);

            virtualTrackpad.emitEvent(EV_ABS, ABS_MT_SLOT, 1);
            virtualTrackpad.emitEvent(EV_ABS, ABS_MT_TRACKING_ID, -1);

            virtualTrackpad.sync();

            console.log("Linux Multi-Touch Scroll Executed!");
        }


        // MAC EXECUTION (sendMouse)

        else if(isMac) {
            console.log("Running Mac Mouse/Touch sequence...");
            
            console.log("[TEST 1] Moving the cursor...");
            for (let i = 0; i < 40; i++) {
                await new Promise(r => setTimeout(r, 10)); 
                virtualTrackpad.sendMouse(0, 5, 5); 
            }
            
            console.log("[TEST 2] Executing Click...");
            virtualTrackpad.sendMouse(1, 1, 0); 
            await new Promise(r => setTimeout(r, 50)); 
            virtualTrackpad.sendMouse(1, 0, 0); 
            await new Promise(r => setTimeout(r, 500)); 

            console.log("[TEST 3] Scrolling...");
            for(let i = 0; i < 10; i++) {
                 await new Promise(r => setTimeout(r, 50)); 
                 virtualTrackpad.sendMouse(2, -120, 0); 
            }

            console.log("[TEST 4] Skipping Touchscreen tests on Mac.");
        }


        // WINDOWS EXECUTION (sendMouse/SendTouch)

        else {
            console.log("Running Windows Hybrid sequence...");
            
            // Still use mouse for basic cursor targeting
            console.log("[TEST 1 & 2] Moving Cursor & Clicking...");
            for (let i = 0; i < 40; i++) {
                await new Promise(r => setTimeout(r, 10)); 
                virtualTrackpad.sendMouse(0, 5, 5); 
            }
            virtualTrackpad.sendMouse(1, 1, 0); 
            await new Promise(r => setTimeout(r, 50)); 
            virtualTrackpad.sendMouse(1, 0, 0); 
            await new Promise(r => setTimeout(r, 500));

            // Windows Legacy Scroll via SendInput (Global Compatibility)
            console.log("TEST 3] Scrolling via SendInput (Virtual Mouse)...");
            for(let i = 0; i < 10; i++) {
                 await new Promise(r => setTimeout(r, 50)); 
                 // action 2 = Scroll.
                 virtualTrackpad.sendMouse(2, -120, 0); 
            }

            await new Promise(r => setTimeout(r, 500));

            // Windows Native Pinch-to-Zoom (Uses Touch API!)
            console.log("[TEST 4] Executing Pinch-to-Zoom...");
            let f1_x = 450, f1_y = 500; 
            let f2_x = 550, f2_y = 500; 

            virtualTrackpad.sendTouch(0, 100, f1_x, f1_y);
            virtualTrackpad.sendTouch(1, 101, f2_x, f2_y);
            virtualTrackpad.sync();

            for (let i = 0; i < 30; i++) {
                await new Promise(r => setTimeout(r, 15)); 
                f1_x -= 5; f2_x += 5; 
                virtualTrackpad.sendTouch(0, 100, f1_x, f1_y);
                virtualTrackpad.sendTouch(1, 101, f2_x, f2_y);
                virtualTrackpad.sync();
            }

            virtualTrackpad.sendTouch(0, -1, f1_x, f1_y);
            virtualTrackpad.sendTouch(1, -1, f2_x, f2_y);
            virtualTrackpad.sync();
            
            console.log("Windows Hybrid sequence complete!");
        }

        setTimeout(() => virtualTrackpad.destroyDevice(), 100);
        
    }, 3000);
} else {
    console.log("Failed to initialize device.");
}