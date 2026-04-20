from fastapi import APIRouter, Depends, HTTPException, status
from sqlalchemy.orm import Session
from uuid import UUID
from typing import List
from database import get_db
from models import Log
from schemas import Log as LogSchema, LogCreate

router = APIRouter(prefix="/logs", tags=["logs"])

@router.get("/", response_model=List[LogSchema])
def list_logs(log_level: str = None, limit: int = 100, db: Session = Depends(get_db)):
    """Get logs, optionally filtered by log level"""
    query = db.query(Log)
    if log_level:
        query = query.filter(Log.log_level == log_level)
    return query.order_by(Log.timestamp.desc()).limit(limit).all()

@router.post("/", response_model=List[LogSchema], status_code=status.HTTP_201_CREATED)
def create_log(logs: List[LogCreate], db: Session = Depends(get_db)):
    """Create a batch of log entries."""
    db_logs = [
        Log(
            log_level=log.log_level.value,
            message=log.message,
            timestamp=log.timestamp,
        )
        for log in logs
    ]
    for log in db_logs:
        db.add(log)
    db.commit()
    for log in db_logs:
        db.refresh(log)
    return db_logs

@router.get("/{log_id}", response_model=LogSchema)
def get_log(log_id: UUID, db: Session = Depends(get_db)):
    """Get a log entry by ID"""
    log = db.query(Log).filter(Log.id == log_id).first()
    if not log:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Log not found")
    return log
