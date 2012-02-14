from libcpp cimport bool

"""
Init
[gameloop]
"""
cdef extern from "c_lib.hpp":
    int init_c_lib()
    void close_c_lib()
def init():
    init_c_lib()
def close():
    close_c_lib()

"""
Timer
[gameloopm netout]
"""
cdef extern from "../c_lib/time/physics_timer.h":
    void _START_CLOCK()
def START_CLOCK():
    _START_CLOCK()

"""
Network
[gameloop, netclient]
"""
cdef extern from "./net_lib/host.hpp":
    void client_connect_to(int a, int b, int c, int d, unsigned short port)
def NetClientConnectTo(int a, int b,int c, int d, unsigned short _port):
    client_connect_to(a, b, c, d, _port)

"""
sound
[sound]
-- needs a listdir (or hardcode each soundfile [no]) to move into C
-- dirent is probably cross platform
"""
cdef extern from "./sound/sound.hpp" namespace "Sound":
    void set_volume(float vol)
    void set_enabled(int y)
    void set_sound_path(char* path)
    void load_sound(char* file)

class Sound(object):

    @classmethod
    def init(cls, char* sound_path, sounds=[], enabled=True, float sfxvol=1.0, float musicvol=1.0):
        if not len(sounds):
            set_enabled(0)
        if sfxvol <= 0. and musicvol <= 0.:
            set_enabled(0)

        set_enabled(int(enabled))
        set_volume(sfxvol)
        set_sound_path(sound_path)

        for snd in sounds:
            load_sound(snd)

"""
SDL
[gameloop]
"""

cdef extern from "./SDL/SDL_functions.h":
    int _set_resolution(int xres, int yres, int fullscreen)
def set_resolution(xres, yres, fullscreen = 0):
    _set_resolution(xres, yres, fullscreen)

"""
Options & Settings
[options]
-- this is one of the few things to keep in cython until the end
"""

cdef extern from "./game/ctf.hpp":
    cdef cppclass CTF:
        bool auto_assign

cdef extern from "./state/client_state.hpp" namespace "ClientState":
    CTF ctf

def load_options(opts):
    ctf.auto_assign = opts.auto_assign_team

"""
Font
[hud]
-- The hud depdnency is only the calling of the init() method
-- No python is dependent on this code here; its mostly doing parsing
-- It does need to add font set metadata, however
"""
cdef extern from "./hud/font.hpp" namespace "HudFont":
    int load_font(char* filename)

    void add_glyph(
        int c,
        float x, float y,
        float xoff, float yoff,
        float w, float h,
        float xadvance
    )
    void set_missing_character(int cc)

import os.path

class Font:

    font_path = "./media/fonts/"
    font_ext = ".fnt"
    png_ext = "_0.png"
    missing_character = '?'
    glyphs = {}
    info = {}
    font = None

    ready = False
    
    @classmethod
    def init(cls):
        if not os.path.exists(cls.font_path):
            print "ERROR c_lib_fonts.pyx :: cannot find font path %s" % (cls.font_path,)
            cls.ready = False
            return

        import opts
        cls.font = cls(opts.opts.font)
        cls.font.parse()
        cls.font.load()

    def __init__(self, fn):
        self.fontfile = fn
        self.pngfile = ''
        self.process_font_filename()
        
    def process_font_filename(self):
        fn = self.fontfile
        fn = fn.split('.')[0]
        fn += self.font_ext
        fn = self.font_path + fn
        self.fontfile = fn
        if not os.path.exists(self.fontfile):
            print "ERROR c_lib_fonts.pyx :: cannot find font file %s in %s" % (fn, self.font_path,)
            self.ready = False
            return
        self.ready = True
            
    def parse(self):
        png = ""
        
        # parse .fnt
        with open(self.fontfile) as f:
            lines = f.readlines()
            for line in lines:
                line = line.strip()
                tags = line.split()
                name = tags[0]
                tags = dict(map(lambda a: a.split('='), tags[1:]))

                if name == 'page':
                    png = tags['file'].strip('"')
                elif name == 'info':
                    self.info.update(tags)
                    print "Font: %s" % (line,)
                elif name == 'common':
                    self.info.update(tags)
                    print "Font: %s" % (line,)
                elif name == 'chars':
                    print '%s characters in font set' % (tags['count'],)
                elif name == 'char':
                    self.glyphs[int(tags['id'])] = tags

        # process png filename
        if not png:
            png = self.fontfile + self.png_ext
        fp_png = self.font_path + png
        if not os.path.exists(fp_png):
            print "ERROR c_lib_fonts.pyx :: cannot find font png file %s in %s" % (fp_png, self.font_path,)
            self.ready = False
            return
        self.pngfile = fp_png

        self.clean_glyphs()
        self.missing_character_available()

    def add_glyphs_to_c(self):
        for char_code, glyph in self.glyphs.items():
            x = float(glyph['x'])
            y = float(glyph['y'])
            xoff = float(glyph['xoffset'])
            yoff = float(glyph['yoffset'])
            w = float(glyph['width'])
            h = float(glyph['height'])
            xadvance = float(glyph['xadvance'])
            add_glyph(char_code, x, y, xoff, yoff, w,h, xadvance)

            if char_code == ord(' '):
                add_glyph(ord('\t'), x,y, xoff, yoff, w,h, xadvance)
                
    def clean_glyphs(self):
        for kc, glyph in self.glyphs.items():
            for k,v in glyph.items():
                try:
                    glyph[k] = int(glyph[k])
                except ValueError:
                    pass

    def missing_character_available(self):
        cc = ord(self.missing_character)
        if cc not in self.glyphs:
            print "ERROR Missing character placeholder %s is not a known glyph" % (self.missing_character,)
            self.ready = False
            return False
        set_missing_character(cc)
        return True
        
    def load(self):
        if not load_font(self.pngfile):
            self.ready = False
            return
        self.add_glyphs_to_c()
        self.ready = True

"""
Misc
"""
cdef extern from "./state/client_state.hpp" namespace "ClientState":
    void set_desired_name(char* name)
def choose_name(name):
    set_desired_name(name)

cdef extern from "./loop.hpp":
    void main_loop()
def run_game():
    main_loop()
