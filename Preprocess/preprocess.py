import os
import pandas as pd

BEFORE_FILE_DIR = 'original/'
AFTER_FILE_DIR = 'after2/'
SLIDING_WINDOW = 10


for files in os.listdir(BEFORE_FILE_DIR):
    print(f'start {files}')
    # read the data
    data = pd.read_csv(BEFORE_FILE_DIR + files, header=None)
    # check row length
    if len(data.columns) == 8:
        label = ['Time', 'Humidity_Clo', 'TempC_Clo', 'TempF_Clo',
                 'Humidity_Sur', 'TempC_Sur', 'TempF_Sur', 'Weight']
        data.columns = label

    elif len(data.columns) == 10:
        label = ['Time', 'Humidity_Clo', 'TempC_Clo', 'TempF_Clo',
                 'Humidity_Sur', 'TempC_Sur', 'TempF_Sur', 'Weight', 'Wet_Sur', 'Wet_Clo']
        data.columns = label
    
    # loop through the data, 如果重量與前十分鐘相差超過 50 克，則不計算
    for i in range(1, len(data)-1):
        if abs(data['Weight'][i] - data['Weight'][i-1]) > 50 and abs(data['Weight'][i+1] - data['Weight'][i]) > 50:
            # 重量 = 前後平均
            data.loc[i, 'Weight'] = (
                data['Weight'][i-1] + data['Weight'][i+1]) / 2
            continue
        if i == 1:
            # 第一筆資料
            if data['Weight'][i-1] < 80:
                data.loc[i-1, 'Weight'] = 0
                continue
        if data['Weight'][i] < 80 and i < 100:
            # 重量小於 50 克，不計算
            data.loc[i, 'Weight'] = 0
            continue

    # drop the row that weight is 0
    data = data[data['Weight'] != 0]
    # # write back to the csv file
    data.to_csv(AFTER_FILE_DIR + files, index=False, header=False)
