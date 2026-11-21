from pathlib import Path
import shutil
import subprocess
import os

root=Path(__file__).resolve().parent.parent
base=Path('/tmp/dslibris-emulator')
fat=Path(os.environ.get('DSLIBRIS_TEST_FAT',str(base/'fat')));fat.mkdir(exist_ok=True)
shutil.copytree(root/'test/Font',fat/'Font',dirs_exist_ok=True,ignore=shutil.ignore_patterns('*.otf'))
(fat/'Font'/'invalid.ttf').write_text('This intentionally invalid font exercises safe selection.')
books=fat/'Book';books.mkdir(exist_ok=True)
paragraph='Uma boa leitura comeca com conforto. O papel amarelado e as letras suaves ajudam a acompanhar cada pagina. Este e um teste de leitura no Nintendo DS. '
if(books/'leitura.xhtml').exists():(books/'leitura.xhtml').unlink()
shutil.copy2(root/'etc/filesystem/en/Book/quickstart.epub',books/'quickstart.epub')
mode=os.environ.get('DSLIBRIS_TEST_MODE','normal')
if mode=='empty':(books/'quickstart.epub').unlink()
if mode=='invalid':(books/'000-bad.epub').write_text('Intentionally damaged EPUB')
(fat/'dslibris-modern.xml').write_text('''<dslibris>
<screen brightness="1" invert="0" flip="{flip}"/>
<reading theme="3" animation="{animation}"/>
<font size="14" normal="LiberationSerif-Regular.ttf" bold="LiberationSerif-Bold.ttf" italic="LiberationSerif-Italic.ttf"/>
<books path="Book" reopen="0"/>
</dslibris>'''.format(flip=os.environ.get('DSLIBRIS_TEST_FLIP','0'),animation=os.environ.get('DSLIBRIS_TEST_ANIMATION','1')))
rom=base/'dslibris-modern.nds'
shutil.copy2(root/'dslibris-modern.nds',rom)
subprocess.run(['/tmp/dslibris-sdk/opt/devkitpro/tools/bin/dlditool',str(root/'etc/dldi/mpcf.dldi'),str(rom)],check=True)
