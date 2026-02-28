# IMU CLI Commands

Use these commands from CM33 CLI prompt (`cli>`).

> **Warning**
> `imu fusion ...` commands require BSXlite fusion support on CM33, which is available only with the `GCC_ARM` toolchain.
> Fusion functionality is not supported on other toolchains.

## Main IMU command

- `imu help`
  - Show IMU command usage.

- `imu status`
  - Show IMU/fusion runtime status (ready flags, stream/fusion state, sample/stream rates, loop/read counters).

- `imu data`
  - Print one-shot current sensor data (`acc`, `gyro`, `quat`).

## Stream control

- `imu stream status`
  - Show whether quaternion/euler stream is enabled and the rate (same as sample rate).

- `imu stream on`
  - Enable stream. Stream frequency equals the sample rate (see `imu sample rate`).

- `imu stream off`
  - Disable stream.

## Sampling control

- `imu sample status`
  - Show current sampling rate in Hz.

- `imu sample rate <hz>`
  - Set sampling rate in Hz.

## Fusion control

- `imu fusion status`
  - Show fusion enabled/disabled and output mode (`quat`, `euler`, or `data`).

- `imu fusion mode quat`
  - Set fusion output mode to quaternion.

- `imu fusion mode euler`
  - Set fusion output mode to euler angles.

- `imu fusion mode data`
  - Set fusion output mode to data (full line: acc, gyro, quat). When stream is on, prints `[CM33.IMU.Data] acc=... gyro=... quat=...` at the sample rate.

- `imu fusion on`
  - Enable fusion processing.

- `imu fusion off`
  - Disable fusion processing.

## Calibration

- `imu calib status`
  - Show calibration status (ready/supported, accel, gyro, magnetometer unsupported in current 6-axis path).

- `imu calib reset`
  - Request calibration reset.

## Swap (Y/Z axes)

- `imu swap status`
  - Show whether Y/Z swap is on or off.

- `imu swap on`
  - Enable Y/Z swap: accelerometer and gyroscope Y and Z components are swapped before fusion. All outputs (acc, gyr, quaternion, Euler) are then in the swapped frame.

- `imu swap off`
  - Disable Y/Z swap (default at boot).
