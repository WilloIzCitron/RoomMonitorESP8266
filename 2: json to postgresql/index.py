from time import sleep
from datetime import datetime
import json
import requests
import psycopg2
import asyncio


try:
    conn = psycopg2.connect(database="roomMonitor2", host="127.0.0.1", user="admin", port="5432", sslmode="prefer")
except:
    print("cant connect to database")
cursor = conn.cursor()
print("JSON to PostgreSQL. if you want to terminate it, press Ctrl+\\ on main console or in Linux, run killall python on secondary terminal.")

#Make this commented if was exist, im tired to make it if statement
#Grafana only read 50 rows, 1 row is used for live fetch, no logging
# cursor.execute("CREATE SEQUENCE ids MINVALUE 1 MAXVALUE 5 CYCLE")

# cursor.execute("""
# CREATE TABLE continuosRoomData (
#     room_id integer PRIMARY KEY DEFAULT nextval('ids'), 
#     date TIMESTAMP WITH TIME ZONE NOT NULL, 
#     temp FLOAT(32) NOT NULL, 
#     humi FLOAT(32) NOT NULL, 
#     airQuality INT NOT NULL, 
#     airQualityStatus VARCHAR(120) NOT NULL
# );
# """)

# Chronical Data Table
# cursor.execute("""
# CREATE TABLE chronicalRoomData (
#     room_id SERIAL PRIMARY KEY, 
#     date TIMESTAMP WITH TIME ZONE NOT NULL, 
#     temp FLOAT(32) NOT NULL, 
#     humi FLOAT(32) NOT NULL, 
#     airQuality INT NOT NULL, 
#     airQualityStatus VARCHAR(120) NOT NULL
# );
# """)


# Set Time Zone
cursor.execute("SET TIME ZONE '+7';")

#keepalive ahh
async def publishContinuousData():
        req = requests.get("http://192.168.18.20/api")

        jsonData = json.loads(req.content)
        temp = jsonData["temp"]
        humi = jsonData["humi"]
        airQuality = jsonData["airQuality"]
        airQualityStatus = jsonData["airQualityStatus"]
        cursor.execute("INSERT INTO continuosRoomData(date, temp, humi, airQuality, airQualityStatus) VALUES(NOW(), "+temp+", "+humi+", "+airQuality+", \'"+airQualityStatus+"\') ON CONFLICT(room_id) DO UPDATE SET date = EXCLUDED.date, temp = EXCLUDED.temp, humi = EXCLUDED.humi, airQuality = EXCLUDED.airQuality, airQualityStatus = EXCLUDED.airQualityStatus RETURNING *;")
        # print("data added!")
        conn.commit()
        await asyncio.sleep(1)

async def publishChronicalData():
        req = requests.get("http://192.168.18.20/api")

        jsonData = json.loads(req.content)
        temp = jsonData["temp"]
        humi = jsonData["humi"]
        airQuality = jsonData["airQuality"]
        airQualityStatus = jsonData["airQualityStatus"]
        cursor.execute("INSERT INTO chronicalRoomData(date, temp, humi, airQuality, airQualityStatus) VALUES(NOW(), "+temp+", "+humi+", "+airQuality+", \'"+airQualityStatus+"\') RETURNING *;")
        # print("data added!")
        conn.commit()
        await asyncio.sleep(1)

async def main():
    await asyncio.gather(publishChronicalData(), publishContinuousData())

#keepalive ahh
while True:
    asyncio.run(main())
