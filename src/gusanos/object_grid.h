#ifndef VERMES_OBJECT_GRID_H
#define VERMES_OBJECT_GRID_H

#include <vector>
#include <boost/utility.hpp>
#include "game/CGameObject.h"

#include <iostream>
#include <cassert>
using std::cerr;
using std::endl;

template<class T>
struct GridObjectList
{
	typedef size_t size_type;
	
	GridObjectList()
	: firstS(0), firstD(0), count(0)
	{
	}
	
	struct iterator
	{
		friend struct GridObjectList;
		
		iterator()
		: ptr(0)
		{
		}
		
		explicit iterator(T* ptr_)
		: ptr(ptr_), prev(0)
		{
		}
		
		explicit iterator(T* prev_, T* ptr_)
		: ptr(ptr_), prev(prev_)
		{
		}
		
		bool operator==(iterator const& b) const
		{
			return ptr == b.ptr;
		}
		
		bool operator!=(iterator const& b) const
		{
			return ptr != b.ptr;
		}
		
		iterator& operator++()
		{
			prev = ptr;
			ptr = ptr->nextS_;
			
			return *this;
		}
		
		iterator next()
		{
			return iterator(ptr, ptr->nextS_);
		}
				
		operator T*() const
		{
			return ptr;
		}
		
		T& operator*() const
		{
			return *ptr;
		}
		
		T* operator->() const
		{
			return ptr;
		}
		
	private:
		
		T* ptr;
		T* prev;
	};
	
	struct light_iterator
	{
		friend struct GridObjectList;
		
		light_iterator()
		: ptr(0)
		{
		}
		
		explicit light_iterator(T* ptr_)
		: ptr(ptr_)
		{
		}
		
		bool operator==(light_iterator const& b) const
		{
			return ptr == b.ptr;
		}
		
		bool operator!=(light_iterator const& b) const
		{
			return ptr != b.ptr;
		}
		
		light_iterator& operator++()
		{
			ptr = ptr->nextD_;
			
			return *this;
		}
				
		operator T*() const
		{
			return ptr;
		}
		
		T& operator*() const
		{
			return *ptr;
		}
		
		T* operator->() const
		{
			return ptr;
		}
		
	private:
		
		
		T* ptr;
	};
	
	struct const_iterator
	{
		friend struct GridObjectList;
		
		const_iterator()
		: ptr(0)
		{
		}
		
		const_iterator(iterator const& iter)
		: ptr(iter.ptr)
		{
		}
		
		const_iterator(T const* ptr_)
		: ptr(ptr_)
		{
		}
		
		bool operator==(const_iterator const& b)
		{
			return ptr == b.ptr;
		}
		
		bool operator!=(const_iterator const& b)
		{
			return ptr != b.ptr;
		}
		
		const_iterator& operator++()
		{
			ptr = ptr->nextS_;
		}
				
		operator T const*() const
		{
			return ptr;
		}
		
		T const& operator*() const
		{
			return *ptr;
		}
		
		T const* operator->() const
		{
			return ptr;
		}
		
	private:
		
		
		T const* ptr;
	};
	
	void insertS(T* obj)
	{
		obj->nextS_ = firstS;
		firstS = obj;
		++count;
	}
	
	void insertAfter(light_iterator i, T* obj)
	{
		obj->nextS_ = firstS;
		obj->nextD_ = i->nextD_;
		obj->prevD_ = i;
		if(i->nextD_)
			i->nextD_->prevD_ = obj;
		i->nextD_ = obj;

		firstS = obj;
		++count;
	}
	
	void moveAfter(light_iterator i, light_iterator obj)
	{
		unlinkD(obj);
		obj->nextD_ = i->nextD_;
		obj->prevD_ = i;
		if(i->nextD_)
			i->nextD_->prevD_ = obj;
		i->nextD_ = obj;
	}
	
	// Inserts in doubly-linked sequence only.
	// Can thus not be removed.
	void insertD(T* obj)
	{
		obj->nextD_ = firstD;
		
		if(firstD)
			firstD->prevD_ = obj;
		firstD = obj;
		//++count; // Not counted
	}
	
	void unlink(iterator& iter)
	{
		if(iter.prev)
			iter.prev->nextS_ = iter->nextS_;
		else
			firstS = iter->nextS_;

		// If iter->prevD_ is null and this node isn't a guard node,
		// this node isn't inserted into the doubly-linked sequence.
		// Guard nodes can't be reached via iterator anyway.
		if(iter->prevD_)
		{
			iter->prevD_->nextD_ = iter->nextD_;

			if(iter->nextD_)
				iter->nextD_->prevD_ = iter->prevD_;
		}

		T* ptr = iter->nextS_;
		iter->prevD_ = NULL;
		iter->nextD_ = NULL;
		iter->nextS_ = NULL;
		iter->cellIndex_ = -1;
		iter.ptr = ptr;

		--count;
	}
	
	
	
