CREATE TABLE cpu_events (
    id bigint GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    received_at timestamptz NOT NULL DEFAULT now(),
    kind text NOT NULL CHECK (kind IN ('alert', 'recovered')),
    cpu_percent double precision NOT NULL CHECK (cpu_percent BETWEEN 0 AND 100)
);