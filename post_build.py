import os
import time
import subprocess

Import("env")

def post_build(source, target, env):
    build_dir = env.subst("$BUILD_DIR")
    bootloader_path = os.path.join(build_dir, "bootloader.bin")
    partitions_path = os.path.join(build_dir, "partitions.bin")
    firmware_path = os.path.join(build_dir, "firmware.bin")

    if os.path.exists(bootloader_path) and os.path.exists(partitions_path) and os.path.exists(firmware_path):
        timestamp = time.strftime("%Y%m%d_%H%M%S")
        merged_name = f"firmware_full_{timestamp}.bin"
        merged_path = os.path.join(build_dir, merged_name)

        # Merge command
        cmd = [
            "esptool.py", "--chip", "esp32", "merge_bin",
            "-o", merged_path,
            "--flash_mode", "dio",
            "--flash_freq", "40m",
            "--flash_size", "4MB",
            "0x1000", bootloader_path,
            "0x8000", partitions_path,
            "0x10000", firmware_path
        ]

        try:
            subprocess.run(cmd, check=True)
            print(f"Full firmware image created: {merged_name}")
        except subprocess.CalledProcessError as e:
            print(f"Error merging firmware: {e}")

    # Rename the original firmware.bin as well
    if os.path.exists(firmware_path):
        timestamp = time.strftime("%Y%m%d_%H%M%S")
        new_name = f"firmware_{timestamp}.bin"
        new_path = os.path.join(build_dir, new_name)
        os.rename(firmware_path, new_path)
        print(f"Firmware renamed to: {new_name}")

# Register the post-build function
env.AddPostAction("buildprog", post_build)