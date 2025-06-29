/* See LICENSE file for copyright and license details. */

/* appearance */
static const unsigned int borderpx  = 1;        /* border pixel of windows */
static const unsigned int snap      = 32;       /* snap pixel */
static const int showbar            = 1;        /* 0 means no bar */
static const int topbar             = 1;        /* 0 means bottom bar */
static const char *fonts[]          = { "monospace:size=10" };
static const char dmenufont[]       = "monospace:size=10";
static const char col_gray1[]       = "#222222";
static const char col_gray2[]       = "#444444";
static const char col_gray3[]       = "#bbbbbb";
static const char col_gray4[]       = "#eeeeee";
static const char col_cyan[]        = "#478061";
static const char *colors[][3]      = {
	/*               fg         bg         border   */
	[SchemeNorm] = { col_gray3, col_gray1, col_gray2 },
	[SchemeSel]  = { col_gray4, col_cyan,  col_cyan  },
};

/* tagging */
static const char *tags[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };

static const Rule rules[] = {
	/* xprop(1):
	 *	WM_CLASS(STRING) = instance, class
	 *	WM_NAME(STRING) = title
	 */
	/* class      instance    title       tags mask     isfloating   monitor */
	{ "Gimp",     NULL,       NULL,       0,            1,           -1 },
	{ "Firefox",  NULL,       NULL,       1 << 8,       0,           -1 },
};

/* layout(s) */
static const float mfact     = 0.55; /* factor of master area size [0.05..0.95] */
static const int nmaster     = 1;    /* number of clients in master area */
static const int resizehints = 1;    /* 1 means respect size hints in tiled resizals */
static const int lockfullscreen = 1; /* 1 will force focus on the fullscreen window */

static const Layout layouts[] = {
	/* symbol     arrange function */
	{ "[]=",      tile },    /* first entry is default */
	{ "><>",      NULL },    /* no layout function means floating behavior */
	{ "[M]",      monocle },
};

/* key definitions */
#define MODKEY Mod1Mask
#define XF86AudioMute 0x1008ff12
#define XF86AudioLowerVolume 0x1008ff11
#define XF86AudioRaiseVolume 0x1008ff13
#define TAGKEYS(KEY,TAG) \
	{ MODKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
	{ MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} },

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

static const int dmenudesktop = 1; /* 1 means dmenu will use only desktop files from [/usr/share/applications/] */

/* commands */
static char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */
static const char *dmenucmd[] = { "dmenu_run", "-m", dmenumon, "-fn", dmenufont, "-nb", col_gray1, "-nf", col_gray3, "-sb", col_cyan, "-sf", col_gray4, NULL };
static const char *termcmd[]  = { "st", NULL };
static const char *cmdsoundnotify[] = {"sh", "-c", "dunstify -t 800 -u low \"Volume\" -r 10000 -h int:value:$(amixer sget Master | tail -1 | awk '{print $4}' | tr -d '[%]')", NULL};
static const char *cmdsoundtoggle[] = { "amixer", "-q", "sset", "Master", "toggle", NULL };
static const char *cmdsoundup[] = { "amixer", "-q", "sset", "Master", "5%+", NULL};
static const char *cmdsounddown[] = { "amixer", "-q", "sset", "Master", "5%-", NULL};
static unsigned int last_vol_notify_time = 0;

#define NOTIFY_COOLDOWN_MS 200  // Minimum time between notifications
#include <sys/time.h> 

static void volumechange(const Arg *arg) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    unsigned int now = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    
    // Only send notification if cooldown has passed
    if (now - last_vol_notify_time > NOTIFY_COOLDOWN_MS) {
		system(arg->v);
        char cmd[256];
        // Get current volume and create notification command
        snprintf(cmd, sizeof(cmd), "amixer get Master | awk -F'[][%%]' '/%/ {print $2}' | head -n 1 | xargs -I {} dunstify -t 800 -u low -r 10000 'Volume' -h int:value:{}");
        system(cmd);
        last_vol_notify_time = now;
    }
}

/* Volume control actions */
#define VOL_TOGGLE 0
#define VOL_DOWN   1
#define VOL_UP     2

