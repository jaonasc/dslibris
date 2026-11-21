"""Run a homebrew ROM headlessly through the real DeSmuME libretro core.

Usage: python3 test/emulator_smoke.py CORE ROM OUTPUT_DIR
Set DSLIBRIS_TEST_FAT to a disposable directory containing Font and Book.
Snapshots are raw emulator video, not mockups. No desktop UI is driven.
"""
import ctypes as c
import pathlib
import sys
import struct
import zlib
import os

core=c.CDLL(sys.argv[1])
out=pathlib.Path(sys.argv[3]);out.mkdir(parents=True,exist_ok=True)
pixel_format=0
video=None
pressed=set()
variables={}
directory=str(out).encode()
class Variable(c.Structure):
    _fields_=[('key',c.c_char_p),('value',c.c_char_p)]
class Game(c.Structure):
    _fields_=[('path',c.c_char_p),('data',c.c_void_p),('size',c.c_size_t),('meta',c.c_char_p)]
@c.CFUNCTYPE(None,c.c_int,c.c_char_p)
def log(level,message):
    pass # The core requires a non-null logger. Variadic arguments are unused.
@c.CFUNCTYPE(c.c_bool,c.c_uint,c.c_void_p)
def environment(command,data):
    global pixel_format
    if command==27:
        c.cast(data,c.POINTER(c.c_void_p))[0]=c.cast(log,c.c_void_p).value;return True
    if command in (9,31):
        c.cast(data,c.POINTER(c.c_char_p))[0]=directory;return True
    if command==10:
        pixel_format=c.cast(data,c.POINTER(c.c_int))[0];return True
    if command==16:
        vals=c.cast(data,c.POINTER(Variable));i=0
        while vals[i].key:
            desc=vals[i].value.decode()
            variables[vals[i].key]=desc.split(';',1)[1].strip().split('|')[0].encode()
            i+=1
        variables[b'desmume_cpu_mode']=b'interpreter'
        variables[b'desmume_internal_resolution']=b'256x192'
        variables[b'desmume_screens_layout']=b'top/bottom'
        return True
    if command==15:
        var=c.cast(data,c.POINTER(Variable))
        if var[0].key in variables:
            var[0].value=variables[var[0].key];return True
        return False
    if command==17:
        c.cast(data,c.POINTER(c.c_bool))[0]=False;return True
    if command==52:
        c.cast(data,c.POINTER(c.c_uint))[0]=0;return True
    if command==39:
        c.cast(data,c.POINTER(c.c_uint))[0]=0;return True
    return False
@c.CFUNCTYPE(None,c.c_void_p,c.c_uint,c.c_uint,c.c_size_t)
def refresh(data,width,height,pitch):
    global video
    if data and data!=c.c_void_p(-1).value:
        video=(c.string_at(data,pitch*height),width,height,pitch,pixel_format)
@c.CFUNCTYPE(None,c.c_int16,c.c_int16)
def audio(left,right): pass
@c.CFUNCTYPE(c.c_size_t,c.c_void_p,c.c_size_t)
def audio_batch(data,frames):return frames
@c.CFUNCTYPE(None)
def poll(): pass
@c.CFUNCTYPE(c.c_int16,c.c_uint,c.c_uint,c.c_uint,c.c_uint)
def input_state(port,device,index,button):
    return int(device==1 and button in pressed)
for name,fn in [('environment',environment),('video_refresh',refresh),('audio_sample',audio),
                ('audio_sample_batch',audio_batch),('input_poll',poll),('input_state',input_state)]:
    getattr(core,'retro_set_'+name)(fn)
core.retro_load_game.argtypes=[c.POINTER(Game)];core.retro_load_game.restype=c.c_bool
print('Initializing emulator',flush=True)
core.retro_init()
game=Game(str(pathlib.Path(sys.argv[2]).resolve()).encode(),None,0,None)
print('Loading ROM',flush=True)
assert core.retro_load_game(c.byref(game)), 'ROM load failed'
print('Running native ARM code',flush=True)
def run(count):
    for _ in range(count):core.retro_run()
