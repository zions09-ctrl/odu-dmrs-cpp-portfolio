#!/usr/bin/env python3
"""Read-only cross-check of previously reported *aggregates* (no raw trace replay)."""
import csv
import json
from pathlib import Path
from statistics import median
root = Path(__file__).resolve().parents[1]
with (root / 'historical/campaign4_hook_per_run.csv').open() as f:
    hooks=list(csv.DictReader(f))
with (root / 'historical/campaign4_external_six_events.csv').open() as f:
    events=list(csv.DictReader(f))
assert len(hooks)==9
assert len(events)==6
p50s=[int(x['ctrl_p50_ns']) for x in hooks]
p99s=[int(x['ctrl_p99_ns']) for x in hooks]
maxima=[int(x['ctrl_max_ns']) for x in hooks]
remote=[float(x['reported_command_to_first_applied_us']) for x in events]
result={
    'source_type':'previously_reported_aggregates_not_raw_reproduction',
    'runs':len(hooks),
    'hook_p50_range_ns':[min(p50s),max(p50s)],
    'hook_p99_max_ns':max(p99s),
    'hook_max_observed_ns':max(maxima),
    'remote_n':len(remote),
    'remote_median_us':median(remote),
    'remote_median_slots_at_500us':median(remote)/500,
}
assert result['hook_p50_range_ns']==[273,524]
assert result['hook_p99_max_ns']==1349
assert result['hook_max_observed_ns']==3096322
assert abs(result['remote_median_us']-1420.2)<1e-7
print(json.dumps(result,indent=2))
