from fastapi import APIRouter, Depends, HTTPException, status
from sqlalchemy.orm import Session
from uuid import UUID
from typing import List
from datetime import date
from database import get_db
from models import WateringSchedule, WateringScheduleDate
from schemas import (
    WateringSchedule as WateringScheduleSchema,
    WateringScheduleCreate,
    WateringScheduleUpdate,
    WateringScheduleDate as WateringScheduleDateSchema,
    WateringScheduleDateBase,
)

router = APIRouter(prefix="/watering-schedules", tags=["watering-schedules"])

@router.get("/", response_model=List[WateringScheduleSchema])
def list_watering_schedules(db: Session = Depends(get_db)):
    """Get all watering schedules"""
    return db.query(WateringSchedule).all()

@router.post("/", response_model=WateringScheduleSchema, status_code=status.HTTP_201_CREATED)
def create_watering_schedule(schedule: WateringScheduleCreate, db: Session = Depends(get_db)):
    """Create a new watering schedule"""
    db_schedule = WateringSchedule(**schedule.dict())
    db.add(db_schedule)
    db.commit()
    db.refresh(db_schedule)
    return db_schedule

@router.get("/{schedule_id}", response_model=WateringScheduleSchema)
def get_watering_schedule(schedule_id: UUID, db: Session = Depends(get_db)):
    """Get a watering schedule by ID"""
    schedule = db.query(WateringSchedule).filter(WateringSchedule.id == schedule_id).first()
    if not schedule:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Schedule not found")
    return schedule

@router.put("/{schedule_id}", response_model=WateringScheduleSchema)
def update_watering_schedule(schedule_id: UUID, schedule_update: WateringScheduleUpdate, db: Session = Depends(get_db)):
    """Update a watering schedule"""
    schedule = db.query(WateringSchedule).filter(WateringSchedule.id == schedule_id).first()
    if not schedule:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Schedule not found")
    
    update_data = schedule_update.dict(exclude_unset=True)
    for key, value in update_data.items():
        setattr(schedule, key, value)
    
    db.commit()
    db.refresh(schedule)
    return schedule

@router.delete("/{schedule_id}", status_code=status.HTTP_204_NO_CONTENT)
def delete_watering_schedule(schedule_id: UUID, db: Session = Depends(get_db)):
    """Delete a watering schedule"""
    schedule = db.query(WateringSchedule).filter(WateringSchedule.id == schedule_id).first()
    if not schedule:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Schedule not found")
    
    db.delete(schedule)
    db.commit()

# Schedule dates endpoints
@router.get("/{schedule_id}/dates", response_model=List[WateringScheduleDateSchema])
def get_schedule_dates(schedule_id: UUID, db: Session = Depends(get_db)):
    """Get all dates for a watering schedule"""
    schedule = db.query(WateringSchedule).filter(WateringSchedule.id == schedule_id).first()
    if not schedule:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Schedule not found")
    
    return db.query(WateringScheduleDate).filter(WateringScheduleDate.schedule_id == schedule_id).all()

@router.post("/{schedule_id}/dates", response_model=WateringScheduleDateSchema, status_code=status.HTTP_201_CREATED)
def add_schedule_date(schedule_id: UUID, date_entry: WateringScheduleDateBase, db: Session = Depends(get_db)):
    """Add a date to a watering schedule"""
    schedule = db.query(WateringSchedule).filter(WateringSchedule.id == schedule_id).first()
    if not schedule:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Schedule not found")
    
    db_date = WateringScheduleDate(schedule_id=schedule_id, **date_entry.dict())
    db.add(db_date)
    db.commit()
    db.refresh(db_date)
    return db_date

@router.delete("/{schedule_id}/dates/{start_date}", status_code=status.HTTP_204_NO_CONTENT)
def remove_schedule_date(schedule_id: UUID, start_date: date, db: Session = Depends(get_db)):
    """Remove a date from a watering schedule"""
    date_entry = db.query(WateringScheduleDate).filter(
        WateringScheduleDate.schedule_id == schedule_id,
        WateringScheduleDate.start_date == start_date
    ).first()
    
    if not date_entry:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Schedule date not found")
    
    db.delete(date_entry)
    db.commit()
