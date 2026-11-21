from pathlib import Path
import shutil
import subprocess
root=Path(__file__).resolve().parent.parent
folder=Path('/tmp/dslibris-host/font-case');folder.mkdir(exist_ok=True)
shutil.copytree(root/'test/Font',folder/'Font',dirs_exist_ok=True)
(folder/'Font/invalid.ttf').write_text('invalid font test')
result=subprocess.run(['/tmp/dslibris-host/font-test'],cwd=folder)
raise SystemExit(result.returncode)