	void unlinkD(light_iterator const& iter)
	{
		if(iter->prevD_)
			iter->prevD_->nextD_ = iter->nextD_;
		else
			firstD = iter->nextD_;
			
		if(iter->nextD_)
			iter->nextD_->prevD_ = iter->prevD_;
	}
			
	void erase(iterator& iter)
	{
		T* p = iter.ptr;
		unlink(iter);
		p->deleteThis();
	}
	
	light_iterator beginD()
	{
		return light_iterator(firstD);
	}
	
	light_iterator endD()
	{
		return light_iterator(0);
	}
	
	iterator beginS()
	{
		return iterator(firstS);
	}
	
	iterator endS()
	{
		return iterator(0);
	}
	
	const_iterator begin() const
	{
		return const_iterator(firstS);
	}
	
	const_iterator end() const
	{
		return const_iterator(0);
	}
	
	size_type size() const
	{
		return count;
	}

	T* firstS; // singly-linked series
	T* firstD; // doubly-linked series
	size_t      count;
};

class Grid : boost::noncopyable
{
public:
	static const int shift = 5;
	static const int squareSide = (1 << shift);
	static const int maxObjectRange = 16;
	//static const int colLayers = 10;
	//static const int renderLayers = 10;
	
	
	enum ColLayer
	{
		WormColLayer = 0,
		NoColLayer = 1,
		CustomColLayerStart = 2,
		ColLayerCount = 10,
	};
	
	enum RenderLayer
	{
		WormRenderLayer = 4,
		RenderLayerCount = 10,
	};
	
	static const int layerCount = ColLayerCount * RenderLayerCount;
	
	typedef GridObjectList<CGameObject> ObjectList;
	
	struct Layer
	{
		struct Square
		{
			CGameObject* guardNode;
			
			Square() : guardNode(NULL) {}
			
			ObjectList::light_iterator guard()
			{
				return ObjectList::light_iterator(guardNode);
			}
		};
		
		Layer(int s)
		: grid(s + 1), firstDelayed(0), firstDelayedNoCol(0)
		{}
				
		void clear()
		{
			ObjectList::iterator next, i;
			for(i = list.beginS(); i;)
			{
				list.erase(i);
			}
		}
		
		void insertDelayed(CGameObject* obj)
		{
			obj->nextS_ = firstDelayed;
			firstDelayed = obj;
		}
		
		void insertDelayedNoCol(CGameObject* obj)
		{
			obj->nextS_ = firstDelayedNoCol;
			firstDelayedNoCol = obj;
		}
		
		void flushDelayed()
		{
			CGameObject* obj = firstDelayed;
			while(obj)
			{
				CGameObject* next = obj->nextS_;
				list.insertAfter(grid[obj->cellIndex_].guard(), obj);
				obj = next;
			}
			
			firstDelayed = 0;
			
			obj = firstDelayedNoCol;
			while(obj)
			{
				CGameObject* next = obj->nextS_;
				list.insertS(obj);
				obj = next;
			}
			
			firstDelayedNoCol = 0;
		}
		
		std::vector<Square> grid;
		CGameObject* firstDelayed;
		CGameObject* firstDelayedNoCol;
		ObjectList list;
	};
	
	Grid()
	: width(0), height(0)
	{
		resize(0,0,100,100);
	}
	
	void resize(int x1_, int y1_, int x2_, int y2_);
	
	friend struct area_iterator;
	
	struct area_iterator
	{
		friend class Grid;
		
		area_iterator() // curObj gets initialized to an iterator returning false
		{
		}
				
		area_iterator& operator++()
		{
			++curObj;
			findFirstValid();
			
			return *this;
		}
		
		CGameObject& operator*()
		{
			return *curObj;
		}
		
		CGameObject* operator->()
		{
			return (CGameObject *)curObj;
		}
		
		operator bool()
		{
			return curObj != NULL;
		}
		
		void findFirstValid()
		{
			while(curObj == nextGuard)
			{
				if(--leftOnRow <= 0)
				{
					if(--rowsLeft <= 0)
					{
						
						if(--layersLeft <= 0)
						{
							// No more layers
							curObj = ObjectList::light_iterator();
							return;
						}
						else
						{
							curLayer += layerPitch;
							
							if(!skipEmptyLayers())
								return;
						}
					}
					else
					{
						curSquare += pitch;
						curObj = curSquare->guard();
					}
					leftOnRow = squaresPerRow;
				}
				else
					++curSquare;
				
				
				nextGuard = (curSquare + 1)->guard();
				++curObj; // Skip guard
			}
		}
		
