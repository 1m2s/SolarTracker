"""
Notes
Compares the DS1302 clock with the Mac clock, live.
The Mac clock is synced from the internet, so it is the reference.
We compare in MEZ = UTC+1 all year, because the RTC is always MEZ.
Before: pio test -e nanotest -f test_rtc
Run:    ~/.platformio/penv/bin/python test/check_rtc.py
"""
import re
import sys
import time
from datetime import datetime, timedelta, timezone

import serial

PORT = sys.argv[1] if len(sys.argv) > 1 else "/dev/cu.wchusbserial140"
MARGIN_S = 60
SAMPLES = 5
MEZ = timezone(timedelta(hours=1))
LINE = re.compile(r"^(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2})$")

diffs = []
with serial.Serial(PORT, 115200, timeout=1) as ser:
    deadline = time.time() + 20
    while len(diffs) < SAMPLES and time.time() < deadline:
        raw = ser.readline().decode(errors="ignore").strip()
        m = LINE.match(raw)
        if not m:
            continue
        pc = datetime.now(MEZ).replace(tzinfo=None)
        rtc = datetime.strptime(m.group(1), "%Y-%m-%d %H:%M:%S")
        diff = (rtc - pc).total_seconds()
        diffs.append(diff)
        print(f"RTC {rtc}  Mac {pc:%Y-%m-%d %H:%M:%S}  diff {diff:+.1f} s")

if len(diffs) < SAMPLES:
    print("FAIL: no time lines received (is test_rtc flashed?)")
    sys.exit(1)

worst = max(diffs, key=abs)
print(f"worst diff {worst:+.1f} s, margin ±{MARGIN_S} s")
if abs(worst) <= MARGIN_S:
    print("PASS")
    sys.exit(0)
print("FAIL")
sys.exit(1)