import numpy as np
import pandas as pd
from statsmodels.tsa.ar_model import AutoReg
import matplotlib.pyplot as plt

# Generate a sample time series
np.random.seed(0)
n_samples = 100
time_series = pd.Series(np.random.randn(n_samples)).cumsum()

# Chossing lag order
lag_order = 1

# Create and fit the AutoReg model
model = AutoReg(time_series, lags=lag_order)
model_fit = model.fit()

# Make predictions
predictions = model_fit.predict(start=lag_order, end=len(time_series) -1)

# Make future predictions
future_steps = 10
future_predictions = model_fit.predict(start=len(time_series), end=len(time_series) + future_steps - 1)

# Plot the results
plt.figure(figsize=(10, 6))
plt.plot(time_series, label='Original Time Series')
plt.plot(predictions, label='Predictions', color='red')
plt.plot(range(len(time_series), len(time_series) + future_steps), future_predictions, label='Future Predictions', color='green')
plt.legend()
plt.title('AutoReg Time Series Forecasting')
plt.xlabel('Time')
plt.ylabel('Value')
# plt.show()

# Print summary
print(model_fit.summary())

# Calculate correlation coefficient
data = {'value': [10, 12, 15, 13, 16, 18, 20, 19, 22, 25]}
df = pd.DataFrame(data)

# Calculate lag values
df['lag1'] = df['value'].shift(lag_order)
df['lag2'] = df['value'].shift(lag_order * 2)

# Calculate correlation matrrix
correlation_matrix = df.corr()

# Print correlation matrix
print("Correlation Matrix:")
print(correlation_matrix)

