"""Finds the compass calibration from one or more test_compass_cal runs.

Usage (from AutoFirmware/):
    ~/.platformio/penv/bin/python test/setup/test_compass_cal/fit_compass.py RUN1.txt [RUN2.txt ...]

Every run must be an EXACT half turn (180 deg, table edge to table edge),
held still at the start and at the end.

Prints the five numbers for include/config/Calibration.h:
    MX_CENTRE, MY_CENTRE   where the middle of the oval is
    SOFT_XX, SOFT_XY, SOFT_YY   how to stretch the oval back into a circle
Only the Python standard library is used.
"""

import math
import re
import sys

MAG_SCALE = 1200.0 / 4096.0    # same as Heading.cpp
STILL = 10                     # lines at the start and at the end that must be still
MAX_STILL_SPREAD = 4           # raw units: more -> the tracker moved while it should be still
GOOD_TURN = (175.0, 185.0)     # a 180 deg turn must read inside this after the correction
MAX_RADIUS_SPREAD = 20.0       # percent: more -> the oval correction did not make a circle


def read_points(path):
    pattern = re.compile(r"CAL\s+(-?\d+)\s+(-?\d+)\s+(-?\d+)")
    points = []
    with open(path, encoding="utf-8", errors="replace") as f:
        for line in f:
            m = pattern.search(line)
            if m:
                points.append((int(m.group(1)), int(m.group(2))))
    return points


def mean(points):
    n = len(points)
    return (sum(p[0] for p in points) / n, sum(p[1] for p in points) / n)


def spread(points):
    xs = [p[0] for p in points]
    ys = [p[1] for p in points]
    return max(max(xs) - min(xs), max(ys) - min(ys))


def solve3(m, v):
    """Solves the 3x3 system m * x = v (Gauss elimination)."""
    a = [row[:] + [vi] for row, vi in zip(m, v)]
    for i in range(3):
        p = max(range(i, 3), key=lambda k: abs(a[k][i]))
        a[i], a[p] = a[p], a[i]
        for k in range(3):
            if k != i:
                f = a[k][i] / a[i][i]
                a[k] = [x - f * y for x, y in zip(a[k], a[i])]
    return [a[i][3] / a[i][i] for i in range(3)]


def fit_oval(points, c):
    """Oval around a known centre c: (p-c)^T Q (p-c) = 1. Gives back Q = (a, b, d)."""
    s = [[0.0] * 3 for _ in range(3)]
    t = [0.0] * 3
    for x, y in points:
        dx, dy = x - c[0], y - c[1]
        row = [dx * dx, 2 * dx * dy, dy * dy]
        for i in range(3):
            t[i] += row[i]
            for j in range(3):
                s[i][j] += row[i] * row[j]
    return solve3(s, t)


def stretch_matrix(q):
    """W = square root of Q, scaled so that det(W) = 1 (area stays the same).
    W turns the oval into a circle. Also gives back the two oval radii."""
    a, b, d = q
    half_trace = (a + d) / 2
    root = math.sqrt(half_trace ** 2 - (a * d - b * b))
    l1, l2 = half_trace + root, half_trace - root          # eigenvalues of Q
    angle = 0.5 * math.atan2(2 * b, a - d)                  # direction of l1
    c, s = math.cos(angle), math.sin(angle)
    k = 1.0 / math.sqrt(math.sqrt(l1 * l2))                 # makes det(W) = 1
    r1, r2 = math.sqrt(l1) * k, math.sqrt(l2) * k
    wxx = r1 * c * c + r2 * s * s
    wxy = (r1 - r2) * c * s
    wyy = r1 * s * s + r2 * c * c
    return (wxx, wxy, wyy), (1 / math.sqrt(l2), 1 / math.sqrt(l1))


def angle_of(p, c, w=None):
    dx, dy = p[0] - c[0], p[1] - c[1]
    if w is not None:
        dx, dy = w[0] * dx + w[1] * dy, w[1] * dx + w[2] * dy
    return math.degrees(math.atan2(dy, dx))


def radius(p, c, w=None):
    dx, dy = p[0] - c[0], p[1] - c[1]
    if w is not None:
        dx, dy = w[0] * dx + w[1] * dy, w[1] * dx + w[2] * dy
    return math.hypot(dx, dy)


def percent(values):
    """Spread of values around their middle, in percent: (max - min) / 2 / mean."""
    m = sum(values) / len(values)
    return 100.0 * (max(values) - min(values)) / 2.0 / m


def turn(start, end, c, w=None):
    d = angle_of(start, c, w) - angle_of(end, c, w)
    return abs((d + 180.0) % 360.0 - 180.0)


def calibrate(runs):
    """runs: list of (name, points). Gives back centre, W and the oval radii."""
    mids = []
    for _, pts in runs:
        s, e = mean(pts[:STILL]), mean(pts[-STILL:])
        mids.append(((s[0] + e[0]) / 2, (s[1] + e[1]) / 2))
    c = mean(mids)
    w, radii = stretch_matrix(fit_oval([p for _, pts in runs for p in pts], c))
    return c, w, radii


