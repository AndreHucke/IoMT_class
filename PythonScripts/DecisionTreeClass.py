# Decision tree imports
from sklearn.tree import DecisionTreeClassifier

# Random forest imports
from sklearn.ensemble import RandomForestClassifier
from sklearn.datasets import make_classification

# Data imports
from sklearn.datasets import load_iris
from sklearn.model_selection import train_test_split
from sklearn.metrics import accuracy_score

# Visualization imports
import matplotlib.pyplot as plt
from sklearn.tree import plot_tree

# Decision Tree part
# Iris dataset
iris = load_iris()
X = iris.data
y = iris.target

# split the data into training and test sets
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.3, random_state=42)

# Create a Decision Tree Classifier
dtree = DecisionTreeClassifier(max_depth=3)

# Train the model
dtree.fit(X_train, y_train)

# Make predictions
y_pred = dtree.predict(X_test)

# Calculate the accuracy
print("Accuracy:", accuracy_score(y_test, y_pred))

# Plot the tree
plt.figure(figsize=(20,20))
plot_tree(dtree)
# plt.show()

# Random Forest part
# Generate a synthetic dataset
X, y = make_classification(n_samples=20000, n_features=20, n_informative=10, n_redundant=2, random_state=42)

# Split the data into training and test sets
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.3, random_state=42)

# Create Random Forest Classifier
rf = RandomForestClassifier(n_estimators=100, random_state=42)

# Train the model
rf.fit(X_train, y_train)

# Make predictions
y_pred = rf.predict(X_test)

# Calculate the accuracy
print("Accuracy:", accuracy_score(y_test, y_pred))

# Plot the tree
plt.figure(figsize=(20,20))
plot_tree(rf.estimators_[0], filled=True)
# plt.show()