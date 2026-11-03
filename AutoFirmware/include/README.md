# include/

Header files (`.h`), grouped by job. The matching definitions (`.cpp`) are in `../src/`.

- Each function ported from MATLAB names its source file in a `From MATLAB: X.m` line. The `@file` block of each header lists all the `.m` files it comes from.
- `config/` holds the settings: `Board.h` (pins, I2C) and `Settings.h` (timings, location, limits, modes).