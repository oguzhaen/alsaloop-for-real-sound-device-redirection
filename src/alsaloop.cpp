/**
 * @file alsaloop.cpp
 * @author Oguzhan MUTLU (oguzhan.mutlu@daiichi.com)
 * @brief Bu modülde Alsa Loop device kullanmak için 
 *        gerekli yardımcı fonksiyonlar bulunur.
 * @version 0.1
 * @date 2019-11-27
 * 
 * @copyright Copyright (c) 2019
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#include <errno.h>
#include <getopt.h>
#include <alsa/asoundlib.h>
#include <sys/time.h>
#include <math.h>
#include <pthread.h>
#include <thread>
#include <signal.h>
#include <exception>
#include <syslog.h>
#include <iostream>
#include <functional>

#include "alsaloop.h"


AlsaLoop::AlsaLoop(/* args */)
{
}

AlsaLoop::~AlsaLoop()
{
}

static void myExit(struct loopbackThread *thread, int exitcode)
{
	int i;

	std::cout << "Exit from thread" << std::endl;

	for (i = 0; i < thread->loopbacks_count; i++)
	{
		pcmjob_done(thread->loopbacks[i]);
	}
		
	if (thread->threaded) 
	{
		thread->exitcode = exitcode;
		pthread_exit(0);
	}

	exit(exitcode);
}

static int createLoopbackHandle(struct loopback_handle **_handle,
				  				const char *device,
				  				const char *ctldev,
				  				const char *id)
{
	char idbuf[1024];
	struct loopback_handle *handle;

	handle = (struct loopback_handle *) calloc(1, sizeof(*handle));
	if (handle == NULL)
		return -ENOMEM;
	if (device == NULL)
		device = "hw:0,0";
	handle->device = strdup(device);
	if (handle->device == NULL)
		return -ENOMEM;
	if (ctldev) {
		handle->ctldev = strdup(ctldev);
		if (handle->ctldev == NULL)
			return -ENOMEM;
	} else {
		handle->ctldev = NULL;
	}
	snprintf(idbuf, sizeof(idbuf)-1, "%s %s", id, device);
	idbuf[sizeof(idbuf)-1] = '\0';
	handle->id = strdup(idbuf);
	handle->access = SND_PCM_ACCESS_RW_INTERLEAVED;
	handle->format = SND_PCM_FORMAT_S16_LE;
	handle->rate = handle->rate_req = 48000;
	handle->channels = 2;
	handle->resample = 0;
	*_handle = handle;
	return 0;
}

static int createLoopback(struct loopback **_handle,
			   struct loopback_handle *play,
			   struct loopback_handle *capt,
			   snd_output_t *output)
{
	struct loopback *handle;

	handle = (struct loopback *) calloc(1, sizeof(*handle));
	if (handle == NULL)
		return -ENOMEM;
	handle->play = play;
	handle->capt = capt;
	play->loopback = handle;
	capt->loopback = handle;
	handle->latency_req = 0;
	handle->latency_reqtime = 10000;
	handle->loop_time = ~0UL;
	handle->loop_limit = ~0ULL;
	handle->output = output;
	handle->state = output;
#ifdef USE_SAMPLERATE
	handle->src_enable = 1;
	handle->src_converter_type = SRC_SINC_BEST_QUALITY;
#endif
	*_handle = handle;
	return 0;
}

static void setScheduler(void)
{
	struct sched_param sched_param;

	if (sched_getparam(0, &sched_param) < 0)
	{
		logit(LOG_WARNING, "Scheduler getparam failed.\n");
		return;
	}
	sched_param.sched_priority = sched_get_priority_max(SCHED_RR);
	if (!sched_setscheduler(0, SCHED_RR, &sched_param)) 
	{
		if (verbose)
			logit(LOG_WARNING, 
				"Scheduler set to Round Robin with priority %i\n", 
				sched_param.sched_priority);
		return;
	}
	if (verbose)
		logit(LOG_INFO, 
			"!!!Scheduler set to Round Robin with priority %i FAILED!\n", 
			sched_param.sched_priority);
}

static long timeDiff(struct timeval t1, struct timeval t2)
{
	signed long l;

	t1.tv_sec -= t2.tv_sec;
	l = (signed long) t1.tv_usec - (signed long) t2.tv_usec;
	if (l < 0) 
	{
		t1.tv_sec--;
		l = 1000000 + l;
		l %= 1000000;
	}
	return (t1.tv_sec * 1000000) + l;
}

