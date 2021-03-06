// c2gmsksupport.h

//////////////////////
// API version of the GMSK modem for 10m / VHF / UHF communication
// using codec2
// version 4800/0: 4800 bps, 1/3 repetition code FEC
// version 2400/15: 2400 bps, golay code FEC


/* Copyright (C) 2013 Kristoff Bonne ON1ARF

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License version 2.1, as
  published by the Free Software Foundation.  This program is
  distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
  License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with this program; if not, see <http://www.gnu.org/licenses/>.
*/


// Release information
// version 20130310 initial release
// Version 20130314: API c2gmsk version / bitrate control + versionid codes
// Version 20130324: convert into .so shared library
// Version 20130506: 2400/15 bps modem
// Version 20130601: golay FEC for 2400/15 modem
// Version 20130606: change license from GPL to LGPL
// Version 20130614: raw gmskbits option


// c2gmsk API user-level support functions

// functions:
// c2gmsk_param_init
// c2gmsk_msgchain_search
// c2gmsk_msgchain_search_tod
// c2gmsk_msgdecode_printbit
// c2gmsk_msgdecode_numeric
// c2gmsk_msgdecode_codec2
// c2gmsk_getapiversion
// c2gmsk_msgdecode_gmsk
// c2gmsk_msgdecode_gmsk_p


// support functions

///////////////////////////////////////
// INIT GLOBAL PARAMS 

///// create and initiaise parameters
int c2gmsk_param_init (struct c2gmsk_param * param) {


// do nothing if pointer = NULL
if (param) {
	// copy signature for global params
	memcpy(param->signature,PARAM_SIGNATURE,4);

	// init all values to 0 or NULL
	// global params
	param->expected_apiversion=0;

	// modulation
	// (-1 = disable modulation), will be initialised further on
	param->m_bitrate=-1;
	param->m_version=-1;

	// demodulation
	// disable audiolevel check: copy from global vars
	param->d_disableaudiolevelcheck=DISABLE_AUDIOLEVELCHECK;

	// output format: set to 0 (PCM audio output)
	param->outputformat=0;

	// not disable modulator
	param->m_disabled = C2GMSK_NOTDISABLED;

	// not disable demodulator
	param->d_disabled = C2GMSK_NOTDISABLED;
}; // end if

return(C2GMSK_RET_OK);
}; // end function c2gmsk_params_init


///// check signature
int checksign_param (struct c2gmsk_param * param) {
// does it point somewhereN
if (!param) {
	return(C2GMSK_RET_NOVALIDPARAMS);
}; // end if

// check signature
if (memcmp(param->signature,PARAM_SIGNATURE,4)) {
	return(C2GMSK_RET_NOVALIDPARAMS);
}; // end if

return(C2GMSK_RET_OK);

}; // end if


////////////////////////////////////////////////
///////////// MSGCHAIN SEARCH 

void * c2gmsk_msgchain_search (int where, struct c2gmsk_msgchain * chain, int * tod, int *datasize, int * numelem) {
int ret;

void * posnext;

// sanity checking
ret=checksign_chain(chain);
if (ret != C2GMSK_RET_OK) {
	return(NULL);
}; // end if

// if "where" is "C2GSM_SEARCH_POSSTART", reset counter
if (where == C2GMSK_SEARCH_POSSTART)  {
	chain->s_pos=0;
	chain->s_numelem=0;
}; // end if

if ((chain->s_pos < chain->used) && (chain->s_numelem < chain->numelem)) {
	struct c2gmsk_msg * thismsg;

	posnext=&chain->data[chain->s_pos];

	thismsg = (struct c2gmsk_msg*) posnext;

	// does the message completely fit in the chain?
	if ((chain->s_pos + thismsg->datasize + sizeof(struct c2gmsk_msg)) <= chain->used) {

		// set "tod" and "numelem" values
		// only set them if the pointers do actually point somewhere
		if (tod) {
			*tod=thismsg->tod;
		}; // end if

		if (datasize) {
			*datasize=thismsg->datasize;
		}; // end if

		if (numelem) {
			*numelem=chain->s_numelem;
		}; // end if

		// OK, we have a valid message
		chain->s_pos += (sizeof(struct c2gmsk_msg) + thismsg->datasize);
		chain->s_numelem ++;	

		// now return, pointing to next msg
		return(posnext);
	}; 
}; // end if

// nothing found
// clear type-of-data field
if (tod) {
	*tod=0;
}; // end if
return(NULL);
	
}; // end function msgchain  search