def key(button):
    pressed.add(button);run(4);pressed.clear();run(160)
def snapshot(name):
    assert video,'No emulator video'
    data,w,h,pitch,fmt=video
    output=bytearray()
    bpp=4 if fmt==1 else 2
    for y in range(h):
        for x in range(w):
            pos=y*pitch+x*bpp
            value=int.from_bytes(data[pos:pos+bpp],'little')
            if fmt==1:r,g,b=(value>>16)&255,(value>>8)&255,value&255
            elif fmt==2:r,g,b=((value>>11)&31)*255//31,((value>>5)&63)*255//63,(value&31)*255//31
            else:r,g,b=((value>>10)&31)*255//31,((value>>5)&31)*255//31,(value&31)*255//31
            output.extend((r,g,b))
    (out/(name+'.ppm')).write_bytes(('P6\n%d %d\n255\n'%(w,h)).encode()+output)
    # Display the emulator's two screens in the reader's book orientation.
    rotated=bytearray()
    for y in range(w):
        rotated.append(0) # PNG filter: none
        for x in range(h):
            pos=((h-1-x)*w+y)*3 if os.environ.get('DSLIBRIS_TEST_FLIP')=='1' else (x*w+(w-1-y))*3
            rotated.extend(output[pos:pos+3])
    def chunk(kind,payload):
        return struct.pack('>I',len(payload))+kind+payload+struct.pack('>I',zlib.crc32(kind+payload)&0xffffffff)
    png=b'\x89PNG\r\n\x1a\n'+chunk(b'IHDR',struct.pack('>IIBBBBB',h,w,8,2,0,0,0))
    png+=chunk(b'IDAT',zlib.compress(rotated))+chunk(b'IEND',b'')
    (out/(name+'.png')).write_bytes(png)
    print('Snapshot',name,w,h,flush=True)
run(240);snapshot('01-library')
mode=os.environ.get('DSLIBRIS_TEST_MODE','normal')
if mode in ('empty','invalid'):
    key(8);snapshot('02-safe-open-failure')
    if mode=='invalid':
        key(7 if os.environ.get('DSLIBRIS_TEST_FLIP')=='1' else 6)
        key(8);snapshot('03-valid-book-after-failure')
    core.retro_unload_game();core.retro_deinit()
    print('PASS: native failure flow captured: '+mode,flush=True)
    raise SystemExit(0)
key(2);snapshot('02-settings') # SELECT
key(8);snapshot('03-next-theme') # A
key(0);key(8);run(180);snapshot('04-reading') # B, A
first_page=video[0]
key(8);snapshot('05-next-page')
assert video[0]!=first_page,'Forward page turn did not change the spread'
key(0);snapshot('06-previous-page')
assert video[0]==first_page,'Backward page turn did not restore the first spread'
key(9);snapshot('07-reading-theme') # X
assert video[0]!=first_page,'Theme shortcut did not change reading colors'
key(9);snapshot('08-black-reading')
key(9);snapshot('09-paper-reading')
key(3);key(2) # START, SELECT
menu_next=7 if os.environ.get('DSLIBRIS_TEST_FLIP')=='1' else 6
for _ in range(4):key(menu_next) # Settings page 2, regular font
key(8);snapshot('10-fonts')
for _ in range(5):key(menu_next) # Deliberately invalid font added after the five bundled fonts
key(8);snapshot('11-invalid-font')
key(0);key(0);key(8);snapshot('12-reading-after-invalid-font')
reopened_page=video[0]
for repeat in range(5):
    key(3);key(8)
    assert video[0]==reopened_page,'Repeated open did not restore the same spread'
snapshot('13-reading-after-repeated-open')
print('PASS: native EPUB forward/backward, theme shortcut; font failure flow captured',flush=True)
core.retro_unload_game();core.retro_deinit()
