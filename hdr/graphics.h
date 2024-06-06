#ifndef _GRAPHICS_H
#define _GRAPHICS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <termios.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <curses.h>
// #include <cursesw.h>

#include <stdlib.h>
#include <unistd.h>
#include <malloc.h>
#include <signal.h>
#include <mqueue.h>
#include "network.h"

#define GFX_ELEM_VOFF                   0
#define GFX_ELEM_HOFF                   1

#define SUBWND_SET_H                    size.ws_row
#define SUBWND_DEF_H                    10

#define SUBWND_SET_W                    size.ws_col
#define SUBWND_DEF_W                    120

#define WLCM_WND_H_RU                   5
#define WLCM_WND_W_RU                   95

#define WLCM_WND_H_EN                   7
#define WLCM_WND_W_EN                   45

#define PANEL_SET_H                     12
#define PANEL_MIN_H                     12
#define PANEL_DEF_H                     12

#define PANEL_SET_W                     60
#define PANEL_MIN_W                     40
#define PANEL_DEF_W                     40

#define ELEM_SET_H                      ELEM_MIN_H
#define ELEM_MIN_H                      ELEM_DEF_H
#define ELEM_DEF_H                      3

#define ELEM_SET_W                      ELEM_MIN_W
#define ELEM_MIN_W                      ELEM_DEF_W
#define ELEM_DEF_W                      20

#define LABEL_LEN                       20

#define MENU_SCR_LABEL                  "Welcome to Linux Messenger"
#define MENU_SCR_NOTE_LABEL             "ACS Characters:"
#define MENU_SCR_CLT_BTN_LABEL          "Join server"
#define MENU_SCR_SRV_BTN_LABEL          "Create a server"
#define MENU_SCR_CFG_BTN_LABEL          "Configuration"
#define MENU_SCR_QUIT_BTN_LABEL         "Quit"
#define MENU_SCR_NOTE_CONNECTED         "F1: Help\tF2: Reconnect\tF3: Disconnect\tF4: Quit"
#define MENU_SCR_NOTE_DISCONNECTED      "F1: Help\tF2: Connect\tF3: Change address\tF4: Quit"
#define MENU_SCR_NOTE_CONNECTING        "F1: Help\tF4: Quit"

#define JOIN_SCR_LABEL                  "Join server"
#define JOIN_SCR_SRV_NAME_LABEL         "Server name"
#define JOIN_SCR_SRV_ADDR_LABEL         "Address"
#define JOIN_SCR_CONN_USERS_LABEL       "Users"
#define JOIN_SCR_MAN_ADDR_LABEL         "Input address:"
#define JOIN_SCR_JOIN_BTN_LABEL         "Join"
#define JOIN_SCR_REFRESH_BTN_LABEL      "Refresh"
#define JOIN_SCR_CLEAR_BTN_LABEL        "Clear"
#define JOIN_SCR_NOTE                   "TAB: Change mode\tF1: Help\tF3: Back\tF4: Quit"

#define CREATE_SCR_LABEL                "Create server"
#define CREATE_SCR_SRV_INFO_TEMPLATE    "Server properties:\n* Server name:\t\t%s\n* Maximum users:\t%s\n* Address:\t\t%s"
#define CREATE_SCR_SRV_NAME_LABEL       "Server name"
#define CREATE_SCR_CONN_USERS_LABEL     "Maximum users"
#define CREATE_SCR_RESTR_USERS_LABEL    "Users restriction"
#define CREATE_SCR_SRV_ADDR_LABEL       "Address"
#define CREATE_SCR_SRV_PORT_LABEL       "Port"
#define CREATE_SCR_LCL_ADDR_LABEL       "Local address"
#define CREATE_SCR_AUTO_PORT_LABEL      "Auto port"
#define CREATE_SCR_CREATE_BTN_LABEL     "Create"
#define CREATE_SCR_DEFAULT_BTN_LABEL    "Default"
#define CREATE_SCR_CLEAR_BTN_LABEL      "Clear"
#define CREATE_DEF_SRV_NAME             "Default"
#define CREATE_DEF_USR_RESTR            10
#define CREATE_DEF_SRV_IP               "127.0.0.1"
#define CREATE_DEF_SRV_PORT             27015
#define CREATE_SCR_NOTE                 "TAB: Change mode\tF1: Help\tF3: Back\tF4: Quit"

