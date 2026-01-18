import os
import shutil
import pathlib

X64_BUILD_DIR = ".\\x64\\Release"
X86_BUILD_DIR = ".\\Release"
X64_PUBLISH_DIR = ".\\publish\\x64"
X86_PUBLISH_DIR = ".\\publish\\x86"

os.makedirs(X64_PUBLISH_DIR, exist_ok=True)
os.makedirs(X86_PUBLISH_DIR, exist_ok=True)

previous_files = os.listdir(X64_PUBLISH_DIR)
for file in previous_files:
    full_path = pathlib.Path(os.path.join(X64_PUBLISH_DIR, file))
    if full_path.suffix not in (".7z"):
        os.remove(full_path)

previous_files = os.listdir(X86_PUBLISH_DIR)
for file in previous_files:
    full_path = pathlib.Path(os.path.join(X86_PUBLISH_DIR, file))
    if full_path.suffix not in (".7z"):
        os.remove(full_path)

files = os.listdir(X64_BUILD_DIR)
for file in files:
    full_path = pathlib.Path(os.path.join(X64_BUILD_DIR, file))
    if full_path.suffix in (".dll", ".exe", ".config"):
        shutil.copy(full_path, X64_PUBLISH_DIR)
        shutil.copy(full_path, X86_PUBLISH_DIR)

files = os.listdir(X86_BUILD_DIR)
for file in files:
    full_path = pathlib.Path(os.path.join(X86_BUILD_DIR, file))
    if full_path.suffix in (".dll", ".exe", ".config"):
        shutil.copy(full_path, X86_PUBLISH_DIR)
