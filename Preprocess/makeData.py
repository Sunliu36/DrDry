from sklearn.preprocessing import MinMaxScaler, RobustScaler, StandardScaler
from keras._tf_keras.keras.utils import to_categorical
from keras._tf_keras.keras.layers import Dropout
from keras._tf_keras.keras.layers import Dense
from keras._tf_keras.keras.models import Sequential
import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
import os
# os.environ['TF_ENABLE_ONEDNN_OPTS'] = '0'



TEST_FILE_NAME = '0428.csv'
SLIDING_WINDOW = 10
scaler = MinMaxScaler()  # None, StandardScaler(), MinMaxScaler(), RobustScaler()
# 每分鐘資料量: 6
dataPointsPerMin = 6
test_data = pd.read_csv(TEST_FILE_NAME, header=None)

if len(test_data.columns) == 8:
    label = ['Time', 'Humidity_Clo', 'TempC_Clo', 'TempF_Clo',
             'Humidity_Sur', 'TempC_Sur', 'TempF_Sur', 'Weight']
    test_data.columns = label
    # Sliding window 計算乾燥時間
    for i in range(SLIDING_WINDOW, len(test_data)):
        if test_data['Weight'][i] < 50:
            continue
        y = test_data['Weight'][i-SLIDING_WINDOW:i]
        # mean slope
        slope = (y.iloc[-1] - y.iloc[0]) / SLIDING_WINDOW
        if abs(slope) < 0.01:
            dryTime = i
            break
elif len(test_data.columns) == 10:
    label = ['Time', 'Humidity_Clo', 'TempC_Clo', 'TempF_Clo',
             'Humidity_Sur', 'TempC_Sur', 'TempF_Sur', 'Weight', 'Wet_Sur', 'Wet_Clo']
    test_data.columns = label
    # Sliding window 計算乾燥時間
    for i in range(SLIDING_WINDOW, len(test_data)):
        if test_data['Weight'][i] < 50:
            continue
        if test_data['Wet_Clo'][i] <= 3 and i > 15:
            dryTime = i
            break

X = np.array([])
for i in range(len(test_data)):
        if test_data['Weight'][i] > test_data['Weight'].mean():
            firstWeightIndex = i
            break
    
test_data=test_data.iloc[firstWeightIndex:,:]
test_data = test_data.reset_index(drop=True)

for i in range(SLIDING_WINDOW, len(test_data)):
    tenMinData = np.array([])
    for j in range(i-SLIDING_WINDOW, i):
        oneMinData = np.array([])
        oneMinData = np.append(oneMinData, test_data['Weight'][0])
        oneMinData = np.append(oneMinData, test_data['Humidity_Clo'][j])
        oneMinData = np.append(oneMinData, test_data['TempC_Clo'][j])
        oneMinData = np.append(oneMinData, test_data['Humidity_Sur'][j])
        oneMinData = np.append(oneMinData, test_data['TempC_Sur'][j])
        oneMinData = np.append(oneMinData, test_data['Weight'][j])
        tenMinData = np.append(tenMinData, oneMinData)
    if (i < dryTime):
        tenMinData = np.append(tenMinData, (dryTime - i))
    else:
        tenMinData = np.append(tenMinData, 0)
    X = np.append(X, tenMinData)
X = X.reshape(-1, SLIDING_WINDOW*dataPointsPerMin+1)
Y = X[:, -1]
X = X[:, :-1]

print(f'X={X}')
print(f'Y={Y}')
plt.figure()
plt.plot(Y)
plt.show()

data_df = pd.DataFrame(X)
data_df.to_csv('0428_test_2.csv', index=False, header=True)