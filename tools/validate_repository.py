"""Offline snapshot checks. Does not import runtime modules or open devices/network."""
from pathlib import Path
import ast
import csv
import hashlib
import json
import math
import posixpath
import re
import struct
import xml.etree.ElementTree as ET
import zipfile
from urllib.parse import unquote, urlsplit

ROOT = Path(__file__).resolve().parents[1]
errors = []
counts = {'source_files': 0, 'cad_files': 0, 'attachments': 0, 'python_files': 0, 'local_links': 0, 'bilingual_documents': 0, 'bom_items': 0, 'bom_sheets': 0}

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
documents += list((ROOT / 'hardware').glob('README*.md'))
documents += list((ROOT / 'firmware').glob('*/IMPORT*.md'))
documents += list((ROOT / 'firmware/v4-pro/variants').glob('README*.md'))
documents += list((ROOT / 'software').glob('*/IMPORT*.md'))
documents += list((ROOT / 'software/v4-pro-jetson').glob('README*.md'))
for path in documents:
    if not path.exists():
        errors.append(f'Missing document: {path.relative_to(ROOT)}')
        continue
    text = re.sub(r'```.*?```', '', path.read_text(encoding='utf-8-sig'), flags=re.S)
    if '.en.' not in path.name:
        english = path.with_name(path.stem + '.en.md')
        check(english.is_file(), f'Missing English companion: {path.relative_to(ROOT)}')
        if english.is_file():
            check(english.name in text, f'Missing language navigation: {path.relative_to(ROOT)}')
            check(path.name in english.read_text(encoding='utf-8-sig'), f'Missing Chinese navigation: {english.relative_to(ROOT)}')
            counts['bilingual_documents'] += 1
    for target in re.findall(r'!?\[[^\]]*\]\(([^)]+)\)', text):
        target = target.strip().strip('<>')
        if urlsplit(target).scheme or target.startswith('#'):
            continue
        local = unquote(target.split('#', 1)[0])
        if local:
            check((path.parent / local).exists(), f'Broken link: {path.relative_to(ROOT)} -> {local}')
            counts['local_links'] += 1

# Reconcile delivered CSV and XLSX values to the bilingual BOM dataset.
# Read OOXML directly using the standard library; no Excel installation is needed.
bom_dir = ROOT / 'hardware/bom'
bom = json.loads((bom_dir / 'bom-data.json').read_text(encoding='utf-8-sig'))
for entry in (bom_dir / 'SHA256SUMS.txt').read_text(encoding='utf-8').splitlines():
    digest, filename = entry.split('  ', 1)
    check(hashlib.sha256((bom_dir / filename).read_bytes()).hexdigest() == digest, f'BOM checksum mismatch: {filename}')
items = bom['items']
sources = {source['id']: source for source in bom['sources']}
check(len({item['id'] for item in items}) == len(items), 'Duplicate BOM item IDs')
for item in items:
    check(item['variant'] in {'V4', 'V4-pro'}, f"Unknown BOM variant: {item['id']}")
    check(item['quantity'] is None or (type(item['quantity']) in (int, float) and item['quantity'] > 0), f"Invalid BOM quantity: {item['id']}")
    for lang in ('zh', 'en'):
        for field in ('category', 'item', 'unit', 'spec', 'status'):
            check(bool(item[f'{field}_{lang}']), f"Missing {field}_{lang}: {item['id']}")
        if item['quantity'] is None:
            check(bool(item[f'quantity_note_{lang}']), f"Unexplained unknown quantity: {item['id']} {lang}")
    check(bool(item['source_ids']) and all(s in sources for s in item['source_ids']), f"Invalid BOM provenance: {item['id']}")
counts['bom_items'] = len(items)
csv_fields = ['id', 'variant', 'category_zh', 'category_en', 'item_zh', 'item_en', 'quantity', 'unit_zh', 'unit_en', 'quantity_note_zh', 'quantity_note_en', 'spec_zh', 'spec_en', 'status_zh', 'status_en', 'purchase_url', 'source_ids', 'source_urls']
for variant in ('V4', 'V4-pro'):
    selected = [item for item in items if item['variant'] == variant]
    with (bom_dir / f'{variant}-BOM.csv').open(encoding='utf-8-sig', newline='') as handle:
        rows = list(csv.reader(handle))
    expected_rows = []
    for item in selected:
        row = []
        for field in csv_fields:
            if field == 'source_ids':
                value = '; '.join(item['source_ids'])
            elif field == 'source_urls':
                value = '; '.join(sources[s]['url'] for s in item['source_ids'])
            else:
                value = item[field]
            row.append('' if value is None else str(value))
        expected_rows.append(row)
    check(len(rows[0]) == len(csv_fields), f'{variant} CSV column count')
    check(rows[1:] == expected_rows, f'{variant} CSV differs from BOM data')

