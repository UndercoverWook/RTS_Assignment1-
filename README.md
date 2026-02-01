# Module 1 – ESP32 FreeRTOS Tasks (Wokwi)

**Name:** Xander M. Levine  
**UCFID:** 5519900
**Wokwi Project Link:** (https://wokwi.com/projects/454793192358059009)
**Theme/Context:** Audio System Health Monitor (heartbeat + status LED)

## Project Summary
This project uses ESP-IDF + FreeRTOS on an ESP32 to run two tasks at the same time:
- `blink_task` toggles an LED on GPIO 2 at ~2 Hz (250 ms on / 250 ms off).
- `print_task` prints a heartbeat message about every 10 seconds.

Both tasks use `vTaskDelay(pdMS_TO_TICKS(...))` so they yield the CPU instead of busy-waiting.

## Q1. Vary Priorities
I changed the print task priority from 1 to 2 (higher than blink’s 1). With the current code, both tasks spend most of their time sleeping, so the difference is small. In theory, if both tasks were ready at the same time, the higher priority print task would run first (preempt the blink task).

- **If I increase the delay inside the print task:** nothing noticeable happens to the LED. The print task runs less often, and the blink task keeps blinking normally because it still gets CPU time.
- **If I increase the number of characters printed:** the LED still mostly blinks normally, but if printing takes longer, you can start seeing small jitter (tiny timing shifts) because printing uses CPU time while the blink task is waiting to run. It’s not huge here, but this would matter more in CPU-heavy tasks later.

## Q2. Increase Load (Remove vTaskDelay in print task)
I removed `vTaskDelay` from the print task, turning it into a tight loop that prints continuously. This floods the console and causes the print task to hog the CPU.
**Observed behavior:** the LED blinking becomes very inconsistent and may appear to freeze or stop blinking on schedule. This happens because the print task does not yield, so it can starve the blink task (starvation). After observing a few lines, I put the delay back to avoid overwhelming the simulator/serial output.

## Q3. Thematic Customization + Verbosity Considerations
My theme is an audio system / embedded health monitor. The printed message is a heartbeat such as “audio monitor running” and uptime.

As a developer, I wouldn’t want the system to print too much because:
- Too much printing can slow the system down and affect timing.
- It can flood logs, making it harder to find important messages.
- In real devices, extra logs can waste power, memory, or bandwidth.

This system benefits from correct functionality at predictable times because real-time systems care about *when* things happen, not just *what* happens. For example, a status indicator or periodic telemetry should occur on a schedule so operators can trust it. If timing slips, it could hide failures or cause late responses in real applications.

---

# Real-Time Concept Check

## Q4. Identify/Verify the period of each task
### (a) How I measured the periods
I measured periods using the timestamps printed to the serial console. In the code, it prints uptime in milliseconds and also prints `dt` (time difference) between events. This makes it easy to confirm the timing.

### (b) LED blink task period
The LED toggles every 250 ms. Since a full blink cycle is ON + OFF, the period is about:
- 250 ms (ON) + 250 ms (OFF) = **500 ms period** (~2 Hz)

### (c) Print task period
The print task delays for 10,000 ms each loop, so the period is about:
- **10,000 ms** (10 seconds)

## Q5. Did our system tasks meet the timing requirements?
Yes. The LED blink rate looks steady at ~2 Hz in Wokwi, and the serial logs show timing close to the target delays (with small jitter). The print task outputs about every 10 seconds.

## Q5 (How do you know / how did you verify?)
I verified it by:
- visually observing the LED blink rate in the simulator
- checking the serial console timing (`uptime` and `dt` values) to confirm the blink toggles and prints happen near the expected intervals

## Q6. Super-loop (single-loop) effect
A single-loop design can disturb timing because printing can take variable time (especially if printing a lot). In a super-loop, if printing takes longer, the LED update happens later, so the blink timing can drift.

- **Did I try running a super-loop version?** I did not keep a final super-loop in the repo, but the starvation test in Q2 shows the same idea: if the CPU is busy printing, the LED timing suffers.
- **Can I cause the LED to miss timing requirements?** Yes—by removing the delay in the print task (or by printing huge messages repeatedly), the print task can dominate CPU time and cause the LED to miss its schedule.

## Q7. Agree/Disagree statement on determinism
**I agree (mostly).** Multitasking helps achieve more deterministic timing because each task has its own schedule and uses `vTaskDelay` to yield, letting the scheduler run tasks in a predictable way. However, determinism depends on tasks behaving well (yielding/blocking) and not hogging the CPU. If a task never yields (like in Q2), determinism breaks and other tasks can be starved.
  
## AI Usage Disclosure
AI assistance (ChatGPT 5.2) was used to help write and organize the FreeRTOS tasks and help clarify and better understand the behaviors of the system after changes were made. All code and answers were reviewed and understood before submission.