#define PREFS_SCR_LABEL                 "Preferences"
#define PREFS_USERNAME_LABEL            "Username"
#define PREFS_LANG_LABEL                "Language"
#define PREFS_IP_LABEL                  "IP Address"
#define PREFS_PORT_LABEL                "Port"
#define PREFS_LANG_EN_LABEL             "English"
#define PREFS_LANG_RU_LABEL             "Russian"
#define PREFS_SCR_NOTE                  "F1: Save\tF2: Menu\tF3: Back\tF4: Quit\tF5: Save\tF6: Cancel\tF7: Reset"

#define OPTIONS_TRUE_LABEL              "Yes"
#define OPTIONS_FALSE_LABEL             "No"

enum cur_wnd_enum
{
    WND_NONE = 0,
    WND_MAIN_MENU,
    WND_JOIN_SRV,
    WND_CREATE_SRV,
    WND_PREFS
};

enum popup_wnd_type
{
    POPUP_W_BLOCK = 1,
    POPUP_W_WAIT,
    POPUP_W_CONFIRM,
    POPUP_W_INFO
};

enum join_wnd_tab_mode
{
    MODE_LIST = 1,
    MODE_TEXTBOX
};

enum create_wnd_tab_mode
{
    MODE_PAD = 1,
    MODE_BUTTONS
};

enum cfg_entry_type
{
    ENTRY_TYPE_STRING = 1,
    ENTRY_TYPE_SHORT,
    ENTRY_TYPE_INT,
    ENTRY_TYPE_FLOAT,
    ENTRY_TYPE_OPTION,
    ENTRY_TYPE_IP
};

struct label_t
{
    int size;
    char *text;
};

struct string_t
{
    int str_len;
    char text[LABEL_LEN+1];
};

struct note_labels_t
{
    struct label_t mmenu_connected;
    struct label_t mmenu_disconnected;
    struct label_t mmenu_connecting;
    struct label_t join_srv;
    struct label_t create_srv;
    struct label_t prefs_wnd;
};

struct elem_wnd_t
{
    WINDOW *wnd;
    struct label_t lbl;
};

struct option_t
{
    int value;
    int str_len;
    char option[LABEL_LEN+1];
};

struct list_t
{
    int size;
    struct option_t *options;
};

struct entry_t
{
    struct elem_wnd_t elem;
    int type;
    void *value_ptr;
    int ptr_size;
    int selection;
    // int str_len;
    union
    {
        struct list_t value_list;
        struct string_t value_str;
    };
};

struct global_dims_t
{
    int wnd_h;
    int wnd_w;
    int subwnd_h;
    int subwnd_w;
    int elem_h;
    int elem_w;
    int note_h;
    int note_w;
    int h_spacer;
};

struct global_axis_t
{
    int wnd_y;
    int wnd_x;
    int subwnd_y;
    int subwnd_x;
    int note_y;
    int note_x;
};

struct global_wnds_t
{
    WINDOW *wnd;
    WINDOW *note_w;
    WINDOW *note_sw;
};

struct mmenu_dims_t
{
    int header_h;
    int header_w;
    int status_h;
    int status_w;
    int btns_h;
    int btns_w;
    int btns_border_h;// = (elem_h*4)+2+(GFX_ELEM_VOFF*5);
    int btns_border_w;// = elem_w+2+(GFX_ELEM_HOFF*2);
    int panel_h;
    int panel_w;
};

struct mmenu_axis_t
{
    int header_y;
    int header_x;
    int status_y;
    int status_x;
    int btns_border_y;
    int btns_border_x;
    int btns_y[4];
    int btns_x[4];
    int panel_y;
    int panel_x;
};

struct mmenu_wnd_t
{
    WINDOW *header_w;
    WINDOW *status_w;
    WINDOW *status_sw;
    WINDOW *btns_border;
    struct elem_wnd_t btns[4];
    WINDOW *panel_w;
    WINDOW *panel_sw;
    int selection;
};

struct join_dims_t
{
    int sinfo_h;
    int sinfo_w;
    int susers_h;
    int susers_w;
    int saddr_h;
    int saddr_w;
    int sname_h;
    int sname_w;
    int caddr_h;
    int caddr_w;
    int lbl_h;
    int lbl_w;
    int tb_h;
    int tb_w;
    int btns_border_h;
    int btns_border_w;
    int btns_h;
    int btns_w;
    int pad_border_h;
    int pad_border_w;
    int vis_pad_h;
    int vis_pad_w;
    int pad_h;
    int pad_w;
};

