# 영천 스마트벌통 펌웨어

1. Built in led
1~3번
1번. 와이파이 연결.
2번. 밸브A.
3번. 밸브B.

2. Serial commend

Serial 접속 상태에서 엔터키를 누르면, Help 메뉴가 나옴.
Help 메뉴에 대한 설명.

2.01. reboot: 보드 재부팅다. 보드에 부착된 리셋 버튼을 누른것과 동일함.
2.02. gap: 업로드 주기 변경<단위 분>. ex>gap 5
2.03. time: DS3231 rtc모듈의 시간을 표시.

2.04. sensor: sht3x의 온습도, 수위 표시.

2.05. valve a,b <켜는시간> <닫는시간>: 
      ex> valve a 09 13 => 밸브 A 오전 9시 열고 오후 1시 닫기.
      ex> valve b 18 21 => 밸브 A 오후 6시 열고 오후 9시 닫기.

2.06. heat <켜는시간> <닫는시간>:
      ex> heat 20 08 => 히터 오후 8시 켜고 오전 8시 끄기.

2.07. test: test mode 를 입력할 경우 메뉴얼 모드로 변경되며,
메뉴얼 모드에서 test on, test off 명령어로 MOSFET을 수동으로 켜고 끌 수 있음. 
주의사항: 메뉴얼 모드로 변경할 경우 에서는 자동 가온 기능이 작동하지 않음. 메뉴얼 모드를 종료하는 방법은 test 만 입력하거나, 재부팅을 하면 됨. (test data 추가: 데이터 서버로 전송)

2.08. ssid: ssid 를 입력함. 2G만 연결가능. ex> ssid avatar2G
2.09. pass: password 를 입력함. ex> pass q1w2e3r4

2.10. wifi: wifi 만 입력할 경우 설정된 2.09, 2.10 에서 설정한 ssid와 password로 wifi에 접속을 시도함.
wifi scan 을 입력할 경우 주변에 있는 ssid를 검색함.
wifi stop을 입력할 경우 wifi 접속을 종료함.

#update
2025.03.10: 기본 코드 구성
2025.03.27: 초음파센서 추가
2025.03.28: 기능구현 완료.