#!/usr/bin/env python3

import subprocess
import sys

subprocess.run(["make"])
print(sys.argv)
for letter in "abcde":
    if len(sys.argv) == 1 or letter in sys.argv[1]:
        subprocess.run(["./solve ../data/" + letter + "*.in"], shell = True)