		// Sets curObj to the guard node of the first non-empty
		// layer, or 0 if none exists.
		bool skipEmptyLayers()
		{
			while(curLayer->list.size() == 0)
			{
				if(--layersLeft == 0)
				{
					// No more layers
					
					curObj = ObjectList::light_iterator();
					return false;
				}
				
				curLayer += layerPitch;
			}
			
			curSquare = curLayer->grid.begin() + squareIdxBegin;
			curObj = curSquare->guard();
			rowsLeft = rowsPerLayer;
			
			return true;
		}
		
	private:
		area_iterator(Grid& grid, int x1, int y1, int w, int h, int layerBegin, int layerStep, int layerCount)
		: rowsLeft(h), rowsPerLayer(h), squaresPerRow(w), leftOnRow(w)
		, pitch(grid.squaresH - w + 1), layerPitch(layerStep)
		, layersLeft(layerCount), squareIdxBegin(grid.getSquareIdx(x1, y1))
		{
			curLayer = grid.layers.begin() + layerBegin;
			
			if(!skipEmptyLayers())
				return;
			nextGuard = (curSquare + 1)->guard();
			++curObj; // Skip guard
			findFirstValid();
		}
		
		int rowsLeft;
		int rowsPerLayer;
		int squaresPerRow;
		int leftOnRow;
		int pitch;
		int layerPitch;
		int layersLeft;
		int squareIdxBegin;
		ObjectList::light_iterator curObj;
		ObjectList::light_iterator nextGuard;
		std::vector<Layer::Square>::iterator curSquare;
		std::vector<Layer>::iterator curLayer;
	};
	
	struct iterator
	{
		friend class Grid;
		
		iterator() {} // curObj gets initialized to an iterator returning false
		
		iterator& operator++()
		{
			++curObj;
			findFirstValid();

			return *this;
		}
		
		CGameObject& operator*() { return *curObj; }		
		CGameObject* operator->() { return (CGameObject *)curObj; }
		operator bool() { return curObj != NULL; }

		Layer& curLayer() { return *curLayerIt; }
		
		void erase()
		{
			curLayerIt->list.erase(curObj);
			findFirstValid();
		}
		
		void unlink()
		{
			curLayerIt->list.unlink(curObj);
			findFirstValid();
		}
				
	private:
		iterator(Grid& grid, int layerBegin, int layerStep, int layerCount)
		: layersPitch(layerStep), layersLeft(layerCount)
		{
			curLayerIt = grid.layers.begin() + layerBegin;
			curObj = curLayerIt->list.beginS();
			findFirstValid();
		}
		
		void findFirstValid()
		{
			while(!curObj)
			{
				if(--layersLeft <= 0)
				{
					return;
				}
				curLayerIt += layersPitch;
				curObj = curLayerIt->list.beginS();
			}
		}

		int layersPitch;
		int layersLeft;
		ObjectList::iterator curObj;
		std::vector<Layer>::iterator curLayerIt;
	};
	
	int getListIndex(Vec const& v)
	{
		int x = (int(v.x) + offsetX) >> shift;
		int y = (int(v.y) + offsetY) >> shift;
		
		if(x < 0) x = 0;
		else if(x >= squaresH) x = squaresH - 1;
		
		if(y < 0) y = 0;
		else if(y >= squaresV) y = squaresV - 1;
		
		return x + y*squaresH;
	}
	
	std::vector<Layer::Square>::iterator getSquare(int x, int y, Layer& layer)
	{
		return layer.grid.begin() + x + y*squaresH;
	}
	
	int getSquareIdx(int x, int y)
	{
		return x + y*squaresH;
	}
	
	void insert(CGameObject* obj, int colLayer, int renderLayer)
	{
		assert(obj->cellIndex_ < 0); // not yet set
		int squareIndex = getListIndex(obj->pos());
		obj->cellIndex_ = squareIndex;

		Layer& layer = layers[colLayer + ColLayerCount*renderLayer];
		assert((unsigned int)squareIndex < layer.grid.size());
		layer.insertDelayed(obj);
	}
	
	void insert(CGameObject* obj, int renderLayer)
	{
		Layer& layer = layers[NoColLayer + ColLayerCount*renderLayer];
		layer.insertDelayedNoCol(obj);
	}
	
	void insertImmediately(CGameObject* obj, int colLayer, int renderLayer)
	{
		Layer& layer = layers[colLayer + ColLayerCount*renderLayer];
		insertImmediately(obj, layer);
	}
	