static void volumecontrol(const Arg *arg) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    unsigned int now = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    
    if (now - last_vol_notify_time > NOTIFY_COOLDOWN_MS) {
        const char *action;
        switch(arg->i) {  // Note: using .i for integer
            case VOL_UP:
                action = "[ \"$(amixer get Master | tail -1 | awk '{print $6}' | tr -d '[]')\" = \"off\" ] && amixer set Master unmute; "
                         "amixer set Master 5%+";
                break;
            case VOL_DOWN:
                action = "amixer set Master 5%-";
                break;
            case VOL_TOGGLE:
                action = "amixer set Master toggle";
                break;
            default:
                return;
        }

        char cmd[512];
        snprintf(cmd, sizeof(cmd),
            "%s && "
            "output=$(amixer get Master) && "
            "muted=$(echo \"$output\" | tail -1 | awk '{print $6}' | tr -d '[]') &&"
            "vol=$(echo \"$output\" | awk -F'[][]' '/%/ {print $2}' | head -n 1) && "
            "if [ \"$muted\" = \"off\" ]; then "
            "    dunstify -t 800 -u critical -r 10000 'Volume' -h int:value:0; "
            "else "
            "    dunstify -t 800 -u low -r 10000 'Volume' -h int:value:$vol; "
            "fi",
            action);
        
        system(cmd);
        last_vol_notify_time = now;
    }
}

#include "exitdwm.c"
static const Key keys[] = {
	/* modifier                     key        function        argument */
	{ MODKEY,                       XK_p,      spawn,          {.v = dmenucmd } },
	{ MODKEY|ShiftMask,             XK_Return, spawn,          {.v = termcmd } },
	{ MODKEY,                       XK_b,      togglebar,      {0} },
	{ MODKEY,                       XK_j,      focusstack,     {.i = +1 } },
	{ MODKEY,                       XK_k,      focusstack,     {.i = -1 } },
	{ MODKEY,                       XK_i,      incnmaster,     {.i = +1 } },
	{ MODKEY,                       XK_d,      incnmaster,     {.i = -1 } },
	{ MODKEY,                       XK_h,      setmfact,       {.f = -0.05} },
	{ MODKEY,                       XK_l,      setmfact,       {.f = +0.05} },
	{ MODKEY,                       XK_Return, zoom,           {0} },
	{ MODKEY,                       XK_Tab,    view,           {0} },
	{ MODKEY|ShiftMask,             XK_c,      killclient,     {0} },
	{ MODKEY,                       XK_t,      setlayout,      {.v = &layouts[0]} },
	{ MODKEY,                       XK_f,      setlayout,      {.v = &layouts[1]} },
	{ MODKEY,                       XK_m,      setlayout,      {.v = &layouts[2]} },
	{ MODKEY,                       XK_space,  setlayout,      {0} },
	{ MODKEY|ShiftMask,             XK_space,  togglefloating, {0} },
	{ MODKEY,                       XK_0,      view,           {.ui = ~0 } },
	{ MODKEY|ShiftMask,             XK_0,      tag,            {.ui = ~0 } },
	{ MODKEY,                       XK_comma,  focusmon,       {.i = -1 } },
	{ MODKEY,                       XK_period, focusmon,       {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_comma,  tagmon,         {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_period, tagmon,         {.i = +1 } },
	{ 0,				            XF86AudioMute,volumecontrol,		{.i = VOL_TOGGLE} },
	{ 0,				            XF86AudioRaiseVolume,volumecontrol,	{.i = VOL_UP} },
	{ 0,				            XF86AudioLowerVolume,volumecontrol,	{.i = VOL_DOWN} },
	// { 0,				            XF86AudioMute,spawn,		{.v = cmdsoundnotify} },
	// { 0,				            XF86AudioRaiseVolume,spawn,	{.v = cmdsoundnotify} },
	// { 0,				            XF86AudioLowerVolume,spawn,	{.v = cmdsoundnotify} },
	TAGKEYS(                        XK_1,                      0)
	TAGKEYS(                        XK_2,                      1)
	TAGKEYS(                        XK_3,                      2)
	TAGKEYS(                        XK_4,                      3)
	TAGKEYS(                        XK_5,                      4)
	TAGKEYS(                        XK_6,                      5)
	TAGKEYS(                        XK_7,                      6)
	TAGKEYS(                        XK_8,                      7)
	TAGKEYS(                        XK_9,                      8)
	{ MODKEY|ShiftMask,             XK_q,      exitdwm,           {0} },
	{ MODKEY|ControlMask|ShiftMask, XK_q,      quit,           {1} }, 
};

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static const Button buttons[] = {
	/* click                event mask      button          function        argument */
	{ ClkLtSymbol,          0,              Button1,        setlayout,      {0} },
	{ ClkLtSymbol,          0,              Button3,        setlayout,      {.v = &layouts[2]} },
	{ ClkWinTitle,          0,              Button2,        zoom,           {0} },
	{ ClkStatusText,        0,              Button2,        spawn,          {.v = termcmd } },
	{ ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
	{ ClkClientWin,         MODKEY,         Button2,        togglefloating, {0} },
	{ ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
	{ ClkTagBar,            0,              Button1,        view,           {0} },
	{ ClkTagBar,            0,              Button3,        toggleview,     {0} },
	{ ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
	{ ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
};