///////////////////////////////////////////////
/// function msgchain search for particular type-of-data
void * c2gmsk_msgchain_search_tod (int where, struct c2gmsk_msgchain * chain, int tod, int * datasize, int * numelem) {
int ret;
int thistod;
int thisdatasize;

struct c2gmsk_msg * posnext;

// sanity checking
ret=checksign_chain(chain);
if (ret != C2GMSK_RET_OK) {
	return(NULL);
}; // end if

// if where is "C2GSM_SEARCH_POSSTART", reset counter
if (where == C2GMSK_SEARCH_POSSTART)  {
	chain->s_pos=0;
	chain->s_numelem=0;
}; // end if

// "ret" is now used as tempory value for the "numelem" var

// loop while something in queue
posnext = c2gmsk_msgchain_search(C2GMSK_SEARCH_POSCURRENT, chain, &thistod, &thisdatasize, &ret);

while (posnext) {
	// ok, we still have something in the queue

	// is the the type-of-data we are looking for?
	if (thistod == tod) {
		// set "datasize" value
		if (datasize) {
			*datasize=thisdatasize;
		}; // end if

		// set "numelem" value
		if (numelem) {
			*numelem=ret;
		}; // end if

		// done, now return
		return(posnext);
	}; // end if

	// not correct type-of-data, try next message in chain
	posnext = c2gmsk_msgchain_search(C2GMSK_SEARCH_POSCURRENT, chain, &thistod, &thisdatasize, &ret);
}; // end while


// nothing found, return NULL
return(NULL);

}; // end function msgchain search for a particular type-of-data



// checksignature function for messages
int checksign_msg (struct c2gmsk_msg * msg) {
// used for sanity checking

// does it point somewhere
if (!msg) {
	return(C2GMSK_RET_NOVALIDMSG);
}; // end if

// check signature
if (memcmp(msg->signature,MSG_SIGNATURE,4)) {
	return(C2GMSK_RET_NOVALIDMSG);
}; // end if

// ok
return(C2GMSK_RET_OK);
}; // end function check signature for "message"

//////////////////////////////////////////////////
// function to decode "printbit" message
// a message-buffer of up to 203 octets is expected.
// message itself can be up to 192 chars
// add 8 chars for PLL syncing
// add 2 chars for marker
// add 1 char for terminating NULL


char * c2gmsk_msgdecode_printbit (struct c2gmsk_msg * msg, char * txtbuffer, int marker) {
int ret;

// some sanity cheching
// sanity checking
ret=checksign_msg(msg);

if (ret != C2GMSK_RET_OK) {
	return(NULL);
}; // end if

// does the text buffer point somewhere?
if (!txtbuffer) {
	return(NULL);
}; // end if


// for "printbit modulated" type
if (msg->tod == C2GMSK_PRINTBIT_MOD) {
c2gmsk_msgrealwithmarker * pmsg;
	int size;
	char * pchar;

	pmsg=(c2gmsk_msgrealwithmarker*) msg;

	size=pmsg->realsize;

	if (size <= 0) {
		// return if no data
		return(NULL);
	}; // end if

	if (size > 200) {
		// memory size protection
		size=200;
	}; // end if

	// copy text
	memcpy(txtbuffer,pmsg->data,size);

	// set pointer to char after text
	pchar=&txtbuffer[size];

	// do we need a add a marker?
	if (marker) {
		*pchar=' '; pchar++;
		*pchar=(char) pmsg->marker; pchar++;
	}; // end if

	// add terminating NULL
	*pchar=0;

	// done, return pointer to textbuffer
	return(txtbuffer);
}; // end if


// for "printbit ALL" type
if (msg->tod == C2GMSK_PRINTBIT_ALL) {
	c2gmsk_msgreal * pmsg;
	int size;

	pmsg=(c2gmsk_msgreal*) msg;

	size=pmsg->realsize;

	if (size <= 0) {
		// return if no data
		return(NULL);
	}; // end if

	if (size > 202) {
		// memory size protection
		size=202;
	}; // end if

	// copy text
	memcpy(txtbuffer,pmsg->data,size);

	// add terminating NULL
	txtbuffer[size]=0;

	
	// done, return pointer to textbuffer
	return(txtbuffer);
}; // end if

// wrong type-of-data
return(NULL);

}; // end function c2gmsk_msgdecode_printbit (c2gmsk_msg * msg, char * txtbuffer, int marker) {


