from pathlib import Path
import shutil
import os

Import("env")


ASSET_MAP = (
    ("128x16",   ("128x16_frame.raw", "128x16_logo.raw")),
    ("192x64",   ("192x64_frame.raw", "192x64_logo.raw")),
    ("128x64x2", ("256x64_frame.raw", "256x64_logo.raw")),
    ("256x64",   ("256x64_frame.raw", "256x64_logo.raw")),
    ("128x64",   ("128x32_frame.raw", "128x32_logo.raw")),
    ("128x32",   ("128x32_frame.raw", "128x32_logo.raw")),
)


project_dir = Path(env.subst("$PROJECT_DIR")).resolve()
env_name = env.subst("$PIOENV")
staged_data_dir = project_dir / ".pio" / "fsdata" / env_name

if env_name.startswith("ppucdmd_"):
    source_data_dir = project_dir / "data_ppucdmd"
else:
    source_data_dir = Path(env.subst("$PROJECT_DATA_DIR")).resolve()

allowed_files = None
for resolution, asset_files in ASSET_MAP:
    if resolution in env_name:
        allowed_files = asset_files
        break

if allowed_files is None:
    raise ValueError(f"No filesystem asset mapping defined for env '{env_name}'")

if staged_data_dir.exists():
    shutil.rmtree(staged_data_dir)
staged_data_dir.mkdir(parents=True, exist_ok=True)

missing_files = []
for file_name in allowed_files:
    source_file = source_data_dir / file_name
    if source_file.is_file():
        shutil.copy2(source_file, staged_data_dir / file_name)
    else:
        missing_files.append(file_name)

if missing_files:
    missing_list = ", ".join(missing_files)
    raise FileNotFoundError(f"Missing filesystem asset(s): {missing_list}")

env.Replace(PROJECT_DATA_DIR=str(staged_data_dir))
print(f"Filesystem data staged in {staged_data_dir}: {', '.join(allowed_files)}")

def merge_binaries(source, target, map, **kwargs):
    build_dir = env.subst("$BUILD_DIR")
    
    firmware_elf = os.path.join(build_dir, "firmware.elf")
    firmware_bin = os.path.join(build_dir, "firmware.bin")
    littlefs_bin = os.path.join(build_dir, "littlefs.bin")
    combined_bin = os.path.join(build_dir, "firmware_with_fs.bin")
    
    # 1. Use PlatformIO's internal toolchain to convert ELF -> BIN on the fly
    env.Execute(f'"{env.subst("$OBJCOPY")}" -O binary "{firmware_elf}" "{firmware_bin}"')

    if not os.path.exists(firmware_bin) or not os.path.exists(littlefs_bin):
        print("--> Error: Required binaries for merging are missing.")
        return

    print(f"--> Generating combined BIN: {combined_bin}")
    fs_offset = 0x1bf000 
    
    with open(firmware_bin, "rb") as f_fw:
        fw_data = f_fw.read()
    with open(littlefs_bin, "rb") as f_fs:
        fs_data = f_fs.read()
        
    padded_fw = fw_data.ljust(fs_offset, b'\xFF')
    with open(combined_bin, "wb") as f_out:
        f_out.write(padded_fw + fs_data)
        
    print("--> Success: Combined monolithic BIN successfully created!")

env.AddPostAction(os.path.join(env.subst("$BUILD_DIR"), "littlefs.bin"), merge_binaries)
