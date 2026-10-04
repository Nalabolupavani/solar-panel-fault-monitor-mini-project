# System Architecture

## Current Prototype

The prototype uses two independent observations:

1. Solar-panel voltage
2. Relative light intensity from an LDR

The LDR is not used as a direct voltage replacement. It is an independent reference used to identify whether a voltage reading is plausible.

## Signal Flow

```text
Solar Panel ──> Voltage Measurement ──> ESP32
                                      LDR ───────────────────────────────────> ESP32
                                           |
                                           v
                                  Consistency Check
                                      /       \
                                   Normal    Fault
                                      |         |
                                      v         v
                              Measured Data   LDR Reference
                                      \       /
                                       v     v
                                     Dashboard
```

## Future AI Integration

Historical records will be used to train a regression model.

Possible features:

- Light intensity
- Time of day
- Previous voltage
- Previous light readings

Target:

- Solar-panel voltage

Initial model:

**Linear Regression**

Evaluation metrics:

- MAE
- RMSE
- R²

The prediction model should only be added after enough representative data has been collected.
