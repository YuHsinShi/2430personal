/*
 * iTE castor3 media player
 *
 * @file	castor3player.h
 * @author	Evan Chang
 * @version	1.0.0
 *
*/
#ifndef CASTOR3_PLAYER_HEADER
#define CASTOR3_PLAYER_HEADER

typedef enum PLAYER_MODE {
	NORMAL_PLAYER,
	DVR_PLAYER
} PLAYER_MODE;

typedef struct ithMediaPlayer
{
	/********************************************************
	 * function: init
	 *
	 * @param
	 *   mode: Assign to be Normal/DVR player.
	 *
	 * @return value
	 *   0: Success
	 *  <0: Error occur
	 ********************************************************/
	int (*init)(PLAYER_MODE mode);

	/********************************************************
	 * function: select
	 *
	 * @param
	 *   file: Assign file name and path.
	 *         Ex, A:\mediafile.mp4
	 *   width: Video source width
	 *   height: Video source height
	 *
	 * @return value
	 *   0: Success
	 *  <0: Error occur
	 ********************************************************/
	int (*select)(const char* file, unsigned width, unsigned height);

	/********************************************************
	 * function: play
	 *
	 * @return value
	 *   0: Success
	 *  <0: Error occur
	 ********************************************************/
	int (*play)(void);

	/********************************************************
	 * function: pause
	 *
	 * @return value:
	 *   0: pause -> play
	 *   1: play  -> pause
	 *  <0: Error occur
	 ********************************************************/
	int (*pause)(void);

	/********************************************************
	 * function: stop
	 *
	 * @return value
	 *   0: Success
	 *  <0: Error occur
	 ********************************************************/
	int (*stop)(void);

	/********************************************************
	 * function: deinit
	 *
	 * @return value
	 *   0: Success
	 *  <0: Error occur
	 ********************************************************/
	int (*deinit)(void);

	/********************************************************
	 * function: vol_up
	 *
	 * @return value
	 *   0: Success
	 *  <0: Error occur
	 ********************************************************/
	int (*vol_up)(void);

	/********************************************************
	 * function: vol_down
	 *
	 * @return value
	 *   0: Success
	 *  <0: Error occur
	 ********************************************************/
	int (*vol_down)(void);

	/********************************************************
	 * function: mute
	 *
	 * @return value
	 *   0: mute    -> un-mute
	 *   1: un-mute -> mute
	 *  <0: Error occur
	 ********************************************************/
	int (*mute)(void);
} ithMediaPlayer;

/*
 * Media player
 *
 * Do not release pointer, just init/deinit
 *
*/
extern ithMediaPlayer *media_player;

void xxx_drop_to_align(void);

#endif
