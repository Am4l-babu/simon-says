#!/usr/bin/env python3
"""
Copy the PlatformIO sources into Arduino IDE sketch folders.

PlatformIO is the source of truth:
    src/main.cpp                        ->  arduino/SimonSays/SimonSays.ino
    hardware_tests/test_buttons/main.cpp ->  arduino/tests/Test_Buttons/Test_Buttons.ino
    hardware_tests/test_rgb/main.cpp     ->  arduino/tests/Test_RGB/Test_RGB.ino
    hardware_tests/test_oled/main.cpp    ->  arduino/tests/Test_OLED/Test_OLED.ino

The Arduino IDE needs   <folder>/<folder>.ino   so each sketch gets its own folder.

Usage:  python tools/sync_arduino.py
"""
from pathlib import Path
import shutil

ROOT = Path(__file__).resolve().parent.parent

MAPPING = {
    "src/main.cpp":                          "arduino/SimonSays/SimonSays.ino",
    "hardware_tests/test_buttons/main.cpp":  "arduino/tests/Test_Buttons/Test_Buttons.ino",
    "hardware_tests/test_rgb/main.cpp":      "arduino/tests/Test_RGB/Test_RGB.ino",
    "hardware_tests/test_oled/main.cpp":     "arduino/tests/Test_OLED/Test_OLED.ino",
}

for src, dst in MAPPING.items():
    dst_path = ROOT / dst
    dst_path.parent.mkdir(parents=True, exist_ok=True)
    shutil.copyfile(ROOT / src, dst_path)
    print(f"  {src}  ->  {dst}")

print("Done.")
