# Configure Credentials for **TESA Digital Twin**

The **TESA Digital Twin** extension connects to **TESA** cloud services via MQTT over TLS/WSS to enable real-time communication with IoT devices and sensors. To establish this connection, you need to configure your MQTT credentials in the extension settings. This tutorial will guide you through setting up your credentials.

## Why Configure Credentials?

Configuring MQTT credentials allows the **TESA Digital Twin** extension to:

- Establish secure connections to **TESA** cloud MQTT brokers
- Send and receive real-time data from IoT devices
- Synchronize digital twin simulations with live device data
- Enable bidirectional communication between your 3D simulations and cloud services

## Prerequisites

Before you begin, make sure you have:

- The **TESA Digital Twin** extension installed (see [Tutorial 1](../01-install_extension/README.md))
- The **CA certificate** installed (see [Tutorial 2](../02-install-ca-certificate/README.md))
- A device created in the **Device Management** of the **TESAIoT Platform**
- Your MQTT credentials ready:
  - **Username** (Device ID)
  - **Password**
  - **Host**: `mqtt.tesaiot.com`
  - **Port**: `8085`

## Step-by-Step Instructions

Follow these steps to configure your MQTT credentials:

![Credentials Settings panel in TESA Digital Twin extension](images/credentials-settings.png)

### Step 1: Open Settings

1. **Click the Settings button** in the bottom quick tool bar of the **TESA Digital Twin** extension panel.

### Step 2: Access Credentials Settings

2. **Select the Credentials Settings panel** from the settings menu.

### Step 3: Enter Connection Details

3. **Enter the Host**: Type `mqtt.tesaiot.com` in the **Host** field.

4. **Enter the Port**: Type `8085` in the **Port** field.

5. **Enter the Username**: Type your **Username** (Device ID) in the **Username** field. This is the device ID you received when creating the device in the **TESAIoT Platform**.

6. **Enter the Password**: Type your **Password** in the **Password** field. This is the password associated with your device.

### Step 4: Validate Connection

7. **Validate the credentials** by clicking the **Validate Connection** button. This will test the connection to the MQTT broker using the credentials you provided.

### Step 5: Verify Success

8. **Check the validation result**: If the validation is successful, you will see the message **"Connection successful! Credentials are valid."**

9. **Check the connection status**: You can also see the **`Connected`** status indicator in the **Right panel** of the extension, confirming that the connection is active.

## Verification

After configuring your credentials, verify that everything is working correctly:

1. **Check Connection Status**: Look for the **`Connected`** status in the right panel of the extension
2. **Test Data Flow**: If you have devices publishing data, you should see real-time updates in the extension
3. **Monitor Connection**: The connection status should remain stable and show as **`Connected`**

## Troubleshooting

**Connection Validation Failed?**

- Verify that your **Username** (Device ID) and **Password** are correct
- Ensure you have created the device in the **TESAIoT Platform** Device Management
- Check that the **Host** is set to `mqtt.tesaiot.com` and **Port** is `8085`
- Make sure the **CA certificate** is properly installed (see [Tutorial 2](../02-install-ca-certificate/README.md))
- Verify your internet connection is active

**Status Shows "Disconnected"?**

- Click the **Validate Connection** button again to reconnect
- Check that your credentials haven't changed in the **TESAIoT Platform**
- Ensure VS Code and the extension are up to date
- Try restarting VS Code if the connection persists

**Certificate Errors?**

- Make sure you have completed [Tutorial 2](../02-install-ca-certificate/README.md) and installed the **CA certificate**
- Restart VS Code after installing the certificate
- Verify the certificate is installed correctly in your system's trust store

## Next Steps

Now that you have configured your MQTT credentials, your **TESA Digital Twin** extension is ready to connect to **TESA** cloud services and receive real-time data from your IoT devices. Continue to the next tutorial to learn how to use the extension to create and interact with 3D digital twin simulations.