///////////////////////////////////////////////
// function message decode for message returning numeric data

// returns number of data elements (max 4)
// return 0 if not a type-of-data valid for msgdecode numeric
// return -1 on failure of sanity checks


int c2gmsk_msgdecode_numeric (struct c2gmsk_msg * msg, int *data) {
int ret;

// sanity checking
ret=checksign_msg(msg);

// error: return value < 0
if (ret != C2GMSK_RET_OK) {
	return(-ret);
}; // end if

if (!data) {
	return(-C2GMSK_RET_NOVALIDRETPOINT);
}; // end if

// check changes per type of data
switch (msg->tod) {
	// no vars returned
	case C2GMSK_MSG_NODATA:
	case C2GMSK_MSG_AUXDATA_DONE:
		{
		return(0);
		}; // done

	// 1 var returned
	case C2GMSK_MSG_AUDIOAVGLEVEL:
	case C2GMSK_MSG_INAUDIOINVERT:
	case C2GMSK_MSG_FECSTATS:
	case C2GMSK_MSG_BITRATE:
		{
		c2gmsk_msg_1 * msg1;

		msg1=(c2gmsk_msg_1*) msg;
		data[0]=msg1->data0;

		return(1);
		};
	// 2 vars returned
	case C2GMSK_MSG_STATECHANGE:
	case C2GMSK_MSG_CAPABILITIES:
		{
		c2gmsk_msg_2 * msg2;

		msg2=(c2gmsk_msg_2*) msg;
		data[0]=msg2->data0;
		data[1]=msg2->data1;

		return(2);
		};
	// 3 vars returned
		// (none)

	// 3 vars returned
		// (none)

	// 3 vars returned
	case C2GMSK_MSG_VERSIONID:
		{
		c2gmsk_msg_4 * msg4;

		msg4=(c2gmsk_msg_4*) msg;
		data[0]=msg4->data0;
		data[1]=msg4->data1;
		data[2]=msg4->data2;
		data[3]=msg4->data3;

		return(4);
		};

	// all the rest, return nothing
	default:
		return(0);
}; // end switch

// we should never get here
return(-1);
}; // end function msgdecode numeric


///////////////////////////////////////////
///////// function MSG DECODE codec2
// c2buff should point to buffer of at least 8 octets to hold c2data in all bitrates

int c2gmsk_msgdecode_c2 (struct c2gmsk_msg * msg, unsigned char * c2buff) {
c2gmsk_msg_codec2 *msgc2;
int ret;


// sanity checking
ret=checksign_msg(msg);

if (ret != C2GMSK_RET_OK) {
	return(-ret);
}; // end if

// function is only valid for a "codec2" message
if (msg->tod != C2GMSK_MSG_CODEC2) {
	return(-C2GMSK_RET_NOVALIDTOD);
}; // end if

if (!c2buff) {
	return(-C2GMSK_RET_NOVALIDRETPOINT);
}; // end fi

msgc2=(c2gmsk_msg_codec2*) msg;


// copy codec2 data: 1400 bps: 7 octets
if (msgc2->version == C2GMSK_CODEC2_1400) {
	memcpy(c2buff,msgc2->c2data,7);
	// return version of codec2
	return(msgc2->version);
}; // end if

// copy codec2 data: 1200 or 2400 bps: 8 octets
memcpy(c2buff,msgc2->c2data,8);
// return version of codec2
return(msgc2->version);

}; // end function msgdecode codec2


///////////////////////////////////////////
///////// function MSG DECODE pcm48k


int c2gmsk_msgdecode_pcm48k (struct c2gmsk_msg * msg, int16_t pcmbuff[]) {
int size;
c2gmsk_msg_pcm48k *msgp;
int ret;

// sanity checking
ret=checksign_msg(msg);

if (ret != C2GMSK_RET_OK) {
	return(-ret);
}; // end if

if (!pcmbuff) {
	return(C2GMSK_RET_NOVALIDRETPOINT);
}; // end fi

// function is only valid for a "codec2" message
if (msg->tod != C2GMSK_MSG_PCM48K) {
	return(C2GMSK_RET_NOVALIDTOD);
}; // end if

msgp=(c2gmsk_msg_pcm48k*) msg;

size=msgp->datasize;
if (size > 3840) {
	size=3840; // 1920 samples @ 16 bit/sample = 3840 octets
}; // end if

memcpy(pcmbuff,msgp->pcm,size);

return(size>>1); ; // size = octets: divide by 2 to have samples

}; // end function msgdecode pcm48k


