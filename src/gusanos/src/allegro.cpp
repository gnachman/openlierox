/*
 *  allegro.cpp
 *  Gusanos
 *
 *  Created by Albert Zeyer on 30.11.09.
 *  code under LGPL
 *
 */

#include <SDL.h>
#include <SDL_image.h>
#include "allegro.h"
#include "FindFile.h"
#include "Debug.h"
#include "GfxPrimitives.h"
#include "StringUtils.h"



static BITMAP *create_bitmap_from_sdl(SDL_Surface* surf) {	
	BITMAP* bmp = new BITMAP();

	bmp->surf = surf;
	bmp->w = surf->w;
	bmp->h = surf->h;
	bmp->cl = 0;
	bmp->cr = surf->w;
	bmp->ct = 0;
	bmp->cb = surf->h;

	bmp->line = new unsigned char* [surf->h];
	for(int y = 0; y < surf->h; ++y)
		bmp->line[y] = (Uint8 *)surf->pixels + (y * surf->pitch);

	return bmp;
}

BITMAP *load_bitmap(const char *filename, RGB *pal) {
	SDL_Surface* img = IMG_Load(filename);
	if(!img) return NULL;
	return create_bitmap_from_sdl(img);
}

BITMAP *create_bitmap_ex(int color_depth, int width, int height) {
	SDL_Surface* surf = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, color_depth, 0,0,0,0);
	if(!surf) return NULL;
	return create_bitmap_from_sdl(surf);
}

BITMAP *create_bitmap(int width, int height) {
	return create_bitmap_ex(16, width, height);
}

BITMAP *create_sub_bitmap(BITMAP *parent, int x, int y, int width, int height) { return NULL; }

void destroy_bitmap(BITMAP *bmp) {
	if(bmp == NULL) return;
	SDL_FreeSurface(bmp->surf);
	delete[] bmp->line;
	delete bmp;
}



int set_gfx_mode(int card, int w, int h, int v_w, int v_h) { return 0; }
int SCREEN_W = 640, SCREEN_H = 480;

BITMAP* screen = NULL;

int allegro_error = 0;





SDL_PixelFormat defaultFallbackFormat =
	{
         NULL, //SDL_Palette *palette;
         32, //Uint8  BitsPerPixel;
         4, //Uint8  BytesPerPixel;
         0, 0, 0, 0, //Uint8  Rloss, Gloss, Bloss, Aloss;
         24, 16, 8, 0, //Uint8  Rshift, Gshift, Bshift, Ashift;
         0xff000000, 0xff0000, 0xff00, 0xff, //Uint32 Rmask, Gmask, Bmask, Amask;
         0, //Uint32 colorkey;
         255 //Uint8  alpha;
	};

SDL_PixelFormat* mainPixelFormat = &defaultFallbackFormat;