	void insertImmediately(CGameObject* obj, Layer& layer) {
		assert(obj->cellIndex_ < 0); // not yet set
		int cellIndex = getListIndex(obj->pos());
		obj->cellIndex_ = cellIndex;
		
		layer.list.insertAfter(layer.grid[cellIndex].guard(), obj);		
	}
	
	void unlink(const CGameObject* obj) {
		for ( iterator iter = beginAll(); iter;)
		{
			if( &*iter == obj )
				iter.unlink();
			else
				++iter;
		}
	}

	void flush()
	{
		for(std::vector<Layer>::iterator i = layers.begin(); i != layers.end(); ++i)
		{
			i->flushDelayed();
		}
	}
		
	void relocateIfNecessary(iterator o)
	{
		if(!o->prevD_) // Not a relocatable node
			return;
		
		int curIndex = o->cellIndex_;
		int realIndex = getListIndex(o->pos());
		if(realIndex != curIndex)
		{
			assert((unsigned int)realIndex < o.curLayerIt->grid.size());
			o->cellIndex_ = realIndex;
			o.curLayerIt->list.moveAfter(o.curLayerIt->grid[realIndex].guard(), ObjectList::light_iterator(o.curObj));
		}
	}

	area_iterator beginArea(int x1, int y1, int x2, int y2, int layer)
	{
		int gridx1 = (x1 + offsetX - maxObjectRange) >> shift;
		int gridy1 = (y1 + offsetY - maxObjectRange) >> shift;
		int gridx2 = (x2 + squareSide + offsetX + maxObjectRange) >> shift;
		int gridy2 = (y2 + squareSide + offsetY + maxObjectRange) >> shift;
		
		if(gridx2 < 1) gridx2 = 1;
		else if(gridx2 > squaresH) gridx2 = squaresH;
		if(gridy2 < 1) gridy2 = 1;
		else if(gridy2 > squaresV) gridy2 = squaresV;
		
		if(gridx1 < 0) gridx1 = 0;
		else if(gridx1 >= squaresH) gridx1 = squaresH - 1;
		if(gridy1 < 0) gridy1 = 0;
		else if(gridy1 >= squaresV) gridy1 = squaresV - 1;
		
		if(gridx1 >= gridx2 || gridy1 >= gridy2) return area_iterator();
		
		return area_iterator(*this
			, gridx1
			, gridy1
			, gridx2 - gridx1
			, gridy2 - gridy1
			, layer, RenderLayerCount, ColLayerCount);
	}
	
	iterator beginAll()
	{
		if(layers.size() == 0)
			return iterator();
			
		return iterator(*this, 0, 1, (int)layers.size());
	}
	
	iterator beginColLayer(int layer)
	{
		return iterator(*this, layer, RenderLayerCount, ColLayerCount);
	}
	
	void clear()
	{
		for(std::vector<Layer>::iterator i = layers.begin(); i != layers.end(); ++i)
		{
			i->clear();
		}
	}
	
	size_t size()
	{
		size_t s = 0;
		for(std::vector<Layer>::iterator i = layers.begin(); i != layers.end(); ++i)
		{
			s += i->list.size();
		}
		return s;
	}
	
	
private:

	int width;
	int height;
	int offsetX;
	int offsetY;
	int squaresH;
	int squaresV;
	int x1;
	int y1;
	int x2;
	int y2;
	
	std::vector<Layer> layers;
};

/*
	New grid
	
	struct Cell
	{
		T* prev;  // First previous object
		T* first; // First object in this cell
		T* last;  // Last object in this cell
		T* next;  // First object after this cell
	};
	
	def insert(Object o, Cell c):
		if c.first == 0
			insert o after c.prev
			set next = o on all previous cells with next == c.next
			set prev = o on all next cells with prev == c.prev
			set c.last = c.first = o
		else
			if c.first == c.last // Only one object in this cell
				set prev = o on all next cells with prev == c.first
				set c.last = o
			insert after c.first
			
	def unlinkD(Object o, Cell c):
		if c // The object is in a cell
			if c.first == c.last // c.first and c.last must be o
				set c.first = c.last = 0
				set next = c.next on all previous cells with next == c.first
				set prev = c.prev on all next cells with prev == c.last
			else if c.first == o
				set next = c.first.next on all previous cells with next == c.first
				set c.first = c.first.next
			else if c.last == o
				set prev = c.last.prev on all next cells with prev == c.last
				set c.last = c.last.prev
			
			// There's always a prev and a next
			c.prev.next = c.next
			c.next.prev = c.prev
		
		

*/

#endif //VERMES_OBJECT_GRID_H
