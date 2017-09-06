import re
import time
import json
import requests
from datetime import datetime,timedelta

update = {}
update['data'] = {}
ammount = 0

DESTINATION_URL = "https://dev.balon.cf/api/flight/161/telemetry"
LOG_FILE = "fldigi20170607.log"

while 1:
	lines = [line.rstrip('\n') for line in open(LOG_FILE)]
	#print("len(lines)=",len(lines))
	#print("ammount=",ammount)
	if ammount == len(lines):
		print("Nothing new... sleeping....zzzZZZZzzz....")
		time.sleep(1)
		continue
	print(lines[ammount:])
	for line in lines[ammount:]:
		try:
			found = re.search('^.*RTTY.*:\s([^*]*)\*(\S+)$', line).groups()
			if found == 0:
				continue
			print(found[0],"->",found[1])
			checksum = hex(found[1])
			line = found[0]
			if (line != ""):
				#line = "30,26.30,26.40,988,48.158454,17.063751,159.899993,1:17:7,4"
				line_bytes = str.encode(line)
				checksum_line = hex(crc16.crc16xmodem(line_bytes,0xFFFF))
				if (checksum_line != checksum):
					continue
				update['data']['parameters'] = []
				line_parsed = line.strip().split(",")
				try:
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
				except IndexError as e:
					print(e)
					continue
				except ValueError as e:
					print(e)
					continue

				json_data = json.dumps(update)
				#print(json_data)
				#response = requests.post("http://posttestserver.com/post.php", json=json_data, headers = {'content-type': 'application/json'})
				response = requests.post(DESTINATION_URL, data=json_data, headers = {'Content-Type': 'application/json'})
				#print(json_data)
				#print(requests)
				print(response)
				print(response.text)
			
		except AttributeError:
			found = ''
	if re.match('^.*RTTY.*:\s([^*]*)\*(\S+)$', lines[-1]):
		ammount = len(lines)
	time.sleep(1)