void * AlsaLoop::threadJob1(void *_data)
{
	struct loopbackThread *thread = (struct loopbackThread *) _data;
	snd_output_t *output = thread->output;
	struct pollfd *pfds = NULL;
	int pfds_count = 0;
	int i, j, err, wake = 1000000;

	std::cout << "Thread Entered" << std::endl;

	setScheduler();

	for (i = 0; i < thread->loopbacks_count; i++) 
	{
		err = pcmjob_init(thread->loopbacks[i]);
		if (err < 0) 
		{
			logit(LOG_CRIT, "Loopback initialization failure.\n");
			myExit(thread, EXIT_FAILURE);
		}
	}
	for (i = 0; i < thread->loopbacks_count; i++) 
	{
		err = pcmjob_start(thread->loopbacks[i]);
		if (err < 0) 
		{
			logit(LOG_CRIT, "Loopback start failure.\n");
			myExit(thread, EXIT_FAILURE);
		}
		pfds_count += thread->loopbacks[i]->pollfd_count;
		j = thread->loopbacks[i]->wake;
		if (j > 0 && j < wake)
			wake = j;
	}
	if (wake >= 1000000)
		wake = -1;
	pfds = (pollfd *) calloc(pfds_count, sizeof(struct pollfd));
	if (pfds == NULL || pfds_count <= 0) 
	{
		logit(LOG_CRIT, "Poll FDs allocation failed.\n");
		myExit(thread, EXIT_FAILURE);
	}
	while (!quit) 
	{
		struct timeval tv1, tv2;
		for (i = j = 0; i < thread->loopbacks_count; i++) 
		{
			err = pcmjob_pollfds_init(thread->loopbacks[i], &pfds[j]);
			if (err < 0) 
			{
				logit(LOG_CRIT, "Poll FD initialization failed.\n");
				myExit(thread, EXIT_FAILURE);
			}
			j += err;
		}
		if (verbose > 10)
			gettimeofday(&tv1, NULL);
		err = poll(pfds, j, wake);
		if (err < 0)
			err = -errno;
		if (verbose > 10) 
		{
			gettimeofday(&tv2, NULL);
			snd_output_printf(output, "pool took %lius\n", timeDiff(tv2, tv1));
		}
		if (err < 0) 
		{
			if (err == -EINTR || err == -ERESTART)
				continue;
			logit(LOG_CRIT, "Poll failed: %s\n", strerror(-err));
			myExit(thread, EXIT_FAILURE);
		}
		for (i = j = 0; i < thread->loopbacks_count; i++) 
		{
			struct loopback *loop = thread->loopbacks[i];
			if (j < loop->active_pollfd_count)
			 {
				err = pcmjob_pollfds_handle(loop, &pfds[j]);
				if (err < 0) 
				{
					logit(LOG_CRIT, "pcmjob failed.\n");
					exit(EXIT_FAILURE);
				}
			}
			j += loop->active_pollfd_count;
		}
	}

	myExit(thread, EXIT_SUCCESS);
}

void AlsaLoop::addLoop(struct loopback *loop)
{
	loopbacks = (struct loopback **) realloc(loopbacks, (loopbacks_count + 1) *
						sizeof(struct loopback *));
	if (loopbacks == NULL) 
	{
		logit(LOG_CRIT, "No enough memory\n");
		exit(EXIT_FAILURE);
	}
	loopbacks[loopbacks_count++] = loop;
}

