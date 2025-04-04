import numpy as np
from dtw import dtw
import json
from pathlib import Path

# Best thing out-of-the-box for time series data classification

data_path = Path('Example_python/iomt_data_1.json').resolve()
data_path_2 = Path('Example_python/iomt_data_2.json').resolve()

with open(data_path, 'r') as file:
    data = json.load(file)

key_list = list(data.keys())
dtw_out = np.zeros((len(key_list), len(key_list)))
for i in range(len(key_list)):
    for j in range(len(key_list)):
        dtw_out[i, j] = dtw(data[key_list[i]], data[key_list[j]]).distance

# print(out)

from sklearn.neighbors import KNeighborsClassifier

# Perform K-NN over DTW distances
# Classes are 0-99 is class 1 and 100-199 is class 2

knn = KNeighborsClassifier(n_neighbors=3)
classes = np.concatenate((np.zeros(100), np.ones(100)))
knn.fit(dtw_out, classes)

# Predict class of new data
# Load iomt_data_2.json
with open(data_path_2, 'r') as file:
    data_2 = json.load(file)

key_list_2 = list(data_2.keys())
dtw_out_2 = np.zeros((len(key_list_2), len(key_list)))
for i in range(len(key_list_2)):
    for j in range(len(key_list)):
        dtw_out_2[i, j] = dtw(data_2[key_list_2[i]], data[key_list[j]]).distance

# print(out_2)

print(knn.predict(dtw_out_2))



