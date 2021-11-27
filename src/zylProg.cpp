/**
 * @file zylProg.cpp
 * @author Bernhard Stöffler
 * @brief 
 * @version 0.1
 * @date 2021-11-15
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "zylProg.h"

//***************** ZylProg base class functions *******************
zylProg::zylProg()
{	// automatically add new zylProg to linked list
	zylProgManager::add(this);
}

zylProg::zylProg(bool add)
{	// automatically add new zylProg to linked list
	//! to use this, derived zylProg needs "using zylProg::zylProg"
	if(add)
		zylProgManager::add(this);
}

int zylProg::push()
{	//pushes program to the top of the render queue
	if(m_pAbove || m_pBelow){
		return 1;
	}else{
		m_pAbove = 						&zylProgManager::s_FG;	//set own pointers
		m_pBelow = 						zylProgManager::s_FG.m_pBelow;
		zylProgManager::s_FG.m_pBelow = this;					//redirect other's pointers
		m_pBelow->m_pAbove = 			this;
		return 0;
	}
}

int zylProg::pop()
{	//removes program from render queue, if it's in there
	if(m_pAbove || m_pBelow){
		m_pAbove->m_pBelow = 	m_pBelow; 	//redirect other's pointers
		m_pBelow->m_pAbove = 	m_pAbove;
		m_pAbove = 				NULL;		//remove own pointers
		m_pBelow = 				NULL;
		return 0;
	}else
		return 1;
}

int zylProg::move(bool up)
{	//swaps program with one above(/below) if in render queue
	if(m_pAbove == NULL || m_pBelow == NULL){
		return 1;	// program not in queue
	}else if((m_pAbove == &zylProgManager::s_FG && up)
	 ||(m_pBelow == &zylProgManager::s_BG && !up)){
		return 2;	// program at edge
	}else{
		if(up){		//get a whiteboard and draw some arrows if unclear
			zylProg* oldAbove = 	m_pAbove;
			m_pAbove = 				oldAbove->m_pAbove;
			oldAbove->m_pAbove =	this;
			oldAbove->m_pBelow = 	m_pBelow;
			m_pBelow->m_pAbove = 	oldAbove;
			m_pBelow = 				oldAbove;
			m_pAbove->m_pBelow = 	this;
			return 0;
		}else{
			zylProg* oldBelow = 	m_pBelow;
			m_pBelow = 				oldBelow->m_pBelow;
			oldBelow->m_pBelow = 	this;
			oldBelow->m_pAbove = 	m_pAbove;
			m_pAbove->m_pBelow = 	oldBelow;
			m_pAbove = 				oldBelow;
			m_pBelow->m_pAbove = 	this;
			return 0;
		}
	}
}

//************************* Program Manager **********************
int 		zylProgManager::s_Count =				0;
zylProg*	zylProgManager::s_pHead = 				NULL;
zylProg*	zylProgManager::s_pActive =				NULL;
CRGB 		zylProgManager::s_aColors[MAX_COLORS] = {CRGB::Black};
int 		zylProgManager::s_ActiveColorIndex = 	0;
zylProg		zylProgManager::s_FG(false);
zylProg		zylProgManager::s_BG(false);


void zylProgManager::add(zylProg* ptr)
{
	ptr->m_pNext = 	s_pHead;
	s_pHead = 		ptr;
	s_Count++;
}

int zylProgManager::focus(int id)
{
	zylProg* ptr = s_pHead;
	while(ptr != NULL){
		if(ptr->m_Id==id){
			Serial.printf("Found program with ID %d\n", id);
			s_pActive = ptr;
			s_pActive->activate();
			return 0;
		}
		ptr = ptr->m_pNext;
	}
	return 1;
}

void zylProgManager::input(uint8_t x, uint8_t y, uint8_t z){
	s_pActive->input(x, y, z);
}

//TODO init(): FG/BG pointers, push/activate first, NULL handling

int zylProgManager::initPrograms(){
	Serial.printf("initializing %d programs\n", s_Count);
	int error=0;
	zylProg* ptr = s_pHead;
	for(int i=0; i<s_Count; i++){
		error += ptr->init();
		Serial.printf("Program with ID %d at %p\n", ptr->m_Id, ptr);
		ptr = ptr->m_pNext;
	}
	return error;
}

int zylProgManager::init(){
	s_aColors[0] = 		CRGB::Green;
	s_FG.m_pAbove = 	&s_FG;
	s_FG.m_pBelow = 	&s_BG;
	s_BG.m_pAbove = 	&s_FG;
	s_BG.m_pBelow = 	&s_BG;
	if(s_pHead == NULL){
		Serial.println("NULL POINTER ERROR, no programs loaded\n");
		return 1;
	}
	s_pActive = s_pHead;	//focus first program and push it on the render list
	Serial.printf("Pushing Program %d on the list, wish me luck\n", s_pActive->m_Id);
	s_pActive->push();
	s_pActive->activate();
	return 0;
}

//TODO only render programs in renderlist
void zylProgManager::renderPrograms()
{
	zylProg* ptr = s_pHead;
	for(int i=0; i<s_Count; i++){
		ptr->render();
		ptr = ptr->m_pNext;
	}
}

//TODO renderlist; merge with renderPrograms()
void zylProgManager::composite(CRGB fb_in[X_RES][Y_RES])
{	//currently only composite active program
	for(int x=0;x<X_RES;x++)
		for(int y=0;y<Y_RES;y++)
			fb_in[x][y] = s_pActive->m_FB[x][y];
}

int zylProgManager::changeComposition(int x)
{
	switch(x){
	case 0: //push
		return s_pActive->push();
	case 1: //pop
		return s_pActive->pop();
	case 2: //move up
		return s_pActive->move(true);
	case 3: //move down
		return s_pActive->move(false);
	default: //invalid input
		return 2;
	}
}

void zylProgManager::printComposition()
{
	Serial.printf("\nComposition:\n");
	zylProg* ptr = &s_FG;
	for(int i=0;;i++){
		if(ptr == &s_FG){
			Serial.printf("Layer %d: Foreground\n", i);
		}else if(ptr == &s_BG){
			Serial.printf("Layer %d: Background\n", i);
			break;
		}else{
			Serial.printf("Layer %d: Program %d%s\n", i, ptr->m_Id, (ptr==s_pActive)?"(Active)":"");
		}
		ptr = ptr->m_pBelow;
	}
}

void zylProgManager::selectColor(int i)
{	//activates color i
	s_ActiveColorIndex = i;
}

void zylProgManager::setColor(CRGB c, int i)
{	//changes a specific color and leaves index on it
	if(i>=0 && i<MAX_COLORS){
		s_ActiveColorIndex = i;
		setColor(c);
 	}
}
 
void zylProgManager::setColor(CRGB c)
{	//changes the currently active color
	s_aColors[s_ActiveColorIndex] = c;
}

CRGB zylProgManager::getColor(int i)
{	//returns a specific color without activating it
	return s_aColors[i];
}

CRGB zylProgManager::getColor()
{	//returns the currently selected color
	return s_aColors[s_ActiveColorIndex];
}