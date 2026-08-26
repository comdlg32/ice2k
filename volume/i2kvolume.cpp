#include "res/leftside1.xbm"
#include "res/leftside2.xbm"

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <fx.h>
#include <FXPNGIcon.h>
#include <alsa/asoundlib.h>
#include <ice2k/i2ktray/I2KTrayApp.h>
#include <ice2k/i2ktray/I2KTrayIcon.h>
#include <ice2k/branding.h>

snd_mixer_t* s_mixer;
snd_mixer_selem_id_t* s_sid;
snd_mixer_elem_t* s_elem;
long min, max;

// #include <ice2k/comctl32.h>

#include "res/foxres.h"



// thank you stack overflow
// https://stackoverflow.com/questions/20733215/how-to-make-a-window-always-on-top
// since fox 1.6 doesnt support setting the window on top, we need to use the x11 api directly

#define _NET_WM_STATE_REMOVE        0    /* remove/unset property */
#define _NET_WM_STATE_ADD           1    /* add/set property */
#define _NET_WM_STATE_TOGGLE        2    /* toggle property  */

void mapWindowFlags(FXApp* app, FXWindow* win) {
	Display* display = (Display*)app->getDisplay();
	Window xid = (Window)win->id();


	Atom win_layer = XInternAtom(display, "_WIN_LAYER", False);
	if (win_layer == 0) return;
	long layerval = 9;

	XChangeProperty(display, xid, win_layer, XA_CARDINAL, 32, PropModeReplace,
			(unsigned char*)&layerval, 1);

	Atom wm_state = XInternAtom(display, "_NET_WM_STATE", False);
	if (wm_state == 0) return;
	Atom skip_taskbar = XInternAtom(display, "_NET_WM_STATE_SKIP_TASKBAR", False);
	if (skip_taskbar == 0) return;

	XChangeProperty(display, xid, wm_state, XA_ATOM, 32, PropModeReplace,
			(unsigned char*)&skip_taskbar, 1);
}

FXIcon* ico_volume;
//FXIcon* mainIcon;

class Volume : public FXMainWindow {
	FXDECLARE(Volume);

protected:
	Volume() {}

private:
	FXBitmap* bmp1;
	FXBitmap* bmp2;
	FXImage* img;

	FXVerticalFrame* main_frm;
    FXPopup* popup;
    I2KTrayIcon* tray;
	FXCheckButton* mute_chk;
	FXSlider* slider;
public:
	long onUnfocus(FXObject*, FXSelector, void*);
	long onChangeVol(FXObject*, FXSelector, void*);
	long onCmdMute(FXObject*, FXSelector, void*);
	long onPlaySnd(FXObject*, FXSelector, void*);


	long onCmdTray(FXObject*, FXSelector, void*);
	long onCmdVol(FXObject*, FXSelector, void*);

	long onCmdMixer(FXObject*, FXSelector, void*);
	long onMousewheelTray(FXObject*, FXSelector, void*);






public:
	enum {
		ID_VOLUME = FXMainWindow::ID_LAST,
		ID_MUTE,
		ID_TRAY,
		ID_MIXER,
		ID_LAST
	};

public:
	Volume(FXApp* a);

	virtual void create();
	void setFocus() {};
	virtual ~Volume();
};

FXDEFMAP(Volume) VolumeMap[] = {
	//FXMAPFUNC(SEL_COMMAND,           Volume::ID_HELLO,  Volume::onCmdHello),
	FXMAPFUNC(SEL_FOCUSOUT,           0,  Volume::onUnfocus),
	FXMAPFUNC(SEL_CHANGED,           Volume::ID_VOLUME,  Volume::onChangeVol),
	FXMAPFUNC(SEL_COMMAND,           Volume::ID_VOLUME,  Volume::onCmdVol),
	FXMAPFUNC(SEL_LEFTBUTTONRELEASE,           Volume::ID_VOLUME,  Volume::onPlaySnd),
	FXMAPFUNC(SEL_MIDDLEBUTTONRELEASE,           Volume::ID_VOLUME,  Volume::onPlaySnd),




	FXMAPFUNC(SEL_COMMAND,           Volume::ID_MUTE,  Volume::onCmdMute),
	FXMAPFUNC(SEL_COMMAND,           Volume::ID_TRAY,  Volume::onCmdTray),
	FXMAPFUNC(SEL_MOUSEWHEEL,        Volume::ID_TRAY,  Volume::onMousewheelTray),

	FXMAPFUNC(SEL_COMMAND,           Volume::ID_MIXER,  Volume::onCmdMixer),





};

