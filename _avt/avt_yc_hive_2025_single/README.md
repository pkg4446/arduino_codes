# 영천 스마트벌통 펌웨어

1. Serial commend

Serial 접속 상태에서 엔터키를 누르면, Help 메뉴가 나옴.
Help 메뉴에 대한 설명.

1.01. reboot: 보드 재부팅다. 보드에 부착된 리셋 버튼을 누른것과 동일함.
1.02. temp: sht3x의 온,습도 표시.

1.03. run: run on 을 입력할 경우 자동 가온기능이 켜지며, set 명령어로 설정한 온도까지 가온을 함. 기본값은 on
run on 이 아닌, 
run,
run off 등 명령어를 입력할 경우 자동 가온기능이 꺼짐.

1.04. set: set <온도> 를 입력할 경우 해당 번호의 벌통 가온 목표온도를 수정 할 수 있음. 기본값은 3도
ex> set 25: 벌통 목표온도 25도로 설정. 

1.05. config: 벌통의 설정(업로드 주기, 가온설정(ON&OFF,목표온도)) 표시.
1.06. gap: 센서 업로드 주기 변경<단위 분>. ex>gap 5

1.07. test: test mode 를 입력할 경우 메뉴얼 모드로 변경되며,
메뉴얼 모드에서 test on, test off 명령어로 MOSFET을 수동으로 켜고 끌 수 있음. 
주의사항: 메뉴얼 모드로 변경할 경우 에서는 자동 가온 기능이 작동하지 않음. 메뉴얼 모드를 종료하는 방법은 test 만 입력하거나, 재부팅을 하면 됨. (test data 추가: 데이터 서버로 전송)

1.08. ssid: ssid 를 입력함. 2G만 연결가능. ex> ssid avatar2G
1.09. pass: password 를 입력함. ex> pass q1w2e3r4

1.10. wifi: wifi 만 입력할 경우 설정된 2.09, 2.10 에서 설정한 ssid와 password로 wifi에 접속을 시도함.
wifi scan 을 입력할 경우 주변에 있는 ssid를 검색함.
wifi stop을 입력할 경우 wifi 접속을 종료함.

#update
2025.11.04: 최초 코드 작성
2025.11.04: 최초 코드 완성