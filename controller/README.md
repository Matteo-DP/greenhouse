# Greenhouse Controller (C++)

Hardware controller for the first supported device set:

- `SENSOR` (generic)
- `MOISTURE`
- `LIGHT`

## Design

- `Device`: abstract base class for all devices.
- `SensorDevice`: abstract sensor specialization with reading history.
- `GenericSensor`, `MoistureSensor`, `LightSensor`: concrete sensor implementations.
- `GreenhouseController`: registry + sensor reading orchestration.
- `HardwareSensor`: abstraction for physical sensor reads.
- `SensorRuntime`: polling workflow from hardware -> in-memory validation -> API push.
- `RestApiClient`: HTTP client for backend API (`POST /devices`, `POST /sensor-readings`).

## API flow

1. Create remote devices via `POST /devices`.
2. Bind local devices to remote UUIDs.
3. Poll hardware once (or repeatedly in your own loop).
4. Send each reading to `POST /sensor-readings` with:
	- `sensor_id` (remote UUID)
	- `time` (UTC ISO8601)
	- `value`

## Hardware integration

Implement `HardwareSensor` for your platform (I2C/SPI/ADC/GPIO), then bind each concrete sensor instance.

`FixedValueSensor` is only a demo placeholder.

## Build

```bash
cmake -S . -B build
cmake --build build
```

## Run demo

```bash
./build/greenhouse_controller_demo
```
