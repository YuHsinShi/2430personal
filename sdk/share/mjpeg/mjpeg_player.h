#ifndef MJPEG_PLAYER_H
#define MJPEG_PLAYER_H

typedef enum
{
    MJPEG_PLAYER_EVENT_EOF,
    MJPEG_PLAYER_EVENT_OPEN_FILE_FAIL
} MJPEG_PLAYER_EVENT;

typedef void (*mjpeg_cb_handler_t)(MJPEG_PLAYER_EVENT event_id, void *arg);

void ITE_PlayMJpeg(int x, int y, int width, int height, int volume, char *filepath, mjpeg_cb_handler_t callback);
void ITE_StopMJpeg(void);

#endif
