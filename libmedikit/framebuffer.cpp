#include "framebuffer.h"


bool AstFrameBuffer::Add(const ast_frame * f)
{
	DWORD seq;
	
	if (!ast_test_flag(f, AST_FRFLAG_HAS_TIMING_INFO) || f->len < 2 || f->ts < 0)
	    return false;
	    
	seq = f->rxseqno;
	
	//Lock
	pthread_mutex_lock(&mutex);

	//If already past
	if (next!=(DWORD)-1 && seq < next)
	{
		bigJumps++;
		//Unlock
		pthread_mutex_unlock(&mutex);
		//Delete pacekt
		//Skip it and lost forever
		if ( bigJumps > 20)
		{
			ast_log(LOG_WARNING, "Too many out of sequence packet. Resyncing.\n");
			next=(DWORD)-1;
		}
		else
		{
			//Delete pacekt
			//ast_frfree(f);
			ast_log(LOG_WARNING, "-Out of order non recoverable packet: seq=%d, next=%d\n");
			return false;
		}
	}

	//Add event
	packets[seq] = ast_frdup(f);

	//Unlock
	pthread_mutex_unlock(&mutex);

	//Signal
	pthread_cond_signal(&cond);

	return true;
}

struct ast_frame * AstFrameBuffer::Wait()
{
	//NO packet
	struct ast_frame * rtp = NULL;

	//Get default wait time
	DWORD timeout = maxWaitTime;

	//Lock
	pthread_mutex_lock(&mutex);

	//While we have to wait
	while (!cancel)
	{
		//Check if we have somethin in queue
		if (!packets.empty())
		{
			int ret = ETIMEDOUT;
			//Get first
			RTPOrderedPackets::iterator it = packets.begin();
			//Get first seq num
			DWORD seq = it->first;
			//Get packet
			struct ast_frame * candidate = it->second;
			//Get time of the packet
			QWORD time = candidate->GetTime();

			//Check if first is the one expected or wait if not
			if (next==(DWORD)-1 || seq==next || time+maxWaitTime<getTime()/1000 || hurryUp)
			{
				//We have it!
				rtp = candidate;
				//Update next
				next = seq+1;
				//Remove it
				packets.erase(it);
				//Return it!
				break;
			}

			//We have to wait
			timespec ts;
			//Calculate until when we have to sleep
			ts.tv_sec  = (time_t) ((time+maxWaitTime) / 1e6) ;
			ts.tv_nsec = (long) ((time+maxWaitTime) - ts.tv_sec*1e6);
			
			//Wait with time out
			if (blocking) ret = pthread_cond_timedwait(&cond,&mutex,&ts);
			//Check if there is an errot different than timeout
			if (ret && ret!=ETIMEDOUT)
				//Print error
				Error("-WaitQueue cond timedwait error [%d,%d]\n",ret,errno);
			
		} else {
			//Not hurryUp more
			hurryUp = false;
			//Wait until we have a new rtp pacekt
			if (blocking) ret = pthread_cond_wait(&cond,&mutex);
			//Check error
			if (ret &  && ret!=ETIMEDOUT)
				//Print error
				Error("-WaitQueue cond timedwait error [%rd,%d]\n",ret,errno);
		}
	}
	
	//Unlock
	pthread_mutex_unlock(&mutex);

	//canceled
	return rtp;
}

struct AstFb *AstFbCreate(struct AstFb *fb, DWORD maxWaitTime, int blocking)
{
	AstFrameBuffer * fb = new AstFrameBuffer((bool) blocking);
	if (fb)
	{
		fb->SetMaxWaitTime(maxWaitTime);
	}
	return (struct AstFb *) fb;
}

int AstFbAddFrame( struct AstFb *fb, struct ast_frame *f )
{
	return ((AstFrameBuffer *) fb)->Add( f );
}

struct ast_frame * AstFbGetFrame(struct AstFb *fb)
{
	return ((AstFrameBuffer *) fb)->Wait();
}

void AstFbCancel(struct AstFb *fb)
{
	((AstFrameBuffer *) fb)->Cancel();
}

void AstFbReset(struct AstFb *fb)
{
	((AstFrameBuffer *) fb)->Reset();
} 
 
DWORD AstFbLength(struct AstFb *fb)
{
	return ((AstFrameBuffer *) fb)->Length();
}
void AstFbDestroy(struct AstFb *fb)
{
	AstFrameBuffer * fb2 = (AstFrameBuffer *) fb;
	if (fb2) delete fb2;
}