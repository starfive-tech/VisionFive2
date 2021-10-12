/********************************************************************
 Copyright (C) 2002-2009 Xiph.org Foundation

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

- Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

- Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

- Neither the name of the Xiph.org Foundation nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION
OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************/

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "internal.h"



/*This is more or less the same as strncasecmp, but that doesn't exist
everywhere, and this is a fairly trivial function, so we include it.
Note: We take advantage of the fact that we know _n is less than or equal to
the length of at least one of the strings.*/
static int32_t oc_tagcompare(const char *_s1,const char *_s2,int32_t _n)
{
    int32_t c;
    for(c=0;c<_n;c++){
        if(toupper(_s1[c])!=toupper(_s2[c]))return !0;
    }
    return _s1[c]!='=';
}



void th_info_init(th_info *_info)
{
    memset(_info,0,sizeof(*_info));
    _info->version_major=TH_VERSION_MAJOR;
    _info->version_minor=TH_VERSION_MINOR;
    _info->version_subminor=TH_VERSION_SUB;
    _info->keyframe_granule_shift=6;
}

void th_info_clear(th_info *_info){
    memset(_info,0,sizeof(*_info));
}



void th_comment_init(th_comment *_tc)
{
    memset(_tc,0,sizeof(*_tc));
}

void th_comment_add(th_comment *_tc,char *_comment)
{
    char **user_comments;
    int32_t   *comment_lengths;
    int32_t    comment_len;
    user_comments=realloc(_tc->user_comments,
        (_tc->comments+2)*sizeof(*_tc->user_comments));
    if(user_comments==NULL)return;
    _tc->user_comments=user_comments;
    comment_lengths=realloc(_tc->comment_lengths,
        (_tc->comments+2)*sizeof(*_tc->comment_lengths));
    if(comment_lengths==NULL)return;
    _tc->comment_lengths=comment_lengths;
    comment_len=strlen(_comment);
    comment_lengths[_tc->comments]=comment_len;
    user_comments[_tc->comments]=malloc(comment_len+1);
    if(user_comments[_tc->comments]==NULL)return;
    memcpy(_tc->user_comments[_tc->comments],_comment,comment_len+1);
    _tc->comments++;
    _tc->user_comments[_tc->comments]=NULL;
}

void th_comment_add_tag(th_comment *_tc,char *_tag,char *_val)
{
    char *comment;
    int32_t   tag_len;
    int32_t   val_len;
    tag_len=strlen(_tag);
    val_len=strlen(_val);
    /*+2 for '=' and '\0'.*/
    comment=malloc(tag_len+val_len+2);
    if(comment==NULL)return;
    memcpy(comment,_tag,tag_len);
    comment[tag_len]='=';
    memcpy(comment+tag_len+1,_val,val_len+1);
    th_comment_add(_tc,comment);
    free(comment);
}

char *th_comment_query(th_comment *_tc,char *_tag,int32_t _count)
{
    long i;
    int32_t  found;
    int32_t  tag_len;
    tag_len=strlen(_tag);
    found=0;
    for(i=0;i<_tc->comments;i++){
        if(!oc_tagcompare(_tc->user_comments[i],_tag,tag_len)){
            /*We return a pointer to the data, not a copy.*/
            if(_count==found++)return _tc->user_comments[i]+tag_len+1;
        }
    }
    /*Didn't find anything.*/
    return NULL;
}

int32_t th_comment_query_count(th_comment *_tc,char *_tag)
{
    long i;
    int32_t  tag_len;
    int32_t  count;
    tag_len=strlen(_tag);
    count=0;
    for(i=0;i<_tc->comments;i++){
        if(!oc_tagcompare(_tc->user_comments[i],_tag,tag_len))count++;
    }
    return count;
}

void th_comment_clear(th_comment *_tc)
{
    if(_tc!=NULL){
        long i;
        for(i=0;i<_tc->comments;i++) free(_tc->user_comments[i]);
        free(_tc->user_comments);
        free(_tc->comment_lengths);
        free(_tc->vendor);
        memset(_tc,0,sizeof(*_tc));
    }
}
