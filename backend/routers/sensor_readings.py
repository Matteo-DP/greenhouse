from fastapi import APIRouter, Depends, HTTPException, status
from sqlalchemy.orm import Session
from uuid import UUID
from typing import List
from datetime import datetime
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

@router.post("/", response_model=SensorReadingSchema, status_code=status.HTTP_201_CREATED)
def create_sensor_reading(reading: SensorReadingCreate, db: Session = Depends(get_db)):
    """Create a new sensor reading"""
    # Verify sensor exists
    device = db.query(Device).filter(Device.id == reading.sensor_id).first()
    if not device:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Sensor device not found")
    
    db_reading = SensorReading(**reading.dict())
    db.add(db_reading)
    db.commit()
    db.refresh(db_reading)

    cache_payload = {
        "sensor_id": str(db_reading.sensor_id),
        "time": db_reading.time.isoformat(),
        "value": db_reading.value,
    }
    set_json(
        _latest_sensor_key(db_reading.sensor_id),
        cache_payload,
        ttl_seconds=settings.redis_sensor_cache_ttl_seconds,
    )

    return db_reading

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