ns_xml = {'m': 'http://schemas.openxmlformats.org/spreadsheetml/2006/main'}
rel_attr = '{http://schemas.openxmlformats.org/officeDocument/2006/relationships}id'
with zipfile.ZipFile(bom_dir / 'V4-V4pro-BOM.xlsx') as archive:
    shared = []
    if 'xl/sharedStrings.xml' in archive.namelist():
        for si in ET.fromstring(archive.read('xl/sharedStrings.xml')).findall('m:si', ns_xml):
            shared.append(''.join(t.text or '' for t in si.findall('.//m:t', ns_xml)))
    def cell_value(cell):
        value = cell.find('m:v', ns_xml)
        if cell.get('t') == 's':
            return shared[int(value.text)] if value is not None else ''
        if cell.get('t') == 'inlineStr':
            return ''.join(t.text or '' for t in cell.findall('.//m:t', ns_xml))
        if value is None or value.text is None:
            return ''
        if cell.get('t') in {'str', 'e'}:
            return value.text
        return float(value.text)
    relationships = {rel.get('Id'): rel.get('Target') for rel in ET.fromstring(archive.read('xl/_rels/workbook.xml.rels'))}
    sheets = ET.fromstring(archive.read('xl/workbook.xml')).findall('m:sheets/m:sheet', ns_xml)
    expected_sheets = [('V4', 'zh', 'V4 中文'), ('V4', 'en', 'V4 English'), ('V4-pro', 'zh', 'V4-pro 中文'), ('V4-pro', 'en', 'V4-pro English')]
    check([sheet.get('name') for sheet in sheets] == [entry[2] for entry in expected_sheets], 'Unexpected BOM worksheet names/order')
    for sheet, (variant, lang, name) in zip(sheets, expected_sheets):
        target = relationships[sheet.get(rel_attr)]
        target = target.lstrip('/') if target.startswith('/') else posixpath.normpath(posixpath.join('xl', target))
        xml = ET.fromstring(archive.read(target))
        cells = {cell.get('r'): cell_value(cell) for cell in xml.findall('.//m:sheetData/m:row/m:c', ns_xml)}
        selected = [item for item in items if item['variant'] == variant]
        link_rel_path = posixpath.join(posixpath.dirname(target), '_rels', posixpath.basename(target) + '.rels')
        link_relationships = {rel.get('Id'): rel.get('Target') for rel in ET.fromstring(archive.read(link_rel_path))}
        hyperlinks = {link.get('ref'): link_relationships[link.get(rel_attr)] for link in xml.findall('m:hyperlinks/m:hyperlink', ns_xml)}
        for number, item in enumerate(selected, 7):
            expected = [item['id'], item[f'category_{lang}'], item[f'item_{lang}'], item['quantity'] if item['quantity'] is not None else '', item[f'unit_{lang}'], item[f'spec_{lang}'], item[f'quantity_note_{lang}'], item[f'status_{lang}'], item['purchase_url'] or '', '\n'.join(sources[s][f'title_{lang}'] for s in item['source_ids']), '\n'.join(sources[s]['url'] for s in item['source_ids'])]
            actual = [cells.get(f'{chr(65 + column)}{number}', '') for column in range(11)]
            check(actual == expected, f'XLSX data mismatch: {name}, {item["id"]}')
            if item['purchase_url']:
                check(hyperlinks.get(f'I{number}') == item['purchase_url'], f'XLSX supplier hyperlink mismatch: {name}, {item["id"]}')
        check(xml.find('m:sheetViews/m:sheetView/m:pane', ns_xml) is not None, f'Missing frozen headers: {name}')
        check(xml.find('m:tableParts/m:tablePart', ns_xml) is not None, f'Missing filterable table: {name}')
        counts['bom_sheets'] += 1

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
