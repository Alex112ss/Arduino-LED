import cv2
import numpy as np
import copy
import math
import requests
import time  # 导入time模块

# 参数
cap_region_x_begin = 0.5  # 起始点/总宽度
cap_region_y_end = 0.8  # 起始点/总宽度
threshold = 60  # 二值化阈值
blurValue = 41  # 高斯模糊参数
bgSubThreshold = 50
learningRate = 0

# 变量
isBgCaptured = 0  # 是否捕获背景
triggerSwitch = False  # 如果为True，则键盘模拟器起作用

def printThreshold(thr):
    print("! Changed threshold to " + str(thr))

def removeBG(frame):
    fgmask = bgModel.apply(frame, learningRate=learningRate)
    kernel = np.ones((3, 3), np.uint8)
    fgmask = cv2.erode(fgmask, kernel, iterations=1)
    res = cv2.bitwise_and(frame, frame, mask=fgmask)
    return res

def calculateFingers(res, drawing):
    hull = cv2.convexHull(res, returnPoints=False)
    if len(hull) > 3:
        defects = cv2.convexityDefects(res, hull)
        if type(defects) != type(None):
            cnt = 0
            blue_dot_count = 0  # 用于计算两根手指之间的蓝点数量
            for i in range(defects.shape[0]):
                s, e, f, d = defects[i][0]
                start = tuple(res[s][0])
                end = tuple(res[e][0])
                far = tuple(res[f][0])
                a = math.sqrt((end[0] - start[0]) ** 2 + (end[1] - start[1]) ** 2)
                b = math.sqrt((far[0] - start[0]) ** 2 + (far[1] - start[1]) ** 2)
                c = math.sqrt((end[0] - far[0]) ** 2 + (end[1] - far[1]) ** 2)
                angle = math.acos((b ** 2 + c ** 2 - a ** 2) / (2 * b * c))  # 余弦定理
                if angle <= math.pi / 2:
                    cnt += 1
                    cv2.line(drawing, far, start, [211, 200, 200], 2)
                    cv2.line(drawing, far, end, [211, 200, 200], 2)
                    cv2.circle(drawing, far, 8, [211, 84, 0], -1)
                # 如果蓝点位于两根手指之间，增加蓝点计数
                if start[1] < far[1] and end[1] < far[1]:
                    blue_dot_count += 1
            return True, cnt, blue_dot_count
    return False, 0, 0

# 相机
camera = cv2.VideoCapture(0)
camera.set(10, 150)
cv2.namedWindow('trackbar')
cv2.createTrackbar('trh1', 'trackbar', threshold, 100, printThreshold)

# 标志用于检查是否已执行LED关闭和打开操作
led_closed = False
led_opened = False
brighter_triggered = False
dimmer_triggered = False

while camera.isOpened():
    ret, frame = camera.read()
    threshold = cv2.getTrackbarPos('trh1', 'trackbar')
    frame = cv2.bilateralFilter(frame, 5, 50, 100)
    frame = cv2.flip(frame, 1)
    cv2.rectangle(frame, (int(cap_region_x_begin * frame.shape[1]), 0),
                  (frame.shape[1], int(cap_region_y_end * frame.shape[0])), (255, 0, 0), 2)
    cv2.imshow('original', frame)

    if isBgCaptured == 1:
        img = removeBG(frame)
        img = img[0:int(cap_region_y_end * frame.shape[0]),
                  int(cap_region_x_begin * frame.shape[1]):frame.shape[1]]
        gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
        blur = cv2.GaussianBlur(gray, (blurValue, blurValue), 0)
        ret, thresh = cv2.threshold(blur, threshold, 255, cv2.THRESH_BINARY)

        thresh1 = copy.deepcopy(thresh)
        contours, hierarchy = cv2.findContours(thresh1, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)
        length = len(contours)
        maxArea = -1
        if length > 0:
            for i in range(length):
                temp = contours[i]
                area = cv2.contourArea(temp)
                if area > maxArea:
                    maxArea = area
                    ci = i

            res = contours[ci]
            hull = cv2.convexHull(res)
            drawing = np.zeros(img.shape, np.uint8)
            cv2.drawContours(drawing, [res], 0, (0, 255, 0), 2)
            cv2.drawContours(drawing, [hull], 0, (0, 0, 255), 3)

            isFinishCal, cnt, blue_dot_count = calculateFingers(res, drawing)
            
            # 在背景差分窗口上显示两根手指之间的蓝点数量
            cv2.putText(drawing, f"Blue Dots: {blue_dot_count}", (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 0, 255), 2)

            # 当检测到第一个蓝点且未执行过关闭操作时执行关闭操作
            if blue_dot_count == 0 and not led_closed:
                url = 'http://192.168.137.15/off'
                data = {'button': 'off'}
                response = requests.post(url, data=data)
                if response.status_code == 200:
                    print('LED關閉')
                    led_closed = True  # 设置标志，以确保操作只执行一次

            # 当检测到两个蓝点且未执行过打开操作时执行打开操作
            if blue_dot_count == 2 and not led_opened:
                url = 'http://192.168.137.15/on'
                data = {'button': 'on'}
                response = requests.post(url, data=data)
                if response.status_code == 200:
                    print('LED打開')
                    led_opened = True  # 设置标志，以确保操作只执行一次
                    
            # 当检测到3个蓝点且未执行过亮一点操作时执行亮一点操作
            if blue_dot_count == 3 and not brighter_triggered:
                url = 'http://192.168.137.15/brighter'
                data = {'button': 'brighter'}
                response = requests.post(url, data=data)
                if response.status_code == 200:
                    print('亮一點')
                    brighter_triggered = True  # 设置标志，以确保操作只执行一次

            # 当检测到4个蓝点且未执行过暗一点操作时执行暗一点操作
            if blue_dot_count == 4 and not dimmer_triggered:
                url = 'http://192.168.137.15/dimmer'
                data = {'button': 'dimmer'}
                response = requests.post(url, data=data)
                if response.status_code == 200:
                    print('暗一點')
                    dimmer_triggered = True        

        cv2.imshow('output', drawing)

    k = cv2.waitKey(10)
    if k == 27:
        camera.release()
        cv2.destroyAllWindows()
        break
    elif k == ord('b'):
        bgModel = cv2.createBackgroundSubtractorMOG2(0, bgSubThreshold)
        isBgCaptured = 1
        print('!!!Background Captured!!!')
        led_closed = False  # 重置LED关闭标志
        led_opened = False  # 重置LED打开标志
        brighter_triggered = False
        dimmer_triggered = False
        time.sleep(5)  # 等待5秒

    elif k == ord('r'):
        bgModel = None
        triggerSwitch = False
        isBgCaptured = 0
        print('!!!Reset BackGround!!!')

