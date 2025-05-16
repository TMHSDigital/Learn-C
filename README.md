# 🚀 Learn-C: Modern Password Generator

![Banner](step-1.png)

A beautiful, feature-rich, native Windows password generator built in C (Win32 API). Fast, portable, and easy to hack on.

---

## ✨ Why use this?
- ⚡ **Blazing fast** native Windows app
- 🛠️ **No dependencies** (just GCC/MinGW)
- 🖥️ **Modern, polished GUI**
- 🔒 **Strong, customizable passwords**
- 🧑‍💻 **Great demo for C/Win32 GUI programming**

---

## 🛠️ Prerequisites
- **GCC/MinGW**: Install [MinGW-w64](https://www.mingw-w64.org/) and add its `bin` directory to your PATH
- **make** (optional): For Makefile support, install make (e.g., from ezwinports, MSYS2, or GnuWin32) and add to PATH
- **comctl32.dll**: Standard on Windows, but you must link with `-lcomctl32`
- **Icon conversion tool**: Convert `C-icon.jpg` to `C-icon.ico` (any free online converter)
- **Windows 10+** recommended

---

## 🎯 Features
- 🪟 Native Windows GUI (Win32 API)
- 🖋️ Modern Segoe UI font and grouped layout
- 🖼️ Custom app icon (C-icon.ico)
- 💡 Tooltips for all controls
- ⌨️ Keyboard shortcuts
- 📋 Copy to clipboard
- 🟢 Password strength indicator (color-coded)
- 🧩 Select which character sets to include
- 🔢 Generate multiple passwords at once
- 💾 Remembers last settings
- 🪟 Window centers on startup
- 🗨️ About dialog (F1)
- 🏷️ Status bar for feedback

---

## ⌨️ Keyboard Shortcuts
| Shortcut   | Action                  |
|-----------|-------------------------|
| Enter     | Generate password(s)    |
| Ctrl+C    | Copy passwords          |
| F1        | About dialog            |

---

## 🏗️ How to Build
1. **Convert your icon:**
   - Convert `C-icon.jpg` to `C-icon.ico` (use any free online converter)
   - Place `C-icon.ico` in the project directory
2. **Build with GCC (MinGW):**
   ```
   gcc main.c -o learn.exe -mwindows -lcomctl32
   ```
3. **Run:**
   ```
   learn.exe
   ```

---

## 🧑‍💻 Usage
- Set password length and count
- Select which character sets to include
- Click **Generate** or press **Enter**
- Copy results with **Copy** button or **Ctrl+C**
- See password strength below output
- Status bar shows feedback
- Press **F1** for About dialog
- Settings are saved automatically

---

## 📸 Screenshots
<table>
<tr>
<td align="center"><b>Default view</b><br><img src="step-1.png" width="350"></td>
<td align="center"><b>After generating a password</b><br><img src="step-2.png" width="350"></td>
</tr>
</table>

---

Created by [TMHSDigital](https://github.com/TMHSDigital)