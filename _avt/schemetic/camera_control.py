#!/usr/bin/env python3
"""
라즈베리파이 듀얼 카메라 제어 시스템
GPIO와 OpenCV를 사용한 자동 촬영 시스템
"""

import RPi.GPIO as GPIO
import cv2
import time
from datetime import datetime
import os

# GPIO 핀 설정
LIGHT1 = 5
LIGHT2 = 6
LIMIT1 = 16
LIMIT2 = 17
BUTTON1 = 20
BUTTON2 = 21
BUTTON3 = 22
RELAY1 = 26
RELAY2 = 27

# 카메라 인덱스
available_cameras = []

# 저장 디렉토리
SAVE_DIR = "/home/pi/camera_images"

class DualCameraController:
    def __init__(self):
        # GPIO 초기화
        GPIO.setmode(GPIO.BCM)
        GPIO.setwarnings(False)
        
        # 출력 핀 설정
        GPIO.setup(LIGHT1, GPIO.OUT)
        GPIO.setup(LIGHT2, GPIO.OUT)
        GPIO.setup(RELAY1, GPIO.OUT)
        GPIO.setup(RELAY2, GPIO.OUT)
        
        # 입력 핀 설정 (풀다운 저항)
        GPIO.setup(LIMIT1, GPIO.IN, pull_up_down=GPIO.PUD_UP)
        GPIO.setup(LIMIT2, GPIO.IN, pull_up_down=GPIO.PUD_UP)
        GPIO.setup(BUTTON1, GPIO.IN, pull_up_down=GPIO.PUD_UP)
        GPIO.setup(BUTTON2, GPIO.IN, pull_up_down=GPIO.PUD_UP)
        GPIO.setup(BUTTON3, GPIO.IN, pull_up_down=GPIO.PUD_UP)
        
        # 초기 상태 설정
        GPIO.output(LIGHT1, GPIO.LOW)
        GPIO.output(LIGHT2, GPIO.LOW)
        GPIO.output(RELAY1, GPIO.LOW)
        GPIO.output(RELAY2, GPIO.LOW)
        
        # 저장 디렉토리 생성
        if not os.path.exists(SAVE_DIR):
            os.makedirs(SAVE_DIR)

        print("카메라 검색 중...")
        for i in range(5):
            cap = cv2.VideoCapture(i)
            if cap.isOpened():
                ret, frame = cap.read()
                if ret:
                    # width = cap.get(cv2.CAP_PROP_FRAME_WIDTH)
                    # height = cap.get(cv2.CAP_PROP_FRAME_HEIGHT)
                    # fps = cap.get(cv2.CAP_PROP_FPS)
                    
                    # print(f"\n카메라 {i} 발견!")
                    # print(f"  해상도: {int(width)} x {int(height)}")
                    # print(f"  FPS: {fps}")
                    
                    available_cameras.append(i)
                cap.release()

        if len(available_cameras)<2:
            print(f"\n카메라가 {len(available_cameras)}개 연결되어 있어 종료합니다.")
            self.cleanup()
            quit()
        
        print("듀얼 카메라 제어 시스템 초기화 완료")
    
    def wait_for_limit(self, limit_pin, timeout=30):
        """리미트 스위치가 HIGH가 될 때까지 대기"""
        start_time = time.time()
        print(f"리미트 스위치 {limit_pin} 대기 중...")
        
        while GPIO.input(limit_pin) == GPIO.LOW:
            if time.time() - start_time > timeout:
                print(f"경고: 리미트 스위치 {limit_pin} 타임아웃 ({timeout}초)")
                return False
            time.sleep(0.1)
        
        print(f"리미트 스위치 {limit_pin} 감지됨")
        return True
    
    def capture_image(self, camera_index, filename):
        """카메라로 사진 촬영"""
        print(f"카메라 {camera_index} 촬영 시작...")
        
        # 카메라 열기
        cap = cv2.VideoCapture(camera_index)
        
        if not cap.isOpened():
            print(f"오류: 카메라 {camera_index}를 열 수 없습니다")
            return False
        
        # 카메라 워밍업
        time.sleep(0.5)
        
        # 프레임 읽기
        ret, frame = cap.read()
        
        if ret:
            # 이미지 저장
            cv2.imwrite(filename, frame)
            print(f"이미지 저장 완료: {filename}")
            success = True
        else:
            print(f"오류: 카메라 {camera_index}에서 프레임을 읽을 수 없습니다")
            success = False
        
        # 카메라 해제
        cap.release()
        
        return success
    
    def button1_sequence(self):
        """버튼1 누름 시 실행되는 시퀀스"""
        print("\n=== 버튼1 시퀀스 시작 ===")
        
        try:
            # 1. RELAY1 ON (LIMIT1이 HIGH가 될 때까지)
            print("1단계: RELAY1 ON")
            GPIO.output(RELAY1, GPIO.HIGH)
            
            if not self.wait_for_limit(LIMIT1):
                print("RELAY1 동작 실패")
                GPIO.output(RELAY1, GPIO.LOW)
                return
            
            GPIO.output(RELAY1, GPIO.LOW)
            print("RELAY1 OFF")
            time.sleep(0.3)
            
            # 2. LIGHT1 ON, 카메라1 촬영
            print("2단계: LIGHT1 ON, 카메라1 촬영")
            GPIO.output(LIGHT1, GPIO.HIGH)
            time.sleep(0.3)  # 조명 안정화
            
            timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
            filename1 = os.path.join(SAVE_DIR, f"camera1_{timestamp}.jpg")
            
            self.capture_image(available_cameras[0], filename1)
            
            GPIO.output(LIGHT1, GPIO.LOW)
            print("LIGHT1 OFF")
            time.sleep(0.3)
            
            # 3. LIGHT2 ON, 카메라2 촬영
            print("3단계: LIGHT2 ON, 카메라2 촬영")
            GPIO.output(LIGHT2, GPIO.HIGH)
            time.sleep(0.3)  # 조명 안정화
            
            filename2 = os.path.join(SAVE_DIR, f"camera2_{timestamp}.jpg")
            
            self.capture_image(available_cameras[1], filename2)
            
            GPIO.output(LIGHT2, GPIO.LOW)
            print("LIGHT2 OFF")
            time.sleep(0.3)
            
            # 4. RELAY2 ON (LIMIT2가 HIGH가 될 때까지)
            print("4단계: RELAY2 ON")
            GPIO.output(RELAY2, GPIO.HIGH)
            
            if not self.wait_for_limit(LIMIT2):
                print("RELAY2 동작 실패")
                GPIO.output(RELAY2, GPIO.LOW)
                return
            
            GPIO.output(RELAY2, GPIO.LOW)
            print("RELAY2 OFF")
            
            print("=== 버튼1 시퀀스 완료 ===\n")
            
        except Exception as e:
            print(f"오류 발생: {e}")
            # 안전을 위해 모든 출력 OFF
            GPIO.output(LIGHT1, GPIO.LOW)
            GPIO.output(LIGHT2, GPIO.LOW)
            GPIO.output(RELAY1, GPIO.LOW)
            GPIO.output(RELAY2, GPIO.LOW)
    
    def run(self):
        """메인 루프"""
        print("시스템 실행 중... (Ctrl+C로 종료)")
        
        try:
            while True:
                # 버튼1 감지
                if GPIO.input(BUTTON1) == GPIO.LOW:
                    print("버튼1 감지!")
                    self.button1_sequence()
                    
                    # 디바운싱
                    while GPIO.input(BUTTON1) == GPIO.LOW:
                        time.sleep(0.1)
                    time.sleep(0.2)
                
                # 버튼2, 버튼3은 필요시 추가 구현
                
                time.sleep(0.05)
                
        except KeyboardInterrupt:
            print("\n프로그램 종료")
        finally:
            self.cleanup()
    
    def cleanup(self):
        """GPIO 정리"""
        GPIO.output(LIGHT1, GPIO.LOW)
        GPIO.output(LIGHT2, GPIO.LOW)
        GPIO.output(RELAY1, GPIO.LOW)
        GPIO.output(RELAY2, GPIO.LOW)
        GPIO.cleanup()
        print("GPIO 정리 완료")

if __name__ == "__main__":
    controller = DualCameraController()
    controller.run()
