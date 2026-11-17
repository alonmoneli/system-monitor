# System Monitor Design

The monitor answers two questions: how busy is this Linux system right now, and when did it enter or recover from sustained high CPU usage? The terminal shows live CPU and memory readings. The API exposes a persistent history of CPU alert and recovery events.

## From measurement to stored event

1. The C++ collector reads cumulative CPU counters from `/proc/stat` twice, roughly one second apart. It calculates CPU usage as `100 × (totalDelta − idleDelta) / totalDelta`. It also reads `MemTotal` and `MemAvailable` from `/proc/meminfo` to calculate memory usage.
2. The collector prints each sample and passes its CPU percentage to one `CpuAlertRule` instance. Keeping that instance alive across samples lets it count consecutive high readings.
3. When the rule emits an event, the collector uses libcurl to send its kind and triggering CPU percentage as JSON to `POST /events`.
4. FastAPI validates the request with Pydantic. The handler uses Psycopg to insert the event into PostgreSQL and returns its generated ID after the transaction commits.
5. A client can call `GET /events` to read up to 20 stored events, ordered by descending ID.

## Alert behavior

| Current state | Reading | Outcome |
| --- | --- | --- |
| Monitoring | Below 80% | Reset the high-reading count. |
| Monitoring | At least 80% | Increment the count. On the third consecutive high reading, emit one alert and enter the active state. |
| Alert active | At least 80% | Stay active. |
| Alert active | Below 80% | Emit one recovery, reset the count, and resume monitoring. |

For example, readings of `82, 86, 85, 90, 20` produce two events: an alert at 85% and a recovery at 20%. The rule takes one reading as input, so its behavior can be tested without waiting for a particular CPU load.

## API and storage

| Endpoint | Contract |
| --- | --- |
| `GET /health` | Reports whether the API can respond. Database connectivity is outside this check. |
| `POST /events` | Accepts `kind` (`alert` or `recovered`) and `cpu_percent` (0–100 inclusive). Returns the stored event's ID. |
| `GET /events` | Returns recent events with their IDs, receipt timestamps, kinds, and CPU percentages. |

The collector owns the alert decision. The backend validates the reported event and owns database access. PostgreSQL generates the ID and receipt timestamp and enforces constraints on kind and percentage. The timestamp records when the database transaction began. It is distinct from the moment the collector sampled the CPU.

## Runtime boundaries

The collector takes five samples per run. Its alert state lasts for that run. PostgreSQL retains committed events across collector and API restarts. Individual CPU and memory samples appear in the terminal; only CPU alert transitions are stored. If delivery fails, the collector reports the error and continues sampling. The current implementation has no retry queue.

## Verification

- **C++ unit test:** feeds fixed readings into the alert rule to check the threshold boundary, interrupted streaks, and duplicate-event suppression.
- **Backend tests:** use FastAPI's test client. One posts an event and retrieves the same ID and values through a separate request backed by a dedicated PostgreSQL test database. Another verifies that a CPU value above 100 receives HTTP 422.
- **Manual integration check:** running the collector with the API verifies the libcurl-to-FastAPI connection, which the two automated suites exercise separately.

See [README.md](README.md) for setup and usage.