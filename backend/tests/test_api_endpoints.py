from datetime import date, datetime


def _create_device(client, name: str = "Temp Sensor", device_type: str = "SENSOR"):
    response = client.post(
        "/devices/",
        json={
            "name": name,
            "description": "test device",
            "device_type": device_type,
            "location": "Zone A",
        },
    )
    assert response.status_code == 201
    return response.json()


def test_health_endpoint(client):
    response = client.get("/health")
    assert response.status_code == 200
    payload = response.json()
    assert payload["status"] in {"healthy", "degraded"}
    assert payload["redis"] in {"up", "down"}


def test_devices_crud(client):
    created = _create_device(client)
    device_id = created["id"]

    listed = client.get("/devices/")
    assert listed.status_code == 200
    assert any(d["id"] == device_id for d in listed.json())

    fetched = client.get(f"/devices/{device_id}")
    assert fetched.status_code == 200
    assert fetched.json()["name"] == "Temp Sensor"

    updated = client.put(f"/devices/{device_id}", json={"name": "Updated Sensor", "location": "Zone B"})
    assert updated.status_code == 200
    assert updated.json()["name"] == "Updated Sensor"
    assert updated.json()["location"] == "Zone B"

    deleted = client.delete(f"/devices/{device_id}")
    assert deleted.status_code == 204

    missing = client.get(f"/devices/{device_id}")
    assert missing.status_code == 404


def test_sensor_readings_endpoints(client, fake_sensor_cache):
    device = _create_device(client, name="Humidity Sensor", device_type="SENSOR")
    sensor_id = device["id"]

    create_response = client.post(
        "/sensor-readings/",
        json={
            "sensor_id": sensor_id,
            "time": "2026-04-04T10:00:00Z",
            "value": 42.5,
        },
    )
    assert create_response.status_code == 201

    all_readings = client.get("/sensor-readings/")
    assert all_readings.status_code == 200
    assert len(all_readings.json()) == 1

    filtered_readings = client.get(f"/sensor-readings/?sensor_id={sensor_id}&limit=10")
    assert filtered_readings.status_code == 200
    assert filtered_readings.json()[0]["sensor_id"] == sensor_id

    sensor_readings = client.get(f"/sensor-readings/sensor/{sensor_id}")
    assert sensor_readings.status_code == 200
    assert len(sensor_readings.json()) == 1

    latest = client.get(f"/sensor-readings/latest/{sensor_id}")
    assert latest.status_code == 200
    assert latest.json()["value"] == 42.5

    cache_key = f"sensor:{sensor_id}:latest"
    assert cache_key in fake_sensor_cache


def test_light_schedules_crud_and_dates_endpoints(client):
    created = client.post(
        "/light-schedules/",
        json={
            "name": "Morning",
            "description": "Morning light schedule",
            "light_on_start": "06:00:00",
            "light_on_end": "12:00:00",
            "brightness_level": 80,
        },
    )
    assert created.status_code == 201
    schedule_id = created.json()["id"]

    listed = client.get("/light-schedules/")
    assert listed.status_code == 200
    assert any(s["id"] == schedule_id for s in listed.json())

    fetched = client.get(f"/light-schedules/{schedule_id}")
    assert fetched.status_code == 200

    updated = client.put(f"/light-schedules/{schedule_id}", json={"brightness_level": 65})
    assert updated.status_code == 200
    assert updated.json()["brightness_level"] == 65

    add_date = client.post(
        f"/light-schedules/{schedule_id}/dates",
        json={"start_date": "2026-04-05", "enabled": True},
    )
    assert add_date.status_code == 201

    dates = client.get(f"/light-schedules/{schedule_id}/dates")
    assert dates.status_code == 200
    assert dates.json()[0]["start_date"] == "2026-04-05"

    remove_date = client.delete(f"/light-schedules/{schedule_id}/dates/2026-04-05")
    assert remove_date.status_code == 204

    deleted = client.delete(f"/light-schedules/{schedule_id}")
    assert deleted.status_code == 204


