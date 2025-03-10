# Heat Array

1. WIFI connect:
2. Sensor mapping: 
2.1. TCA9548 7개를 이용해서 50개의 TMP112로 부터 온도 데이터를 가져옴. 데이터 수집 주기는 300milli sec. 10개 데이터를 이동평균으로 사용.

3. Sensor upload: 2.Sensor mapping 데이터를 json 형식으로 파싱하여 서버로 전송.
4. Sdcard backup: 2.Sensor mapping 데이터를 csv 형식 데이터로 sdcard에 저장.