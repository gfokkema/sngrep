/**************************************************************************
 **
 ** sngrep - SIP callflow viewer using ngrep
 **
 ** Copyright (C) 2013 Ivan Alonso (Kaian)
 ** Copyright (C) 2013 Irontec SL. All rights reserved.
 **
 ** This program is free software: you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License as published by
 ** the Free Software Foundation, either version 3 of the License, or
 ** (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU General Public License for more details.
 **
 ** You should have received a copy of the GNU General Public License
 ** along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **
 ****************************************************************************/
/**
 * @file ui_call_flow.h
 * @author Ivan Alonso [aka Kaian] <kaian@irontec.com>
 *
 * @brief Functions to manage Call Flow screen
 *
 * This file contains the functions and structures to manage the call flow
 * screen.
 *
 */
#ifndef __UI_CALL_FLOW_H
#define __UI_CALL_FLOW_H
#include "ui_manager.h"

//! Sorter declaration of struct call_flow_info
typedef struct call_flow_info call_flow_info_t;

/**
 * @brief Call flow status information
 *
 * This data stores the actual status of the panel. It's stored in the
 * PANEL user pointer.
 */
struct call_flow_info
{
    sip_call_t *call;
    sip_msg_t *first_msg;
    sip_msg_t *cur_msg;
    int linescnt;
    int cur_line;
};

/**
 * @brief Create Call Flow panel
 *
 * This function will allocate the ncurses pointer and draw the static
 * stuff of the screen (which usually won't be redrawn)
 * It will also create an information structure of the panel status and
 * store it in the panel's userpointer
 *
 * @return the allocated ncurses panel
 */
extern PANEL *
call_flow_create();

/**
 * @brief Destroy panel
 *
 * This function will hide the panel and free all allocated memory.
 *
 * @return panel Ncurses panel pointer
 */
extern void
call_flow_destroy(PANEL *panel);

/**
 * @brief Check if the panel requires to be redrawn
 *
 * During online mode, this function will be invoked if this is the topmost
 * panel every time a new message has been readed.
 *
 * @param panel Ncurses panel pointer
 * @param msg New readed message
 * @return 0 if the panel needs to be redrawn, -1 otherwise
 */
extern int
call_flow_redraw_required(PANEL *panel, sip_msg_t *msg);

/**
 * @brief Draw the Call flow panel
 *
 * This function will drawn the panel into the screen based on its stored
 * status
 *
 * @param panel Ncurses panel pointer
 * @return 0 if the panel has been drawn, -1 otherwise
 */
extern int
call_flow_draw(PANEL *panel);

/**
 * @brief Handle Call flow key strokes
 *
 * This function will manage the custom keybindings of the panel. If this
 * function returns -1, the ui manager will check if the pressed key
 * is one of the common ones (like toggle colors and so).
 *
 * @param panel Ncurses panel pointer
 * @param key Pressed keycode
 * @return 0 if the function can handle the key, -1 otherwise
 */
extern int
call_flow_handle_key(PANEL *panel, int key);

/**
 * @brief Request the panel to show its help
 *
 * This function will request to panel to show its help (if any) by
 * invoking its help function.
 *
 * @param panel Ncurses panel pointer
 * @return 0 if the screen has help, -1 otherwise
 */
extern int
call_flow_help(PANEL *panel);

/**
 * @brief Set the active call of the panel
 *
 * This function will access the panel information and will set the
 * call pointer to the processed call.
 *
 * @param call Call pointer to be set in the internal info struct
 */
extern int
call_flow_set_call(sip_call_t *call);

#endif