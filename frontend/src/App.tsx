import { useEffect, useMemo, useState } from 'react'
import { Area, AreaChart, CartesianGrid, ResponsiveContainer, Tooltip, XAxis, YAxis } from 'recharts'
import { CalendarClock, Droplets, Leaf, RefreshCcw, SunMedium } from 'lucide-react'

import { ThemeToggle } from '@/components/theme-toggle'
import { Badge } from '@/components/ui/badge'
import { Button } from '@/components/ui/button'
import { Card, CardContent, CardDescription, CardHeader, CardTitle } from '@/components/ui/card'
import { Input } from '@/components/ui/input'
import { Label } from '@/components/ui/label'
import { Select } from '@/components/ui/select'
import {
  Table,
  TableBody,
  TableCell,
  TableHead,
  TableHeader,
  TableRow,
} from '@/components/ui/table'
import { Textarea } from '@/components/ui/textarea'
import { api } from '@/lib/api'
import type { Device, LightSchedule, LightSchedulePayload, SensorReading, WateringSchedule, WateringSchedulePayload } from '@/types/api'

type View = 'history' | 'schedules'

const blankLightForm: LightSchedulePayload = {
  name: '',
  description: '',
  light_on_start: '06:00:00',
  light_on_end: '18:00:00',
  brightness_level: 100,
}

const blankWateringForm: WateringSchedulePayload = {
  name: '',
  duration_seconds: 45,
  moisture_threshold_max: null,
  moisture_threshold_auto: null,
  moisture_auto_enabled: false,
  frequency_hours: 12,
}

