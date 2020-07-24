#include<stdio.h>
#include<stdlib.h>
#include <string.h>
#include "i2s/i2s.h"
#include "wavheader.h"

#define le_uint32(a) (a)
#define le_uint16(a) (a)
#define le_int16(a) (a)

playdata *INITplaydata()
{
     playdata *dptr;
     dptr = (playdata*)malloc(sizeof(playdata));
     dptr->hsize=0;
     dptr->playlenth=0;
     dptr->fd = NULL;
     return dptr;     
}

void DEINITplaydata(playdata *dptr)
{
     dptr->hsize=0;
     dptr->playlenth=0;
     if(dptr->fd) fclose(dptr->fd);
     dptr->fd = NULL;
     free(dptr);
}

void openfile(playdata *d,char *filename)
{
   
    if (filename != NULL)
    {
        if(d->fd) {
            fclose(d->fd);
            d->fd=NULL;
        }
        d->fd = fopen(filename, "rb");
        printf("file %s exit\n",filename);
    }else{
        printf("file %s not exit\n",filename);
    }
    return ;
}

void openfilewb(playdata *d,char *filename)
{
   
    if (filename != NULL)
    {
        d->fd = fopen(filename, "wb");
        printf("file %s exit\n",filename);
    }else{
        printf("file %s not exit\n",filename);
    }
    return ;
}

void get_wav_header(playdata *d,wave_header_t *Wheader)
{
    double wavtime;
    char header1[sizeof(riff_t)];
    char header2[sizeof(format_t)];
    char header3[sizeof(data_t)];
    unsigned long len = 0;
    riff_t *riff_chunk=(riff_t*)header1;
    format_t *format_chunk=(format_t*)header2;
    data_t *data_chunk=(data_t*)header3;
    
    int count;

    len = fread(header1,sizeof(header1),1,d->fd);
    printf("len = %d sizeof(header1) = %d\n",len,sizeof(header1));

    if (0!=strncmp(riff_chunk->riff, "RIFF", 4) || 0!=strncmp(riff_chunk->wave, "WAVE", 4)){
        printf("RIFF WAVE head error\n");
        goto not_a_wav;
    }else{
          fseek(d->fd, sizeof(riff_t), SEEK_SET);
    }
    
    fread(header2, sizeof(header2),1,d->fd) ;
    if(0!=strncmp(format_chunk->fmt,"fmt ",4)){
        printf("fmt head error\n");
        goto not_a_wav;
    }
    
    d->hsize=sizeof(wave_header_t)-0x10+format_chunk->len;
    
    if (format_chunk->len-0x10>0)
    {
        fseek(d->fd,(format_chunk->len-0x10),SEEK_CUR);
    }
    fread(header3, sizeof(header3),1,d->fd) ;

    count=0;
    while (strncmp(data_chunk->data, "data", 4)!=0 && count<30)
    {
        printf("skipping chunk=%s len=%i\n", data_chunk->data, data_chunk->len);
        fseek(d->fd,data_chunk->len,SEEK_CUR);
        count++;
        d->hsize=d->hsize+len+data_chunk->len;
        fread(header3, sizeof(header3),1,d->fd) ;
    }

    memcpy((*Wheader).riff_chunk.riff,riff_chunk->riff,sizeof(riff_chunk->riff));
    (*Wheader).riff_chunk.len = riff_chunk->len;
    memcpy((*Wheader).riff_chunk.wave,riff_chunk->wave,sizeof(riff_chunk->wave));

    memcpy((*Wheader).format_chunk.fmt,format_chunk->fmt,sizeof(format_chunk->fmt));
    (*Wheader).format_chunk.len = format_chunk->len;
    (*Wheader).format_chunk.type = format_chunk->type;
    (*Wheader).format_chunk.channel = format_chunk->channel;
    (*Wheader).format_chunk.rate = format_chunk->rate;
    (*Wheader).format_chunk.bps = format_chunk->bps;
    (*Wheader).format_chunk.blockalign = format_chunk->blockalign;
    (*Wheader).format_chunk.bitpspl = format_chunk->bitpspl;
   
    memcpy((*Wheader).data_chunk.data,data_chunk->data,sizeof(data_chunk->data));    
    (*Wheader).data_chunk.len = data_chunk->len;

    d->playlenth = data_chunk->len;
    
    return ;

    not_a_wav:
        printf("not a wav\n");
        return ;        
     
}

void write_wav_header(playdata *d, STRC_I2S_SPEC *i2s_spec,int size){
	wave_header_t header;
	memcpy(&header.riff_chunk.riff,"RIFF",4);
	header.riff_chunk.len=le_uint32(size+36);
	memcpy(&header.riff_chunk.wave,"WAVE",4);

	memcpy(&header.format_chunk.fmt,"fmt ",4);
	header.format_chunk.len=le_uint32(0x10);
	header.format_chunk.type=le_uint16(0x1);
	header.format_chunk.channel=le_uint16(i2s_spec->channels);
	header.format_chunk.rate=le_uint32(i2s_spec->sample_rate);
    header.format_chunk.bps=le_uint32(i2s_spec->sample_rate*2);

	header.format_chunk.blockalign=le_uint16(2);
	header.format_chunk.bitpspl=le_uint16(16);

	memcpy(&header.data_chunk.data,"data",4);
	header.data_chunk.len=le_uint32(size);
    fwrite(&header,sizeof(header),1,d->fd);
}

void printf_header(wave_header_t header)
{
    printf("riff = %s \n",header.riff_chunk.riff);
    printf("len  = %d \n",header.riff_chunk.len);
    printf("WAVE = %s \n",header.riff_chunk.wave);
    
    printf("fmt        = %s \n",header.format_chunk.fmt);
    printf("len        = %d \n",header.format_chunk.len);  
    printf("type       = %d \n",header.format_chunk.type);  
    printf("channel    = %d \n",header.format_chunk.channel);  
    printf("rate       = %d \n",header.format_chunk.rate);  
    printf("bps        = %d \n",header.format_chunk.bps);  
    printf("blockalign = %d \n",header.format_chunk.blockalign);  
    printf("bitpspl    = %d \n",header.format_chunk.bitpspl);      

    printf("data = %s \n",header.data_chunk.data);
    printf("len  = %d \n",header.data_chunk.len);

}