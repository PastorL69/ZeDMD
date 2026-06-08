import os
from os.path import join
Import("env")

def merge_binaries(source, target, map):
    # Path to your build directory
    build_dir = env.subst("$BUILD_DIR")
    
    firmware_bin = join(build_dir, "firmware.bin")
    littlefs_bin = join(build_dir, "littlefs.bin")
    combined_bin = join(build_dir, "firmware_with_fs.bin")
    
    # Wait until both files actually exist
    if not os.path.exists(firmware_bin) or not os.path.exists(littlefs_bin):
        print("Waiting for binaries to be generated...")
        return

    print(f"Generating combined BIN: {combined_bin}")
    
    # RP2040/RP2350 Flash starts at 0x10000000. 
    # fs offset for ppuc/dmd is 0x101bf000, meaning the relative flash offset is 0x1bf000
    fs_offset = 0x1bf000 
    
    with open(firmware_bin, "rb") as f_fw:
        fw_data = f_fw.read()
        
    with open(littlefs_bin, "rb") as f_fs:
        fs_data = f_fs.read()
        
    # Pad the firmware with 0xFF up to the filesystem offset, then append FS
    padded_fw = fw_data.ljust(fs_offset, b'\xFF')
    
    with open(combined_bin, "wb") as f_out:
        f_out.write(padded_fw + fs_data)
        
    print("Combined BIN successfully created!")

# Trigger this after the filesystem image ('littlefs.bin') is built
env.AddPostAction("$BUILD_DIR/littlefs.bin", merge_binaries)