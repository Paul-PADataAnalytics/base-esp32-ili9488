# Chapter 5: Complete API Reference & Specification

## 5.1 The `ui` Namespace

| Function Signature | Input Parameters & Types | Default Values | Return Type | Description & Side Effects |
| :--- | :--- | :--- | :--- | :--- |
| `ui.createFrame(x, y, w, h, title)` | `x` *(integer)*: Left coordinate<br>`y` *(integer)*: Top coordinate<br>`w` *(integer)*: Frame width<br>`h` *(integer)*: Frame height<br>`title` *(string)*: Window header title | `title`: `""` | `lightuserdata` | Instantiates a titled UI frame container. Adds the container to the active `UIManager`. |
| `ui.createLabel(parent, x, y, w, h, text)` | `parent` *(lightuserdata)*: Container frame or `nil`<br>`x, y, w, h` *(integer)*: Bounds<br>`text` *(string)*: Label text | None | `lightuserdata` | Spawns a text label widget attached to `parent` (or root if `nil`). |
| `ui.createButton(parent, x, y, w, h, label, callbackFunc)` | `parent` *(lightuserdata)*: Container or `nil`<br>`x, y, w, h` *(integer)*: Bounds<br>`label` *(string)*: Button text<br>`callbackFunc` *(string)*: Lua callback function name | `callbackFunc`: `nil` | `lightuserdata` | Spawns an interactive button. Triggers `callbackFunc()` in Lua when pressed. |
| `ui.createCheckBox(parent, x, y, w, h, label, initialVal, callbackFunc)` | `parent` *(lightuserdata)*: Container or `nil`<br>`x, y, w, h` *(integer)*: Bounds<br>`label` *(string)*: Checkbox label<br>`initialVal` *(boolean)*: Initial state<br>`callbackFunc` *(string)*: Lua callback function name | `callbackFunc`: `nil` | `lightuserdata` | Spawns a toggle checkbox. Invokes `callbackFunc(1.0)` for checked, `callbackFunc(0.0)` for unchecked. |
| `ui.createSlider(parent, x, y, w, h, label, minVal, maxVal, initVal, callbackFunc)` | `parent` *(lightuserdata)*: Container or `nil`<br>`x, y, w, h` *(integer)*: Bounds<br>`label` *(string)*: Slider title<br>`minVal` *(number)*: Minimum bound<br>`maxVal` *(number)*: Maximum bound<br>`initVal` *(number)*: Starting value<br>`callbackFunc` *(string)*: Lua callback function name | `minVal`: `0.0`<br>`maxVal`: `1.0`<br>`initVal`: `0.5`<br>`callbackFunc`: `nil` | `lightuserdata` | Spawns a continuous value slider widget. Invokes `callbackFunc(currentVal)` on position change. |
| `ui.createOptionSelector(parent, x, y, w, h, label, optionsTable, callbackFunc)` | `parent` *(lightuserdata)*: Container or `nil`<br>`x, y, w, h` *(integer)*: Bounds<br>`label` *(string)*: Selector title<br>`optionsTable` *(table)*: Array of option strings<br>`callbackFunc` *(string)*: Lua callback function name | `callbackFunc`: `nil` | `lightuserdata` | Spawns a multi-option selector with `<` and `>` arrows. Invokes `callbackFunc(0-based Index)` on change. |
| `ui.setText(widgetHandle, text)` | `widgetHandle` *(lightuserdata)*: Target widget<br>`text` *(string)*: New text string | None | `nil` | Mutates the display string of a label, button, or frame header. Triggers a UI redraw. |
| `ui.showToast(message, color, duration)` | `message` *(string)*: Notification text<br>`color` *(integer)*: RGB565 hex color code<br>`duration` *(number)*: Display duration in seconds | `color`: `0x07FF` (Cyan)<br>`duration`: `2.5` | `nil` | Displays an animated floating notification banner across the top of the screen. |
| `ui.clear()` | None | None | `nil` | Destroys all active widgets, frees allocated memory, and clears the UI manager canvas. |

---

## 5.2 The `game` Namespace

| Function Signature | Input Parameters & Types | Default Values | Return Type | Description & Side Effects |
| :--- | :--- | :--- | :--- | :--- |
| `game.setVar(name, value)` | `name` *(string)*: Key identifier<br>`value` *(number)*: Floating point numeric value | None | `nil` | Stores a numeric variable in the C++ sync table and updates the corresponding Lua global `name`. |
| `game.getVar(name)` | `name` *(string)*: Key identifier | None | `number` | Retrieves the numeric variable value associated with `name`. Returns `0.0` if key does not exist. |
