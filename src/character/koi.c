/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "koi.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//Koi character structure
enum
{
	Koi_ArcMain_Idle0,
	Koi_ArcMain_Idle1,
	Koi_ArcMain_Left,
	Koi_ArcMain_Down,
	Koi_ArcMain_Up,
	Koi_ArcMain_Right,
	
	Koi_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[Koi_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_Koi;

//Koi character definitions
static const CharFrame char_koi_frame[] = {
	{Koi_ArcMain_Idle0, {  0,   0, 78, 186}, { 44, 179}}, //0 idle 1
	{Koi_ArcMain_Idle0, {89,   0, 79, 186}, { 45, 179}}, //1 idle 2
	{Koi_ArcMain_Idle1, {  0,   0, 80, 185}, { 46, 178}}, //2 idle 3
	{Koi_ArcMain_Idle1, {81,   0, 79, 184}, { 45, 177}}, //3 idle 4
	
	{Koi_ArcMain_Left, {  0,   0, 85, 188}, { 54, 184}}, //4 left 1
	{Koi_ArcMain_Left, {97,   0, 83, 188 },{ 51, 184}}, //5 left 2
	
	{Koi_ArcMain_Down, {  0,   0, 87, 174}, { 45, 169}}, //6 down 1
	{Koi_ArcMain_Down, {119,   0, 85, 174}, { 48, 168}}, //7 down 2
	
	{Koi_ArcMain_Up, {  0,   0,  83, 199}, { 50, 185}}, //8 up 1
	{Koi_ArcMain_Up, { 97,   0,  83, 199}, { 50, 185 }}, //9 up 2
	//What the fuck is wrong
	{Koi_ArcMain_Right, {  0,   0, 76, 186}, { 39, 179}}, //10 right 1
	{Koi_ArcMain_Right, {83,   0, 77, 186}, { 42, 178}}, //11 right 2
};

static const Animation char_koi_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0,  1,  2,  3, ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){ 4,  5, ASCR_BACK, 1}},         //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_LeftAlt
	{2, (const u8[]){ 6,  7, ASCR_BACK, 1}},         //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_DownAlt
	{2, (const u8[]){ 8,  9, ASCR_BACK, 1}},         //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_UpAlt
	{2, (const u8[]){10, 11, ASCR_BACK, 1}},         //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_RightAlt
};

//Koi character functions
void Char_Koi_SetFrame(void *user, u8 frame)
{
	Char_Koi *this = (Char_Koi*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_koi_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_Koi_Tick(Character *character)
{
	Char_Koi *this = (Char_Koi*)character;
	
	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
		Character_PerformIdle(character);
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_Koi_SetFrame);
	Character_Draw(character, &this->tex, &char_koi_frame[this->frame]);
}

void Char_Koi_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_Koi_Free(Character *character)
{
	Char_Koi *this = (Char_Koi*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_Koi_New(fixed_t x, fixed_t y)
{
	//Allocate koi object
	Char_Koi *this = Mem_Alloc(sizeof(Char_Koi));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_Koi_New] Failed to allocate koi object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_Koi_Tick;
	this->character.set_anim = Char_Koi_SetAnim;
	this->character.free = Char_Koi_Free;
	
	Animatable_Init(&this->character.animatable, char_koi_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 11;
	
	this->character.focus_x = FIXED_DEC(65,1);
	this->character.focus_y = FIXED_DEC(-85,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\KOI.ARC;1");
	
	const char **pathp = (const char *[]){
		"idle0.tim", //Koi_ArcMain_Idle0
		"idle1.tim", //Koi_ArcMain_Idle1
		"left.tim",  //Koi_ArcMain_Left
		"down.tim",  //Koi_ArcMain_Down
		"up.tim",    //Koi_ArcMain_Up
		"right.tim", //Koi_ArcMain_Right
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
