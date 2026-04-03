from fastapi import APIRouter, Depends, HTTPException, status
from sqlalchemy.orm import Session
from uuid import UUID
from typing import List
from datetime import date
from database import get_db
from models import LightSchedule, LightScheduleDate
from schemas import (
    LightSchedule as LightScheduleSchema,
    LightScheduleCreate,
    LightScheduleUpdate,
    LightScheduleDate as LightScheduleDateSchema,
    LightScheduleDateBase,
)

router = APIRouter(prefix="/light-schedules", tags=["light-schedules"])

@router.get("/", response_model=List[LightScheduleSchema])
def list_light_schedules(db: Session = Depends(get_db)):
    """Get all light schedules"""
    return db.query(LightSchedule).all()

@router.post("/", response_model=LightScheduleSchema, status_code=status.HTTP_201_CREATED)
def create_light_schedule(schedule: LightScheduleCreate, db: Session = Depends(get_db)):
    """Create a new light schedule"""
    db_schedule = LightSchedule(**schedule.dict())
    db.add(db_schedule)
    db.commit()
    db.refresh(db_schedule)
    return db_schedule

@router.get("/{schedule_id}", response_model=LightScheduleSchema)
def get_light_schedule(schedule_id: UUID, db: Session = Depends(get_db)):
    """Get a light schedule by ID"""
    schedule = db.query(LightSchedule).filter(LightSchedule.id == schedule_id).first()
    if not schedule:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Schedule not found")
    return schedule

@router.put("/{schedule_id}", response_model=LightScheduleSchema)
def update_light_schedule(schedule_id: UUID, schedule_update: LightScheduleUpdate, db: Session = Depends(get_db)):
    """Update a light schedule"""
    schedule = db.query(LightSchedule).filter(LightSchedule.id == schedule_id).first()
    if not schedule:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Schedule not found")
    
    update_data = schedule_update.dict(exclude_unset=True)
    for key, value in update_data.items():
        setattr(schedule, key, value)
    
    db.commit()
    db.refresh(schedule)
    return schedule

@router.delete("/{schedule_id}", status_code=status.HTTP_204_NO_CONTENT)
def delete_light_schedule(schedule_id: UUID, db: Session = Depends(get_db)):
    """Delete a light schedule"""
    schedule = db.query(LightSchedule).filter(LightSchedule.id == schedule_id).first()
    if not schedule:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Schedule not found")
    
    db.delete(schedule)
    db.commit()

# Schedule dates endpoints
@router.get("/{schedule_id}/dates", response_model=List[LightScheduleDateSchema])
def get_schedule_dates(schedule_id: UUID, db: Session = Depends(get_db)):
    """Get all dates for a light schedule"""
    schedule = db.query(LightSchedule).filter(LightSchedule.id == schedule_id).first()
    if not schedule:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Schedule not found")
    
    return db.query(LightScheduleDate).filter(LightScheduleDate.schedule_id == schedule_id).all()

@router.post("/{schedule_id}/dates", response_model=LightScheduleDateSchema, status_code=status.HTTP_201_CREATED)
def add_schedule_date(schedule_id: UUID, date_entry: LightScheduleDateBase, db: Session = Depends(get_db)):
    """Add a date to a light schedule"""
    schedule = db.query(LightSchedule).filter(LightSchedule.id == schedule_id).first()
    if not schedule:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Schedule not found")
    
    db_date = LightScheduleDate(schedule_id=schedule_id, **date_entry.dict())
    db.add(db_date)
    db.commit()
    db.refresh(db_date)
    return db_date

@router.delete("/{schedule_id}/dates/{start_date}", status_code=status.HTTP_204_NO_CONTENT)
def remove_schedule_date(schedule_id: UUID, start_date: date, db: Session = Depends(get_db)):
    """Remove a date from a light schedule"""
    date_entry = db.query(LightScheduleDate).filter(
        LightScheduleDate.schedule_id == schedule_id,
        LightScheduleDate.start_date == start_date
    ).first()
    
    if not date_entry:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Schedule date not found")
    
    db.delete(date_entry)
    db.commit()
