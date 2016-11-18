## Telemetry Update
Updates balloon position, altitude, temperature and other parameters  

JSON Example: balloonUpdates.json  
**POST to URL:** *`/api/flight/<int:flight_number>/telemetry`*

## Special Events
Updates special events of balloon flight: Burst, Landing, Liftoff...  

JSON Example: specialEvent.json  
**POST to URL:** *`/api/flight/<int:flight_number>/event/<string:event>`*