///////////////////
// Set the video mode
static bool SetVideoMode(int bpp = 32) {
	bool resetting = false;

	int DoubleBuf = false;
	int vidflags = 0;

	// Check that the bpp is valid
	switch (bpp) {
	case 0:
	case 16:
	case 24:
	case 32:
		break;
	default: bpp = 16;
	}
	notes << "ColorDepth: " << bpp << endl;

	// BlueBeret's addition (2007): OpenGL support
	bool opengl = false; //tLXOptions->bOpenGL;

	// Initialize the video
	if(/*tLXOptions->bFullscreen*/ false)  {
		vidflags |= SDL_FULLSCREEN;
	}

	if (opengl) {
		vidflags |= SDL_OPENGL;
		vidflags |= SDL_OPENGLBLIT; // SDL will behave like normally
		SDL_GL_SetAttribute (SDL_GL_DOUBLEBUFFER, 1); // always use double buffering in OGL mode
	}	

	vidflags |= SDL_SWSURFACE;

	if(DoubleBuf && !opengl)
		vidflags |= SDL_DOUBLEBUF;

#ifdef WIN32
	UnSubclassWindow();  // Unsubclass before doing anything with the window
#endif

#ifdef WIN32
	static bool firsttime = true;
	// Reset the video subsystem under WIN32, else we get a "Could not reset OpenGL context" error when switching mode
	if (opengl && !firsttime)  {  // Don't reset when we're setting up the mode for first time
		SDL_QuitSubSystem(SDL_INIT_VIDEO);
		SDL_InitSubSystem(SDL_INIT_VIDEO);
	}
	firsttime = false;
#endif

	int scrW = SCREEN_W;
	int scrH = SCREEN_H;
setvideomode:
	if( SDL_SetVideoMode(scrW, scrH, bpp, vidflags) == NULL) {
		if (resetting)  {
			errors << "Failed to reset video mode"
					<< " (ErrorMsg: " << SDL_GetError() << "),"
					<< " let's wait a bit and retry" << endl;
			SDL_Delay(500);
			resetting = false;
			goto setvideomode;
		}

		if(bpp != 0) {
			errors << "Failed to use " << bpp << " bpp"
					<< " (ErrorMsg: " << SDL_GetError() << "),"
					<< " trying automatic bpp detection ..." << endl;
			bpp = 0;
			goto setvideomode;
		}

		if(vidflags & SDL_OPENGL) {
			errors << "Failed to use OpenGL"
					<< " (ErrorMsg: " << SDL_GetError() << "),"
					<< " trying without ..." << endl;
			vidflags &= ~(SDL_OPENGL | SDL_OPENGLBLIT | SDL_HWSURFACE | SDL_HWPALETTE | SDL_HWACCEL);
			goto setvideomode;
		}
		
		if(vidflags & SDL_FULLSCREEN) {
			errors << "Failed to set full screen video mode "
					<< scrW << "x" << scrH << "x" << bpp
					<< " (ErrorMsg: " << SDL_GetError() << "),"
					<< " trying window mode ..." << endl;
			vidflags &= ~SDL_FULLSCREEN;
			goto setvideomode;
		}

		errors << "Failed to set the video mode " << scrW << "x" << scrH << "x" << bpp << endl;
		errors << "nErrorMsg: " << std::string(SDL_GetError()) << endl;
		return false;
	}

	SDL_WM_SetCaption("Gusanos", NULL);
	SDL_ShowCursor(SDL_DISABLE);

#ifdef WIN32
	if (false/*!tLXOptions->bFullscreen*/)  {
		SubclassWindow();
	}
#endif

	mainPixelFormat = SDL_GetVideoSurface()->format;
	//DumpPixelFormat(mainPixelFormat);
	if(SDL_GetVideoSurface()->flags & SDL_DOUBLEBUF)
		notes << "using doublebuffering" << endl;

	if(SDL_GetVideoSurface()->flags & SDL_OPENGL)
		hints << "using OpenGL" << endl;
	
	FillSurface(SDL_GetVideoSurface(), Color(0, 0, 0));
	
	notes << "video mode was set successfully" << endl;
	return true;
}

static bool sdl_video_init() {
	if(getenv("SDL_VIDEODRIVER"))
		notes << "SDL_VIDEODRIVER=" << getenv("SDL_VIDEODRIVER") << endl;

	// Solves problem with FPS in fullscreen
#ifdef WIN32
	if(!getenv("SDL_VIDEODRIVER")) {
		notes << "SDL_VIDEODRIVER not set, setting to directx" << endl;
		putenv((char*)"SDL_VIDEODRIVER=directx");
	}
#endif

	// Initialize SDL
	int SDLflags = SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE;
	if(SDL_Init(SDLflags) == -1) {
		errors << "Failed to initialize the SDL system!\nErrorMsg: " << std::string(SDL_GetError()) << endl;
#ifdef WIN32
		// retry it with any available video driver
		unsetenv("SDL_VIDEODRIVER");
		if(SDL_Init(SDLflags) != -1)
			hints << "... but we have success with the any driver" << endl;
		// retry with windib
		else if(putenv((char*)"SDL_VIDEODRIVER=windib") == 0 && SDL_Init(SDLflags) != -1)
			hints << "... but we have success with the windib driver" << endl;
		else
#endif
			return false;
	}

	if(!SetVideoMode())
		return false;

    // Enable the system events
	SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);
	SDL_EventState(SDL_VIDEOEXPOSE, SDL_ENABLE);

	// Enable unicode and key repeat
	SDL_EnableUNICODE(1);
	SDL_EnableKeyRepeat(200,20);

	return true;
}

void allegro_init() {
	if(!sdl_video_init()) {
		errors << "Allegro init: video init failed" << endl;
		return;
	}

	screen = create_bitmap(SCREEN_W, SCREEN_H);
}

