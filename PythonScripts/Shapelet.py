import matplotlib.pyplot as plt
import numpy as np
from pyts.classification import LearningShapelets
from pyts.datasets import load_gunpoint
from pyts.utils import windowed_view

# Shapelet
X, _, y, _=load_gunpoint(return_X_y=True)
clf = LearningShapelets(random_state=42, tol=0.01, verbose=1)
clf.fit(X, y)

# Select two shapelets
shapelets = np.asarray([clf.shapelets_[0, -9], clf.shapelets_[0, -12]])

# Plot the shapelets
plt.figure(figsize=(12, 6))
plt.plot(shapelets[0].T, label='Shapelet 1')
plt.plot(shapelets[1].T, label='Shapelet 2')
plt.title('Shapelets')
plt.xlabel('Time')
plt.ylabel('Value')
plt.legend()
plt.tight_layout()
plt.show()

