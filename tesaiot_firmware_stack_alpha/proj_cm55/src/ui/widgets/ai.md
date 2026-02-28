

## LVGL On-Screen Keyboard Lag / Stuck Key Issue — Summary for Firmware Fix

### Observed Behavior

* When a key is pressed on the LVGL on-screen keyboard, it stays visually **pressed (dark)** for ~**1 second** after finger release.
* While the key is stuck in pressed state, **other keys do not respond**.
* Fast typing causes **missed characters**.

---

## Root Causes (Confirmed from Code & Behavior)

### 1) LVGL task sleeps too long (main cause of ~1s lockup)

Current LVGL task loop uses:

```c
time_till_next = lv_timer_handler();
vTaskDelay(pdMS_TO_TICKS(time_till_next));
```

Problem:

* `lv_timer_handler()` can return **hundreds or ~1000 ms**
* LVGL task sleeps that long
* During sleep:

  * no input is processed
  * no release events handled
  * UI appears frozen
* Result: key remains pressed ~1 second and blocks other keys

---

### 2) Touch input polling is too slow

In `lv_port_indev.c`:

```c
#define INDEV_READ_PERIOD_MS 100U
```

Problem:

* Touch is sampled at **10 Hz**
* Release detection can be delayed up to 100 ms
* Fast taps can be missed

---

### 3) LVGL tick timing must be correct

LVGL requires `lv_tick_inc()` to advance time properly.
If tick is missing or irregular:

* LVGL timers behave incorrectly
* `lv_timer_handler()` returns large sleep times
* Responsiveness degrades

---

## Required Fixes (Action Items)

### ✅ Fix 1 — Run LVGL handler at a fixed fast rate (CRITICAL)

**Do NOT sleep for the full `time_till_next`.**

Replace LVGL task loop with:

```c
for(;;) {
    lv_timer_handler();
    vTaskDelay(pdMS_TO_TICKS(5));   // or cap delay to 5–10 ms
}
```

(or cap `time_till_next` to max 5–10 ms)

➡ This removes the ~1 second “stuck pressed” behavior.

---

### ✅ Fix 2 — Increase touch polling rate

Change:

```c
#define INDEV_READ_PERIOD_MS 100U
```

To:

```c
#define INDEV_READ_PERIOD_MS 10U   // 10–20 ms recommended
```

➡ Improves release responsiveness and prevents missed fast taps.

---

### ✅ Fix 3 — Ensure LVGL tick is correct

Make sure **one** of the following exists:

```c
lv_tick_inc(1);   // called every 1 ms (timer ISR or RTOS tick hook)
```

Common FreeRTOS solution:

```c
void vApplicationTickHook(void)
{
    lv_tick_inc(1);
}
```

➡ Prevents incorrect timer scheduling and long handler delays.

---

### (Optional but Recommended) Fix 4 — Touch read robustness

If touch read occasionally fails:

* Do **not** immediately switch to RELEASED on a single failed read
* Hold PRESSED for 1–2 missed reads to avoid false PR→REL→PR glitches
* Prevents double characters and visual flicker

---

## Expected Result After Fix

* Keys release immediately after finger lift
* Keyboard remains responsive during fast typing
* No blocked input / no missing characters
* Smooth visual feedback

---

### TL;DR for Firmware Dev

* **Call `lv_timer_handler()` every ~5 ms**
* **Do NOT sleep for 1 second**
* **Poll touch at 10–20 ms**
* **Ensure `lv_tick_inc()` runs every 1 ms**

---