FXIMPLEMENT(Volume, FXMainWindow, VolumeMap, ARRAYNUMBER(VolumeMap));

long Volume::onChangeVol(FXObject*, FXSelector, void* ptr) {
	snd_mixer_selem_set_playback_volume_all(s_elem, ((long)(FXival)ptr)*max/100);
	return 1;
}
long Volume::onCmdMute(FXObject*, FXSelector, void* ptr) {
	snd_mixer_selem_set_playback_switch_all(s_elem, !(long)(FXival)ptr);
	return 1;
}


long Volume::onCmdVol(FXObject*, FXSelector, void* ptr) {
	snd_mixer_selem_set_playback_volume_all(s_elem, ((long)(FXival)ptr)*max/100);
	return 1;
}

long Volume::onPlaySnd(FXObject*, FXSelector, void* ptr) {
	system("aplay -q -- \"$ICE2KSYS_SOUND_DING\" &");
	return 1;
}

long Volume::onCmdMixer(FXObject*, FXSelector, void* ptr) {
	system("xterm -e alsamixer &");
	return 1;
}

long Volume::onMousewheelTray(FXObject*, FXSelector, void* ptr) {
	long newvol = 0;

	snd_mixer_handle_events(s_mixer);
	snd_mixer_selem_get_playback_volume_range(s_elem, &min, &max);
	snd_mixer_selem_get_playback_volume(s_elem, (snd_mixer_selem_channel_id_t)0, &newvol);
	newvol = newvol * 100 / max;
	
	if (newvol < 0) newvol = 0;
	else if (newvol > 100) newvol = 100;

	if (((FXEvent*)ptr)->code < 0) {
		newvol -= 5;
	} else {
		newvol += 5;
	}

	slider->setValue( ((newvol)*max/100)*100/max );

	snd_mixer_selem_set_playback_volume_all(s_elem, (newvol)*max/100);


	return 1;
}

long Volume::onCmdTray(FXObject*, FXSelector, void* ptr) {
	long vol = 0;
	int mute = 0;

	//s_elem = snd_mixer_find_selem(s_mixer, s_sid);
	snd_mixer_handle_events(s_mixer);
	snd_mixer_selem_get_playback_volume_range(s_elem, &min, &max);
	snd_mixer_selem_get_playback_volume(s_elem, (snd_mixer_selem_channel_id_t)0, &vol);
	snd_mixer_selem_get_playback_switch(s_elem, (snd_mixer_selem_channel_id_t)0, &mute);

	FXint mousex, mousey;
	unsigned int mousebtn;

	getApp()->getRootWindow()->getCursorPosition(mousex, mousey, mousebtn);
	//printf("mousex: %d, mousey: %d\n", mousex, mousey);
	position(mousex-(getWidth()/2), mousey - getHeight()-6, getWidth(), getHeight());

	mute_chk->setCheck(!mute);
	slider->setValue(vol * 100 / max);
	slider->setFocus();
	//printf("volume: %d\n", vol);


	if (!shown()) {
		mapWindowFlags(getApp(), this);
		show();
		slider->setFocus();
	}

	return 1;
}

	

