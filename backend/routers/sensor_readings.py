from fastapi import APIRouter, Depends, HTTPException, status
from sqlalchemy.orm import Session
from uuid import UUID
from typing import List
from datetime import datetime
from database import get_db
from models import SensorReading, Device
from schemas import SensorReading as SensorReadingSchema, SensorReadingCreate

router = APIRouter(prefix="/sensor-readings", tags=["sensor-readings"])

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
