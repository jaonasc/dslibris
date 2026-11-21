"""Extract the DS SDK from the official, version-pinned devkitPro image.

Does not install a container engine or modify system configuration.
Usage: python3 scripts/fetch-sdk.py /tmp/dslibris-sdk
"""
import hashlib
import json
import pathlib
import sys
import tarfile
import urllib.request

root = pathlib.Path(sys.argv[1]).resolve()
root.mkdir(parents=True, exist_ok=True)
token = json.load(urllib.request.urlopen(
    'https://auth.docker.io/token?service=registry.docker.io&scope=repository:devkitpro/devkitarm:pull'))['token']
digest = 'bd45faf9f7df6a18bef80e6665876814f32eeeb772dbdae5ae7726b02794f1b1'
archive = root / 'sdk-layer.tar.gz'
if not archive.exists():
    req = urllib.request.Request('https://registry-1.docker.io/v2/devkitpro/devkitarm/blobs/sha256:' + digest,
                                 headers={'Authorization': 'Bearer ' + token})
    print('Downloading official devkitpro/devkitarm:20240511 SDK layer (217 MB)', flush=True)
    with urllib.request.urlopen(req) as src, archive.open('wb') as dst:
        while True:
            data = src.read(1024 * 1024)
            if not data:
                break
            dst.write(data)
assert hashlib.sha256(archive.read_bytes()).hexdigest() == digest, 'SDK digest mismatch'
prefixes = ('opt/devkitpro/devkitARM/', 'opt/devkitpro/libnds/',
            'opt/devkitpro/tools/', 'opt/devkitpro/portlibs/nds/')
with tarfile.open(archive) as tar:
    for entry in tar:
        name = entry.name.removeprefix('./')
        if not name.startswith(prefixes):
            continue
        target = (root / name).resolve()
        if not target.is_relative_to(root):
            raise ValueError('Unsafe SDK path')
        if entry.islnk() or (entry.issym() and entry.linkname.startswith('/')):
            raise ValueError('Unexpected absolute SDK link')
        tar.extract(entry, root)
print('SDK ready: ' + str(root / 'opt/devkitpro'), flush=True)
