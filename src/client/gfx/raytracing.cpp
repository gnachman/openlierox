/*
 *  raytracing.cpp
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 06.04.10.
 *  code under GPL
 *
 */

#include "GfxPrimitives.h"
#include "raytracing.h"
#include "util/macros.h"
#include "game/Game.h"
#include "CMap.h"

static GamePixelInfo getGamePixelInfo_MapOnly(int x, int y) {
	GamePixelInfo info;
	info.type = GamePixelInfo::GPI_Material;
	info.material = game.gameMap()->getMaterialIndex(x, y);

	{
		SDL_Surface* image = game.gameMap()->image->surf.get();
		if(LockSurface(image)) {
			Uint32 col = GetPixel(image, x, y);
			info.color = Color(image->format, col);
			UnlockSurface(image);
			if(IsTransparent(image, col)) info.color.a = SDL_ALPHA_TRANSPARENT;
		}
	}
	
	if(info.color.a != SDL_ALPHA_OPAQUE) {
		SDL_Surface* paralax = game.gameMap()->paralax->surf.get();
		if(LockSurface(paralax)) {
			// TODO: real alpha blending
			info.color = Color(paralax->format, GetPixel(paralax, x, y));
			UnlockSurface(paralax);
		}	
	}
	
	return info;
}

// basically, this is CClient::DrawViewport_Game backwards
GamePixelInfo getGamePixelInfo(int x, int y) {
	CVec p(x, y);
	
	forrange_bool(object, game.objects.beginArea(x, y, x+1, y+1, /* layer */ 0)) {
		IVec size = object->size();
		if( abs((int)object->pos().x - x) + 1 <= size.x && abs((int)object->pos().y - y) + 1 <= size.y ) {
			GamePixelInfo info;
			info.type = GamePixelInfo::GPI_Object;
			info.object.obj = &*object;
			info.object.relX = x - (int)object->pos().x;
			info.object.relY = y - (int)object->pos().y;
			
			info.color = info.object.obj->renderColorAt(info.object.relX, info.object.relY);
			if(info.color.a != SDL_ALPHA_OPAQUE)
				// TODO: real alpha blending
				info.color = getGamePixelInfo_MapOnly(x, y).color;
			
			return info;
		}
	}

	return getGamePixelInfo_MapOnly(x, y);
}

Color getGamePixelColor(int x, int y) {
	return getGamePixelInfo(x, y).color;
}