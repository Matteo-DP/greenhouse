from __future__ import annotations

import time
from uuid import uuid4

import requests

BASE_URL = "http://api:8000"
TIMEOUT = 10


def _wait_for_api() -> None:
    deadline = time.time() + 60
    while time.time() < deadline:
        try:
            response = requests.get(f"{BASE_URL}/health", timeout=2)
            if response.status_code == 200:
                return
        except requests.RequestException:
            pass
        time.sleep(1)
    raise RuntimeError("API did not become ready in time")


def test_full_stack_health() -> None:
    _wait_for_api()
    response = requests.get(f"{BASE_URL}/health", timeout=TIMEOUT)
    assert response.status_code == 200
    payload = response.json()
    assert payload["status"] in {"healthy", "degraded"}
    assert payload["redis"] in {"up", "down"}


def test_full_stack_devices_crud() -> None:
    device_name = f"sensor-{uuid4()}"

    create = requests.post(
        f"{BASE_URL}/devices/",
        json={
            "name": device_name,
            "description": "integration device",
            "device_type": "SENSOR",
            "location": "integration-zone",
        },
        timeout=TIMEOUT,
    )
    assert create.status_code == 201
    created = create.json()
    device_id = created["id"]

    listed = requests.get(f"{BASE_URL}/devices/", timeout=TIMEOUT)
    assert listed.status_code == 200
    assert any(d["id"] == device_id for d in listed.json())

    fetched = requests.get(f"{BASE_URL}/devices/{device_id}", timeout=TIMEOUT)
    assert fetched.status_code == 200
    assert fetched.json()["name"] == device_name

    updated = requests.put(
        f"{BASE_URL}/devices/{device_id}",
        json={"name": f"{device_name}-updated"},
        timeout=TIMEOUT,
    )
    assert updated.status_code == 200
    assert updated.json()["name"].endswith("-updated")

    deleted = requests.delete(f"{BASE_URL}/devices/{device_id}", timeout=TIMEOUT)
    assert deleted.status_code == 204


def test_full_stack_sensor_readings_crud_like_flow() -> None:
    sensor = requests.post(
        f"{BASE_URL}/devices/",
        json={
            "name": f"humidity-{uuid4()}",
            "description": "sensor for readings",
            "device_type": "SENSOR",
            "location": "greenhouse",
        },
        timeout=TIMEOUT,
    )
    assert sensor.status_code == 201
    sensor_id = sensor.json()["id"]

    created = requests.post(
        f"{BASE_URL}/sensor-readings/",
        json={
            "sensor_id": sensor_id,
            "time": "2026-04-04T12:00:00Z",
            "value": 55.5,
        },
        timeout=TIMEOUT,
    )
    assert created.status_code == 201

    listed = requests.get(f"{BASE_URL}/sensor-readings/?sensor_id={sensor_id}&limit=10", timeout=TIMEOUT)
    assert listed.status_code == 200
    assert len(listed.json()) >= 1

    latest = requests.get(f"{BASE_URL}/sensor-readings/latest/{sensor_id}", timeout=TIMEOUT)
    assert latest.status_code == 200
    assert float(latest.json()["value"]) == 55.5


def test_full_stack_light_schedules_crud_and_dates() -> None:
    created = requests.post(
        f"{BASE_URL}/light-schedules/",
        json={
            "name": f"lights-{uuid4()}",
            "description": "integration light schedule",
            "light_on_start": "07:00:00",
            "light_on_end": "11:00:00",
            "brightness_level": 70,
        },
        timeout=TIMEOUT,
    )
    assert created.status_code == 201
    schedule_id = created.json()["id"]

    fetched = requests.get(f"{BASE_URL}/light-schedules/{schedule_id}", timeout=TIMEOUT)
    assert fetched.status_code == 200

    updated = requests.put(
        f"{BASE_URL}/light-schedules/{schedule_id}",
        json={"brightness_level": 60},
        timeout=TIMEOUT,
    )
    assert updated.status_code == 200
    assert updated.json()["brightness_level"] == 60

    add_date = requests.post(
        f"{BASE_URL}/light-schedules/{schedule_id}/dates",
        json={"start_date": "2026-04-06", "enabled": True},
        timeout=TIMEOUT,
    )
    assert add_date.status_code == 201

    list_dates = requests.get(f"{BASE_URL}/light-schedules/{schedule_id}/dates", timeout=TIMEOUT)
    assert list_dates.status_code == 200
    assert any(d["start_date"] == "2026-04-06" for d in list_dates.json())

    remove_date = requests.delete(f"{BASE_URL}/light-schedules/{schedule_id}/dates/2026-04-06", timeout=TIMEOUT)
    assert remove_date.status_code == 204

    delete_schedule = requests.delete(f"{BASE_URL}/light-schedules/{schedule_id}", timeout=TIMEOUT)
    assert delete_schedule.status_code == 204


