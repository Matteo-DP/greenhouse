import type {
  Device,
  LightSchedule,
  LightSchedulePayload,
  SensorReading,
  WateringSchedule,
  WateringSchedulePayload,
} from '@/types/api'

const API_BASE_URL = import.meta.env.VITE_API_BASE_URL ?? 'http://localhost:8000'

async function apiRequest<T>(path: string, init?: RequestInit): Promise<T> {
  const response = await fetch(`${API_BASE_URL}${path}`, {
    headers: {
      'Content-Type': 'application/json',
      ...(init?.headers ?? {}),
    },
    ...init,
  })

  if (!response.ok) {
    const detail = await response.text()
    throw new Error(detail || `Request failed: ${response.status}`)
  }

  if (response.status === 204) {
    return undefined as T
  }

  return (await response.json()) as T
}

export const api = {
  getDevices: () => apiRequest<Device[]>('/devices/'),
  getSensorReadings: (sensorId: string, limit = 200) =>
    apiRequest<SensorReading[]>(`/sensor-readings/sensor/${sensorId}?limit=${limit}`),

  getLightSchedules: () => apiRequest<LightSchedule[]>('/light-schedules/'),
  createLightSchedule: (payload: LightSchedulePayload) =>
    apiRequest<LightSchedule>('/light-schedules/', { method: 'POST', body: JSON.stringify(payload) }),
  updateLightSchedule: (id: string, payload: Partial<LightSchedulePayload>) =>
    apiRequest<LightSchedule>(`/light-schedules/${id}`, { method: 'PUT', body: JSON.stringify(payload) }),

  getWateringSchedules: () => apiRequest<WateringSchedule[]>('/watering-schedules/'),
  createWateringSchedule: (payload: WateringSchedulePayload) =>
    apiRequest<WateringSchedule>('/watering-schedules/', { method: 'POST', body: JSON.stringify(payload) }),
  updateWateringSchedule: (id: string, payload: Partial<WateringSchedulePayload>) =>
    apiRequest<WateringSchedule>(`/watering-schedules/${id}`, { method: 'PUT', body: JSON.stringify(payload) }),
}
