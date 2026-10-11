#!/usr/bin/env python3
"""Verify deterministic demonstration output; NOT PHY correctness."""
import csv
import sys
records = list(csv.DictReader(sys.stdin))
assert len(records) == 40
valid_masks = {1:'00100000000000',2:'00100000000100',3:'00100001000100',4:'00100100100100'}
changed_count=0
for i,row in enumerate(records):
    assert int(row['slot']) == i
    assert row['status'] == 'ok'
    cqi = int(row['cqi'])
    code = int(row['code'])
    if i < 20:
        assert row['source']=='cqi'
        assert code==(4 if cqi<=6 else 2 if cqi>=13 else 3)
    else:
        assert row['source']=='forced'
        assert code==4
    assert row['mask']==valid_masks[code]
    assert int(row['symbols'])==row['mask'].count('1')
    assert int(row['re_per_prb'])==6*int(row['symbols'])
    changed=code!=3
    assert int(row['changed'])==int(changed)
    assert int(row['applied_pos'])==(code-1 if changed else 255)
    changed_count+=int(changed)
assert changed_count>0
print(f'demo_check=PASS rows={len(records)} changed={changed_count}')