def test_full_stack_watering_schedules_crud_and_dates() -> None:
    created = requests.post(
        f"{BASE_URL}/watering-schedules/",
        json={
            "name": f"watering-{uuid4()}",
            "duration_seconds": 30,
            "moisture_threshold_max": 80,
            "moisture_threshold_auto": 35,
            "moisture_auto_enabled": True,
            "frequency_hours": 24,
        },
        timeout=TIMEOUT,
    )
    assert created.status_code == 201
    schedule_id = created.json()["id"]

    fetched = requests.get(f"{BASE_URL}/watering-schedules/{schedule_id}", timeout=TIMEOUT)
    assert fetched.status_code == 200

    updated = requests.put(
        f"{BASE_URL}/watering-schedules/{schedule_id}",
        json={"frequency_hours": 12},
        timeout=TIMEOUT,
    )
    assert updated.status_code == 200
    assert updated.json()["frequency_hours"] == 12

    add_date = requests.post(
        f"{BASE_URL}/watering-schedules/{schedule_id}/dates",
        json={"start_date": "2026-04-06", "enabled": True},
        timeout=TIMEOUT,
    )
    assert add_date.status_code == 201

    list_dates = requests.get(f"{BASE_URL}/watering-schedules/{schedule_id}/dates", timeout=TIMEOUT)
    assert list_dates.status_code == 200
    assert any(d["start_date"] == "2026-04-06" for d in list_dates.json())

    remove_date = requests.delete(f"{BASE_URL}/watering-schedules/{schedule_id}/dates/2026-04-06", timeout=TIMEOUT)
    assert remove_date.status_code == 204

    delete_schedule = requests.delete(f"{BASE_URL}/watering-schedules/{schedule_id}", timeout=TIMEOUT)
    assert delete_schedule.status_code == 204


def test_full_stack_logs_endpoints() -> None:
    created = requests.post(
        f"{BASE_URL}/logs/",
        json=[
            {"log_level": "INFO", "message": "pump started", "timestamp": "2026-04-20T18:21:35Z"},
            {"log_level": "ERROR", "message": "pump stalled", "timestamp": "2026-04-20T18:21:36Z"},
        ],
        timeout=TIMEOUT,
    )
    assert created.status_code == 201
    created_logs = created.json()
    assert len(created_logs) == 2
    log_id = created_logs[0]["id"]

    listed = requests.get(f"{BASE_URL}/logs/?log_level=INFO", timeout=TIMEOUT)
    assert listed.status_code == 200
    assert any(l["id"] == log_id for l in listed.json())

    fetched = requests.get(f"{BASE_URL}/logs/{log_id}", timeout=TIMEOUT)
    assert fetched.status_code == 200


def test_full_stack_alerts_crud() -> None:
    device = requests.post(
        f"{BASE_URL}/devices/",
        json={
            "name": f"fan-{uuid4()}",
            "description": "fan device",
            "device_type": "FAN",
            "location": "zone-2",
        },
        timeout=TIMEOUT,
    )
    assert device.status_code == 201
    device_id = device.json()["id"]

    created = requests.post(
        f"{BASE_URL}/alerts/?device_id={device_id}",
        json={
            "alert_type": "TEMPERATURE",
            "severity": "WARNING",
            "message": "high temp",
        },
        timeout=TIMEOUT,
    )
    assert created.status_code == 201
    alert_id = created.json()["id"]

    listed = requests.get(f"{BASE_URL}/alerts/?device_id={device_id}&resolved=false&severity=WARNING", timeout=TIMEOUT)
    assert listed.status_code == 200
    assert any(a["id"] == alert_id for a in listed.json())

    fetched = requests.get(f"{BASE_URL}/alerts/{alert_id}", timeout=TIMEOUT)
    assert fetched.status_code == 200

    updated = requests.put(
        f"{BASE_URL}/alerts/{alert_id}",
        json={"resolved": True, "message": "resolved"},
        timeout=TIMEOUT,
    )
    assert updated.status_code == 200
    assert updated.json()["resolved"] is True

    by_device = requests.get(f"{BASE_URL}/alerts/device/{device_id}", timeout=TIMEOUT)
    assert by_device.status_code == 200
    assert any(a["id"] == alert_id for a in by_device.json())

    deleted = requests.delete(f"{BASE_URL}/alerts/{alert_id}", timeout=TIMEOUT)
    assert deleted.status_code == 204
