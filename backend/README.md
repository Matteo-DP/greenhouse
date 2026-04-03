# Greenhouse Management API

A REST API for managing greenhouse sensors, devices, and automated schedules. Built with FastAPI, PostgreSQL, TimescaleDB, and Redis.


### Installation

1. Install dependencies:
```bash
pip install -r requirements.txt
```

2. Set up environment variables:
```bash
cp .env.example .env
```

3. Start the database with Docker Compose:
```bash
docker-compose up -d
```

4. Run the API server:
```bash
python -m uvicorn main:app --reload
```

### API Documentation

Once the server is running, visit:
- **Swagger UI**: http://localhost:8000/docs
- **ReDoc**: http://localhost:8000/redoc

## API Endpoints

### Devices
- `GET /devices` - List all devices
- `POST /devices` - Create a new device
- `GET /devices/{device_id}` - Get a device
- `PUT /devices/{device_id}` - Update a device
- `DELETE /devices/{device_id}` - Delete a device

### Sensor Readings
- `GET /sensor-readings` - List sensor readings
- `POST /sensor-readings` - Create a sensor reading
- `GET /sensor-readings/sensor/{sensor_id}` - Get readings for a sensor
- `GET /sensor-readings/latest/{sensor_id}` - Get latest reading (Redis cache first)

### Light Schedules
- `GET /light-schedules` - List all schedules
- `POST /light-schedules` - Create a schedule
- `GET /light-schedules/{schedule_id}` - Get a schedule
- `PUT /light-schedules/{schedule_id}` - Update a schedule
- `DELETE /light-schedules/{schedule_id}` - Delete a schedule
- `GET /light-schedules/{schedule_id}/dates` - Get schedule dates
- `POST /light-schedules/{schedule_id}/dates` - Add a date to schedule
- `DELETE /light-schedules/{schedule_id}/dates/{start_date}` - Remove a date

### Watering Schedules
- `GET /watering-schedules` - List all schedules
- `POST /watering-schedules` - Create a schedule
- `GET /watering-schedules/{schedule_id}` - Get a schedule
- `PUT /watering-schedules/{schedule_id}` - Update a schedule
- `DELETE /watering-schedules/{schedule_id}` - Delete a schedule
- `GET /watering-schedules/{schedule_id}/dates` - Get schedule dates
- `POST /watering-schedules/{schedule_id}/dates` - Add a date to schedule
- `DELETE /watering-schedules/{schedule_id}/dates/{start_date}` - Remove a date

### Logs
- `GET /logs` - List logs (with optional filtering)
- `POST /logs?device_id={id}` - Create a log entry
- `GET /logs/{log_id}` - Get a log entry
- `GET /logs/device/{device_id}` - Get logs for a device

### Alerts
- `GET /alerts` - List alerts (with optional filtering)
- `POST /alerts?device_id={id}` - Create an alert
- `GET /alerts/{alert_id}` - Get an alert
- `PUT /alerts/{alert_id}` - Update an alert (e.g., mark as resolved)
- `DELETE /alerts/{alert_id}` - Delete an alert
- `GET /alerts/device/{device_id}` - Get alerts for a device

### Health Check
- `GET /health` - API health status

## Example Usage

### Create a Device
```bash
curl -X POST http://localhost:8000/devices \
  -H "Content-Type: application/json" \
  -d '{
    "name": "Temperature Sensor 1",
    "description": "Main greenhouse temp sensor",
    "device_type": "SENSOR",
    "location": "Greenhouse A"
  }'
```

### Record Sensor Reading
```bash
curl -X POST http://localhost:8000/sensor-readings \
  -H "Content-Type: application/json" \
  -d '{
    "sensor_id": "uuid-here",
    "time": "2026-04-03T15:30:00Z",
    "value": 24.5
  }'
```

### Create Light Schedule
```bash
curl -X POST http://localhost:8000/light-schedules \
  -H "Content-Type: application/json" \
  -d '{
    "name": "Morning Lights",
    "light_on_start": "06:00:00",
    "light_on_end": "18:00:00",
    "brightness_level": 100
  }'
```

## Database Schema

The API uses the following main tables:
- `devices` - Device configurations
- `sensor_readings` - Time-series sensor data (TimescaleDB hypertable)
- `light_schedules` - Light automation schedules
- `watering_schedules` - Watering automation schedules
- `logs` - System and device logs
- `alerts` - System alerts and notifications

## Development

### Running Tests
```bash
pytest tests/
```

Be careful not to run the integration tests on your production stack by running ´pytest` without arguments. This is probably not what you want to do.
  
### Integration tests

This runs PostgreSQL + Redis + API + integration tests in one stack.

```bash
docker compose -f docker-compose.test.yml up --build --abort-on-container-exit --exit-code-from tests
```

Cleanup after run:

```bash
docker compose -f docker-compose.test.yml down -v
```

### Database Migrations
```bash
alembic upgrade head
```

## Environment Variables

See `.env.example` for all available configuration options.


## Redis Integration

Redis is used for low-latency caching of each sensor's latest reading. Not really worth tbh but fuck it man Redis for the love of the game.

- Cache key format: `sensor:{sensor_id}:latest`
- Cache is updated on `POST /sensor-readings`
- `GET /sensor-readings/latest/{sensor_id}` reads Redis first, then falls back to PostgreSQL/TimescaleDB
  
<br/>
<br/>
<br/>
<br/>


*4/20*