///////////////////////////////////////////
///////// function MSG DECODE pcm48k, "return pointer" version

// this function does not copy the data to the destination, but just returns a pointer to it
// should save one "memory copy" to do

int c2gmsk_msgdecode_pcm48k_p (struct c2gmsk_msg * msg, int16_t *pcmbuff[]) {
int size;
c2gmsk_msg_pcm48k *msgp;
int ret;

// sanity checking
ret=checksign_msg(msg);

if (ret != C2GMSK_RET_OK) {
	return(-ret);
}; // end if


if (!pcmbuff) {
	return(C2GMSK_RET_NOVALIDRETPOINT);
}; // end fi

// function is only valid for a "codec2" message
if (msg->tod != C2GMSK_MSG_PCM48K) {
	return(C2GMSK_RET_NOVALIDTOD);
}; // end if

msgp=(c2gmsk_msg_pcm48k*) msg;

size=msgp->datasize;
if (size > 3840) {
	size=3840; // 1920 samples @ 16 bit/sample = 3840 octets
}; // end if


// copy pointer to buffer in pcmbuffer
*pcmbuff=msgp->pcm;

return(size>>1); ; // size = octets: divide by 2 to have samples

}; // end function msgdecode pcm48k_p


///////// function MSG DECODE gmsk96
// gmskbuff should point to buffer of at least 12 or 24 unsigned char
// (40 ms @ 2400 bps = 96 bits = 12 octets, 40 ms @ 4800 bps = 192 bits = 24 octets)
int c2gmsk_msgdecode_gmsk (struct c2gmsk_msg * msg, unsigned char * gmskbuff) {
c2gmsk_msg_rawgmsk *msgp;
int ret;

// sanity checking
ret=checksign_msg(msg);

if (ret != C2GMSK_RET_OK) {
	return(-ret);
}; // end if

if (!gmskbuff) {
	return(C2GMSK_RET_NOVALIDRETPOINT);
}; // end fi

// function is only valid for a "rawgmsk" message
if ((msg->tod != C2GMSK_MSG_RAWGMSK_96) && (msg->tod != C2GMSK_MSG_RAWGMSK_192)) {
	return(C2GMSK_RET_NOVALIDTOD);
}; // end if

// OK, it's a GMSK message
msgp=(c2gmsk_msg_rawgmsk*) msg;

// check size.

if ((msg->tod == C2GMSK_MSG_RAWGMSK_96) && (msg->datasize != 12)) {
	return(0);
}; // end if

if ((msg->tod == C2GMSK_MSG_RAWGMSK_192) && (msg->datasize != 24)) {
	return(0);
}; // end if


memcpy(gmskbuff,msgp->data,msg->datasize);

// DONE
return(msg->datasize); 
}; // end function msgdecode gmsk


///////////////////////////////////////////
///////// function MSG DECODE gmskk, "return pointer" version

// this function does not copy the data to the destination, but just returns a pointer to it
// should save one "memory copy" to do

int c2gmsk_msgdecode_gmsk_p (struct c2gmsk_msg * msg, unsigned char ** gmskbuff) {
c2gmsk_msg_rawgmsk *msgp;
int ret;

// sanity checking
ret=checksign_msg(msg);

if (ret != C2GMSK_RET_OK) {
	return(-ret);
}; // end if

if (!gmskbuff) {
	return(C2GMSK_RET_NOVALIDRETPOINT);
}; // end fi

// function is only valid for a "rawgmsk" message
if ((msg->tod != C2GMSK_MSG_RAWGMSK_96) && (msg->tod != C2GMSK_MSG_RAWGMSK_192)) {
	return(C2GMSK_RET_NOVALIDTOD);
}; // end if

// OK, it's a GMSK message
msgp=(c2gmsk_msg_rawgmsk*) msg;

// check size.

if ((msg->tod == C2GMSK_MSG_RAWGMSK_96) && (msg->datasize != 12)) {
	return(0);
}; // end if

if ((msg->tod == C2GMSK_MSG_RAWGMSK_192) && (msg->datasize != 24)) {
	return(0);
}; // end if

// copy pointer to buffer in gmskbuff 
*gmskbuff=msgp->data;


// done
return(msg->datasize); 

}; // end function msgdecode gmsk_p


///////////////////
// get API version
int c2gmsk_getapiversion () {
return(C2GMSK_APIVERSION);
}; // end function get API version
