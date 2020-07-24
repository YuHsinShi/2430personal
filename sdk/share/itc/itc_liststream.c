#include <assert.h>
#include <malloc.h>
#include <string.h>
#include "ite/itc.h"
#include "itc_cfg.h"

static int ListStreamClose(ITCStream* stream)
{
    ITCListStream* ls = (ITCListStream*)stream;
    ITCListNode *curr, *next;
    assert(ls);

    curr = ls->rootnode;
    while (curr)
    {
        next = curr->next;
        free(curr->data);
        free(curr);
        curr = next;
    }
    ls->rootnode = ls->readnode = ls->writenode = NULL;

    return 0;
}

static int ListStreamRead(ITCStream* stream, void* buf, int size)
{
    int readsize, availsize, readsize2, remainsize;
    unsigned char* outbuf = (unsigned char*) buf;
    ITCListStream* ls = (ITCListStream*) stream;
    assert(outbuf);
    assert(ls);

    readsize = 0;

    if (stream->eof)
        goto end;
    
    remainsize = size;
    while (remainsize > 0)
    {
        if (ls->readnode == ls->writenode)
        {
            assert(ls->readpos <= ls->writepos);

            if (ls->readpos == ls->writepos)
            {
                stream->eof = true;
                break;
            }
            availsize = ls->writepos - ls->readpos;
        }
        else
        {
            availsize = ls->datasize - ls->readpos;
        }

        if (remainsize < availsize)
            readsize2 = remainsize;
        else
            readsize2 = availsize;

        memcpy(&outbuf[readsize], &ls->readnode->data[ls->readpos], readsize2);

        ls->readpos += readsize2;
        readsize += readsize2;
        remainsize -= readsize2;

        if (ls->readnode != ls->writenode && ls->readpos == ls->datasize)
        {
            assert(ls->readnode->next);
            ls->readnode = ls->readnode->next;
            ls->readindex++;
            ls->readpos = 0;
        }
    }

end:
    return readsize;
}

static int ListStreamWrite(ITCStream* stream, const void* buf, int size)
{
    int writtensize, availsize, writesize, remainsize;
    ITCListStream* ls = (ITCListStream*)stream;
    unsigned char* bufptr = (unsigned char*)buf;
    assert(ls);
    assert(ls->writenode);

    writtensize = 0;
    remainsize = size;
    while (remainsize > 0)
    {
        if (ls->writepos == ls->datasize)
        {
            assert(!ls->writenode->next);

            ls->writenode->next = malloc(sizeof (ITCListNode));
            if (!ls->writenode->next)
            {
                LOG_ERR "Out of memory: %d\n", sizeof(ITCListNode) LOG_END
                goto end;
            }

            ls->writenode->next->data = malloc(ls->datasize);
            if (!ls->writenode->next->data)
            {
                LOG_ERR "Out of memory: %d\n", size LOG_END
                free(ls->writenode->next);
                ls->writenode->next = NULL;
                goto end;
            }
            ls->writenode->next->prev = ls->writenode;
            ls->writenode->next->next = NULL;
            ls->writenode = ls->writenode->next;
            ls->writeindex++;
            ls->writepos = 0;
        }

        availsize = ls->datasize - ls->writepos;
        if (remainsize < availsize)
            writesize = remainsize;
        else
            writesize = availsize;

        memcpy(&ls->writenode->data[ls->writepos], &bufptr[writtensize], writesize);
        
        ls->writepos += writesize;
        writtensize += writesize;
        remainsize -= writesize;
    }

    if (writtensize > 0)
    {
        stream->eof = false;
        stream->size += writtensize;
    }

end:
    return writtensize;
}