void allegro_exit() {
	destroy_bitmap(screen);
	screen = NULL;
}

void rest(int t) { SDL_Delay(t); }
void vsync() {}


void install_timer() {}
int install_int_ex(void (*proc)(), long speed) { return 0; }



void install_mouse() {}
void remove_mouse() {}

volatile int mouse_x;
volatile int mouse_y;
volatile int mouse_z;
volatile int mouse_b;
void (*mouse_callback)(int flags);


int poll_mouse() { return 0; }





void acquire_screen() {}
void release_screen() {}





int set_display_switch_mode(int mode) { return 0; }



bool exists(const char* filename) { return IsFileAvailable(filename, true); }



int makecol(int r, int g, int b) { return SDL_MapRGB(mainPixelFormat,r,g,b); }
int makecol_depth(int color_depth, int r, int g, int b) { return 0; }


int _rgb_r_shift_15, _rgb_g_shift_15, _rgb_b_shift_15,
    _rgb_r_shift_16, _rgb_g_shift_16, _rgb_b_shift_16,
    _rgb_r_shift_24, _rgb_g_shift_24, _rgb_b_shift_24,
    _rgb_r_shift_32, _rgb_g_shift_32, _rgb_b_shift_32;

int _rgb_scale_5[32], _rgb_scale_6[64];

void rgb_to_hsv(int r, int g, int b, float *h, float *s, float *v) {}





int getpixel(BITMAP *bmp, int x, int y) {
	switch(bmp->surf->format->BitsPerPixel) {
		case 8: return /*bmp->surf->format->palette->colors[*/ bmp->line[y][x] /*]*/;
		case 32: return * ((Uint32*) bmp->line[y] + x);
	}
	return 0;
}
void putpixel(BITMAP *bmp, int x, int y, int color) {
	switch(bmp->surf->format->BitsPerPixel) {
		case 8: bmp->line[y][x] = color;
		case 32: * ((Uint32*) bmp->line[y] + x) = color;
	}
}

void vline(BITMAP *bmp, int x, int y1, int y2, int color) {}
void hline(BITMAP *bmp, int x1, int y, int x2, int color) {}
void line(BITMAP *bmp, int x1, int y1, int x2, int y2, int color) {}
void rectfill(BITMAP *bmp, int x1, int y1, int x2, int y2, int color) {}
void circle(BITMAP *bmp, int x, int y, int radius, int color) {}
void clear_to_color(struct BITMAP *bitmap, int color) {}
void draw_sprite(BITMAP *bmp, BITMAP *sprite, int x, int y) {}
void draw_sprite_h_flip(struct BITMAP *bmp, struct BITMAP *sprite, int x, int y) {}

void blit(BITMAP *source, BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height) {
	
}

void stretch_blit(BITMAP *s, BITMAP *d, int s_x, int s_y, int s_w, int s_h, int d_x, int d_y, int d_w, int d_h) {}
void masked_blit(BITMAP *source, BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height) {}

void clear_bitmap(BITMAP*) {}



unsigned long bmp_write_line(BITMAP *bmp, int line) {
	return (unsigned long) bmp->line[line];
}

void bmp_unwrite_line(BITMAP* bmp) {}




void drawing_mode(int mode, BITMAP *pattern, int x_anchor, int y_anchor) {}
void set_trans_blender(int r, int g, int b, int a) {}
void set_add_blender (int r, int g, int b, int a) {}
void solid_mode() {}

int getr(int c) { Uint8 r,g,b; SDL_GetRGB(c, mainPixelFormat, &r, &g, &b); return r; }
int getg(int c) { Uint8 r,g,b; SDL_GetRGB(c, mainPixelFormat, &r, &g, &b); return g; }
int getb(int c) { Uint8 r,g,b; SDL_GetRGB(c, mainPixelFormat, &r, &g, &b); return b; }

int get_color_conversion() { return 0; }
void set_color_conversion(int mode) {}

int get_color_depth() { return 0; }
void set_color_depth(int depth) {}


void set_clip_rect(BITMAP *bitmap, int x1, int y_1, int x2, int y2) {}
void get_clip_rect(BITMAP *bitmap, int *x1, int *y_1, int *x2, int *y2) {}








void install_keyboard() {}
void remove_keyboard() {}
bool keypressed() { return false; }
int readkey() { return 0; }
int key[KEY_MAX];

void clear_keybuf() {}