function App() {
  const [view, setView] = useState<View>('history')
  const [isLoading, setIsLoading] = useState(true)
  const [error, setError] = useState<string | null>(null)

  const [devices, setDevices] = useState<Device[]>([])
  const [selectedSensorId, setSelectedSensorId] = useState('')
  const [readings, setReadings] = useState<SensorReading[]>([])

  const [lightSchedules, setLightSchedules] = useState<LightSchedule[]>([])
  const [wateringSchedules, setWateringSchedules] = useState<WateringSchedule[]>([])

  const [lightForm, setLightForm] = useState<LightSchedulePayload>(blankLightForm)
  const [editingLightId, setEditingLightId] = useState<string | null>(null)

  const [wateringForm, setWateringForm] = useState<WateringSchedulePayload>(blankWateringForm)
  const [editingWateringId, setEditingWateringId] = useState<string | null>(null)

  const sensorDevices = useMemo(
    () => devices.filter((d) => d.device_type === 'SENSOR' || d.device_type === 'MOISTURE' || d.device_type === 'LIGHT'),
    [devices],
  )

  const chartData = useMemo(
    () =>
      [...readings]
        .reverse()
        .map((r) => ({
          time: new Date(r.time).toLocaleString(),
          value: r.value,
        })),
    [readings],
  )

  const stats = useMemo(() => {
    if (readings.length === 0) {
      return { min: '-', max: '-', avg: '-', latest: '-' }
    }

    const values = readings.map((r) => r.value)
    const sum = values.reduce((acc, value) => acc + value, 0)

    return {
      min: Math.min(...values).toFixed(2),
      max: Math.max(...values).toFixed(2),
      avg: (sum / values.length).toFixed(2),
      latest: values[0].toFixed(2),
    }
  }, [readings])

  async function bootstrap() {
    setIsLoading(true)
    setError(null)
    try {
      const [deviceData, lightData, wateringData] = await Promise.all([
        api.getDevices(),
        api.getLightSchedules(),
        api.getWateringSchedules(),
      ])
      setDevices(deviceData)
      setLightSchedules(lightData)
      setWateringSchedules(wateringData)

      const firstSensor = deviceData.find(
        (d) => d.device_type === 'SENSOR' || d.device_type === 'MOISTURE' || d.device_type === 'LIGHT',
      )
      if (firstSensor && !selectedSensorId) {
        setSelectedSensorId(firstSensor.id)
      }
    } catch (requestError) {
      setError(requestError instanceof Error ? requestError.message : 'Failed to load greenhouse data.')
    } finally {
      setIsLoading(false)
    }
  }

  useEffect(() => {
    void bootstrap()
  }, [])

  useEffect(() => {
    if (!selectedSensorId) {
      setReadings([])
      return
    }

    async function loadReadings() {
      try {
        setError(null)
        const readingData = await api.getSensorReadings(selectedSensorId, 240)
        setReadings(readingData)
      } catch (requestError) {
        setError(requestError instanceof Error ? requestError.message : 'Failed to load sensor readings.')
      }
    }

    void loadReadings()
  }, [selectedSensorId])

  async function handleLightSubmit(event: React.FormEvent<HTMLFormElement>) {
    event.preventDefault()
    const payload: LightSchedulePayload = {
      ...lightForm,
      description: lightForm.description?.trim() ? lightForm.description : null,
      light_on_start: lightForm.light_on_start?.trim() ? lightForm.light_on_start : null,
      light_on_end: lightForm.light_on_end?.trim() ? lightForm.light_on_end : null,
      brightness_level: Number(lightForm.brightness_level),
    }

    try {
      if (editingLightId) {
        await api.updateLightSchedule(editingLightId, payload)
      } else {
        await api.createLightSchedule(payload)
      }
      setLightForm(blankLightForm)
      setEditingLightId(null)
      setLightSchedules(await api.getLightSchedules())
    } catch (requestError) {
      setError(requestError instanceof Error ? requestError.message : 'Failed to save light schedule.')
    }
  }

  async function handleWateringSubmit(event: React.FormEvent<HTMLFormElement>) {
    event.preventDefault()

    const payload: WateringSchedulePayload = {
      ...wateringForm,
      duration_seconds: Number(wateringForm.duration_seconds),
      frequency_hours: Number(wateringForm.frequency_hours),
      moisture_threshold_max: wateringForm.moisture_threshold_max === null ? null : Number(wateringForm.moisture_threshold_max),
      moisture_threshold_auto:
        wateringForm.moisture_threshold_auto === null ? null : Number(wateringForm.moisture_threshold_auto),
    }

    try {
      if (editingWateringId) {
        await api.updateWateringSchedule(editingWateringId, payload)
      } else {
        await api.createWateringSchedule(payload)
      }
      setWateringForm(blankWateringForm)
      setEditingWateringId(null)
      setWateringSchedules(await api.getWateringSchedules())
    } catch (requestError) {
      setError(requestError instanceof Error ? requestError.message : 'Failed to save watering schedule.')
    }
  }

  return (
    <main className="mx-auto w-full max-w-7xl px-4 py-6 sm:px-6 lg:px-8">
      <header className="mb-6 flex flex-col gap-4 rounded-3xl border border-border/70 bg-card/80 p-5 shadow-xl shadow-black/10 backdrop-blur md:flex-row md:items-center md:justify-between">
        <div>
          <p className="font-mono text-xs uppercase tracking-[0.18em] text-muted-foreground">greenhouse command center</p>
          <h1 className="mt-1 text-3xl font-semibold tracking-tight text-foreground sm:text-4xl">Sensors, Schedules, Stability</h1>
          <p className="mt-2 text-sm text-muted-foreground">Historical telemetry + schedule orchestration through your FastAPI backend.</p>
        </div>
        <div className="flex items-center gap-2 self-start md:self-center">
          <Badge variant="secondary">API: {import.meta.env.VITE_API_BASE_URL ?? 'http://localhost:8000'}</Badge>
          <ThemeToggle />
        </div>
      </header>

      <div className="mb-6 flex flex-wrap items-center gap-2">
        <Button variant={view === 'history' ? 'default' : 'outline'} onClick={() => setView('history')}>
          <Leaf className="h-4 w-4" />
          Sensor History
        </Button>
        <Button variant={view === 'schedules' ? 'default' : 'outline'} onClick={() => setView('schedules')}>
          <CalendarClock className="h-4 w-4" />
          Schedule Configuration
        </Button>
        <Button variant="ghost" onClick={() => void bootstrap()}>
          <RefreshCcw className="h-4 w-4" />
          Refresh All
        </Button>
      </div>

      {error && (
        <Card className="mb-6 border-destructive/50 bg-destructive/10">
          <CardContent className="pt-6 text-sm text-destructive">{error}</CardContent>
        </Card>
      )}

      {view === 'history' ? (
        <section className="space-y-6">
          <Card>
            <CardHeader>
              <CardTitle>Historical Sensor Data</CardTitle>
              <CardDescription>Select a device and inspect recent time-series values.</CardDescription>
            </CardHeader>
            <CardContent className="space-y-4">
              <div className="grid grid-cols-1 gap-4 md:grid-cols-2">
                <div className="space-y-2">
                  <Label htmlFor="sensor">Sensor Device</Label>
                  <Select id="sensor" value={selectedSensorId} onChange={(e) => setSelectedSensorId(e.target.value)} disabled={sensorDevices.length === 0 || isLoading}>
                    {sensorDevices.length === 0 ? (
                      <option value="">No sensor-like devices found</option>
                    ) : (
                      sensorDevices.map((device) => (
                        <option key={device.id} value={device.id}>
                          {device.name} ({device.device_type})
                        </option>
                      ))
                    )}
                  </Select>
                </div>
                <div className="grid grid-cols-4 gap-2">
                  <Card className="rounded-xl border-border/70 bg-muted/40">
                    <CardContent className="p-3 text-center">
                      <p className="text-xs text-muted-foreground">Latest</p>
                      <p className="font-mono text-lg font-semibold">{stats.latest}</p>
                    </CardContent>
                  </Card>
                  <Card className="rounded-xl border-border/70 bg-muted/40">
                    <CardContent className="p-3 text-center">
                      <p className="text-xs text-muted-foreground">Min</p>
                      <p className="font-mono text-lg font-semibold">{stats.min}</p>
                    </CardContent>
                  </Card>
                  <Card className="rounded-xl border-border/70 bg-muted/40">
                    <CardContent className="p-3 text-center">
                      <p className="text-xs text-muted-foreground">Max</p>
                      <p className="font-mono text-lg font-semibold">{stats.max}</p>
                    </CardContent>
                  </Card>
                  <Card className="rounded-xl border-border/70 bg-muted/40">
                    <CardContent className="p-3 text-center">
                      <p className="text-xs text-muted-foreground">Avg</p>
                      <p className="font-mono text-lg font-semibold">{stats.avg}</p>
                    </CardContent>
                  </Card>
                </div>
              </div>

              <div className="h-[360px] w-full rounded-2xl border border-border/70 bg-background/60 p-2">
                {chartData.length > 0 ? (
                  <ResponsiveContainer width="100%" height="100%">
                    <AreaChart data={chartData} margin={{ top: 18, right: 18, bottom: 0, left: 0 }}>
                      <defs>
                        <linearGradient id="greenGradient" x1="0" y1="0" x2="0" y2="1">
                          <stop offset="0%" stopColor="var(--color-primary)" stopOpacity={0.8} />
                          <stop offset="95%" stopColor="var(--color-primary)" stopOpacity={0.06} />
                        </linearGradient>
                      </defs>
                      <CartesianGrid strokeDasharray="3 3" stroke="var(--color-border)" />
                      <XAxis dataKey="time" hide />
                      <YAxis stroke="var(--color-muted-foreground)" width={50} />
                      <Tooltip
                        contentStyle={{
                          borderRadius: '12px',
                          borderColor: 'var(--color-border)',
                          backgroundColor: 'var(--color-card)',
                        }}
                      />
                      <Area type="monotone" dataKey="value" stroke="var(--color-primary)" strokeWidth={2.2} fill="url(#greenGradient)" />
                    </AreaChart>
                  </ResponsiveContainer>
                ) : (
                  <div className="flex h-full items-center justify-center text-sm text-muted-foreground">
                    No readings found for this sensor yet.
                  </div>
                )}
              </div>

              <div>
                <h3 className="mb-2 text-sm font-semibold text-foreground">Latest Samples</h3>
                <Table>
                  <TableHeader>
                    <TableRow>
                      <TableHead>Timestamp</TableHead>
                      <TableHead className="text-right">Value</TableHead>
                    </TableRow>
                  </TableHeader>
                  <TableBody>
                    {readings.slice(0, 8).map((reading) => (
                      <TableRow key={`${reading.sensor_id}-${reading.time}`}>
                        <TableCell>{new Date(reading.time).toLocaleString()}</TableCell>
                        <TableCell className="text-right font-mono">{reading.value.toFixed(3)}</TableCell>
                      </TableRow>
                    ))}
                  </TableBody>
                </Table>
              </div>
            </CardContent>
          </Card>
        </section>
      ) : (
        <section className="grid grid-cols-1 gap-6 xl:grid-cols-2">
          <Card>
            <CardHeader>
              <CardTitle className="flex items-center gap-2">
                <SunMedium className="h-5 w-5 text-primary" />
                Light Schedules
              </CardTitle>
              <CardDescription>Create or update light windows and brightness levels.</CardDescription>
            </CardHeader>
            <CardContent className="space-y-6">
              <form className="space-y-4" onSubmit={handleLightSubmit}>
                <div className="space-y-2">
                  <Label htmlFor="light-name">Name</Label>
                  <Input id="light-name" required value={lightForm.name} onChange={(e) => setLightForm((s) => ({ ...s, name: e.target.value }))} />
                </div>
                <div className="space-y-2">
                  <Label htmlFor="light-description">Description</Label>
                  <Textarea
                    id="light-description"
                    value={lightForm.description ?? ''}
                    onChange={(e) => setLightForm((s) => ({ ...s, description: e.target.value }))}
                  />
                </div>
                <div className="grid grid-cols-1 gap-4 sm:grid-cols-3">
                  <div className="space-y-2">
                    <Label htmlFor="light-start">Start</Label>
                    <Input
                      id="light-start"
                      type="time"
                      step={1}
                      value={(lightForm.light_on_start ?? '06:00:00').slice(0, 8)}
                      onChange={(e) => setLightForm((s) => ({ ...s, light_on_start: `${e.target.value}:00`.slice(0, 8) }))}
                    />
                  </div>
                  <div className="space-y-2">
                    <Label htmlFor="light-end">End</Label>
                    <Input
                      id="light-end"
                      type="time"
                      step={1}
                      value={(lightForm.light_on_end ?? '18:00:00').slice(0, 8)}
                      onChange={(e) => setLightForm((s) => ({ ...s, light_on_end: `${e.target.value}:00`.slice(0, 8) }))}
                    />
                  </div>
                  <div className="space-y-2">
                    <Label htmlFor="brightness">Brightness</Label>
                    <Input
                      id="brightness"
                      type="number"
                      min={0}
                      max={100}
                      value={lightForm.brightness_level}
                      onChange={(e) => setLightForm((s) => ({ ...s, brightness_level: Number(e.target.value) }))}
                    />
                  </div>
                </div>
                <div className="flex flex-wrap gap-2">
                  <Button type="submit">{editingLightId ? 'Update' : 'Create'} Light Schedule</Button>
                  {editingLightId && (
                    <Button
                      type="button"
                      variant="outline"
                      onClick={() => {
                        setEditingLightId(null)
                        setLightForm(blankLightForm)
                      }}
                    >
                      Cancel Edit
                    </Button>
                  )}
                </div>
              </form>

              <Table>
                <TableHeader>
                  <TableRow>
                    <TableHead>Name</TableHead>
                    <TableHead>Window</TableHead>
                    <TableHead>Brightness</TableHead>
                    <TableHead className="text-right">Action</TableHead>
                  </TableRow>
                </TableHeader>
                <TableBody>
                  {lightSchedules.map((schedule) => (
                    <TableRow key={schedule.id}>
                      <TableCell>{schedule.name}</TableCell>
                      <TableCell className="font-mono text-xs">
                        {(schedule.light_on_start ?? '--')} to {(schedule.light_on_end ?? '--')}
                      </TableCell>
                      <TableCell>{schedule.brightness_level}%</TableCell>
                      <TableCell className="text-right">
                        <Button
                          size="sm"
                          variant="outline"
                          onClick={() => {
                            setEditingLightId(schedule.id)
                            setLightForm({
                              name: schedule.name,
                              description: schedule.description,
                              light_on_start: schedule.light_on_start,
                              light_on_end: schedule.light_on_end,
                              brightness_level: schedule.brightness_level,
                            })
                          }}
                        >
                          Edit
                        </Button>
                      </TableCell>
                    </TableRow>
                  ))}
                </TableBody>
              </Table>
            </CardContent>
          </Card>

          <Card>
            <CardHeader>
              <CardTitle className="flex items-center gap-2">
                <Droplets className="h-5 w-5 text-primary" />
                Watering Schedules
              </CardTitle>
              <CardDescription>Configure cadence, duration, and moisture automation thresholds.</CardDescription>
            </CardHeader>
            <CardContent className="space-y-6">
              <form className="space-y-4" onSubmit={handleWateringSubmit}>
                <div className="space-y-2">
                  <Label htmlFor="watering-name">Name</Label>
                  <Input
                    id="watering-name"
                    required
                    value={wateringForm.name}
                    onChange={(e) => setWateringForm((s) => ({ ...s, name: e.target.value }))}
                  />
                </div>

                <div className="grid grid-cols-1 gap-4 sm:grid-cols-2">
                  <div className="space-y-2">
                    <Label htmlFor="duration">Duration (seconds)</Label>
                    <Input
                      id="duration"
                      type="number"
                      min={1}
                      value={wateringForm.duration_seconds}
                      onChange={(e) => setWateringForm((s) => ({ ...s, duration_seconds: Number(e.target.value) }))}
                    />
                  </div>
                  <div className="space-y-2">
                    <Label htmlFor="frequency">Frequency (hours)</Label>
                    <Input
                      id="frequency"
                      type="number"
                      min={1}
                      value={wateringForm.frequency_hours}
                      onChange={(e) => setWateringForm((s) => ({ ...s, frequency_hours: Number(e.target.value) }))}
                    />
                  </div>
                </div>

                <div className="grid grid-cols-1 gap-4 sm:grid-cols-2">
                  <div className="space-y-2">
                    <Label htmlFor="threshold-max">Moisture Max Threshold</Label>
                    <Input
                      id="threshold-max"
                      type="number"
                      step="0.1"
                      value={wateringForm.moisture_threshold_max ?? ''}
                      onChange={(e) =>
                        setWateringForm((s) => ({
                          ...s,
                          moisture_threshold_max: e.target.value === '' ? null : Number(e.target.value),
                        }))
                      }
                    />
                  </div>
                  <div className="space-y-2">
                    <Label htmlFor="threshold-auto">Moisture Auto Threshold</Label>
                    <Input
                      id="threshold-auto"
                      type="number"
                      step="0.1"
                      value={wateringForm.moisture_threshold_auto ?? ''}
                      onChange={(e) =>
                        setWateringForm((s) => ({
                          ...s,
                          moisture_threshold_auto: e.target.value === '' ? null : Number(e.target.value),
                        }))
                      }
                    />
                  </div>
                </div>

                <label className="flex items-center gap-2 text-sm font-medium">
                  <input
                    type="checkbox"
                    className="h-4 w-4 rounded border-border"
                    checked={wateringForm.moisture_auto_enabled}
                    onChange={(e) => setWateringForm((s) => ({ ...s, moisture_auto_enabled: e.target.checked }))}
                  />
                  Enable Moisture Auto Mode
                </label>

                <div className="flex flex-wrap gap-2">
                  <Button type="submit">{editingWateringId ? 'Update' : 'Create'} Watering Schedule</Button>
                  {editingWateringId && (
                    <Button
                      type="button"
                      variant="outline"
                      onClick={() => {
                        setEditingWateringId(null)
                        setWateringForm(blankWateringForm)
                      }}
                    >
                      Cancel Edit
                    </Button>
                  )}
                </div>
              </form>

              <Table>
                <TableHeader>
                  <TableRow>
                    <TableHead>Name</TableHead>
                    <TableHead>Duration</TableHead>
                    <TableHead>Frequency</TableHead>
                    <TableHead>Auto</TableHead>
                    <TableHead className="text-right">Action</TableHead>
                  </TableRow>
                </TableHeader>
                <TableBody>
                  {wateringSchedules.map((schedule) => (
                    <TableRow key={schedule.id}>
                      <TableCell>{schedule.name}</TableCell>
                      <TableCell>{schedule.duration_seconds}s</TableCell>
                      <TableCell>{schedule.frequency_hours}h</TableCell>
                      <TableCell>{schedule.moisture_auto_enabled ? 'Enabled' : 'Off'}</TableCell>
                      <TableCell className="text-right">
                        <Button
                          size="sm"
                          variant="outline"
                          onClick={() => {
                            setEditingWateringId(schedule.id)
                            setWateringForm({
                              name: schedule.name,
                              duration_seconds: schedule.duration_seconds,
                              moisture_threshold_max: schedule.moisture_threshold_max,
                              moisture_threshold_auto: schedule.moisture_threshold_auto,
                              moisture_auto_enabled: schedule.moisture_auto_enabled,
                              frequency_hours: schedule.frequency_hours,
                            })
                          }}
                        >
                          Edit
                        </Button>
                      </TableCell>
                    </TableRow>
                  ))}
                </TableBody>
              </Table>
            </CardContent>
          </Card>
        </section>
      )}
    </main>
  )
}

export default App
