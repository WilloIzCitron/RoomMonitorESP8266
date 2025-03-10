from time import sleep
from datetime import datetime
import json
import requests
import psycopg2


try:
    conn = psycopg2.connect(database="roomMonitor2", host="127.0.0.1", user="admin", port="5432", sslmode="prefer")
except:
    print("cant connect to database")
cursor = conn.cursor()
print("JSON to PostgreSQL. if you want to terminate it, press Ctrl+\\ or in Linux, run killall python on secondary terminal.")

#Make this commented if was exist, im tired to make it if statement
# Grafana only read 50 rows
# cursor.execute("CREATE SEQUENCE ids MINVALUE 1 MAXVALUE 50 CYCLE")

# cursor.execute("""
# CREATE TABLE room5 (
#     room_id integer PRIMARY KEY DEFAULT nextval('ids'), 
#     date TIMESTAMP NOT NULL, 
#     temp FLOAT(32) NOT NULL, 
#     humi FLOAT(32) NOT NULL, 
#     airQuality INT NOT NULL, 
#     airQualityStatus VARCHAR(120) NOT NULL
# );
# """)

#keepalive ahh
def publishData():
    room_id = None
    try:
        req = requests.get("http://192.168.18.20/api")

        jsonData = json.loads(req.content)
        temp = jsonData["temp"]
        humi = jsonData["humi"]
        airQuality = jsonData["airQuality"]
        airQualityStatus = jsonData["airQualityStatus"]
        now = datetime.now()
        cursor.execute("INSERT INTO room5(date, temp, humi, airQuality, airQualityStatus) VALUES(CURRENT_TIMESTAMP(1), "+temp+", "+humi+", "+airQuality+", \'"+airQualityStatus+"\') ON CONFLICT(room_id) DO UPDATE SET date = EXCLUDED.date, temp = EXCLUDED.temp, humi = EXCLUDED.humi, airQuality = EXCLUDED.airQuality, airQualityStatus = EXCLUDED.airQualityStatus RETURNING *;")
        rows = cursor.fetchone()
        if rows:
             room_id = rows[0]
        print("data added!")
        conn.commit()
        sleep(1)
    except (Exception, psycopg2.DatabaseError) as error:
        print(error)
    finally:
        return room_id

#keepalive ahh
while True:
    publishData()
