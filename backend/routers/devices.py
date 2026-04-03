from fastapi import APIRouter, Depends, HTTPException, status
from sqlalchemy.orm import Session
from uuid import UUID
from typing import List
from database import get_db
from models import Device
from schemas import Device as DeviceSchema, DeviceCreate, DeviceUpdate

router = APIRouter(prefix="/devices", tags=["devices"])

@router.get("/", response_model=List[DeviceSchema])
def list_devices(db: Session = Depends(get_db)):
    """Get all devices"""
    return db.query(Device).all()

@router.post("/", response_model=DeviceSchema, status_code=status.HTTP_201_CREATED)
def create_device(device: DeviceCreate, db: Session = Depends(get_db)):
    """Create a new device"""
    db_device = Device(**device.dict())
    db.add(db_device)
    db.commit()
    db.refresh(db_device)
    return db_device

@router.get("/{device_id}", response_model=DeviceSchema)
def get_device(device_id: UUID, db: Session = Depends(get_db)):
    """Get a device by ID"""
    device = db.query(Device).filter(Device.id == device_id).first()
    if not device:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Device not found")
    return device

@router.put("/{device_id}", response_model=DeviceSchema)
def update_device(device_id: UUID, device_update: DeviceUpdate, db: Session = Depends(get_db)):
    """Update a device"""
    device = db.query(Device).filter(Device.id == device_id).first()
    if not device:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Device not found")
    
    update_data = device_update.dict(exclude_unset=True)
    for key, value in update_data.items():
        setattr(device, key, value)
    
    db.commit()
    db.refresh(device)
    return device

@router.delete("/{device_id}", status_code=status.HTTP_204_NO_CONTENT)
def delete_device(device_id: UUID, db: Session = Depends(get_db)):
    """Delete a device"""
    device = db.query(Device).filter(Device.id == device_id).first()
    if not device:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Device not found")
    
    db.delete(device)
    db.commit()
