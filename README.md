# Rein Touchpad PoC

A cross-platform virtual touchpad and input device emulator for Node.js. This native module enables programmatic control of mouse movement, touch events, and multi-touch gestures across Linux, macOS, and Windows.

## Features

- **Cross-Platform Support**
  - 🐧 **Linux**: uinput-based virtual input device with multi-touch support
  - 🍎 **macOS**: Native mouse and touch event simulation
  - 🪟 **Windows**: Touch Injection API with hybrid touch and mouse support

- **Multi-Touch Support**: Simulate multiple simultaneous touch points up to 2 fingers
- **Mouse Control**: Move cursor, click, and scroll programmatically
- **Raw Input Events**: Direct access to input event codes for advanced use cases
- **Absolute Positioning**: Precise control over touch coordinates

## Prerequisites

### Linux
- `libuinput-dev` package (for uinput support)
- Kernel support for uinput module

### macOS
- Xcode Command Line Tools
- macOS 10.14 or later

### Windows
- Visual Studio Build Tools
- Windows 10 or later

## Installation

1. **Install dependencies**:
   ```bash
   npm install
   ```

2. **Build the native module**:
   ```bash
   npm run build
   ```

   This uses `node-gyp` to compile the C++ code and generate the `.node` file.

## Usage

### Basic Setup

```javascript
import { createRequire } from 'node:module';
const require = createRequire(import.meta.url);
const virtualTrackpad = require('./build/Release/virtual_trackpad.node');

// Initialize the virtual input device
const isStarted = virtualTrackpad.initDevice();

if (isStarted) {
    console.log('Device initialized successfully!');
}
```

### API Methods

#### `initDevice()`
Initializes the virtual input device.

**Returns**: `boolean` - `true` if initialization succeeded, `false` otherwise

```javascript
const success = virtualTrackpad.initDevice();
```

#### `sendMouse(action, val1, val2)`
Sends mouse events (movement, clicks, scroll).

**Parameters**:
- `action` (integer):
  - `0`: Move cursor (val1 = dx, val2 = dy)
  - `1`: Mouse button press/release (val1 = button state, val2 = unused)
  - `2`: Scroll wheel (val1 = scroll amount)
- `val1` (integer): First value (depends on action)
- `val2` (integer): Second value (depends on action)

**Example - Move cursor**:
```javascript
virtualTrackpad.sendMouse(0, 10, 5); // Move 10px right, 5px down
```

**Example - Click**:
```javascript
virtualTrackpad.sendMouse(1, 1, 0); // Left button down
virtualTrackpad.sendMouse(1, 0, 0); // Left button up
```

**Example - Scroll**:
```javascript
virtualTrackpad.sendMouse(2, 5, 0); // Scroll down
```

#### `emitEvent(type, code, value)`
Sends raw input events (Linux/Windows).

**Parameters**:
- `type` (integer): Event type (e.g., `EV_KEY`, `EV_ABS`, `EV_SYN`)
- `code` (integer): Event code (e.g., `BTN_TOUCH`, `ABS_X`)
- `value` (integer): Event value

**Event Type Constants** (Linux):
```javascript
const EV_SYN = 0x00;      // Synchronization event
const EV_KEY = 0x01;      // Keyboard/button event
const EV_ABS = 0x03;      // Absolute positioning
const EV_REL = 0x02;      // Relative movement
```

**Event Code Constants**:
```javascript
// Buttons
const BTN_LEFT = 0x110;
const BTN_RIGHT = 0x111;
const BTN_TOUCH = 0x14a;
const BTN_TOOL_DOUBLETAP = 0x14d;

// Absolute position
const ABS_X = 0x00;
const ABS_Y = 0x01;

// Multi-touch
const ABS_MT_SLOT = 0x2f;
const ABS_MT_TRACKING_ID = 0x39;
const ABS_MT_POSITION_X = 0x35;
const ABS_MT_POSITION_Y = 0x36;
```

**Example - Multi-touch scroll**:
```javascript
// Touch down
virtualTrackpad.emitEvent(0x01, 0x14a, 1);  // BTN_TOUCH down
virtualTrackpad.emitEvent(0x01, 0x14d, 1);  // BTN_TOOL_DOUBLETAP

// First finger
virtualTrackpad.emitEvent(0x03, 0x2f, 0);   // ABS_MT_SLOT = 0
virtualTrackpad.emitEvent(0x03, 0x39, 100); // ABS_MT_TRACKING_ID
virtualTrackpad.emitEvent(0x03, 0x35, 500); // ABS_MT_POSITION_X
virtualTrackpad.emitEvent(0x03, 0x36, 500); // ABS_MT_POSITION_Y

// Sync
virtualTrackpad.sync();
```

#### `sync()`
Synchronizes and flushes pending input events.

**Returns**: `void`

```javascript
virtualTrackpad.sync();
```


## Project Structure

```
├── binding.gyp              # node-gyp configuration
├── package.json             # Project metadata and scripts
├── test_bridge.js           # Example usage and test suite
├── native/
│   └── virtual_trackpad.cpp # Native C++ implementation
└── build/                   # Compiled output
    ├── Release/
    │   └── virtual_trackpad.node  # Compiled native module
    └── binding.sln          # Visual Studio solution (Windows)
```

## Development

### Rebuild After Changes

```bash
npm run build
```

### Run Tests

```bash
npm start
# or
node test_bridge.js
```

## Platform-Specific Notes

### Linux
- Requires `/dev/uinput` access. May need elevated permissions:
  ```bash
  sudo node test_bridge.js
  ```
- Virtual device appears as "Rein Input" in input device listing
- Supports both uinput keyboard/mouse and multi-touch events

### macOS
- Uses native `ApplicationServices` framework
- Requires `OTHER_LDFLAGS: -framework ApplicationServices` (configured in binding.gyp)
- May require accessibility permissions for the application

### Windows
- Uses Touch Injection API for modern touch input
- Hybrid implementation: touch events + SendInput for compatibility
- Supports up to 2 simultaneous touch points

## Limitations
- **Relative positioning**: Mouse movement uses relative coordinates
- **Raw events**: Direct event emission available only on Linux
- **Platform-specific APIs**: Some methods may behave differently across platforms


