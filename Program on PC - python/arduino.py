import serial
import time
import json
import requests
from datetime import datetime,timedelta

update = {}
update['data'] = {}


ser = serial.Serial('COM4', 9600, timeout=0.5)
while 1:
	try:
		line=(ser.readline().decode("utf8"))
		if (line != ""):
			update['data']['parameters'] = []
			print(line)
			line_parsed = line.strip().split(",")
			update['flightHash'] = 'a1d0c6e83f027327d8461063f4ac58a6'

			# lt = laptop time, at = arduino time
			lt = datetime.utcnow() 
			at = datetime.strptime(line_parsed[7],"%H:%M:%S")
			at = datetime.replace(at,year=lt.year,month=lt.month,day=lt.day)

			#update['data']['timestamp'] = line_parsed[0]
			
			tmp = {}
			tmp['values'] = {}
			tmp['type'] = 'time'
			tmp['values']['timefromstart'] = line_parsed[0]
			update['data']['parameters'].append(tmp)
			
			tmp = {}
			tmp['values'] = {}
			tmp['type'] = 'time'
			if line_parsed[7]=='0:0:0':
				update['data']['timestamp'] = time.mktime(lt.timetuple())
			else:
				update['data']['timestamp'] = time.mktime(at.timetuple())
			#update['data']['timestamp'] = tmp			
			#update['data']['timestamp'] = time.mktime(lt.timeduple())
			
			if line_parsed[4] != '0.000000' and line_parsed[5] != '0.000000' and line_parsed[6] != '0.000000':
				tmp = {}
				tmp['values'] = {}
				tmp['type'] = 'position'
				tmp['values']['lat'] = line_parsed[4]
				tmp['values']['lng'] = line_parsed[5]
				tmp['values']['alt'] = line_parsed[6]
				update['data']['parameters'].append(tmp)

			tmp = {}
			tmp['values'] = {}
			tmp['type'] = 'temperature'
			tmp['values']['in'] = line_parsed[2]
			tmp['values']['out'] = line_parsed[1]
			update['data']['parameters'].append(tmp)
			tmp = {}
			tmp['values'] = {}
			tmp['type'] = 'satellite'
			tmp['values']['count'] = line_parsed[8]
			update['data']['parameters'].append(tmp)

			json_data = json.dumps(update)
			#response = requests.post("http://posttestserver.com/post.php", json=json_data, headers = {'content-type': 'application/json'})
			response = requests.post("http://dev.balon.cf/api/flight/101/telemetry", data=json_data, headers = {'Content-Type': 'application/json'})
			#print(json_data)
			#print(requests)
			print(response)
	except ser.SerialException:
		print('Data could not be read')
	time.sleep(0.1)
	
	#response = requests.post("http://posttestserver.com/post.php", json=json_data)
