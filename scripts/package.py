"""Package the new ROM and corresponding source without touching the old ROM."""
from pathlib import Path
import hashlib
import shutil
import zipfile
import argparse

root=Path(__file__).resolve().parent.parent
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--output-dir',type=Path,default=root.parent)
args=parser.parse_args()
destination=args.output_dir.resolve()
destination.mkdir(parents=True,exist_ok=True)
rom=root/'dslibris-modern.nds'
assert rom.exists(), 'Build the ROM first'
source=destination/'dslibris-modern-source.zip'
excluded={'.git','build','dist','previews','__pycache__'}
with zipfile.ZipFile(source,'w',zipfile.ZIP_DEFLATED) as archive:
    for file in sorted(root.rglob('*')):
        rel=file.relative_to(root)
        if not file.is_file() or any(part in excluded for part in rel.parts):continue
        if file.suffix in ('.nds','.elf','.map','.o','.d','.zip','.pyc'):continue
        archive.write(file,Path('dslibris-modern')/rel)
package=destination/'DSLibris-Modern.zip'
with zipfile.ZipFile(package,'w',zipfile.ZIP_DEFLATED) as archive:
    archive.write(rom,'dslibris-modern.nds')
    for name in ('LEIA-ME.txt','COPYING','VALIDACAO.md'):archive.write(root/name,name)
    for font in sorted((root/'test/Font').glob('*.ttf')):archive.write(font,Path('Font')/font.name)
    for name in ('quickstart.epub','releasenotes.epub'):
        archive.write(root/'etc/filesystem/en/Book'/name,Path('Book')/name)
    archive.write(source,'Codigo-fonte.zip')
    for preview in sorted((root/'previews').rglob('*.png')):archive.write(preview,Path('previews')/preview.relative_to(root/'previews'))
shutil.copy2(rom,destination/rom.name)
for file in (rom,source,package):
    if file.suffix=='.zip':
        with zipfile.ZipFile(file) as archive:assert archive.testzip() is None
    print(file.name,file.stat().st_size,hashlib.sha256(file.read_bytes()).hexdigest())
