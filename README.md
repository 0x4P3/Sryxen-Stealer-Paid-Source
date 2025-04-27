# 🚀 Farewell & Final Notes  

This marks the end of my journey. Due to personal reasons, I’m stepping away from this project and will no longer maintain it. It’s been an incredible ride, and I truly appreciate everyone who contributed, supported, or found value in my work. But as with all things, there comes a time to move on.  

---  

## Telegram:
- https://t.me/ebytelabs
---  

## 🛠️ How to Build This Project  

If you want to **set up and run this project yourself**, follow these steps:  

### 1️⃣ Install Dependencies  

Make sure to install the required libraries using `vcpkg`:  

```sh
.\vcpkg install libsodium:x64-windows-static
.\vcpkg install sqlite3:x64-windows-static
```

### 2️⃣ Configure Your Compiler  

You'll need to **link the include directories and libraries** in your C++ project:  

- **Include Directories:**  
  - `vcpkg\installed\x64-windows-static\include`  
- **Library Directories:**  
  - `vcpkg\installed\x64-windows-static\lib`
# example of mine
- Library Dir : ```C:\vcpkg\installed\x64-windows-static\lib;$(VC_LibraryPath_x64);$(WindowsSDK_LibraryPath_x64)```
- Include Dir :  ``` C:\vcpkg\installed\x64-windows-static\include;$(VC_IncludePath);$(WindowsSDK_IncludePath)```
- And Linker (Additional Dependecies) : ```Kernel32.lib;Advapi32.lib;C:\vcpkg\installed\x64-windows-static\lib\sqlite3.lib;C:\vcpkg\installed\x64-windows-static\lib\libsodium.lib```
- Also Remove generating debug info if youre not skid you know what to do, this was just for skids
- i advise to link mfc statically to get rid of !cl flag
- btw make your own pem private + public + theres one for yall but ye
![image](https://github.com/user-attachments/assets/6c32e006-3a1a-4e5d-a427-319dc9cc69fd)

### 3️⃣ Replace Inputs with Your Own  

- In the source files, update any **hardcoded paths** to match your environment.  

---  

## 💀 Final Words  

**@codepulze is deleted, and this account will be gone soon.**  

Thank you all for the journey. Whether you fork, modify, or build upon this, I hope it serves you well.  

Take care, and goodbye. 🚀


## License
This project is licensed under the MIT License. See the LICENSE file for details.
