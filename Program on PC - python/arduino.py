import serial
import time
import json
import requests

update = {}
update['data'] = {}


ser = serial.Serial('COM5', 9600, timeout=0.1)
while 1:
	try:
		line=(ser.readline().decode("utf8"))
		if (line != ""):
			update['data']['parameters'] = []
			print(line)
			line_parsed = line.split(",")
			update['flightHash'] = '0cc175b9c0f1b6a831c399e269772661'
			update['data']['time'] = line_parsed[0]
			test1 = {}
			test1['values'] = {}
			test1['type'] = 'position'
			test1['values']['lat'] = line_parsed[4]
			test1['values']['lng'] = line_parsed[5]
			test1['values']['alt'] = line_parsed[6]
			test1['timestamp'] = line_parsed[0]
			update['data']['parameters'].append(test1)
			test2 = {}
			test2['values'] = {}
			test2['type'] = 'temperature'
			test2['values']['in'] = line_parsed[2]
			test2['values']['out'] = line_parsed[1]
			update['data']['parameters'].append(test2)
			json_data = json.dumps(update)
			print(json_data)
			response = requests.post("http://posttestserver.com/post.php", json=json_data)
	except ser.SerialTimeoutException:
		print('Data could not be read')
	time.sleep(0.1)