/*
    SDL_mixer:  An audio mixer library based on the SDL library
    Copyright (C) 1997-2004 Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Sam Lantinga
    slouken@libsdl.org
*/

/* $Id: music.c,v 1.4 2005/05/22 01:25:47 smoli Exp $ */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <SDL_endian.h>
#include <SDL_audio.h>
#include <SDL_timer.h>

#include "mixer.h"

#define SDL_SURROUND

#ifdef SDL_SURROUND
#define MAX_OUTPUT_CHANNELS 6
#else
#define MAX_OUTPUT_CHANNELS 2
#endif

#include "mikmod.h"

/*static SDL_AudioSpec used_mixer;*/

int volatile music_active = 1;
static int volatile music_stopped = 0;
static int music_loops = 0;
static char *music_cmd = NULL;
static Mix_Music * volatile music_playing = NULL;
static int music_volume = MIX_MAX_VOLUME;
static int music_swap8;
static int music_swap16;

/* Reference for converting mikmod output to 4/6 channels */
static int current_output_channels;
static Uint16 current_output_format;

/* Used to calculate fading steps */
static int ms_per_step;

/* Local low-level functions prototypes */
static void music_internal_initialize_volume(void);
static void music_internal_volume(int volume);
static int  music_internal_play(Mix_Music *music, double position);
static int  music_internal_position(double position);
static int  music_internal_playing();
static void music_internal_halt(void);


/* Support for hooking when the music has finished */
static void (*music_finished_hook)(void) = NULL;

void Mix_HookMusicFinished(void (*music_finished)(void))
{
	SDL_LockAudio();
	music_finished_hook = music_finished;
	SDL_UnlockAudio();
}


/* Mixing function */
void music_mixer(void *udata, Uint8 *stream, int len)
{
	if ( music_playing && music_active ) {
		/* Handle fading */
		if ( music_playing->fading != MIX_NO_FADING ) {
			if ( music_playing->fade_step++ < music_playing->fade_steps ) {
				int volume;
				int fade_step = music_playing->fade_step;
				int fade_steps = music_playing->fade_steps;

				if ( music_playing->fading == MIX_FADING_OUT ) {
					volume = (music_volume * (fade_steps-fade_step)) / fade_steps;
				} else { /* Fading in */
					volume = (music_volume * fade_step) / fade_steps;
				}
				music_internal_volume(volume);
			} else {
				if ( music_playing->fading == MIX_FADING_OUT ) {
					music_internal_halt();
					if ( music_finished_hook ) {
						music_finished_hook();
					}
					return;
				}
				music_playing->fading = MIX_NO_FADING;
			}
		}
		/* Restart music if it has to loop */
		if ( !music_internal_playing() ) {
			/* Restart music if it has to loop at a high level */
			if ( music_loops && --music_loops ) {
				Mix_Fading current_fade = music_playing->fading;
				music_internal_play(music_playing, 0.0);
				music_playing->fading = current_fade;
			} else {
				music_internal_halt();
				if ( music_finished_hook ) {
					music_finished_hook();
				}
				return;
			}
		}
		switch (music_playing->type) {
			case MUS_MOD:
				if (current_output_channels > 2) {
					int small_len = 2 * len / current_output_channels;
					int i;
					Uint8 *src, *dst;

					VC_WriteBytes((SBYTE *)stream, small_len);
					/* and extend to len by copying channels */
					src = stream + small_len;
					dst = stream + len;

					switch (current_output_format & 0xFF) {
						case 8:
							for ( i=small_len/2; i; --i ) {
								src -= 2;
								dst -= current_output_channels;
								dst[0] = src[0];
								dst[1] = src[1];
								dst[2] = src[0];
								dst[3] = src[1];
								if (current_output_channels == 6) {
									dst[4] = src[0];
									dst[5] = src[1];
								}
							}
							break;
						case 16:
							for ( i=small_len/4; i; --i ) {
								src -= 4;
								dst -= 2 * current_output_channels;
								dst[0] = src[0];
								dst[1] = src[1];
								dst[2] = src[2];
								dst[3] = src[3];
								dst[4] = src[0];
								dst[5] = src[1];
								dst[6] = src[2];
								dst[7] = src[3];
								if (current_output_channels == 6) {
									dst[8] = src[0];
									dst[9] = src[1];
									dst[10] = src[2];
									dst[11] = src[3];
								}
							}
							break;
					}



				}
				else VC_WriteBytes((SBYTE *)stream, len);
				if ( music_swap8 ) {
					Uint8 *dst;
					int i;

					dst = stream;
					for ( i=len; i; --i ) {
						*dst++ ^= 0x80;
					}
				} else
				if ( music_swap16 ) {
					Uint8 *dst, tmp;
					int i;

					dst = stream;
					for ( i=(len/2); i; --i ) {
						tmp = dst[0];
						dst[0] = dst[1];
						dst[1] = tmp;
						dst += 2;
					}
				}
				break;
#ifdef OGG_MUSIC
			case MUS_OGG:
				OGG_playAudio(music_playing->data.ogg, stream, len);
				break;
#endif
			default:
				/* Unknown music type?? */
				break;
		}
	}
}