def main():
    if len(sys.argv) < 2:
        sys.exit("Usage: fit_compass.py RUN1.txt [RUN2.txt ...]")

    runs = []
    for path in sys.argv[1:]:
        pts = read_points(path)
        if len(pts) < 3 * STILL:
            sys.exit(f"{path}: only {len(pts)} CAL lines. Was it run with -v and | tee?")
        runs.append((path, pts))

    ok = True
    print("Runs:")
    for name, pts in runs:
        s_sp, e_sp = spread(pts[:STILL]), spread(pts[-STILL:])
        s, e = mean(pts[:STILL]), mean(pts[-STILL:])
        print(f"  {name}: {len(pts)} lines, start ({s[0]:.1f}, {s[1]:.1f}), end ({e[0]:.1f}, {e[1]:.1f})")
        if max(s_sp, e_sp) > MAX_STILL_SPREAD:
            print("    WARNING: not still at the start or end. Hold still for 2 s at both ends.")
            ok = False

    c, w, radii = calibrate(runs)
    print()
    print(f"Centre (raw):  ({c[0]:.2f}, {c[1]:.2f})")
    print(f"Oval radii:    {radii[0]:.1f} and {radii[1]:.1f}  (ratio {radii[0] / radii[1]:.2f}, 1.00 = circle)")

    print()
    print("Check 1 (centre): each run is a real 180 deg turn. How does it read?")
    for name, pts in runs:
        s, e = mean(pts[:STILL]), mean(pts[-STILL:])
        after = turn(s, e, c, w)
        print(f"  {name}: {after:5.1f} deg")
        if not GOOD_TURN[0] <= after <= GOOD_TURN[1]:
            ok = False

    print()
    print("Check 2 (oval): distance of every point to the centre. A circle has one radius.")
    all_pts = [p for _, pts in runs for p in pts]
    before = [radius(p, c) for p in all_pts]
    after = [radius(p, c, w) for p in all_pts]
    print(f"  before the oval correction: {min(before):.1f} .. {max(before):.1f}  ({percent(before):.0f} %)")
    print(f"  after the oval correction:  {min(after):.1f} .. {max(after):.1f}  ({percent(after):.0f} %)")
    if percent(after) > MAX_RADIUS_SPREAD:
        ok = False
    shift = max(abs((angle_of(p, c, w) - angle_of(p, c) + 180.0) % 360.0 - 180.0) for p in all_pts)
    print(f"  the oval correction moves headings by up to {shift:.0f} deg (check at 90 deg with test_compass)")

    if len(runs) >= 3:
        print()
        print("Fairness check: calibrate WITHOUT one run, then test on that run:")
        for i, (name, pts) in enumerate(runs):
            c2, w2, _ = calibrate(runs[:i] + runs[i + 1:])
            s, e = mean(pts[:STILL]), mean(pts[-STILL:])
            after = turn(s, e, c2, w2)
            print(f"  {name}: {after:5.1f} deg")
            if not GOOD_TURN[0] <= after <= GOOD_TURN[1]:
                ok = False

    print()
    print("For include/config/Calibration.h:")
    print(f"  const float MX_CENTRE = {c[0] * MAG_SCALE:.2f}f;")
    print(f"  const float MY_CENTRE = {c[1] * MAG_SCALE:.2f}f;")
    print(f"  const float SOFT_XX   = {w[0]:.4f}f;")
    print(f"  const float SOFT_XY   = {w[1]:.4f}f;")
    print(f"  const float SOFT_YY   = {w[2]:.4f}f;")
    print()
    if ok:
        print("Looks good.")
    else:
        print(f"WARNING: a check failed (180 deg must read {GOOD_TURN[0]:.0f}..{GOOD_TURN[1]:.0f}). "
              "Repeat the runs before using these numbers.")


if __name__ == "__main__":
    main()


# Notes
# 1) Why the centre is the midpoint: in an exact 180 deg turn, the start and
#    the end point lie exactly opposite each other around the true centre,
#    even when the circle is squashed into an oval. So the centre is halfway
#    between them. With several runs, the midpoints are averaged.
#
# 2) The oval: iron near the sensor (e.g. the stepper motor) squashes the
#    circle into an oval. Q describes that oval: (p-c)^T Q (p-c) = 1.
#    This is linear in the three numbers of Q, so it is a normal
#    least-squares fit, solved by solve3().
#
# 3) The stretch: W = sqrt(Q) turns the oval back into a circle.
#    It is scaled so det(W) = 1, so the size of the field stays about the
#    same and only the shape changes. W is symmetric, so 3 numbers are enough.
#
# 4) Check 1 tests the centre: a 180 deg turn must read about 180 deg.
#    It cannot test the oval, because start and end are opposite each
#    other with or without the oval. Check 2 tests the oval: after the
#    correction every point must have about the same distance to the centre.
#
# 5) Fairness check: a calibration always fits its own data. Testing on a
#    run that was left out shows whether it also works on new data.
#
# 6) What this cannot check: positions in the middle of the turn (e.g. 90 deg).
#    That is what setup/test_compass is for.