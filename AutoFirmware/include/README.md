# include/

Header files (`.h`), grouped by job. The matching definitions (`.cpp`) are in `../src/`.

- Functions ported from MATLAB keep their MATLAB names. The `@file` block of each header lists the `.m` files it comes from, and each function says `From MATLAB: X.m`.
- `config/` holds the settings: `Board.h` (pins, I2C) and `Settings.h` (timings, location, limits, modes).