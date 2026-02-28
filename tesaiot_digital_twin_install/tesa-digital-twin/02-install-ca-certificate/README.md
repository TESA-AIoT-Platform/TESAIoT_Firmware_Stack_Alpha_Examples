# Install the **CA Certificate** for **TESA Digital Twin**

The **TESA Digital Twin** extension uses MQTT over TLS/WSS (WebSocket Secure) connections to communicate with **TESA** cloud services. **TESA** cloud services currently use a custom **CA certificate** to sign the certificates for the MQTT brokers. To establish secure connections, you'll need to install this **CA certificate** to your operating system's trust store. This allows browsers and applications (including VS Code webviews) to trust certificates signed by that **CA**.

## Why Install the CA Certificate?

Browsers and VS Code webviews use the **operating system's certificate store** to validate SSL/TLS certificates. Installing the **CA certificate** at the OS level ensures:

- All browsers automatically trust WSS connections
- VS Code webviews can connect to MQTT brokers over WSS
- No certificate errors when using custom/internal **CA** certificates

## Prerequisites

Before you begin, make sure you have:

- The **TESA Digital Twin** extension installed (see the previous tutorial)
- Administrator/sudo privileges (required for certificate installation)
- The **CA certificate** file (`.pem` or `.crt` format)
- Node.js 18+ installed (if using the CLI method)

## Method 1: Install Using T3D CLI (Recommended)

This is the quickest and most straightforward method. The T3D CLI automatically detects your operating system and installs the certificate to the correct location.

### Step 1: Install T3D CLI

If you haven't already installed the T3D CLI, run the following command:

```bash
npm install -g @ternion/t3d
```

### Step 2: Install the CA Certificate

Run the following command with the path to your **CA certificate** file:

```bash
t3d ca install --cert <path-to-certificate-file>
```

**Example:**

```bash
t3d ca install --cert path/to/ca-chain.pem
```

The CLI will automatically:
- Detect your operating system (Windows, macOS, or Linux)
- Install the certificate to the appropriate system location
- Update the certificate store

### Step 3: Verify Installation

After running the command, you should see a success message. The certificate has been installed to your system's trust store.

### Uninstalling the CA Certificate

If you need to remove the **CA certificate** later, use the uninstall command:

```bash
t3d ca uninstall --cert <path-to-certificate-file>
```

## Method 2: Manual Installation

If you prefer to install the **CA certificate** manually or don't have Node.js installed, follow the platform-specific instructions below.

### Windows

**Option A: Certificate Manager (GUI)**

1. **Open Certificate Manager**: Press `Win + R`, type `certmgr.msc`, and press Enter
2. **Navigate to Trust Store**: Expand `Trusted Root Certification Authorities` → `Certificates`
3. **Import Certificate**: Right-click on `Certificates` → `All Tasks` → `Import`
4. **Select Certificate File**: Browse and select your **CA certificate** file (`.pem` or `.crt`)
5. **Complete Import**: Click `Next` → `Finish` to complete the import
6. **Restart Applications**: Restart your browsers and VS Code

**Option B: PowerShell (Administrator)**

Open PowerShell as Administrator and run:

```powershell
certutil -addstore -f Root "path\to\ca-chain.pem"
```

Replace `"path\to\ca-chain.pem"` with the actual path to your certificate file.

**Note:** You may see a User Account Control (UAC) prompt. Click "Yes" to allow the operation.

### macOS

**Option A: Keychain Access (GUI)**

1. **Open Keychain Access**: Open the "Keychain Access" application (located in Applications → Utilities)
2. **Select System Keychain**: In the sidebar, select "System" keychain (you may need to unlock it first)
3. **Import Certificate**: Go to `File` → `Import Items...`
4. **Select Certificate File**: Browse and select your **CA certificate** file
5. **Trust the Certificate**: Double-click the imported certificate to open it
6. **Set Trust Settings**: Expand the "Trust" section → Set "When using this certificate" to "Always Trust"
7. **Save Changes**: Close the certificate window and enter your administrator password when prompted
8. **Restart Applications**: Restart your browsers and VS Code

**Option B: Command Line**

Open Terminal and run:

```bash
sudo security add-trusted-cert -d -r trustRoot -k /Library/Keychains/System.keychain /path/to/ca-chain.pem
```

Replace `/path/to/ca-chain.pem` with the actual path to your certificate file. You'll be prompted for your administrator password.

### Linux (Ubuntu/Debian)

1. **Copy Certificate**: Copy your **CA certificate** file to the system certificates directory:

```bash
sudo cp ca-chain.pem /usr/local/share/ca-certificates/
```

2. **Update Certificate Store**: Update the system certificate store:

```bash
sudo update-ca-certificates
```

3. **Restart Applications**: Restart your browsers and VS Code

**Note for Other Linux Distributions:**

- **Fedora/RHEL**: Copy the certificate to `/etc/pki/ca-trust/source/anchors/` and run `sudo update-ca-trust`
- **Arch Linux**: Copy the certificate to `/etc/ca-certificates/trust-source/anchors/` and run `sudo trust extract-compat`

## Platform-Specific Notes

- **Windows**: Certificate is added to "Trusted Root Certification Authorities". You may see a User Account Control (UAC) prompt.
- **macOS**: Certificate is added to `/Library/Keychains/System.keychain`. You can verify the installation in the "Keychain Access" app.
- **Linux**: Works best on Debian/Ubuntu-based distributions. Certificate is copied to `/usr/local/share/ca-certificates/`.

## Verification

After installing the **CA certificate**, verify that it's working correctly:

1. **Restart VS Code**: Close and reopen VS Code to ensure it picks up the new certificate
2. **Restart Web Browsers**: Close and restart your web browsers (Chrome, Firefox, Safari, Edge)
3. **Test Connection**: Try connecting to the MQTT broker through the **TESA Digital Twin** extension

If the installation was successful, you should no longer see certificate errors when connecting to **TESA** cloud services.

## Troubleshooting

**Certificate Still Not Trusted?**

- Make sure you restarted VS Code and all browsers after installation
- Verify the certificate file is in the correct format (`.pem` or `.crt`)
- On Windows, ensure you ran the command as Administrator
- On macOS, verify in Keychain Access that the certificate shows "Always Trust"
- On Linux, check that the certificate file has the correct permissions

**Permission Errors?**

- Ensure you have administrator/sudo privileges
- On Windows, run Command Prompt or PowerShell as Administrator
- On macOS and Linux, use `sudo` before commands that require elevated privileges

## Next Steps

Now that you have installed the **CA certificate**, your **TESA Digital Twin** extension is ready to securely connect to **TESA** cloud services over TLS/WSS connections. Continue to the next tutorial to learn how to configure and use the **TESA Digital Twin** extension to connect to **TESA** cloud services, such as the MQTT broker over TLS.
