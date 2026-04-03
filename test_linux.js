import { createRequire } from 'node:module';
const require = createRequire(import.meta.url);
const virtualTrackpad = require('./build/Release/linux_libevdev.node');

// 1. Boot up the libevdev virtual device
const isStarted = virtualTrackpad.initDevice();

if (isStarted) {
    console.log("Starting input test in 3 seconds. Let go of your mouse!...\n");
    
    setTimeout(async () => {
        
        console.log("[TEST 1] Moving the cursor diagonally...");
        for (let i = 0; i < 40; i++) {
            await new Promise(r => setTimeout(r, 10)); 
            // Action 0: Move (dx: 5, dy: 5)
            virtualTrackpad.sendMouse(0, 5, 5); 
        }
        
        console.log("[TEST 2] Executing Left Click...");
        // Action 1: Click (1 = Button Down, 0 = Button Up)
        virtualTrackpad.sendMouse(1, 1, 0); 
        await new Promise(r => setTimeout(r, 50)); 
        virtualTrackpad.sendMouse(1, 0, 0); 
        
        await new Promise(r => setTimeout(r, 500)); 

        console.log("[TEST 3] Scrolling...");
        for(let i = 0; i < 10; i++) {
             await new Promise(r => setTimeout(r, 50)); 
             // Action 2: Scroll (-1 for scrolling down in libevdev)
             virtualTrackpad.sendMouse(2, -1, 0); 
        }

        console.log("\nTests finished. Destroying virtual device...");
        
        // Give it a split second to finish the last scroll before destroying
        setTimeout(() => {
            virtualTrackpad.destroyDevice();
            console.log("Device unplugged successfully.");
        }, 100);
        
    }, 3000);
} else {
    console.log("Failed to initialize device.");
}