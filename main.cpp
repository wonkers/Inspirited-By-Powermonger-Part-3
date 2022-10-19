#include <proto/exec.h>
#include <proto/dos.h>
#include "newdelete.h"
#include "CView.h"
#include "CImage.h"


struct ExecBase *SysBase;
struct DosLibrary *DOSBase;
struct GfxBase *GfxBase;

//points
Point points[9][9] = 
{
	{Point{96,107}, Point{115,105}, Point{133,102}, Point{150,100}, Point{167,98}, Point{182,96}, Point{198,94}, Point{213,92}, Point{225,90}},
	{Point{98,113}, Point{118,110}, Point{137,108}, Point{154,106}, Point{172,103}, Point{188,101}, Point{204,99}, Point{220,97}, Point{232,95}},
	{Point{100,121}, Point{121,117}, Point{141,114}, Point{160,112}, Point{179,109}, Point{194,106}, Point{211,104}, Point{227,101}, Point{239,99}},
	{Point{103,129}, Point{125,125}, Point{146,122}, Point{165,119}, Point{184,116}, Point{202,113}, Point{219,110}, Point{236,107}, Point{249,105}},
	{Point{106,139}, Point{129,134}, Point{151,130}, Point{172,127}, Point{192,123}, Point{210,120}, Point{229,117}, Point{246,114}, Point{260,111}},
	{Point{109,148}, Point{134,143}, Point{157,139}, Point{178,135}, Point{200,131}, Point{220,128}, Point{238,125}, Point{257,121}, Point{272,119}},
    {Point{112,160}, Point{139,155}, Point{164,150}, Point{186,145}, Point{210,141}, Point{230,137}, Point{250,133}, Point{269,130}, Point{285,127}},
    {Point{117,174}, Point{146,168}, Point{172,162}, Point{196,157}, Point{221,152}, Point{242,148}, Point{264,143}, Point{283,139}, Point{299,136}},
    {Point{122,191}, Point{153,184}, Point{181,177}, Point{208,171}, Point{234,165}, Point{256,160}, Point{279,154}, Point{300,150}, Point{317,146}}
};

//height map
WORD height[9][9] =
{
	{2,0,0,10,22,10,10,10,10},
	{5,0,0,10,22,30,10,10,11},
	{10,0,0,20,42,42,42,20,10},
	{14,0,0,10,42,42,42,20,11},
	{18,0,0,10,42,42,42,20,10},
	{24,0,0,15,32,20,20,10,11},
	{16,0,0,10,30,20,20,10,10},
	{0,0,0,10,20,17,13,12,11},
	{0,0,0,8,10,7,3,2,1}
};

//Polygon *polys[8*16];
TexturedPolygon *txPolys[8*16];
UWORD *TilePattern[8][16];
UWORD bm_colors[32];
UWORD heightmap[9][18];
struct Vector3d vertices[9][18];
struct Vector3d normals[9][18];


UWORD Colours[16] = 
{
	0x000,0x682,0x462,0x8a2,
	0xcc2,0xa82,0x862,0x642,
	0xcc8,0xaa8,0x886,0x664,
	0xa40,0x468,0x046,0x0
};

inline short MouseLeft(){return !((*(volatile UBYTE*)0xbfe001)&0x40);}	
inline short MouseRight(){return !((*(volatile UWORD*)0xdff016)&(1<<10));}


