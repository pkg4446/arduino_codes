# ZE03_sensor loger

command
1. reboot : 재부팅.
2. show :   ZE03 센서 데이터값 보기
3. post :   서버로 ZE03 센서 데이터값 전송

4. set:     기기 설정
    4.1. interval : 서버로 데이터값 전송하는 주기 설정(단위: 분) ex> set interval 5 (5분에 한번씩 데이터 전송)
    4.2. farm : 농장 이름 설정 ex> set farm 2113003035  (농장 이름을 2113003035 로 변경)
    4.3. sens : 기기 이름 설정 ex> set sens 01-NH3 (기기 이름을 01-NH3 으로 변경)

5. config : set 명령어에서 설정한 값 보기
6. ssid: ssid 를 입력함. 2G만 연결가능. ex) ssid freeWIFI2G
7. pass: password 를 입력함. ex) pass freePass

8. wifi: 설정된 2.09, 2.10 에서 설정한 ssid와 password로 wifi에 접속을 시도함. ex> wifi
    8.1. scan: 을 입력할 경우 주변에 있는 ssid를 검색함. ex> wifi scan
    8.2. stop: 을 입력할 경우 wifi 접속을 종료함. ex> wifi stop