static int ListStreamSeek(ITCStream* stream, long offset, int origin)
{
    int result = 0;
    ITCListStream* ls = (ITCListStream*) stream;
    assert(ls);

    switch (origin)
    {
    case SEEK_SET:
        if (offset >= 0 && offset < stream->size)
        {
            int i;
            ITCListNode* node;

            ls->readindex = offset / ls->datasize;
            assert(ls->readindex <= ls->writeindex);
            ls->readpos = offset % ls->datasize;

            node = ls->rootnode;
            for (i = 0; i < ls->readindex; ++i)
            {
                node = node->next;
                assert(node);
            }
            ls->readnode = node;
            
            if (ls->readnode == ls->writenode)
            {
                assert(ls->readpos <= ls->writepos);

                if (ls->readpos == ls->writepos)
                {
                    stream->eof = true;
                }
            }
        }
        else
        {
            result = -1;
        }
        break;

    case SEEK_CUR:
        if (offset >= 0)
        {
            if (ls->readindex * ls->datasize + ls->readpos + offset < stream->size)
            {
                int offsetsize, availsize, remainsize;

                remainsize = offset;
                while (remainsize > 0)
                {
                    if (ls->readnode == ls->writenode)
                    {
                        assert(ls->readpos <= ls->writepos);

                        if (ls->readpos == ls->writepos)
                        {
                            stream->eof = true;
                            break;
                        }
                        availsize = ls->writepos - ls->readpos;
                    }
                    else
                    {
                        availsize = ls->datasize - ls->readpos;
                    }

                    if (remainsize < availsize)
                        offsetsize = remainsize;
                    else
                        offsetsize = availsize;

                    ls->readpos += offsetsize;
                    remainsize -= offsetsize;

                    if (ls->readnode != ls->writenode && ls->readpos == ls->datasize)
                    {
                        assert(ls->readnode->next);
                        ls->readnode = ls->readnode->next;
                        ls->readindex++;
                        ls->readpos = 0;
                    }
                }
            }
            else
            {
                result = -1;
            }
        }
        else
        {
            if (ls->readindex * ls->datasize + ls->readpos + offset >= 0)
            {
                int offsetsize, availsize, remainsize;

                remainsize = -offset;
                while (remainsize > 0)
                {
                    if (ls->readnode->prev == NULL)
                    {
                        assert(ls->readindex == 0);

                        if (ls->readpos == 0)
                        {
                            break;
                        }
                    }
                    else if (ls->readpos == 0)
                    {
                        assert(ls->readindex > 0);
                        ls->readnode = ls->readnode->prev;
                        ls->readindex--;
                        ls->readpos = ls->datasize;
                    }

                    availsize = ls->readpos;

                    if (remainsize < availsize)
                        offsetsize = remainsize;
                    else
                        offsetsize = availsize;

                    ls->readpos -= offsetsize;
                    remainsize -= offsetsize;
                }
            }
            else
            {
                result = -1;
            }
        }
        break;

    case SEEK_END:
        if (offset <= 0 && stream->size + offset >= 0)
        {
            int offsetsize, availsize, remainsize;

            ls->readnode = ls->writenode;
            ls->readindex = ls->writeindex;
            ls->readpos = ls->writepos;

            remainsize = -offset;
            while (remainsize > 0)
            {
                if (ls->readnode->prev == NULL)
                {
                    assert(ls->readindex == 0);

                    if (ls->readpos == 0)
                    {
                        break;
                    }
                }
                else if (ls->readpos == 0)
                {
                    assert(ls->readindex > 0);
                    ls->readnode = ls->readnode->prev;
                    ls->readindex--;
                    ls->readpos = ls->datasize;
                }

                availsize = ls->readpos;

                if (remainsize < availsize)
                    offsetsize = remainsize;
                else
                    offsetsize = availsize;

                ls->readpos -= offsetsize;
                remainsize -= offsetsize;
            }
        }
        else
        {
            result = -1;
        }
        break;

    default:
        assert(0);
        result = -1;
        break;
    }

    return result;
}

static long ListStreamTell(ITCStream* stream)
{
    ITCListStream* ls = (ITCListStream*) stream;
    assert(ls);
    return ls->datasize * ls->readindex + ls->readpos;
}

static int ListStreamAvailable(ITCStream* stream)
{
    int avail;
    ITCListStream* ls = (ITCListStream*) stream;
    assert(ls);

    avail = ls->datasize * ls->writeindex + ls->writepos - (ls->datasize * ls->readindex + ls->readpos);
    return avail;
}

int itcListStreamOpen(ITCListStream* lstream, int datasize)
{
    int result = 0;
    assert(lstream);
    assert(datasize > 0);

    itcStreamOpen((ITCStream*) lstream);

    itcStreamSetClose(lstream, ListStreamClose);
    itcStreamSetRead(lstream, ListStreamRead);
    itcStreamSetWrite(lstream, ListStreamWrite);
    itcStreamSetSeek(lstream, ListStreamSeek);
    itcStreamSetTell(lstream, ListStreamTell);
    itcStreamSetAvailable(lstream, ListStreamAvailable);

    lstream->rootnode = malloc(sizeof (ITCListNode));
    if (!lstream->rootnode)
    {
        LOG_ERR "Out of memory: %d\n", sizeof(ITCListNode) LOG_END
        result = __LINE__;
        goto end;
    }

    lstream->rootnode->data = malloc(datasize);
    if (!lstream->rootnode->data)
    {
        LOG_ERR "Out of memory: %d\n", datasize LOG_END
        result = __LINE__;
        goto end;
    }

    lstream->stream.eof = true;
    lstream->rootnode->prev = NULL;
    lstream->rootnode->next = NULL;
    lstream->datasize = datasize;
    lstream->readnode = lstream->rootnode;
    lstream->readindex = 0;
    lstream->readpos = 0;
    lstream->writenode = lstream->rootnode;
    lstream->writeindex = 0;
    lstream->writepos = 0;

end:
    if (result)
    {
        if (lstream->rootnode)
        {
            free(lstream->rootnode->data);
            lstream->rootnode->data = NULL;
            free(lstream->rootnode);
            lstream->rootnode = NULL;
        }
    }
    return result;
}
