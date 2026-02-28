# TESA Digital Twin Tutorials

Welcome to the **TESA Digital Twin** tutorial series. These tutorials will guide you through setting up and using the **TESA Digital Twin** extension in Visual Studio Code to create and interact with 3D digital twin simulations.

## Overview

The **TESA Digital Twin** extension brings powerful 3D simulation capabilities directly to your development environment. It enables you to:

- Create and visualize 3D digital twin simulations
- Connect to **TESA** cloud services via MQTT over TLS/WSS
- Interact with real-time data from IoT devices and sensors
- Build immersive 3D experiences for industrial applications

> **⚠️ Important: Required Setup Steps**
>
> **Before you can use the TESA Digital Twin extension**, you **must** complete the following three setup tutorials in order:
>
> 1. **Install the TESA Digital Twin Extension** - Required to have the extension available in VS Code
> 2. **Install the CA Certificate** - Required for secure TLS/WSS connections to TESA cloud services
> 3. **Configure Credentials** - Required to authenticate and connect to the MQTT broker
>
> These steps are **mandatory** and must be completed before the extension can function properly. Do not skip any of these tutorials.

## Table of Contents

1. [Install the TESA Digital Twin Extension](01-install_extension/README.md)
   - Learn how to install the **TESA Digital Twin** extension in Visual Studio Code
   - Choose between marketplace installation or VSIX file installation
   - Verify that the extension is properly installed and active

2. [Install the CA Certificate for TESA Digital Twin](02-install-ca-certificate/README.md)
   - Install the **CA certificate** required for secure connections to **TESA** cloud services
   - Use the T3D CLI for automatic installation or follow manual platform-specific instructions
   - Verify the certificate installation and troubleshoot common issues

3. [Configure Credentials for TESA Digital Twin](03-credentials-settings/README.md)
   - Configure MQTT credentials to connect to **TESA** cloud services
   - Set up your device credentials (Username/Device ID and Password)
   - Validate the connection and verify successful setup

## Getting Started

Follow the tutorials in order to set up your development environment:

1. **Start with Tutorial 1** to install the extension
2. **Continue to Tutorial 2** to configure secure connections
3. **Proceed to Tutorial 3** to set up your MQTT credentials
4. **Continue with the next tutorials** (coming soon) to learn how to use the extension

## Prerequisites

Before starting these tutorials, make sure you have:

- Visual Studio Code or another VS Code-based IDE installed
- Administrator/sudo privileges (for certificate installation)
- Node.js 18+ installed (if using the CLI method for certificate installation)

## Support

For issues or questions, please refer to the main project documentation or contact the **TESA** support team.

