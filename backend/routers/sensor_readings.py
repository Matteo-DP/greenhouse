from fastapi import APIRouter, Depends, HTTPException, status
from sqlalchemy.exc import IntegrityError
from sqlalchemy.orm import Session
from uuid import UUID
from typing import List, Union
from datetime import datetime, timedelta, timezone
from database import get_db
from models import SensorReading, Device
from schemas import SensorReading as SensorReadingSchema, SensorReadingCreate
from config import settings
from redis_client import get_json, set_json

router = APIRouter(prefix="/sensor-readings", tags=["sensor-readings"])


def _latest_sensor_key(sensor_id: UUID) -> str:
    return f"sensor:{sensor_id}:latest"

@router.get("/", response_model=List[SensorReadingSchema])
def list_sensor_readings(sensor_id: UUID = None, limit: int = 100, db: Session = Depends(get_db)):
    """Get sensor readings, optionally filtered by sensor_id"""
    query = db.query(SensorReading)
    if sensor_id:
        query = query.filter(SensorReading.sensor_id == sensor_id)
    return query.order_by(SensorReading.time.desc()).limit(limit).all()

@router.post(
    "/",
    response_model=List[SensorReadingSchema],
    status_code=status.HTTP_201_CREATED,
)
def create_sensor_readings(
    readings: Union[SensorReadingCreate, List[SensorReadingCreate]],
    db: Session = Depends(get_db),
):
    """Create one or multiple sensor readings."""

    # Normalize payload so the endpoint accepts either a single object or a list.
    if isinstance(readings, SensorReadingCreate):
        readings = [readings]

    if not readings:
        raise HTTPException(
            status_code=status.HTTP_422_UNPROCESSABLE_ENTITY,
            detail="At least one reading must be provided",
        )

    # Get all sensor_ids from input
    sensor_ids = {r.sensor_id for r in readings}

    # Fetch all devices in one query
    existing_devices = {
        d.id for d in db.query(Device.id).filter(Device.id.in_(sensor_ids)).all()
    }

    # Validate all sensors exist
    missing = sensor_ids - existing_devices
    if missing:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail=f"Sensor(s) not found: {list(missing)}",
        )

    # Increase timestamp precision when payload time is second-level and ensure
    # unique composite PK (sensor_id, time) within this request.
    db_readings: List[SensorReading] = []
    used_keys = set()
    for idx, reading in enumerate(readings):
        payload = reading.dict()
        reading_time = payload["time"]

        if reading_time.tzinfo is None:
            reading_time = reading_time.replace(tzinfo=timezone.utc)

        if reading_time.microsecond == 0:
            ingest_us = datetime.now(timezone.utc).microsecond
            reading_time = reading_time.replace(microsecond=ingest_us)

        key = (payload["sensor_id"], reading_time)
        while key in used_keys:
            reading_time = reading_time + timedelta(microseconds=1)
            key = (payload["sensor_id"], reading_time)

        used_keys.add(key)
        payload["time"] = reading_time
        db_readings.append(SensorReading(**payload))

    # Bulk insert
    try:
        db.add_all(db_readings)
        db.commit()
    except IntegrityError:
        db.rollback()
        raise HTTPException(
            status_code=status.HTTP_409_CONFLICT,
            detail="One or more readings already exist for the same sensor_id and time",
        )

    # Refresh all objects
    for r in db_readings:
        db.refresh(r)

        # Cache each reading
        cache_payload = {
            "sensor_id": str(r.sensor_id),
            "time": r.time.isoformat(),
            "value": r.value,
        }
        set_json(
            _latest_sensor_key(r.sensor_id),
            cache_payload,
            ttl_seconds=settings.redis_sensor_cache_ttl_seconds,
        )

    return db_readings

@router.get("/sensor/{sensor_id}", response_model=List[SensorReadingSchema])
def get_sensor_readings(sensor_id: UUID, limit: int = 100, db: Session = Depends(get_db)):
    """Get readings for a specific sensor"""
    # Verify sensor exists
    device = db.query(Device).filter(Device.id == sensor_id).first()
    if not device:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Sensor device not found")
    
    return db.query(SensorReading).filter(
        SensorReading.sensor_id == sensor_id
    ).order_by(SensorReading.time.desc()).limit(limit).all()


@router.get("/latest/{sensor_id}", response_model=SensorReadingSchema)
def get_latest_sensor_reading(sensor_id: UUID, db: Session = Depends(get_db)):
    """Get latest reading for a sensor (Redis cache first, DB fallback)."""
    # Verify sensor exists
    device = db.query(Device).filter(Device.id == sensor_id).first()
    if not device:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Sensor device not found")

    cached = get_json(_latest_sensor_key(sensor_id))
    if cached is not None:
        return SensorReadingSchema(
            sensor_id=UUID(cached["sensor_id"]),
            time=datetime.fromisoformat(cached["time"]),
            value=float(cached["value"]),
        )

    latest = db.query(SensorReading).filter(
        SensorReading.sensor_id == sensor_id
    ).order_by(SensorReading.time.desc()).first()

    if not latest:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="No readings found for this sensor")

    cache_payload = {
        "sensor_id": str(latest.sensor_id),
        "time": latest.time.isoformat(),
        "value": latest.value,
    }
    set_json(
        _latest_sensor_key(latest.sensor_id),
        cache_payload,
        ttl_seconds=settings.redis_sensor_cache_ttl_seconds,
    )

    return latest
