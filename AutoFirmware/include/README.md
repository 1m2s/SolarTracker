# include/

Header files (`.h`), grouped by job. The matching definitions (`.cpp`) are in `../src/`.

- Which MATLAB file each function comes from is listed in [`../MATLAB_MAPPING.md`](../MATLAB_MAPPING.md).
- `config/` holds the settings: `Board.h` (pins, I2C), `Settings.h` (timings, location, limits) and `Calibration.h` (values measured on this tracker: compass calibration, mounting offset, declination). The `Mode` enum is in `Controller.h`.