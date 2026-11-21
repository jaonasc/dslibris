"""Run production parser/book/storage code under ASan/UBSan on the host."""
from pathlib import Path
import shutil
import subprocess
import zipfile
import tempfile
root=Path(__file__).resolve().parent.parent
cases=['lifecycle','depth','markers','invalid-epub','empty-epub','repeat-epub','xml','xml-clamp','xml-recovery','logging']
failed=[]
for case in cases:
    folder=Path(tempfile.mkdtemp(prefix=case+'-',dir='/tmp/dslibris-host'))
    shutil.copy2(root/'etc/filesystem/en/Book/quickstart.epub',folder/'good.epub')
    (folder/'bad.epub').write_bytes(b'broken EPUB')
    with zipfile.ZipFile(folder/'empty.epub','w') as z:
        z.writestr('META-INF/container.xml','<container><rootfiles><rootfile full-path="content.opf"/></rootfiles></container>')
        z.writestr('content.opf','<package><manifest/><spine/></package>')
    result=subprocess.run(['/tmp/dslibris-host/stability-test',case],cwd=folder,capture_output=True,text=True,timeout=30)
    (folder/'result.log').write_text(result.stdout+result.stderr)
    print(case,'PASS' if result.returncode==0 else 'FAIL',flush=True)
    if result.returncode:
        failed.append(case)
        print('\n'.join((result.stdout+result.stderr).splitlines()[:7]),flush=True)
print('RESULT:',len(cases)-len(failed),'/',len(cases),'passed',flush=True)
raise SystemExit(bool(failed))