def test_watering_schedules_crud_and_dates_endpoints(client):
    created = client.post(
        "/watering-schedules/",
        json={
            "name": "Daily Watering",
            "duration_seconds": 30,
            "moisture_threshold_max": 75.0,
            "moisture_threshold_auto": 30.0,
            "moisture_auto_enabled": True,
            "frequency_hours": 24,
        },
    )
    assert created.status_code == 201
    schedule_id = created.json()["id"]

    listed = client.get("/watering-schedules/")
    assert listed.status_code == 200
    assert any(s["id"] == schedule_id for s in listed.json())

    fetched = client.get(f"/watering-schedules/{schedule_id}")
    assert fetched.status_code == 200

    updated = client.put(f"/watering-schedules/{schedule_id}", json={"frequency_hours": 12})
    assert updated.status_code == 200
    assert updated.json()["frequency_hours"] == 12

    add_date = client.post(
        f"/watering-schedules/{schedule_id}/dates",
        json={"start_date": "2026-04-05", "enabled": True},
    )
    assert add_date.status_code == 201

    dates = client.get(f"/watering-schedules/{schedule_id}/dates")
    assert dates.status_code == 200
    assert dates.json()[0]["start_date"] == "2026-04-05"

    remove_date = client.delete(f"/watering-schedules/{schedule_id}/dates/2026-04-05")
    assert remove_date.status_code == 204

    deleted = client.delete(f"/watering-schedules/{schedule_id}")
    assert deleted.status_code == 204


def test_logs_endpoints(client):
    device = _create_device(client, name="Pump 1", device_type="PUMP")
    device_id = device["id"]

    created = client.post(
        f"/logs/?device_id={device_id}",
        json={
            "log_level": "INFO",
            "message": "Pump started",
        },
    )
    assert created.status_code == 201
    log_id = created.json()["id"]

    listed = client.get("/logs/")
    assert listed.status_code == 200
    assert any(l["id"] == log_id for l in listed.json())

    filtered = client.get(f"/logs/?device_id={device_id}&log_level=INFO")
    assert filtered.status_code == 200
    assert len(filtered.json()) == 1

    fetched = client.get(f"/logs/{log_id}")
    assert fetched.status_code == 200
    assert fetched.json()["message"] == "Pump started"

    by_device = client.get(f"/logs/device/{device_id}")
    assert by_device.status_code == 200
    assert len(by_device.json()) == 1


def test_alerts_crud_endpoints(client):
    device = _create_device(client, name="Fan 1", device_type="FAN")
    device_id = device["id"]

    created = client.post(
        f"/alerts/?device_id={device_id}",
        json={
            "alert_type": "TEMPERATURE",
            "severity": "WARNING",
            "message": "Temperature too high",
        },
    )
    assert created.status_code == 201
    alert_id = created.json()["id"]

    listed = client.get("/alerts/")
    assert listed.status_code == 200
    assert any(a["id"] == alert_id for a in listed.json())

    filtered = client.get(f"/alerts/?device_id={device_id}&resolved=false&severity=WARNING")
    assert filtered.status_code == 200
    assert len(filtered.json()) == 1

    fetched = client.get(f"/alerts/{alert_id}")
    assert fetched.status_code == 200

    updated = client.put(f"/alerts/{alert_id}", json={"resolved": True, "message": "Resolved"})
    assert updated.status_code == 200
    assert updated.json()["resolved"] is True
    assert updated.json()["resolved_at"] is not None

    by_device = client.get(f"/alerts/device/{device_id}")
    assert by_device.status_code == 200
    assert len(by_device.json()) == 1

    deleted = client.delete(f"/alerts/{alert_id}")
    assert deleted.status_code == 204

    missing = client.get(f"/alerts/{alert_id}")
    assert missing.status_code == 404
