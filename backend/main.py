from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
from config import settings
from routers import devices, sensor_readings, light_schedules, watering_schedules, logs, alerts

app = FastAPI(
    title=settings.api_title,
    version=settings.api_version,
    description="API for managing greenhouse sensors, devices, and automated schedules"
)

# Add CORS middleware
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# Include routers
app.include_router(devices.router)
app.include_router(sensor_readings.router)
app.include_router(light_schedules.router)
app.include_router(watering_schedules.router)
app.include_router(logs.router)
app.include_router(alerts.router)

@app.get("/health")
def health_check():
    """Health check endpoint"""
    return {"status": "healthy"}

if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=8000, reload=settings.debug)
