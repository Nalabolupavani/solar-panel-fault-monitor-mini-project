# Testing Plan

## 1. Normal Operation

Expose the solar panel to normal illumination and record:

- LDR value
- Voltage
- Timestamp
- System status

Expected result: the voltage and light readings show a reasonable relationship.

## 2. Low-Light Test

Reduce the light reaching the panel/LDR.

Expected result: both readings should change consistently.

## 3. Sensor-Fault Simulation

For controlled testing, introduce a safe fault condition in the voltage-sensing path, such as temporarily disconnecting the sensing input according to the circuit design.

Expected result:

- The system identifies an abnormal voltage/light relationship.
- Fault status is displayed.
- The LDR remains available as a backup reference.

Do not short the solar panel or intentionally create unsafe electrical connections.

## 4. Future ML Test

After collecting sufficient historical data:

1. Split data into training and testing sets.
2. Train Linear Regression.
3. Predict voltage on unseen data.
4. Calculate MAE, RMSE and R².
5. Compare predicted voltage with actual voltage.
