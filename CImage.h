#pragma once
#include "support/gcc8_c_support.h"
#include <exec/types.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/graphics.h>
#include <graphics/gfxmacros.h>
#include "Iff.h"


class CImage : public Image
{
    public:
    CImage(UBYTE* file)
    {
        unsigned char code, temp;
        BPTR filePtr;
        CHUNK chunk;
        BODY Body;
        char id[2];
        
        unsigned char * buffer;

        filePtr = Open((CONST_STRPTR)file, (LONG)MODE_OLDFILE);
        if(filePtr == 0)
        {
            Write(Output(), (APTR)file, strlen((const char*)file));
            Close(filePtr);
            Exit(1);
        }

        Seek(filePtr, 0, OFFSET_BEGINNING);
        Read(filePtr, (VOID *)&chunk, sizeof(CHUNK));

        //get to the start of the 'BODY' ie image data
        while(id[0] != 'B' && id[1] != 'O')
        {
            Read(filePtr, (VOID *)&id, 2);
        }
        Body.ChunkId[0] = 'B';
        Body.ChunkId[1] = 'O';
        Read(filePtr, (VOID *)&Body.ChunkId[2], 2);
        Read(filePtr, (VOID *)&Body.Size, 4);
        
        int counter = 0;
        this->Depth = chunk.BitMapHeader.BitPlanes;
        this->Width = chunk.BitMapHeader.Width;
        this->Height = chunk.BitMapHeader.Height;
        BitMap *mybm = new BitMap();
        
       // Depth = 4;
        ImageData = (UWORD*)AllocMem((Width>>3)*Height * Depth, MEMF_CHIP|MEMF_CLEAR);
       
        InitBitMap(mybm, this->Depth, this->Width, this->Height);
        
            
        for(int plane = 0; plane < mybm->Depth; plane++)
        {
            if((mybm->Planes[plane] = (PLANEPTR)AllocRaster(Width, Height)) == NULL)
            {
                Write(Output(),(APTR)"Cannot allocate memory for bitmap.\n", strlen("Cannot allocate memory for bitmap.\n"));
                Exit(1000);
            }
            BltClear((UBYTE*)mybm->Planes[plane], RASSIZE(Width, Height), 0);
        }

        for(int y = 0; y < mybm->Rows; y++)
        {
            for(int x = 0; x < mybm->Depth; x++)
            {
                buffer = &mybm->Planes[x][y * mybm->BytesPerRow];
                counter = 0;
                while(counter < mybm->BytesPerRow)
                {
                    Read(filePtr, (unsigned char *)&code, 1);
        
                    if(code < 128)
                    {
                        Read(filePtr, (unsigned char *)&buffer[counter], (code + 1));
                        counter = (counter + code + 1);
                    }
                    else if(code > 128)
                    {
                        Read(filePtr, (unsigned char *)&temp, 1);
                        for(int i = counter; i < (counter + 257 - code); i++)
                        {
                            buffer[i] = temp;
                        }
                        counter = counter + 257-code;
                    }
                    else
                    {
                    }	
                }
            }
        }
        Close(filePtr);

        for(int plane = 0; plane < Depth; plane++)
        {
            //image data is in UWORDs (16bits) and not bytes...so.  divide the array postion by 2
            CopyMem(mybm->Planes[plane], &ImageData[plane * (RASSIZE(Width,Height)/2)],  RASSIZE(Width, Height));
            FreeRaster(mybm->Planes[plane], Width, Height);
        }
            
    }
    ~CImage()
    {
        FreeMem(ImageData, RASSIZE(Width, Height)*Depth);
    }
};