Volume::Volume(FXApp *a) : FXMainWindow(a, "Volume Mixer", ico_volume, NULL, DECOR_NONE, 0,0,0,0) {
	snd_mixer_open(&s_mixer, 0);
	snd_mixer_attach(s_mixer, "default");
	snd_mixer_selem_register(s_mixer, NULL, NULL);
	snd_mixer_load(s_mixer);


	snd_mixer_selem_id_alloca(&s_sid);
	snd_mixer_selem_id_set_index(s_sid, 0);
	snd_mixer_selem_id_set_name(s_sid, "Master");
	s_elem = snd_mixer_find_selem(s_mixer, s_sid);

	bmp1 = new FXBitmap(a, leftside1_bits, 0, leftside1_width, leftside1_height);
	bmp2 = new FXBitmap(a, leftside2_bits, 0, leftside2_width, leftside2_height);
	img = new FXImage(a, NULL, IMAGE_OPAQUE, 11,66);
	//dc.setForeground(getBaseColor());
	main_frm = new FXVerticalFrame(this, FRAME_RAISED|FRAME_THICK|LAYOUT_FILL, 0,0,0,0, 5,5,6,1, 8,8);
	main_frm->setBaseColor(main_frm->getHiliteColor()); 
	main_frm->setHiliteColor(main_frm->getBackColor());
	new FXLabel(main_frm, "Volume", NULL, LAYOUT_CENTER_X);

	FXHorizontalFrame* hor_frm = new FXHorizontalFrame(main_frm, LAYOUT_CENTER_X, 0,0,0,0, 4,4,0,4, 7,7);
	new FXImageFrame(hor_frm, img, IMAGE_OPAQUE|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT, 0,0, 11,66);
	slider = new FXSlider(hor_frm, this, ID_VOLUME, SLIDER_VERTICAL|LAYOUT_CENTER_X|LAYOUT_FIX_HEIGHT|LAYOUT_FIX_WIDTH, 0,0,21,67+2, 0,0,1,1);
	new FXFrame(hor_frm, LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT, 0,0, 11+2,66, 0,0,0,0);
	slider->setHeadSize(11);
	slider->setSlotSize(4);

	slider->setRange(0, 100);

	mute_chk = new FXCheckButton(main_frm, "&Mute", this, ID_MUTE);

    tray = new I2KTrayIcon(getApp(), "Volume", ico_volume, 0, this, ID_TRAY, TRAY_CMD_ON_LEFT|TRAY_MENU_ON_RIGHT);
	popup = new FXPopup(tray);


	FXFont* normalfont = a->getNormalFont();

	FXString fname = normalfont->getFont();
	FXuint fsize = normalfont->getSize()/10;
	FXuint fslant = normalfont->getSlant();
	FXuint fencoding = normalfont->getEncoding();
	FXuint fsetwidth = normalfont->getSetWidth();
	FXuint fhints = normalfont->getHints();

	FXFont* boldfont = new FXFont(a, fname, fsize, FXFont::Bold, fslant, fencoding, fsetwidth, fhints);
	if (boldfont == NULL) {
		boldfont = normalfont;
	} else {
		boldfont->create();
	}


	FXMenuCommand* vol_cmd = new FXMenuCommand(popup, "&Open Volume Control", NULL, this, ID_MIXER);
	vol_cmd->setFont(boldfont);
	FXMenuCommand* prop_cmd = new FXMenuCommand(popup, "&Adjust Audio Properties");
	prop_cmd->disable();

	tray->setMenu(popup);

}

Volume::~Volume() {
	snd_mixer_close(s_mixer);
}

void Volume::create() {
	bmp1->create();
	bmp2->create();
	img->create();

	FXDCWindow dc(img);
	dc.setForeground(getApp()->getBaseColor());
	dc.fillRectangle(0,0, 11,66);
	dc.setFillStyle(FILL_STIPPLED);
	dc.setForeground(getApp()->getHiliteColor());
	dc.setStipple(bmp1, 0,0);
	dc.fillRectangle(0,0, 11,66);
	dc.setForeground(getApp()->getShadowColor());
	dc.setStipple(bmp2, 0,0);
	dc.fillRectangle(0,0, 11,66);
	dc.end();

	img->restore();

	FXMainWindow::create();


	//show();
	//show();
	//show(PLACEMENT_CURSOR);

}

long Volume::onUnfocus(FXObject*, FXSelector, void*) {
	hide();
	//getApp()->exit();
	return 1;
}

int main(int argc, char *argv[]) {
	I2KTrayApp application("Volume", "I2KProject");

	application.init(argc, argv);

	if (i2kBGetWinVersionInt() >= ICE2K_BRAND_WINXP) {
		ico_volume = new FXPNGIcon(&application, resico_volumexp);
		ico_volume->blend(application.getBaseColor());
	} else {
		ico_volume = new FXGIFIcon(&application, resico_volume2k);
	}

	new Volume(&application);

	application.create();
	return application.run();
}
