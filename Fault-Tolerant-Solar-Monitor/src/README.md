# ESP32 Source Code

The main firmware is:

`solar_monitor.ino`

## What it does

- Reads solar-panel voltage through an ESP32 ADC pin.
- Reads the LDR analog value.
- Converts the ADC voltage using the configured voltage-divider ratio.
- Estimates an expected voltage from the LDR as a simple backup/reference method.
- Flags a possible sensor fault when the measured and expected values differ beyond the configured threshold.
- Runs a local Wi-Fi dashboard directly from the ESP32.

## Before uploading

Open `solar_monitor.ino` and verify:

- `VOLTAGE_PIN`
- `LDR_PIN`
- `R1`
- `R2`
- `VOLTAGE_CALIBRATION`
- `LDR_DARK`
- `LDR_BRIGHT`
- `EXPECTED_VOLTAGE_MIN`
- `EXPECTED_VOLTAGE_MAX`
- `FAULT_THRESHOLD`

These values must be calibrated to the actual prototype.

## Dashboard

After uploading, connect a phone/laptop to:

`Wi-Fi: SolarMonitor`

`Password: 12345678`

Then open:

`http://192.168.4.1`

## Important

The LDR-based backup in this version is **not machine learning**. It is a simple reference/estimation method.

AI/ML prediction using historical data and Linear Regression is planned as future work.
