#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <Ecore.h>
#include <Ecore_X.h>
#include <Evas.h>

#include <dlog.h>
#include <dynamicbox_errno.h>

#include "debug.h"
#include "util.h"

static inline int get_pid(Ecore_X_Window win)
{
	int pid;
	Ecore_X_Atom atom;
	unsigned char *in_pid;
	int num;

	atom = ecore_x_atom_get("X_CLIENT_PID");
	if (!atom) {
		return DBOX_STATUS_ERROR_INVALID_PARAMETER;
	}

	if (ecore_x_window_prop_property_get(win, atom, ECORE_X_ATOM_CARDINAL,
				sizeof(int), &in_pid, &num) == EINA_FALSE) {
		if (ecore_x_netwm_pid_get(win, &pid) == EINA_FALSE) {
			ErrPrint("Failed to get PID from a window 0x%X\n", win);
			return DBOX_STATUS_ERROR_INVALID_PARAMETER;
		}
	} else if (in_pid) {
		pid = *(int *)in_pid;
		DbgFree(in_pid);
	} else {
		ErrPrint("Failed to get PID\n");
		return DBOX_STATUS_ERROR_INVALID_PARAMETER;
	}

	return pid;
}

static inline Ecore_X_Window get_user_created_window(Ecore_X_Window win)
{
	Ecore_X_Window user_win = 0;
	Ecore_X_Atom atom;

	atom = ecore_x_atom_get("_E_USER_CREATED_WINDOW");
	if (!atom) {
		DbgPrint("Failed to get user created atom\n");
		return 0;
	}

	if (ecore_x_window_prop_xid_get(win, atom, ECORE_X_ATOM_WINDOW, &user_win, 1) == 1 && win) {
		DbgPrint("User window: %x (for %x)\n", user_win, win);
		return user_win;
	}

	return 0;
}

static inline int get_window_info(Ecore_X_Window parent, Ecore_X_Window user_win, int *pid, char **command)
{
	Evas_Coord x, y, w, h;
	int argc;
	char **argv;
	Evas_Coord rx, ry, rw, rh;
	Evas_Coord px, py, pw, ph;

	ecore_x_window_geometry_get(0, &rx, &ry, &rw, &rh);
	ecore_x_window_geometry_get(user_win, &x, &y, &w, &h);
	ecore_x_window_geometry_get(parent, &px, &py, &pw, &ph);

	if (x != rx || y != ry || w != rw || h != rh) {
		DbgPrint("Size mismatch (with user,win)\n");
		return DBOX_STATUS_ERROR_INVALID_PARAMETER;
	}

	if (x != px || y != py || w != pw || h != ph) {
		DbgPrint("Size mismatch (with parent)\n");
		return DBOX_STATUS_ERROR_INVALID_PARAMETER;
	}

	ecore_x_icccm_command_get(user_win, &argc, &argv);
	DbgPrint("Get Command of %x\n", user_win);
	if (argc > 0) {
		int i;

		*pid = get_pid(user_win);
		if (command) {
			*command = argv[0];
			i = 1;
		} else {
			i = 0;
		}

		while (i < argc) {
			DbgFree(argv[i]);
			i++;
		}

		DbgFree(argv);
		return DBOX_STATUS_ERROR_NONE;
	}

	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
}

int util_win_list_get(int (*win_cb)(int pid, Ecore_X_Window win, const char *cmd, void *data), void *data)
{
	Ecore_X_Window root;
	Ecore_X_Window ret;
	struct stack_item *new_item;
	struct stack_item *item;
	Eina_List *win_stack;
	int pid = -1;
	int cnt = 0;
	struct stack_item {
		Ecore_X_Window *wins;
		int nr_of_wins;
		int i;
	};

	root = ecore_x_window_root_first_get();

	new_item = malloc(sizeof(*new_item));
	if (!new_item) {
		ErrPrint("Error(%s)\n", strerror(errno));
		return DBOX_STATUS_ERROR_OUT_OF_MEMORY;
	}

	new_item->nr_of_wins = 0;
	new_item->wins =
		ecore_x_window_children_get(root, &new_item->nr_of_wins);
	new_item->i = new_item->nr_of_wins - 1;

	win_stack = NULL;

	if (new_item->wins) {
		win_stack = eina_list_append(win_stack, new_item);
	} else {
		DbgFree(new_item);
	}

	while (pid < 0 && (item = eina_list_nth(win_stack, 0))) {
		win_stack = eina_list_remove(win_stack, item);

		if (!item->wins) {
			DbgFree(item);
			continue;
		}

		while (item->i >= 0) {
			ret = item->wins[item->i];

			/*
			 * Now we don't need to care about visibility of window,
			 * just check whether it is registered or not.
			 * (ecore_x_window_visible_get(ret))
			 */
			if (ecore_x_window_visible_get(ret) == EINA_TRUE) {
				Ecore_X_Window user_win;
				char *command = NULL;

				user_win = get_user_created_window(ret);
				if (user_win && get_window_info(ret, user_win, &pid, &command) == DBOX_STATUS_ERROR_NONE) {
					int status;

					cnt++;

					status = win_cb(pid, user_win, command, data);
					DbgFree(command);
					if (status == EINA_FALSE) {
						break;
					}
				} else {
					DbgPrint("Failed to get win info: %x\n", ret);
				}
			}

			new_item = malloc(sizeof(*new_item));
			if (!new_item) {
				ErrPrint("Error %s\n", strerror(errno));
				item->i++;
				continue;
			}

			new_item->nr_of_wins = 0;
			new_item->wins =
				ecore_x_window_children_get(ret,
						&new_item->nr_of_wins);
			new_item->i = new_item->nr_of_wins - 1;
			if (new_item->wins) {
				win_stack =
					eina_list_append(win_stack, new_item);
			} else {
				DbgFree(new_item);
			}

			item->i--;
		}

		DbgFree(item->wins);
		DbgFree(item);
	}

	EINA_LIST_FREE(win_stack, item) {
		DbgFree(item->wins);
		DbgFree(item);
	}

	return cnt;
}

/*!
 *
 * usage)
 * app_service(...)
 * {
 * if (power_key pressed) {
 * 	int pid;
 * 	pid = find_top_visible_window();
 * 	if (pid == getpid()) {
 *		// if (first page) {
 *		//     LCD_OFF
 *		// } else {
 *		//     SCROLL TO THE FIRST PAGE
 *		// }
 * 	} else {
 *		// elm_win_activate(my_win);
 * 	}
 * }
 *
 */
