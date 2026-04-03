from fastapi import APIRouter, Depends, HTTPException, status
from sqlalchemy.orm import Session
from uuid import UUID
from typing import List
from database import get_db
from models import Log, Device
from schemas import Log as LogSchema, LogCreate

router = APIRouter(prefix="/logs", tags=["logs"])

@router.get("/", response_model=List[LogSchema])
def list_logs(device_id: UUID = None, log_level: str = None, limit: int = 100, db: Session = Depends(get_db)):
    """Get logs, optionally filtered by device_id or log_level"""
    query = db.query(Log)
    if device_id:
        query = query.filter(Log.device_id == device_id)
    if log_level:
        query = query.filter(Log.log_level == log_level)
    return query.order_by(Log.created_at.desc()).limit(limit).all()

@router.post("/", response_model=LogSchema, status_code=status.HTTP_201_CREATED)
def create_log(log: LogCreate, device_id: UUID, db: Session = Depends(get_db)):
    """Create a new log entry"""
    # Verify device exists
    device = db.query(Device).filter(Device.id == device_id).first()
    if not device:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Device not found")
    
    db_log = Log(device_id=device_id, **log.dict())
    db.add(db_log)
    db.commit()
    db.refresh(db_log)
    return db_log

@router.get("/{log_id}", response_model=LogSchema)
def get_log(log_id: UUID, db: Session = Depends(get_db)):
    """Get a log entry by ID"""
    log = db.query(Log).filter(Log.id == log_id).first()
    if not log:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Log not found")
    return log

@router.get("/device/{device_id}", response_model=List[LogSchema])
def get_device_logs(device_id: UUID, limit: int = 100, db: Session = Depends(get_db)):
    """Get all logs for a device"""
    # Verify device exists
    device = db.query(Device).filter(Device.id == device_id).first()
    if not device:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Device not found")
    
    return db.query(Log).filter(
        Log.device_id == device_id
    ).order_by(Log.created_at.desc()).limit(limit).all()
