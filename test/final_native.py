from pathlib import Path
import hashlib
import os
import subprocess
import sys
root=Path(__file__).resolve().parent.parent
base=Path('/tmp/dslibris-emulator')
env=os.environ.copy();env['DSLIBRIS_TEST_FAT']=str(base/'fat-final-native')
digest=hashlib.sha256((root/'dslibris-modern.nds').read_bytes()).hexdigest()
print('Final ROM SHA-256:',digest,flush=True)
subprocess.run([sys.executable,str(root/'test/setup_smoke.py')],env=env,check=True,stdout=subprocess.DEVNULL)
with (base/'final-native.log').open('w') as log:
    result=subprocess.run([sys.executable,'-u',str(root/'test/emulator_smoke.py'),str(base/'source/desmume/src/frontend/libretro/desmume_libretro.so'),str(base/'dslibris-modern.nds'),str(root/'previews/stability/final')],env=env,stdout=log,stderr=subprocess.STDOUT,timeout=180)
assert digest==hashlib.sha256((root/'dslibris-modern.nds').read_bytes()).hexdigest()
print('\n'.join((base/'final-native.log').read_text().splitlines()[-5:]),flush=True)
raise SystemExit(result.returncode)
