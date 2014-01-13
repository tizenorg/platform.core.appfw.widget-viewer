#include <stdio.h>

static struct info {
	int manual_sync;
	int frame_drop_for_resizing;
	int shared_content;

	double event_filter;
} s_info = {
	.manual_sync = 0,
	.frame_drop_for_resizing = 1,
	.shared_content = 0,

	.event_filter = 0.01f,
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

void conf_set_shared_content(int flag)
{
	s_info.shared_content = flag;
}

int conf_shared_content(void)
{
	return s_info.shared_content;
}

double conf_event_filter(void)
{
	return s_info.event_filter;
}

void conf_set_event_filter(double filter)
{
	s_info.event_filter = filter;
}

/* End of a file */
