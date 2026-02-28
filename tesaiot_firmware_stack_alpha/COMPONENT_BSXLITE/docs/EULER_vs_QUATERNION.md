# Euler vs Quaternion

This document describes the orientation output modes of the IMU fusion (BSXLite), their data format, and the printed output lines.

> **Warning**
> BSXlite fusion on CM33 is available only with the `GCC_ARM` toolchain.
> Fusion functionality is not supported on other toolchains.

---

## 1. Overview

The fusion task always computes both a **quaternion** (rotation vector) and **Euler angles** internally. The CLI and stream output one format at a time, depending on **fusion mode**:

- **Quaternion mode** (`imu fusion mode quat`): stream shows the rotation quaternion (w, x, y, z).
- **Euler mode** (`imu fusion mode euler`): stream shows the orientation as Euler angles (heading, pitch, roll, yaw) in radians.
- **Data mode** (`imu fusion mode data`): stream shows the full line with acc, gyro, and quat in the same format as `imu data`.

Use **quaternion** when you need smooth interpolation, no gimbal lock, or direct use in 3D math. Use **Euler** when you need human-readable angles (e.g. degrees after converting from radians) or compatibility with systems that expect roll/pitch/yaw. Use **data** when you want continuous streaming of acc, gyro, and quaternion in one line.

---

## 2. Data format (BSXLite)

The library fills `bsxlite_out_t` after each `bsxlite_do_step()`:

| Field | Type | Description |
|-------|------|-------------|
| `rotation_vector` | `quaternion_t` | Rotation quaternion (w, x, y, z). |
| `orientation` | `euler_angles_t` | Euler angles in **radians**: heading, pitch, roll, yaw. |

### 2.1 Quaternion (`quaternion_t`)

| Field | Type | Description |
|-------|------|-------------|
| `w` | float | Scalar (real) part. |
| `x` | float | First imaginary component. |
| `y` | float | Second imaginary component. |
| `z` | float | Third imaginary component. |

Unit-free; typically normalized so that w² + x² + y² + z² = 1.

### 2.2 Euler angles (`euler_angles_t`)

| Field | Type | Unit | Description |
|-------|------|------|-------------|
| `heading` | float | rad | Heading (yaw about vertical). |
| `pitch` | float | rad | Pitch angle. |
| `roll` | float | rad | Roll angle. |
| `yaw` | float | rad | Yaw (BSXLite provides four angles; all in radians). |

To get degrees: multiply by `(180.0f / 3.14159f)`.

---

## 3. Printed lines

All IMU-related prints use the prefix **`[CM33.IMU.*]`** (core.source.type). The exact formats are below.

### 3.1 Stream output (when `imu stream on` and fusion enabled)

Printed at the current sample rate (see `imu sample rate`). One line per sample.

**Quaternion mode** (`imu fusion mode quat`):

```text
[CM33.IMU.Quaternion] w, x, y, z
```

- Four decimal floats: **w, x, y, z** (same order as `quaternion_t`).
- Example: `[CM33.IMU.Quaternion] 0.993059, 0.117434, -0.006493, 0.000720`

**Euler mode** (`imu fusion mode euler`):

```text
[CM33.IMU.Euler] heading, pitch, roll, yaw
```

- Four decimal floats in **radians**: **heading, pitch, roll, yaw** (same order as `euler_angles_t`).
- Example: `[CM33.IMU.Euler] 0.123456, -0.001234, 0.004567, 0.118234`

**Data mode** (`imu fusion mode data`):

```text
[CM33.IMU.Data] acc=ax,ay,az gyro=gx,gy,gz quat=qw,qx,qy,qz
```

- Same format as one-shot `imu data`: acc (4 decimals, m/s²), gyro (4 decimals, rad/s), quat (6 decimals).
- Example: `[CM33.IMU.Data] acc=0.0123,-0.0456,9.8012 gyro=0.000100,0.000200,-0.000050 quat=0.993059,0.117434,-0.006493,0.000720`

### 3.2 One-shot data (`imu data`)

Single line with accelerometer, gyroscope, and quaternion (quaternion is always stored in the sample; display mode does not change this command’s format):

```text
[CM33.IMU.Data] acc=ax,ay,az gyro=gx,gy,gz quat=qw,qx,qy,qz
```

- **acc**: ax, ay, az (m/s²), 4 decimal places.
- **gyro**: gx, gy, gz (rad/s), 4 decimal places.
- **quat**: qw, qx, qy, qz, 6 decimal places (same as `rotation_vector`).

Example:

```text
[CM33.IMU.Data] acc=0.0123,-0.0456,9.8012 gyro=0.000100,0.000200,-0.000050 quat=0.993059,0.117434,-0.006493,0.000720
```

Note: `imu data` always prints the **quaternion** from the last sample; it does not switch to Euler. For Euler values, use stream mode with `imu fusion mode euler` and `imu stream on`.

---

## 4. Summary

| Output | Mode | Format | Units |
|--------|------|--------|--------|
| Stream (quat) | `fusion mode quat` | `[CM33.IMU.Quaternion] w, x, y, z` | unit-free (normalized) |
| Stream (euler) | `fusion mode euler` | `[CM33.IMU.Euler] heading, pitch, roll, yaw` | radians |
| Stream (data) | `fusion mode data` | `[CM33.IMU.Data] acc=... gyro=... quat=...` | acc: m/s², gyro: rad/s, quat: unit-free |
| One-shot | — | `[CM33.IMU.Data] acc=... gyro=... quat=qw,qx,qy,qz` | acc: m/s², gyro: rad/s, quat: unit-free |

To swap Y and Z axes (e.g. for board mounting), use `imu swap on` / `imu swap off` at runtime (see [IMU_CLI_COMMANDS.md](IMU_CLI_COMMANDS.md)).
