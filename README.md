# Linux System Monitor

A C++20 command-line monitor that samples Linux CPU and memory usage, detects sustained high CPU usage, and sends alert and recovery events to a FastAPI service. PostgreSQL stores the event history so it can be queried after the monitor exits.

## How it works

The collector takes five samples roughly one second apart. It calculates CPU utilization from changes in `/proc/stat` and memory utilization from `MemTotal` and `MemAvailable` in `/proc/meminfo`. When running under WSL2, the readings describe the Linux environment.

Each sample appears in the terminal. The collector uses libcurl to send an event when the CPU alert changes state.

| CPU readings | Behavior |
| --- | --- |
| Three consecutive samples at or above 80% | Send one `alert` on the third sample. |
| Further samples at or above 80% during an active alert | Keep sampling without sending duplicate alerts. |
| First sample below 80% during an active alert | Send one `recovered` event. |

A reading below 80% resets the consecutive-sample count. A quiet run can finish with five readings and zero events. Each stored event includes its ID, receipt timestamp, kind, and triggering CPU percentage.

## Build and run

These instructions target Ubuntu 24.04, including Ubuntu under WSL2. The C++ build requires a C++20 compiler, CMake 3.20 or later, and libcurl. The backend requires Python and a running PostgreSQL server.

### 1. Install dependencies and build

```bash
sudo apt update
sudo apt install git build-essential cmake libcurl4-openssl-dev python3-venv postgresql postgresql-client curl

git clone https://github.com/alonmoneli/system-monitor.git
cd system-monitor

cmake -S . -B build
cmake --build build

python3 -m venv .venv
source .venv/bin/activate
python -m pip install -r backend/requirements.txt
```

## Run with Docker Compose

From the repository root, with Docker running:

```bash
docker compose up --build -d
docker compose ps
curl -fsS http://127.0.0.1:8000/events
```

Compose starts FastAPI and PostgreSQL. The C++ collector runs directly in Linux
or Ubuntu under WSL2. Install a C++20 compiler, CMake 3.20 or later, and the
libcurl development package (`libcurl4-openssl-dev` on Ubuntu), then build and run it:

```bash
cmake -S . -B build
cmake --build build
./build/system_monitor
```

On a fresh setup, `GET /events` returns `[]` until an event is stored. Stop
both services with `docker compose down`. The named PostgreSQL volume keeps
your events for the next start.

### 2. Initialize PostgreSQL

Start PostgreSQL, then create the project database and apply the schema. On a fresh installation, run all four commands. If a PostgreSQL role with your Ubuntu username already exists, skip the `createuser` command.

```bash
sudo service postgresql start
sudo -u postgres createuser "$USER"
sudo -u postgres createdb --owner="$USER" system_monitor
psql -d system_monitor -f backend/schema.sql
```

The backend connects to the local `system_monitor` database through Psycopg. Ubuntu’s default local peer authentication lets PostgreSQL identify the application by the Ubuntu user running it. The role, database, and schema only need to be created once.

### 3. Start the API and run the collector

In the first terminal, from the repository root with `.venv` active, start the API:

```bash
python -m uvicorn backend.app:app
```

In a second terminal, from the same repository root, run the collector:

```bash
./build/system_monitor
```

The collector exits after five samples. The API continues serving requests until you stop it with `Ctrl+C`.

## Try the API

To try the event history on a quiet machine, submit a sample alert manually. The `85.0` below is a supplied test value. Each POST creates one row and returns its database ID.

```bash
curl -sS http://127.0.0.1:8000/events \
  -H 'Content-Type: application/json' \
  -d '{"kind":"alert","cpu_percent":85.0}'
```

Read the history and look for the same ID:

```bash
curl -sS http://127.0.0.1:8000/events | python3 -m json.tool
```

| Endpoint | Behavior |
| --- | --- |
| `GET /health` | Returns `{"status":"ok"}` when the API responds. This checks the web service only. |
| `POST /events` | Validates and stores an event. Accepts `alert` or `recovered` and a CPU percentage from 0 through 100. |
| `GET /events` | Returns up to 20 stored events, ordered by descending ID. |

## Test the alert rule

After building, run:

```bash
ctest --test-dir build --output-on-failure
```

The C++ test feeds a fixed sequence of readings into the same alert rule used by the collector. It checks that a low reading breaks a streak, exactly 80% counts as high, and repeated readings produce a single alert and a single recovery. The test runs independently of FastAPI and PostgreSQL.