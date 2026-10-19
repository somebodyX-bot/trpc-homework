"""Verify pinned source archives and unpack them for an offline CMake build."""
import hashlib
import json
from pathlib import Path
import sys
import tarfile

vendor = Path(sys.argv[1]).resolve()
root = Path(sys.argv[2]).resolve()
entries = json.loads((vendor / 'sources.json').read_text())
options = []
for entry in entries:
    archive = vendor / (entry['name'] + '.tar.gz')
    actual = hashlib.sha256(archive.read_bytes()).hexdigest()
    if actual != entry['sha256']:
        raise SystemExit('SHA256 mismatch: ' + archive.name)
    target = root if entry['name'] == 'trpc' else root / 'cmake_third_party' / entry['name']
    target.mkdir(parents=True, exist_ok=True)
    with tarfile.open(archive) as source:
        for member in source.getmembers():
            # Documentation and non-C++ test links are not needed by this build.
            if member.issym() or member.islnk():
                continue
            parts = Path(member.name).parts[1:]
            if not parts:
                continue
            member.name = str(Path(*parts))
            if not (target / member.name).resolve().is_relative_to(target):
                raise SystemExit('Unsafe archive path')
            source.extract(member, target, filter='data')
    if entry['cmake_name']:
        options.append(f'set(FETCHCONTENT_SOURCE_DIR_{entry["cmake_name"].upper()} "{target.as_posix()}" CACHE PATH "Pinned source" FORCE)')
(root / 'offline.cmake').write_text('\n'.join(options) + '\n')
print('Verified and extracted', len(entries), 'source archives')
