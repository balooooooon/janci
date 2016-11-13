import serial
import time
import json
import requests

update = {}
update['data'] = {}
update['data']['location'] = {}
update['data']['temperature'] = {}

ser = serial.Serial('COM5', 9600, timeout=0)
while 1:
	try:
		line=(ser.readline().decode("utf8"))
		if (line != ""):
			print(line)
			line_parsed = line.split(",")
			update['type'] = "updateMsg"
			update['data']['time'] = line_parsed[0]
			update['data']['location']['x'] = line_parsed[4]
			update['data']['location']['y'] = line_parsed[5]
			update['data']['location']['z'] = line_parsed[6]
			update['data']['temperature']['in'] = line_parsed[3]
			update['data']['temperature']['out'] = line_parsed[2]
			update['data']['isburst'] = line_parsed[7]
			json_data = json.dumps(update)
			response = requests.post("http://posttestserver.com/post.php", json=json_data)
	except ser.SerialTimeoutException:
		print('Data could not be read')
	time.sleep(0.1)