# GitHub Repository Setup

## Repository Created

The Git repository has been initialized and configured to connect to:
**https://github.com/tolliv3r/insta360-raspberry-pi-control**

## Next Steps

### 1. Create the Repository on GitHub

1. Go to https://github.com/tolliv3r
2. Click "New repository" or go to https://github.com/new
3. Repository name: `insta360-raspberry-pi-control`
4. Description: "Control Insta360 cameras from Raspberry Pi Zero 2 W"
5. Choose visibility (Public or Private)
6. **Do NOT** initialize with README, .gitignore, or license (we already have these)
7. Click "Create repository"

### 2. Push to GitHub

After creating the repository on GitHub, run:

```bash
cd "/Users/jacksonclary/Library/Mobile Documents/com~apple~CloudDocs/University/BU/Current/EC463/Insta360 SDK/insta360_control"
git push -u origin main
```

### 3. Authentication

If you're prompted for authentication:
- **HTTPS**: Use a Personal Access Token (not your password)
  - Go to GitHub Settings → Developer settings → Personal access tokens → Tokens (classic)
  - Generate a token with `repo` permissions
  - Use this token as your password when pushing

- **SSH** (alternative): If you prefer SSH, change the remote URL:
  ```bash
  git remote set-url origin git@github.com:tolliv3r/insta360-raspberry-pi-control.git
  ```

## Current Status

✅ Git repository initialized
✅ Files committed
✅ Remote added (pending repository creation on GitHub)
⏳ Waiting for GitHub repository creation

## Repository Structure

```
insta360-raspberry-pi-control/
├── .gitignore
├── README.md
├── QUICK_START.md
├── IMPLEMENTATION_SUMMARY.md
├── camera_control.cpp
├── Makefile
├── setup.sh
├── run.sh
└── CameraSDK-*/ (SDK headers and example code)
```

## Notes

- The SDK library file (`libCameraSDK.so`) is excluded from git (too large)
- Build artifacts are excluded via `.gitignore`
- Users will need to download the SDK separately or include it as a submodule

