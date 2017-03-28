import serial
import time
import json
import requests

update = {}
update['data'] = {}


ser = serial.Serial('COM4', 9600, timeout=0.5)
while 1:
	try:
		line=(ser.readline().decode("utf8"))
		if (line != ""):
			update['data']['parameters'] = []
			print(line)
			line_parsed = line.split(",")
			update['flightHash'] = 'a1d0c6e83f027327d8461063f4ac58a6'
			update['data']['timestamp'] = line_parsed[0]
			tmp = {}
			tmp['values'] = {}
			tmp['type'] = 'position'
			tmp['values']['lat'] = line_parsed[4]
			tmp['values']['lng'] = line_parsed[5]
			tmp['values']['alt'] = line_parsed[6]
			#tmp['timestamp'] = line_parsed[7]
			update['data']['parameters'].append(tmp)
			tmp = {}
			tmp['values'] = {}
			tmp['type'] = 'temperature'
			tmp['values']['in'] = line_parsed[2]
			tmp['values']['out'] = line_parsed[1]
			update['data']['parameters'].append(tmp)
			json_data = json.dumps(update)
			#response = requests.post("http://posttestserver.com/post.php", json=json_data, headers = {'content-type': 'application/json'})
			response = requests.post("http://live.balon.cf/api/flight/42/telemetry", data=json_data, headers = {'Content-Type': 'application/json'})
			#print(json_data)
			#print(requests)
			print(response)
	except ser.SerialException:
		print('Data could not be read')
	time.sleep(0.1)
	
	#response = requests.post("http://posttestserver.com/post.php", json=json_data)