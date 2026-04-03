from fastapi import APIRouter, Depends, HTTPException, status
from sqlalchemy.orm import Session
from uuid import UUID
from typing import List
from datetime import datetime
from database import get_db
from models import Alert, Device
from schemas import Alert as AlertSchema, AlertCreate, AlertUpdate

router = APIRouter(prefix="/alerts", tags=["alerts"])

@router.get("/", response_model=List[AlertSchema])
def list_alerts(device_id: UUID = None, resolved: bool = None, severity: str = None, limit: int = 100, db: Session = Depends(get_db)):
    """Get alerts, optionally filtered by device_id, resolved status, or severity"""
    query = db.query(Alert)
    if device_id:
        query = query.filter(Alert.device_id == device_id)
    if resolved is not None:
        query = query.filter(Alert.resolved == resolved)
    if severity:
        query = query.filter(Alert.severity == severity)
    return query.order_by(Alert.created_at.desc()).limit(limit).all()

@router.post("/", response_model=AlertSchema, status_code=status.HTTP_201_CREATED)
def create_alert(alert: AlertCreate, device_id: UUID, db: Session = Depends(get_db)):
    """Create a new alert"""
    # Verify device exists
    device = db.query(Device).filter(Device.id == device_id).first()
    if not device:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Device not found")
    
    db_alert = Alert(device_id=device_id, **alert.dict())
    db.add(db_alert)
    db.commit()
    db.refresh(db_alert)
    return db_alert

@router.get("/{alert_id}", response_model=AlertSchema)
def get_alert(alert_id: UUID, db: Session = Depends(get_db)):
    """Get an alert by ID"""
    alert = db.query(Alert).filter(Alert.id == alert_id).first()
    if not alert:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Alert not found")
    return alert

@router.put("/{alert_id}", response_model=AlertSchema)
def update_alert(alert_id: UUID, alert_update: AlertUpdate, db: Session = Depends(get_db)):
    """Update an alert"""
    alert = db.query(Alert).filter(Alert.id == alert_id).first()
    if not alert:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Alert not found")
    
    update_data = alert_update.dict(exclude_unset=True)
    if "resolved" in update_data and update_data["resolved"] and not alert.resolved:
        update_data["resolved_at"] = datetime.utcnow()
    
    for key, value in update_data.items():
        setattr(alert, key, value)
    
    db.commit()
    db.refresh(alert)
    return alert

@router.delete("/{alert_id}", status_code=status.HTTP_204_NO_CONTENT)
def delete_alert(alert_id: UUID, db: Session = Depends(get_db)):
    """Delete an alert"""
    alert = db.query(Alert).filter(Alert.id == alert_id).first()
    if not alert:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Alert not found")
    
    db.delete(alert)
    db.commit()

@router.get("/device/{device_id}", response_model=List[AlertSchema])
def get_device_alerts(device_id: UUID, limit: int = 100, db: Session = Depends(get_db)):
    """Get all alerts for a device"""
    # Verify device exists
    device = db.query(Device).filter(Device.id == device_id).first()
    if not device:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Device not found")
    
    return db.query(Alert).filter(
        Alert.device_id == device_id
    ).order_by(Alert.created_at.desc()).limit(limit).all()