struct join_axis_t
{
    int top_panel_y;
    int top_panel_x;
    int susers_y;
    int susers_x;
    int saddr_y;
    int saddr_x;
    int sname_y;
    int sname_x;
    int pad_border_y;
    int pad_border_x;
    int vis_pad_ys;
    int vis_pad_xs;
    int vis_pad_ye;
    int vis_pad_xe;
    int pad_ys;
    int pad_xs;
    int pad_ye;
    int pad_xe;
    int caddr_y;
    int caddr_x;
    int lbl_y;
    int lbl_x;
    int tb_y;
    int tb_x;
    int btns_border_y;
    int btns_border_x;
    int btns_y[3];
    int btns_x[3];
};

struct join_wnd_t
{
    WINDOW *top_panel;
    WINDOW *sname_w;
    WINDOW *saddr_w;
    WINDOW *susers_w;
    WINDOW *pad_border;
    WINDOW *servers_pad;
    WINDOW *caddr_w;
    WINDOW *label_w;
    WINDOW *tb_w;
    WINDOW *btns_border;
    struct elem_wnd_t btns[3];
    struct server_info_t *servers;
    int servers_count;
    int line;
    // int vis_line;
    int selection;
    int mode;
};

struct create_dims_t
{
    int vis_pad_h;
    int vis_pad_w;
    int pad_h;
    int pad_w;
    int pad_border_h;
    int pad_border_w;
    int btns_h;
    int btns_w;
    int btns_border_h;
    int btns_border_w;
    int entry_h;
    int entry_w;
    int srv_info_h;
    int srv_info_sw_h;
    int srv_info_w;
};

struct create_axis_t
{
    int srv_info_y;
    int srv_info_x;
    int srv_info_sw_y;
    int srv_info_sw_x;
    int pad_border_y;
    int pad_border_x;
    int vis_pad_ys;
    int vis_pad_xs;
    int vis_pad_ye;
    int vis_pad_xe;
    int pad_ys;
    int pad_xs;
    int pad_ye;
    int pad_xe;
    int v_delim_x;
    int entries_y[7];
    int entries_x[7];
    int labels_x[7];
    int fields_x[7];
    int btns_border_y;
    int btns_border_x;
    int btns_y[3];
    int btns_x[3];
};

struct create_wnd_t
{
    WINDOW *subwnd;
    WINDOW *srv_info_w;
    WINDOW *srv_info_sw;
    WINDOW *pad_border;
    WINDOW *pad;
    // struct elem_wnd_t pad_elems[7];
    struct entry_t entries[7];
    // WINDOW *sname_w;
    // WINDOW *musers_w;
    // WINDOW *rusers_w;
    // WINDOW *addr_w;
    // WINDOW *port_w;
    // WINDOW *lcl_addr_w;
    // WINDOW *auto_port_w;
    WINDOW *btns_border;
    struct elem_wnd_t btns[3];
    struct server_info_t server;
    int usr_restrict_flag;
    int local_srv_flag;
    int auto_port_flag;
    int line;
    int selection;
    int mode;
};

struct cfg_dims_t
{
    int pad_border_h;
    int pad_border_w;
    int vis_pad_h;
    int vis_pad_w;
    int pad_h;
    int pad_w;
    int entry_h;
    int entry_w;
    int entry_hspacer;
};

struct cfg_axis_t
{
    int pad_border_y;
    int pad_border_x;
    int vis_pad_ys;
    int vis_pad_xs;
    int vis_pad_ye;
    int vis_pad_xe;
    int pad_ys;
    int pad_xs;
    int pad_ye;
    int pad_xe;
    int entries_y[4];
    int entries_x[4];
    int fields_x[4];
    int v_delim_x;
};

struct cfg_wnd_t
{
    WINDOW *pad_border;
    WINDOW *pad;
    struct entry_t entries[4];
    int line;
};

extern int connection_flag;

/* Global variable, used to store terminal's size in columns and rows */
extern struct winsize size;

int init_graphics();
void deinit_graphics();

void handle_msg(union sigval);

int popup_wnd(char *, int, ...);

int menu_wnd(int *);

int join_srv_wnd(int *);
int create_srv_wnd(int *);

int prefs_wnd(int *);

#endif // _GRAPHICS_H