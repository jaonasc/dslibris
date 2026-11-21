from pathlib import Path
import os
import subprocess
import sys
root=Path(__file__).resolve().parent.parent
base=Path('/tmp/dslibris-emulator')
core=base/'source/desmume/src/frontend/libretro/desmume_libretro.so'
cases=[('animated',{}),('instant',{'DSLIBRIS_TEST_ANIMATION':'0'}),
       ('flipped',{'DSLIBRIS_TEST_FLIP':'1','DSLIBRIS_TEST_ANIMATION':'0'}),
       ('empty',{'DSLIBRIS_TEST_MODE':'empty'}),('invalid',{'DSLIBRIS_TEST_MODE':'invalid'})]
for name,overrides in cases:
    env=os.environ.copy()
    env['DSLIBRIS_TEST_FAT']=str(base/('fat-stability-'+name))
    env.update(overrides)
    print('Testing native ROM:',name,flush=True)
    subprocess.run([sys.executable,str(root/'test/setup_smoke.py')],env=env,check=True,stdout=subprocess.DEVNULL)
    log=base/('stability-'+name+'.log')
    with log.open('w') as stream:
        result=subprocess.run([sys.executable,'-u',str(root/'test/emulator_smoke.py'),str(core),str(base/'dslibris-modern.nds'),str(root/'previews/stability'/name)],env=env,stdout=stream,stderr=subprocess.STDOUT,timeout=180)
    print(name,'PASS' if result.returncode==0 else 'FAIL',flush=True)
    if result.returncode:
        print('\n'.join(log.read_text().splitlines()[-12:]),flush=True)
        raise SystemExit(1)
print('PASS: all five native emulator scenarios',flush=True)