int main() {
	SysBase = *((struct ExecBase**)4UL);

	GfxBase = (struct GfxBase *)OpenLibrary((CONST_STRPTR)"graphics.library",0);
	if (!GfxBase)
		Exit(0);

	DOSBase = (struct DosLibrary*)OpenLibrary((CONST_STRPTR)"dos.library", 0);
	if (!DOSBase)
	{
		CloseLibrary((struct Library *)GfxBase);
		Exit(0);
	}

	//load images
	CImage pattern1 = CImage((UBYTE*)"dh0:pattern1.iff");
	CImage pattern2 = CImage((UBYTE*)"dh0:pattern2.iff");
	CImage pattern_shade1 = CImage((UBYTE*)"dh0:pattern1-Shade.iff");
	CImage pattern_shade2 = CImage((UBYTE*)"dh0:pattern2-Shade.iff");
	CImage pattern_light1 = CImage((UBYTE*)"dh0:pattern1-Light.iff");
	CImage pattern_light2 = CImage((UBYTE*)"dh0:pattern2-Light.iff");
	CImage pattern_dark1 = CImage((UBYTE*)"dh0:pattern1-Dark.iff");
	CImage pattern_dark2 = CImage((UBYTE*)"dh0:pattern2-Dark.iff");

	CBitMap background = CBitMap((UBYTE*)"dh0:background.iff");

	//create map as if it 3d
	//x horizontal, y height, z depth
	//I need this to calculate the normals
	for(int y = 0; y < 9; y++)
	{
		for(int x = 0; x < 18; x++)
		{
			vertices[y][x].x = x;
			vertices[y][x].y = height[y][x/2];
			vertices[y][x].z = y;
		}
	}

	//calc normals and set patterns to array	
	for(int y = 0; y < 9; y++)
	{
		for(int x = 0; x < 16; x++)
		{
			Vector3d U;
			Vector3d V;
			Vector3d N;
			
			if(!(x & 0x1))
			{
			//type1
				//p2-p1
				U.x = vertices[y+1][x].x - vertices[y][x].x;
				U.y = vertices[y+1][x].y - vertices[y][x].y;
				U.z = vertices[y+1][x].z - vertices[y][x].z;
				//p3-p1
				V.x = vertices[y+1][x+2].x - vertices[y][x].x;
				V.y = vertices[y+1][x+2].y - vertices[y][x].y;
				V.z = vertices[y+1][x+2].z - vertices[y][x].z;

				N.x = U.y*V.z - U.z*V.y;
				N.y = U.z*V.x - U.x*V.z;
				N.z = U.x*V.y - U.y*V.x;


				normals[y][x] = N;

				if(N.x < 0 && N.z < 0)
					TilePattern[y][x] = pattern_light1.ImageData;
				else if(N.x > 0 && N.z > 0)
					if(N.y > 2)
						TilePattern[y][x] = pattern_dark1.ImageData;
					else
						TilePattern[y][x] = pattern_shade1.ImageData;
				else if(N.z > 0)
					TilePattern[y][x] = pattern_shade1.ImageData;
				else if(N.z < 0)
					TilePattern[y][x] = pattern_light1.ImageData;
				else
					TilePattern[y][x] = pattern1.ImageData;

			}
			else
			{
			//type2
				//p2-p1
				U.x = vertices[y+1][x+2].x - vertices[y][x].x;
				U.y = vertices[y+1][x+2].y - vertices[y][x].y;
				U.z = vertices[y+1][x+2].z - vertices[y][x].z;
				//p3-p1
				V.x = vertices[y][x+2].x - vertices[y][x].x;
				V.y = vertices[y][x+2].y - vertices[y][x].y;
				V.z = vertices[y][x+2].z - vertices[y][x].z;

				N.x = U.y*V.z - U.z*V.y;
				N.y = U.z*V.x - U.x*V.z;
				N.z = U.x*V.y - U.y*V.x;


				normals[y][x] = N;

				if(normals[y][x].x < 0 && normals[y][x+1].z < 0)
					TilePattern[y][x] = pattern_light2.ImageData;
				else if(normals[y][x].x > 0  && normals[y][x+1].z > 0)
					if(normals[y][x].y >2)
						TilePattern[y][x] = pattern_dark2.ImageData;
					else
						TilePattern[y][x] = pattern_shade2.ImageData;
				else if(normals[y][x].z > 0)
					TilePattern[y][x] = pattern_shade2.ImageData;
				else if(normals[y][x].z < 0)
					TilePattern[y][x] = pattern_light2.ImageData;
				else
					TilePattern[y][x] = pattern2.ImageData;
			}
		}
	}
	

	int p = 0;
	for(int y = 0; y < 8; y++)
	{
		for(int x = 0; x < 8; x++)
		{
			Point p1 = points[y][x]; p1.y -= height[y][x];
			Point p2 = points[y + 1][x]; p2.y -= height[y + 1][x];
			Point p3 = points[y +1 ][x + 1]; p3.y -= height[y + 1][x + 1];
			txPolys[p++] = new TexturedPolygon(p1, p2, p3, TilePattern[y][x*2], 5);		
			
			p1 = points[y][x]; p1.y -= height[y][x];
			p2 = points[y][x + 1]; p2.y -= height[y][x + 1];
			p3 = points[y + 1][x + 1]; p3.y -= height[y + 1][x + 1];
			txPolys[p++] = new TexturedPolygon(p1, p2, p3, TilePattern[y][x*2+1], 5);
		}
	}
	
	//create workhorse View
	CView view = CView(4, 320, 256, 0);
	// set colour palette of Viewport
	LoadRGB4(view.ViewPort(), Colours, 16);

	//clear screen
	view.ViewPort()->Clear(0);
	//set wireframes
	//view.ViewPort()->SetWireFrame(0);
	//enable texture filling
	view.ViewPort()->SetTextureDraw();

	view.ViewPort()->BltImg(&background, 0, 0);

	for(TexturedPolygon* poly : txPolys)
	{
		view.ViewPort()->DrawTexturedPolygon(poly);
	}

	//keep running until left mouse button is pressed
	while(!MouseLeft() && !MouseRight())
	{
				
	}

	//free poly memory
	for(auto& poly : txPolys)
	{
		delete poly;
	}

	//close open libraries
	CloseLibrary((struct Library*)DOSBase);
	CloseLibrary((struct Library*)GfxBase);
}
