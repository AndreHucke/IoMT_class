import numpy as np
import matplotlib.pyplot as plt
from sklearn.linear_model import LogisticRegression

# Logistic regression toy-example
# Hours spend studying vs. pass/fail

# Make data
X = np.array([[1], [2], [3], [4], [5], [6]])
y = np.array([0, 0, 0, 1, 1, 1])

# Create a logistic regression model
model = LogisticRegression()

# Fit the model
model.fit(X, y)

# Predict probabilities for 3.5 hours of studying
probability = model.predict_proba([[3.5]])[0][1]
print(f"Probability of passing after studying for 3.5 hours: {probability:.2f}")

# Predict pass/fail for the3.5 hours of studying
prediction = model.predict([[3.5]])[0]
print(f"Prediction for 3.5 hours of studying: {'Pass' if prediction == 1 else 'Fail'}")

# Visualize data
plt.scatter(X, y, color='blue', label='Data points')
plt.xlabel('Hours Studied')
plt.ylabel('Pass/Fail')

# # Plot the logistic regression curve
# x_values = np.linspace(0, 7, 100)
# y_values = model.predict_proba(x_values.reshape(-1, 1))[:, 1]
# plt.plot(x_values, y_values, color='red', label='Logistic Regression Curve')
# plt.title('Logistic Regression: Hours Studied vs. Pass/Fail')
# plt.legend()

# Show the plot
# plt.show()


# # Make a more interesting dataset
# X = np.random.rand(100, 1) * 8  # 100 random hours between 0 and 8

# # Make the y values  have some some noise
# y = (X + np.random.randn(100, 1) > 5).astype(int).ravel()

# # Refit the model
# model.fit(X, y)

# # Predict probabilities for 3.5 hours of studying
# probability = model.predict_proba([[3.5]])[0][1]
# print(f"Probability of passing after studying for 3.5 hours: {probability:.2f}")

# # Predict pass/fail for the3.5 hours of studying
# prediction = model.predict([[3.5]])[0]
# print(f"Prediction for 3.5 hours of studying: {'Pass' if prediction == 1 else 'Fail'}")

# # Visualize data
# plt.figure(figsize=(10, 6))
# plt.scatter(X, y, color='blue', label='Data points')
# plt.xlabel('Hours Studied')
# plt.ylabel('Pass/Fail')
# # Plot the logistic regression curve
# x_values = np.linspace(0, 9, 100)
# y_values = model.predict_proba(x_values.reshape(-1, 1))[:, 1]
# plt.plot(x_values, y_values, color='red', label='Logistic Regression Curve')
# plt.title('Logistic Regression: Hours Studied vs. Pass/Fail')
# plt.legend()
# Show the plot
# plt.show()

# SVM
from sklearn import datasets
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import StandardScaler
from sklearn.svm import SVC
from sklearn.metrics import accuracy_score

# Load the iris dataset
iris = datasets.load_iris()
X = iris.data
y = iris.target

# Split the dataset into training and testing sets
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.3, random_state=42)

# Feature scaling
scaler = StandardScaler()
X_train = scaler.fit_transform(X_train)
X_test = scaler.transform(X_test)

# Create an SVM classifier
svm_model = SVC(kernel='linear', C=0.5)

# Train the model
svm_model.fit(X_train, y_train)

# Make predictions on test data
y_pred = svm_model.predict(X_test)

# Ecaluate the accuracy
accuracy = accuracy_score(y_test, y_pred)
print(f"Accuracy of SVM model: {accuracy:.2f}")

# Visualize the decision boundary
# Create a mesh grid for plotting
# x_min, x_max = X[:, 0].min() - 1, X[:, 0].max() + 1
# y_min, y_max = X[:, 1].min() - 1, X[:, 1].max() + 1
# xx, yy = np.meshgrid(np.arange(x_min, x_max, 0.1),
#                      np.arange(y_min, y_max, 0.1))

# # Create feature vectors for all points in the mesh
# mesh_points = np.zeros((xx.size, X.shape[1]))
# mesh_points[:, 0] = xx.ravel()
# mesh_points[:, 1] = yy.ravel()

# # Set other features to their mean values
# for i in range(X.shape[1]):
#     if i != 0 and i != 1:
#         mesh_points[:, i] = X[:, i].mean()

# # Predict class for each mesh point
# Z = svm_model.predict(scaler.transform(mesh_points))
# Z = Z.reshape(xx.shape)

# # Plot the decision boundary
# plt.figure(figsize=(10, 6))
# plt.contourf(xx, yy, Z, alpha=0.8)
# plt.scatter(X[:, 0], X[:, 1], c=y, edgecolors='k', marker='o')
# plt.title('SVM Decision Boundary')
# plt.xlabel(f'Feature {0+1}')
# plt.ylabel(f'Feature {1+1}')
# plt.show()