/* Initialize the music players with a certain desired audio format */
int open_music(SDL_AudioSpec *mixer)
{
	int music_error;

	music_error = 0;
	/* Set the MikMod music format */
	music_swap8 = 0;
	music_swap16 = 0;
	switch (mixer->format) {

		case AUDIO_U8:
		case AUDIO_S8: {
			if ( mixer->format == AUDIO_S8 ) {
				music_swap8 = 1;
			}
			md_mode = 0;
		}
		break;

		case AUDIO_S16LSB:
		case AUDIO_S16MSB: {
			/* See if we need to correct MikMod mixing */
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
			if ( mixer->format == AUDIO_S16MSB ) {
#else
			if ( mixer->format == AUDIO_S16LSB ) {
#endif
				music_swap16 = 1;
			}
			md_mode = DMODE_16BITS;
		}
		break;

		default: {
			Mix_SetError("Unknown hardware audio format");
			++music_error;
		}
	}
	current_output_channels = mixer->channels;
	current_output_format = mixer->format;
	if ( mixer->channels > 1 ) {
		if ( mixer->channels > MAX_OUTPUT_CHANNELS ) {
			Mix_SetError("Hardware uses more channels than mixer");
			++music_error;
		}
		md_mode |= DMODE_STEREO;
	}
	md_mixfreq	 = mixer->freq;
	md_device	  = 0;
	md_volume	  = 96;
	md_musicvolume = 128;
	md_sndfxvolume = 128;
	md_pansep	  = 128;
	md_reverb	  = 0;
	md_mode |= DMODE_HQMIXER|DMODE_SOFT_MUSIC|DMODE_SURROUND;
//	if(!MikMod_InfoDriver())
	MikMod_RegisterAllDrivers();
//	if(!MikMod_InfoLoader())
	MikMod_RegisterAllLoaders();
	if ( MikMod_Init(NULL) ) {
		Mix_SetError("%s", MikMod_strerror(MikMod_errno));
		++music_error;
	}
#ifdef OGG_MUSIC
	if ( OGG_init(mixer) < 0 ) {
		++music_error;
	}
#endif
	music_playing = NULL;
	music_stopped = 0;
	if ( music_error ) {
		return(-1);
	}
	Mix_VolumeMusic(SDL_MIX_MAXVOLUME);

	/* Calculate the number of ms for each callback */
	ms_per_step = (int) (((float)mixer->samples * 1000.0) / mixer->freq);

	return(0);
}

/* Portable case-insensitive string compare function */
int MIX_string_equals(const char *str1, const char *str2)
{
	while ( *str1 && *str2 ) {
		if ( toupper((unsigned char)*str1) !=
		     toupper((unsigned char)*str2) )
			break;
		++str1;
		++str2;
	}
	return (!*str1 && !*str2);
}

/* Load a music file */
Mix_Music *Mix_LoadMUS(const char *file)
{
	FILE *fp;
	char *ext;
	Uint8 magic[5], moremagic[9];
	Mix_Music *music;

	/* Figure out what kind of file this is */
	fp = fopen(file, "rb");
	if ( (fp == NULL) || !fread(magic, 4, 1, fp) ) {
		if ( fp != NULL ) {
			fclose(fp);
		}
		Mix_SetError("Couldn't read from '%s'", file);
		return(NULL);
	}
	if (!fread(moremagic, 8, 1, fp)) {
		Mix_SetError("Couldn't read from '%s'", file);
		return(NULL);
	}
	magic[4] = '\0';
	moremagic[8] = '\0';
	fclose(fp);

	/* Figure out the file extension, so we can determine the type */
	ext = strrchr(file, '.');
	if ( ext ) ++ext; /* skip the dot in the extension */

	/* Allocate memory for the music structure */
	music = (Mix_Music *)malloc(sizeof(Mix_Music));
	if ( music == NULL ) {
		Mix_SetError("Out of memory");
		return(NULL);
	}
	music->error = 0;

#ifdef OGG_MUSIC
	/* Ogg Vorbis files have the magic four bytes "OggS" */
	if ( (ext && MIX_string_equals(ext, "OGG")) ||
	     strcmp((char *)magic, "OggS") == 0 ) {
		music->type = MUS_OGG;
		music->data.ogg = OGG_new(file);
		if ( music->data.ogg == NULL ) {
			music->error = 1;
		}
	} else
#endif
	if ( 1 ) {
		music->type = MUS_MOD;
		music->data.module = Player_Load((char *)file, 64, 0);
		if ( music->data.module == NULL ) {
			Mix_SetError("%s", MikMod_strerror(MikMod_errno));
			music->error = 1;
		} else {
			/* Stop implicit looping, fade out and other flags. */
			music->data.module->extspd  = 1;
			music->data.module->panflag = 1;
			music->data.module->wrap    = 0;
			music->data.module->loop    = 0;
#if 0 /* Don't set fade out by default - unfortunately there's no real way
         to query the status of the song or set trigger actions.  Hum. */
			music->data.module->fadeout = 1;
#endif
		}
	} else
	{
		Mix_SetError("Unrecognized music format");
		music->error = 1;
	}
	if ( music->error ) {
		free(music);
		music = NULL;
	}
	return(music);
}

/* Free a music chunk previously loaded */
void Mix_FreeMusic(Mix_Music *music)
{
	if ( music ) {
		/* Stop the music if it's currently playing */
		SDL_LockAudio();
		if ( music == music_playing ) {
			/* Wait for any fade out to finish */
			while ( music->fading == MIX_FADING_OUT ) {
				SDL_UnlockAudio();
				SDL_Delay(100);
				SDL_LockAudio();
			}
			if ( music == music_playing ) {
				music_internal_halt();
			}
		}
		SDL_UnlockAudio();
		switch (music->type) {
			case MUS_MOD:
				Player_Free(music->data.module);
				break;
#ifdef OGG_MUSIC
			case MUS_OGG:
				OGG_delete(music->data.ogg);
				break;
#endif
			default:
				/* Unknown music type?? */
				break;
		}
		free(music);
	}
}

/* Find out the music format of a mixer music, or the currently playing
   music, if 'music' is NULL.
*/
Mix_MusicType Mix_GetMusicType(const Mix_Music *music)
{
	Mix_MusicType type = MUS_NONE;

	if ( music ) {
		type = music->type;
	} else {
		SDL_LockAudio();
		if ( music_playing ) {
			type = music_playing->type;
		}
		SDL_UnlockAudio();
	}
	return(type);
}

/* Play a music chunk.  Returns 0, or -1 if there was an error.
 */
static int music_internal_play(Mix_Music *music, double position)
{
	int retval = 0;

	/* Note the music we're playing */
	if ( music_playing ) {
		music_internal_halt();
	}
	music_playing = music;

	/* Set the initial volume */
	if ( music->type != MUS_MOD ) {
		music_internal_initialize_volume();
	}

	/* Set up for playback */
	switch (music->type) {
	    case MUS_MOD:
		Player_Start(music->data.module);
		/* Player_SetVolume() does nothing before Player_Start() */
		music_internal_initialize_volume();
		break;
#ifdef OGG_MUSIC
	    case MUS_OGG:
		OGG_play(music->data.ogg);
		break;
#endif
	    default:
		Mix_SetError("Can't play unknown music type");
		retval = -1;
		break;
	}

	/* Set the playback position, note any errors if an offset is used */
	if ( retval == 0 ) {
		if ( position > 0.0 ) {
			if ( music_internal_position(position) < 0 ) {
				Mix_SetError("Position not implemented for music type");
				retval = -1;
			}
		} else {
			music_internal_position(0.0);
		}
	}

	/* If the setup failed, we're not playing any music anymore */
	if ( retval < 0 ) {
		music_playing = NULL;
	}
	return(retval);
}
int Mix_FadeInMusicPos(Mix_Music *music, int loops, int ms, double position)
{
	int retval;

	/* Don't play null pointers :-) */
	if ( music == NULL ) {
		Mix_SetError("music parameter was NULL");
		return(-1);
	}

	/* Setup the data */
	if ( ms ) {
		music->fading = MIX_FADING_IN;
	} else {
		music->fading = MIX_NO_FADING;
	}
	music->fade_step = 0;
	music->fade_steps = ms/ms_per_step;

	/* Play the puppy */
	SDL_LockAudio();
	/* If the current music is fading out, wait for the fade to complete */
	while ( music_playing && (music_playing->fading == MIX_FADING_OUT) ) {
		SDL_UnlockAudio();
		SDL_Delay(100);
		SDL_LockAudio();
	}
	music_active = 1;
	music_loops = loops;
	retval = music_internal_play(music, position);
	SDL_UnlockAudio();

	return(retval);
}
int Mix_FadeInMusic(Mix_Music *music, int loops, int ms)
{
	return Mix_FadeInMusicPos(music, loops, ms, 0.0);
}
int Mix_PlayMusic(Mix_Music *music, int loops)
{
	return Mix_FadeInMusicPos(music, loops, 0, 0.0);
}

/* Set the playing music position */
int music_internal_position(double position)
{
	int retval = 0;

	switch (music_playing->type) {
	    case MUS_MOD:
		Player_SetPosition((UWORD)position);
		break;
#ifdef OGG_MUSIC
	    case MUS_OGG:
		OGG_jump_to_time(music_playing->data.ogg, position);
		break;
#endif
	    default:
		/* TODO: Implement this for other music backends */
		retval = -1;
		break;
	}
	return(retval);
}

/* Get the playing music position */
double music_internal_getpos(void)
{
	double retval = -1;

	switch (music_playing->type) {
	    case MUS_MOD:
		retval = Player_GetPosition();
		break;
	    default:
		/* TODO: Implement this for other music backends */
		retval = -1;
		break;
	}
	return(retval);
}

/* Get the playing music length */
double music_internal_getlength(void)
{
	double retval = -1;

	switch (music_playing->type) {
	    case MUS_MOD:
		retval = Player_GetLength();
		break;
	    default:
		/* TODO: Implement this for other music backends */
		retval = -1;
		break;
	}
	return(retval);
}

int Mix_SetMusicPosition(double position)
{
	int retval;

	SDL_LockAudio();
	if ( music_playing ) {
		retval = music_internal_position(position);
		if ( retval < 0 ) {
			Mix_SetError("Position not implemented for music type");
		}
	} else {
		Mix_SetError("Music isn't playing");
		retval = -1;
	}
	SDL_UnlockAudio();

	return(retval);
}

double Mix_GetMusicPosition(void)
{
	double retval;

	SDL_LockAudio();
	if ( music_playing ) {
		retval = music_internal_getpos();
		if ( retval < 0 ) {
			Mix_SetError("Position not implemented for music type");
		}
	} else {
		Mix_SetError("Music isn't playing");
		retval = -1;
	}
	SDL_UnlockAudio();

	return(retval);
}

double Mix_GetMusicLength(void)
{
	double retval;

	SDL_LockAudio();
	if ( music_playing ) {
		retval = music_internal_getlength();
		if ( retval < 0 ) {
			Mix_SetError("Position not implemented for music type");
		}
	} else {
		Mix_SetError("Music isn't playing");
		retval = -1;
	}
	SDL_UnlockAudio();

	return(retval);
}

/* Set the music's initial volume */
static void music_internal_initialize_volume(void)
{
	if ( music_playing->fading == MIX_FADING_IN ) {
		music_internal_volume(0);
	} else {
		music_internal_volume(music_volume);
	}
}

/* Set the music volume */
static void music_internal_volume(int volume)
{
	switch (music_playing->type) {
	    case MUS_MOD:
		Player_SetVolume((SWORD)volume);
		break;
#ifdef OGG_MUSIC
	    case MUS_OGG:
		OGG_setvolume(music_playing->data.ogg, volume);
		break;
#endif
	    default:
		/* Unknown music type?? */
		break;
	}
}
int Mix_VolumeMusic(int volume)
{
	int prev_volume;

	prev_volume = music_volume;
	if ( volume < 0 ) {
		return prev_volume;
	}
	if ( volume > SDL_MIX_MAXVOLUME ) {
		volume = SDL_MIX_MAXVOLUME;
	}
	music_volume = volume;
	SDL_LockAudio();
	if ( music_playing ) {
		music_internal_volume(music_volume);
	}
	SDL_UnlockAudio();
	return(prev_volume);
}

/* Halt playing of music */
static void music_internal_halt(void)
{
	switch (music_playing->type) {
	    case MUS_MOD:
		Player_Stop();
		break;
#ifdef OGG_MUSIC
	    case MUS_OGG:
		OGG_stop(music_playing->data.ogg);
		break;
#endif
	    default:
		/* Unknown music type?? */
		return;
	}
	music_playing->fading = MIX_NO_FADING;
	music_playing = NULL;
}
int Mix_HaltMusic(void)
{
	SDL_LockAudio();
	if ( music_playing ) {
		music_internal_halt();
	}
	SDL_UnlockAudio();

	return(0);
}

/* Progressively stop the music */
int Mix_FadeOutMusic(int ms)
{
	int retval = 0;

	SDL_LockAudio();
	if ( music_playing && (music_playing->fading == MIX_NO_FADING) ) {
		music_playing->fading = MIX_FADING_OUT;
		music_playing->fade_step = 0;
		music_playing->fade_steps = ms/ms_per_step;
		retval = 1;
	}
	SDL_UnlockAudio();

	return(retval);
}

Mix_Fading Mix_FadingMusic(void)
{
	Mix_Fading fading = MIX_NO_FADING;

	SDL_LockAudio();
	if ( music_playing ) {
		fading = music_playing->fading;
	}
	SDL_UnlockAudio();

	return(fading);
}

/* Pause/Resume the music stream */
void Mix_PauseMusic(void)
{
	music_active = 0;
}

void Mix_ResumeMusic(void)
{
	music_active = 1;
}

void Mix_RewindMusic(void)
{
	Mix_SetMusicPosition(0.0);
}

int Mix_PausedMusic(void)
{
	return (music_active == 0);
}

/* Check the status of the music */
static int music_internal_playing()
{
	int playing = 1;
	switch (music_playing->type) {
	    case MUS_MOD:
		if ( ! Player_Active() ) {
			playing = 0;
		}
		break;
#ifdef OGG_MUSIC
	    case MUS_OGG:
		if ( ! OGG_playing(music_playing->data.ogg) ) {
			playing = 0;
		}
		break;
#endif
	    default:
		playing = 0;
		break;
	}
	return(playing);
}
int Mix_PlayingMusic(void)
{
	int playing = 0;

	SDL_LockAudio();
	if ( music_playing ) {
		playing = music_internal_playing();
	}
	SDL_UnlockAudio();

	return(playing);
}

/* Set the external music playback command */
int Mix_SetMusicCMD(const char *command)
{
	Mix_HaltMusic();
	if ( music_cmd ) {
		free(music_cmd);
		music_cmd = NULL;
	}
	if ( command ) {
		music_cmd = (char *)malloc(strlen(command)+1);
		if ( music_cmd == NULL ) {
			return(-1);
		}
		strcpy(music_cmd, command);
	}
	return(0);
}

static int _pl_synchro_value;
void Player_SetSynchroValue(int i)
{
	fprintf(stderr,"SDL_mixer: Player_SetSynchroValue is not supported.\n");
	_pl_synchro_value = i;
}

int Player_GetSynchroValue(void)
{
	fprintf(stderr,"SDL_mixer: Player_GetSynchroValue is not supported.\n");
	return _pl_synchro_value;
}

int Mix_SetSynchroValue(int i)
{
	if ( music_playing && ! music_stopped ) {
		switch (music_playing->type) {
		    case MUS_MOD:
			if ( ! Player_Active() ) {
				return(-1);
			}
			Player_SetSynchroValue(i);
			return 0;
			break;
		    default:
			return(-1);
			break;
		}
		return(-1);
	}
	return(-1);
}

int Mix_GetSynchroValue(void)
{
	if ( music_playing && ! music_stopped ) {
		switch (music_playing->type) {
		    case MUS_MOD:
			if ( ! Player_Active() ) {
				return(-1);
			}
			return Player_GetSynchroValue();
			break;
		    default:
			return(-1);
			break;
		}
		return(-1);
	}
	return(-1);
}


/* Uninitialize the music players */
void close_music(void)
{
	Mix_HaltMusic();
	MikMod_Exit();
	MikMod_UnregisterAllLoaders();
	MikMod_UnregisterAllDrivers();
}

typedef struct
{
	MREADER mr;
	int offset;
	int eof;
	SDL_RWops *rw;
} LMM_MREADER;

BOOL LMM_Seek(struct MREADER *mr,long to,int dir)
{
	int at;
	LMM_MREADER* lmmmr=(LMM_MREADER*)mr;
	if(dir==SEEK_SET)
		to+=lmmmr->offset;
	at=SDL_RWseek(lmmmr->rw, to, dir);
	return at<lmmmr->offset;
}
long LMM_Tell(struct MREADER *mr)
{
	int at;
	LMM_MREADER* lmmmr=(LMM_MREADER*)mr;
	at=SDL_RWtell(lmmmr->rw)-lmmmr->offset;
	return at;
}
BOOL LMM_Read(struct MREADER *mr,void *buf,size_t sz)
{
	int got;
	LMM_MREADER* lmmmr=(LMM_MREADER*)mr;
	got=SDL_RWread(lmmmr->rw, buf, sz, 1);
	return got;
}
int LMM_Get(struct MREADER *mr)
{
	unsigned char c;
	int i=EOF;
	LMM_MREADER* lmmmr=(LMM_MREADER*)mr;
	if(SDL_RWread(lmmmr->rw,&c,1,1))
		i=c;
	return i;
}
BOOL LMM_Eof(struct MREADER *mr)
{
	int offset;
	LMM_MREADER* lmmmr=(LMM_MREADER*)mr;
	offset=LMM_Tell(mr);
	return offset>=lmmmr->eof;
}
MODULE *MikMod_LoadSongRW(SDL_RWops *rw, int maxchan)
{
	LMM_MREADER lmmmr={
		{ LMM_Seek,
	  	LMM_Tell,
	  	LMM_Read,
	  	LMM_Get,
	  	LMM_Eof
    },
    
		0,
		0,
		rw
	};
	MODULE *m;
	lmmmr.offset=SDL_RWtell(rw);
	SDL_RWseek(rw,0,SEEK_END);
	lmmmr.eof=SDL_RWtell(rw);
	SDL_RWseek(rw,lmmmr.offset,SEEK_SET);
	m=Player_LoadGeneric((MREADER*)&lmmmr,maxchan,0);
	return m;
}

Mix_Music *Mix_LoadMUS_RW(SDL_RWops *rw) {
	Uint8 magic[5];	  /*Apparently there is no way to check if the file is really a MOD,*/
	/*		    or there are too many formats supported by MikMod or MikMod does */
	/*		    this check by itself. If someone implements other formats (e.g. MP3) */
	/*		    the check can be uncommented */
	Mix_Music *music;
	int start;

	/* Figure out what kind of file this is */
	start = SDL_RWtell(rw);
	if (SDL_RWread(rw,magic,1,4)!=4) {
		Mix_SetError("Couldn't read from RWops");
		return NULL;
	}
	SDL_RWseek(rw, start, SEEK_SET);
	magic[4]='\0';

	/* Allocate memory for the music structure */
	music=(Mix_Music *)malloc(sizeof(Mix_Music));
	if (music==NULL ) {
		Mix_SetError("Out of memory");
		return(NULL);
	}
	music->error = 0;

#ifdef OGG_MUSIC
	/* Ogg Vorbis files have the magic four bytes "OggS" */
	if ( strcmp((char *)magic, "OggS") == 0 ) {
		music->type = MUS_OGG;
		music->data.ogg = OGG_new_RW(rw);
		if ( music->data.ogg == NULL ) {
			music->error = 1;
		}
	} else
#endif
	if (1) {
		music->type=MUS_MOD;
		music->data.module=MikMod_LoadSongRW(rw,64);
		if (music->data.module==NULL) {
			Mix_SetError("%s",MikMod_strerror(MikMod_errno));
			music->error=1;
		}
	} else
	{
		Mix_SetError("Unrecognized music format");
		music->error=1;
	}
	if (music->error) {
		free(music);
		music=NULL;
	}
	return(music);
}
