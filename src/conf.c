#include <stdio.h>

static struct info {
	int manual_sync;
	int frame_drop_for_resizing;
} s_info = {
	.manual_sync = 0,
	.frame_drop_for_resizing = 1,
};

void conf_set_manual_sync(int flag)
{
	s_info.manual_sync = flag;
}

int conf_manual_sync(void)
{
	return s_info.manual_sync;
}

void conf_set_frame_drop_for_resizing(int flag)
{
	s_info.frame_drop_for_resizing = flag;
}

int conf_frame_drop_for_resizing(void)
{
	return s_info.frame_drop_for_resizing;
}

/* End of a file */