bool AlsaLoop::initConnection(char * realDev, char * loopDev, snd_output_t *output)
{
	int err;
	char *arg_config = NULL;
	char *arg_pdevice = realDev;
	char *arg_cdevice = loopDev;
	char *arg_pctl = NULL;
	char *arg_cctl = NULL;
	unsigned int arg_latency_req = 0;
	unsigned int arg_latency_reqtime = 300000;
	snd_pcm_format_t arg_format = SND_PCM_FORMAT_S16_LE;
	unsigned int arg_channels = 2;
	unsigned int arg_rate = 48000;
	snd_pcm_uframes_t arg_buffer_size = 0;
	snd_pcm_uframes_t arg_period_size = 0;
	unsigned long arg_loop_time = ~0UL;
	int arg_nblock = 0;
	int arg_effect = 0;
	int arg_resample = 0;
#ifdef USE_SAMPLERATE
	int arg_samplerate = SRC_SINC_FASTEST + 1;
#endif
	int arg_sync = SYNC_TYPE_AUTO;
	int arg_slave = SLAVE_TYPE_AUTO;
	int arg_thread = 0;
	struct loopback *loop = NULL;
	char *arg_mixers[MAX_MIXERS];
	int arg_mixers_count = 0;
	char *arg_ossmixers[MAX_MIXERS];
	int arg_ossmixers_count = 0;
	int arg_xrun = arg_default_xrun;
	int arg_wake = arg_default_wake;

	struct loopback_handle *play;
	struct loopback_handle *capt;

	err = createLoopbackHandle(&play, arg_pdevice, arg_pctl, "playback");
	if (err < 0) 
	{
		logit(LOG_CRIT, "Unable to create playback handle.\n");
		return false;
	}
	err = createLoopbackHandle(&capt, arg_cdevice, arg_cctl, "capture");
	if (err < 0) 
	{
		logit(LOG_CRIT, "Unable to create capture handle.\n");
		return false;
	}
	err = createLoopback(&loop, play, capt, output);
	if (err < 0) 
	{
		logit(LOG_CRIT, "Unable to create loopback handle.\n");
		return false;
	}
	
	play->format = capt->format = arg_format;
	play->rate = play->rate_req = capt->rate = capt->rate_req = arg_rate;
	play->channels = capt->channels = arg_channels;
	play->buffer_size_req = capt->buffer_size_req = arg_buffer_size;
	play->period_size_req = capt->period_size_req = arg_period_size;
	play->resample = capt->resample = arg_resample;
	play->nblock = capt->nblock = arg_nblock ? 1 : 0;
	loop->latency_req = arg_latency_req;
	loop->latency_reqtime = arg_latency_reqtime;
	loop->sync = (sync_type_t) arg_sync;
	loop->slave = (slave_type_t) arg_slave;
	loop->thread = arg_thread;
	loop->xrun = arg_xrun;
	loop->wake = arg_wake;

#ifdef USE_SAMPLERATE
	loop->src_enable = arg_samplerate > 0;
	if (loop->src_enable)
		loop->src_converter_type = arg_samplerate - 1;
#endif
	addLoop(loop);
	return true;
}

typedef void * (*THREADFUNCPTR)(void *);

void AlsaLoop::threadJob(struct loopbackThread *thread)
{
	if (!thread->threaded) 
	{
		threadJob1(thread);
		return;
	}

	pthread_create(&thread->thread, NULL, (THREADFUNCPTR) &AlsaLoop::threadJob1, thread);
}

int AlsaLoop::sortThreads(snd_output_t * output)
{
	int i, j, k, l;
	j = -1;
	do 
	{
		k = 0x7fffffff;
		for (i = 0; i < loopbacks_count; i++) 
		{
			if (loopbacks[i]->thread < k &&
			    loopbacks[i]->thread > j)
				k = loopbacks[i]->thread;
		}
		j++;
		for (i = 0; i < loopbacks_count; i++) 
		{
			if (loopbacks[i]->thread == k)
				loopbacks[i]->thread = j;
		}
	} while (k != 0x7fffffff);
	/* fix maximum thread id */
	for (i = 0, j = -1; i < loopbacks_count; i++) 
	{
		if (loopbacks[i]->thread > j)
			j = loopbacks[i]->thread;
	}
	j += 1;
	threads = (struct loopbackThread *) calloc(1, sizeof(struct loopbackThread) * j);
	if (threads == NULL) 
	{
		logit(LOG_CRIT, "No enough memory\n");
		exit(EXIT_FAILURE);
	}
	/* sort all threads */
	for (k = 0; k < j; k++) 
	{
		for (i = l = 0; i < loopbacks_count; i++)
			if (loopbacks[i]->thread == k)
				l++;
		threads[k].loopbacks = (struct loopback **) malloc(l * sizeof(struct loopback *));
		threads[k].loopbacks_count = l;
		threads[k].output = output;
		threads[k].threaded = j > 0;
		for (i = l = 0; i < loopbacks_count; i++)
			if (loopbacks[i]->thread == k)
				threads[k].loopbacks[l++] = loopbacks[i];
	}
	threads_count = j;
	main_job = pthread_self();

	return j;
}

void AlsaLoop::sendToAll(int sig)
{
	struct loopbackThread *thread;
	int i;

	for (i = 0; i < threads_count; i++) 
	{
		thread = &threads[i];
		if (thread->threaded)
		{
			pthread_kill(thread->thread, sig);
		}
	}
}

void AlsaLoop::signalHandler(int sig)
{
	quit = 1;
	sendToAll(SIGUSR2);
}

void AlsaLoop::setQuit()
{
    quit = 1;
}

void AlsaLoop::clearQuit()
{
    quit = 0;
}

void AlsaLoop::runThreads()
{
    for (int k = 0; k < threads_count; k++)
	{
		threadJob(&threads[k]);
	}
}

void AlsaLoop::joinFromThreads()
{
    for (int k = 0; k < threads_count; k++)
	{
		pthread_join(threads[k].thread, NULL);
	}
}

void AlsaLoop::freeThreads()
{
	free(threads);
	loopbacks_count = 0;
}