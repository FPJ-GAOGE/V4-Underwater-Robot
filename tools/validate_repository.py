"""Offline snapshot checks. Does not import runtime modules or open devices/network."""
from pathlib import Path
import ast
import hashlib
import json
import math
import re
import struct
from urllib.parse import unquote, urlsplit

ROOT = Path(__file__).resolve().parents[1]
errors = []
counts = {'source_files': 0, 'cad_files': 0, 'attachments': 0, 'python_files': 0, 'local_links': 0}

def check(condition, message):
    if not condition:
        errors.append(message)

def verify(path, expected_hash, category, git_blob=None):
    check(path.is_file(), f'Missing file: {path.relative_to(ROOT)}')
    if not path.is_file():
        return
    data = path.read_bytes()
    check(hashlib.sha256(data).hexdigest() == expected_hash, f'Hash mismatch: {path.relative_to(ROOT)}')
    if git_blob:
        value = hashlib.sha1(b'blob ' + str(len(data)).encode() + b'\0' + data).hexdigest()
        check(value == git_blob, f'Git blob mismatch: {path.relative_to(ROOT)}')
    counts[category] += 1

for folder in ['firmware/v4', 'firmware/v4-pro', 'software/v4-topside']:
    manifest_path = ROOT / folder / 'UPSTREAM.json'
    check(manifest_path.is_file(), f'Missing manifest: {folder}')
    if not manifest_path.is_file():
        continue
    for item in json.loads(manifest_path.read_text(encoding='utf-8-sig'))['files']:
        verify(ROOT / folder / item['path'], item['sha256'], 'source_files', item.get('git_blob'))

for archive in json.loads((ROOT / 'hardware/assets-manifest.json').read_text(encoding='utf-8')):
    for item in archive['files']:
        if item['repository_path']:
            verify(ROOT / item['repository_path'], item['sha256'], 'cad_files')
for item in json.loads((ROOT / 'source-manifest.json').read_text(encoding='utf-8'))['attachments']:
    verify(ROOT / item['path'], item['sha256'], 'attachments')

for path in (ROOT / 'software').rglob('*.py'):
    try:
        compile(path.read_bytes(), str(path), 'exec')
        counts['python_files'] += 1
    except (SyntaxError, UnicodeError) as exc:
        errors.append(f'Syntax error: {path.relative_to(ROOT)}: {exc}')

documents = list((ROOT / 'docs').glob('*.md')) + list(ROOT.glob('*.md'))
documents += [ROOT / 'hardware/README.md']
documents += list((ROOT / 'firmware').glob('*/IMPORT.md'))
documents += [ROOT / 'firmware/v4-pro/variants/README.md']
documents += list((ROOT / 'software').glob('*/IMPORT.md'))
documents += [ROOT / 'software/v4-pro-jetson/README.md']
for path in documents:
    if not path.exists():
        errors.append(f'Missing document: {path.relative_to(ROOT)}')
        continue
    text = re.sub(r'```.*?```', '', path.read_text(encoding='utf-8-sig'), flags=re.S)
    for target in re.findall(r'!?\[[^\]]*\]\(([^)]+)\)', text):
        target = target.strip().strip('<>')
        if urlsplit(target).scheme or target.startswith('#'):
            continue
        local = unquote(target.split('#', 1)[0])
        if local:
            check((path.parent / local).exists(), f'Broken link: {path.relative_to(ROOT)} -> {local}')
            counts['local_links'] += 1

# Extract only reviewed pure definitions; never execute module-level serial/socket setup.
source = ROOT / 'software/v4-pro-jetson/upstream/V5StreamerNX.py'
tree = ast.parse(source.read_text(encoding='utf-8-sig'))
names = {'JoystickState', 'crc16_modbus', 'normalize_value', 'forward_to_serial'}
definitions = [n for n in tree.body if isinstance(n, (ast.FunctionDef, ast.ClassDef)) and n.name in names]
check({n.name for n in definitions} == names, 'Missing protocol definitions')
ns = {'struct': struct, 'serial_send_count': 0}
exec(compile(ast.Module(body=definitions, type_ignores=[]), '<isolated-protocol>', 'exec'), ns)
check(ns['crc16_modbus'](b'123456789') == 0x4B37, 'CRC16/Modbus check vector failed')
check(struct.calcsize('6f15B2b') == 41, 'Host native joystick layout is not 41 bytes')
check(struct.calcsize('5f3B2b') == 25, 'Host native payload layout is not 25 bytes')

class MemorySerial:
    is_open = True
    def write(self, value):
        self.data = value

serial = MemorySerial()
ns['ser'] = serial
buttons = [0] * 15
buttons[0] = buttons[6] = 1
axes = [-0.25, 0.0, 0.75, -0.5, 0.5, -0.5]
state = ns['JoystickState']()
state.from_bytes(struct.pack('6f15B2b', *axes, *buttons, -1, 1))
ok, payload = ns['forward_to_serial'](state)
check(ok, 'In-memory serial encoding failed')
if ok:
    values = struct.unpack('5f3B2b', payload)
    expected = [0.25, 0.5, -0.75, 0.75, 0.25, 1, 1, 0, -1, 1]
    check(all(math.isclose(a, b) for a, b in zip(values, expected)), 'Joystick field mapping mismatch')
    check(len(serial.data) == 28 and serial.data[0] == 0xAA, 'UART frame header/length mismatch')
    check(struct.unpack('<H', serial.data[-2:])[0] == ns['crc16_modbus'](serial.data[:-2]), 'Frame CRC mismatch')

print(json.dumps({'passed': not errors, 'checks': counts, 'protocol': 'offline-memory-only', 'errors': errors}, ensure_ascii=True, indent=2))
raise SystemExit(bool(errors))
