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


TRAIN_FILE_DIR = 'after2/'
TEST_FILE_NAME = '0428.csv'
SLIDING_WINDOW = 10
scaler = MinMaxScaler()  # None, StandardScaler(), MinMaxScaler(), RobustScaler()
# 每分鐘資料量: 6
dataPointsPerMin = 6

# 檢查 TEST_FILE_NAME 是否存在
if not os.path.isfile(TEST_FILE_NAME):
    print(f'{TEST_FILE_NAME} not found!')
    exit()

# 檢查 TRAIN_FILE_DIR 是否有檔案
if not os.path.isdir(TRAIN_FILE_DIR):
    print(f'{TRAIN_FILE_DIR} not found!')
    exit()

X = np.array([])
temp = np.array([])
Y = np.array([])

# read every csv file in TRAIN_FILE_DIR
for files in os.listdir(TRAIN_FILE_DIR):
    data = pd.read_csv(TRAIN_FILE_DIR + files, header=None)
    if len(data.columns) == 8:
        label = ['Time', 'Humidity_Clo', 'TempC_Clo', 'TempF_Clo',
                 'Humidity_Sur', 'TempC_Sur', 'TempF_Sur', 'Weight']
        data.columns = label
        # Sliding window 計算乾燥時間
        for i in range(SLIDING_WINDOW, len(data)):
            if data['Weight'][i] < 50:
                continue
            y = data['Weight'][i-SLIDING_WINDOW:i]
            # mean slope
            slope = (y.iloc[-1] - y.iloc[0]) / SLIDING_WINDOW
            if abs(slope) < 0.01:
                dryTime = i
                break

    elif len(data.columns) == 10:
        label = ['Time', 'Humidity_Clo', 'TempC_Clo', 'TempF_Clo',
                 'Humidity_Sur', 'TempC_Sur', 'TempF_Sur', 'Weight', 'Wet_Sur', 'Wet_Clo']
        data.columns = label
        # Sliding window 計算乾燥時間
        for i in range(SLIDING_WINDOW, len(data)):
            if data['Weight'][i] < 50:
                continue
            if data['Wet_Clo'][i] <= 3 and i > 15:
                dryTime = i
                break
    firstWeightIndex = 0

    #判斷是否第一個重量
    for i in range(len(data)):
        if data['Weight'][i] > data['Weight'].mean():
            firstWeightIndex = i
            break
    
    data=data.iloc[firstWeightIndex:,:]
    data = data.reset_index(drop=True)
    for i in range(SLIDING_WINDOW, len(data)):
        tenMinData = np.array([])
        for j in range(i-SLIDING_WINDOW, i):
            oneMinData = np.array([])
            oneMinData = np.append(oneMinData, data['Weight'][0])
            oneMinData = np.append(oneMinData, data['Humidity_Clo'][j])
            oneMinData = np.append(oneMinData, data['TempC_Clo'][j])
            oneMinData = np.append(oneMinData, data['Humidity_Sur'][j])
            oneMinData = np.append(oneMinData, data['TempC_Sur'][j])
            oneMinData = np.append(oneMinData, data['Weight'][j])
            tenMinData = np.append(tenMinData, oneMinData)
        if (i < dryTime):
            tenMinData = np.append(tenMinData, (dryTime - i)) # 原：(dryTime - i) // 60
        else:
            tenMinData = np.append(tenMinData, 0)
        temp = np.append(temp, tenMinData)
    temp = temp.reshape(-1, SLIDING_WINDOW*dataPointsPerMin+1)
    print(f'{files} done! Amount of data points: {len(temp)}, dryTime: {dryTime}')
    # # 移除Y 為 0 的資料至保留最多 60 筆
    numRemove = 0
    for i in range(len(temp)):
        if temp[i, -1] == 0:
            numRemove += 1
        else:
            break
    if numRemove > 60:
        numRemove = 60
    temp = temp[numRemove:]
    Y = np.append(Y, temp[:, -1])
    X = np.append(X, temp[:, :-1])
    X = X.reshape(-1, SLIDING_WINDOW*dataPointsPerMin)
    temp = np.array([])



print(f'X={X}')
print(f'Y={Y}')
plt.figure()
plt.plot(Y)
plt.show()

if len(Y.shape) == 1 :
    Y = Y.reshape(-1, 1)
    print('Y is reshaped to 2D array !')
    print(Y)
data = np.concatenate((X, Y), axis=1)
data_df = pd.DataFrame(data)
data_df.to_csv('DataCluster5.csv', index=False, header=True)