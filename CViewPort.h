#pragma once
#include "support/gcc8_c_support.h"
#include <exec/types.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <intuition/intuition.h>
#include <graphics/gfxbase.h>
#include <graphics/view.h>
#include <graphics/gfxmacros.h>
#include "gels.h"
#include "newdelete.h"
#include "CBitMap.h"
#include "CRastPort.h"
#include "CImage.h"

class CViewPort : public ViewPort
{
    private:
    struct RasInfo *m_RasInfos[2];
    
    CBitMap *BitMaps[4];

    public:
    CRastPort *m_RastPorts[2];
    CViewPort(UWORD depth, ULONG width, ULONG height, UWORD modes)
    {
        //initialize ViewPort
        InitVPort(this);
        Modes = modes;
        DHeight = height;
        DWidth = width;

        //I always just set maximum (32) (could use (2^depth)&0x20)
        ColorMap = GetColorMap(32);  

        //create viewport bitmap
        BitMaps[0] = new CBitMap(depth, width, height);
        
        //create rasinfo and assign bitmap
        m_RasInfos[0] = new struct RasInfo();
        RasInfo = m_RasInfos[0];
        RasInfo->BitMap = BitMaps[0];

        //create rastport
        m_RastPorts[0] = new CRastPort(width, height, BitMaps[0]);        
       
    }
    ~CViewPort()
    {
        delete m_RastPorts[0];
        delete m_RasInfos[0];
        delete BitMaps[0];
    }

    void Clear(int colour)
    {
        SetRast(m_RastPorts[0], colour);
    }

    void SetForeColour(int colour)
    {
        SetAPen(m_RastPorts[0], colour);
    }

    void SetColour(WORD i, UWORD colour)
    {
        SetRGB4(this, i, colour>>8, (colour&0xf0)>>4, colour&0x0f);
    }

    void WriteText(LONG x, LONG y, STRPTR text)
    {
        Move(m_RastPorts[0], x, y);
        Text(m_RastPorts[0], text, strlen(text));
    }

    void SetWireFrame(int colour)
    {
        SetOPen(m_RastPorts[0], colour);
    }

    void SetTextureDraw()
    {
        SetAPen(m_RastPorts[0], 255);
        SetBPen(m_RastPorts[0], 0);
        SetDrMd(m_RastPorts[0], JAM2);
    }
    void SetFillDraw(ULONG colour)
    {
        m_RastPorts[0]->AreaPtrn = 0;
        SetAPen(m_RastPorts[0], colour);
    }

    void DrawFilledPolygon(Polygon *poly)
    {
        AreaMove(m_RastPorts[0], poly->p1.x, poly->p1.y);
        AreaDraw(m_RastPorts[0], poly->p2.x, poly->p2.y);
        AreaDraw(m_RastPorts[0], poly->p3.x, poly->p3.y);
        AreaEnd(m_RastPorts[0]);
    }

    void DrawTexturedPolygon(TexturedPolygon *tx_poly)
    {
        SetAfPt(m_RastPorts[0], tx_poly->texture, -tx_poly->height);
        DrawFilledPolygon((Polygon*)tx_poly);
    }

    void DrawImg(CImage *img, LONG x, LONG y)
    {

    }
    void BltImg(CBitMap *bm, LONG x, LONG y)
    {
        BltBitMap(bm, 0, 0, BitMaps[0], x, y, bm->BytesPerRow<<3, bm->Rows, 0xc0, 0xff, NULL);
    }
};