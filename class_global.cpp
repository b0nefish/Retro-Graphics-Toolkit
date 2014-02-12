/*
 This file is part of Retro Graphics Toolkit

    Retro Graphics Toolkit is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or any later version.

    Retro Graphics Toolkit is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Retro Graphics Toolkit.  If not, see <http://www.gnu.org/licenses/>.
    Copyright Sega16 (or whatever you wish to call me (2012-2014)
*/
#include "global.h"
#include "class_global.h"
editor *window = new editor(800,600,"Retro Graphics Toolkit");//this creates the window
static void rect_alpha_grid(uint8_t rgba[4],uint16_t x,uint16_t y){
	uint8_t grid[32*32*3];
	//first generate grid
	uint8_t * ptr_grid=grid;
	int e=16,c;
	while(e--){
		c=48;
		while(c--)
			*ptr_grid++=255;
		c=48;
		while(c--)
			*ptr_grid++=160;
	}
	e=16;
	while(e--){
		c=48;
		while(c--)
			*ptr_grid++=160;
		c=48;
		while(c--)
			*ptr_grid++=255;
	}
	if (rgba[3]==0){//no need to mix in picture if alpha is 0
		//just draw grid and return
		fl_draw_image(grid,x,y,32,32,3);
		return;
	}
	ptr_grid=grid;
	double percent=rgba[3]/255.0;
	for (uint16_t c=0;c<32*32;++c){
		for (uint8_t e=0;e<3;++e){
			//*ptr_grid++=((double)rgba[e]*percent)+((double)*ptr_grid*(1.0-percent));//undefined
			uint8_t gridNerd=*ptr_grid;
			*ptr_grid++=((double)rgba[e]*percent)+((double)gridNerd*(1.0-percent));
		}
			
	}
	fl_draw_image(grid,x,y,32,32,3);
	
}
void editor::draw_non_gui(){
	//When resizing the window things move around so we need to compensate for that
	uint8_t per_row;
	uint8_t per_row_rgb;
	switch (currentProject->gameSystem){
		case sega_genesis:
			per_row=16;
			per_row_rgb=48;
		break;
		case NES:
			per_row=4;
			per_row_rgb=12;
		break;
		default:
			show_default_error
		break;
	}
	int16_t x,y;//we will need to reuse these later
	uint8_t box_size=pal_size->value();
	uint8_t tiles_size=tile_size->value();
	uint8_t placer_tile_size=place_tile_size->value();
	switch (mode_editor){
		case pal_edit:
			palEdit.updateSize();
			palEdit.draw_boxes();
		break;
		case tile_edit:
			tileEdit_pal.updateSize();
			//draw truecolor preview box
			true_color_box_y=(double)((double)h()/600.0)*(double)default_true_color_box_y;
			true_color_box_x=(double)((double)w()/800.0)*(double)default_true_color_box_x;
			//fl_rectf(true_color_box_x,true_color_box_y,true_color_box_size,true_color_box_size,truecolor_temp[0],truecolor_temp[1],truecolor_temp[2]);
			rect_alpha_grid(truecolor_temp,true_color_box_x,true_color_box_y);
			tile_edit_truecolor_off_x=(double)((double)w()/800.0)*(double)default_tile_edit_truecolor_off_x;
			tile_edit_truecolor_off_y=(double)((double)h()/600.0)*(double)default_tile_edit_truecolor_off_y;
			tile_edit_offset_y=(double)((double)h()/600.0)*(double)default_tile_edit_offset_y;
			tile_edit_offset_x=(tiles_size*9)+tile_edit_truecolor_off_x;//I muliplyed it by 9 instead of 8 to give spacing between the tiles
			currentProject->tileC->draw_truecolor(currentProject->tileC->current_tile,tile_edit_truecolor_off_x,tile_edit_truecolor_off_y,false,false,tiles_size);
			//draw palette selection box
			tileEdit_pal.draw_boxes();
			currentProject->tileC->draw_tile(tile_edit_offset_x,tile_edit_offset_y,currentProject->tileC->current_tile,tiles_size,tileEdit_pal.theRow,false,false);
			if (show_grid){
				//draw the grid
				if (tiles_size > 4){
					for (y=0;y<8;y++){
						for (x=0;x<8;x++)
							fl_draw_box(FL_EMBOSSED_FRAME,(x*tiles_size)+tile_edit_offset_x,(y*tiles_size)+tile_edit_offset_y,tiles_size,tiles_size,0);
					}
					for (y=0;y<8;y++){
						for (x=0;x<8;x++)
							fl_draw_box(FL_EMBOSSED_FRAME,(x*tiles_size)+tile_edit_truecolor_off_x,(y*tiles_size)+tile_edit_truecolor_off_y,tiles_size,tiles_size,0);
					}
				}
			}
		break;
		case tile_place:
			tileMap_pal.updateSize();
			tile_placer_tile_offset_y=(double)((double)h()/600.0)*(double)default_tile_placer_tile_offset_y;
			tileMap_pal.draw_boxes();
			//now draw the tile
			currentProject->tileC->draw_tile(tile_placer_tile_offset_x,tile_placer_tile_offset_y,currentProject->tileC->current_tile,placer_tile_size,tileMap_pal.theRow,G_hflip,G_vflip);
			//currentProject->tileC->draw_truecolor(currentProject->tileC->current_tile,tile_placer_tile_offset_x,tile_placer_tile_offset_y,G_hflip,G_vflip,placer_tile_size);
			//convert posistion
			map_off_y=(double)((double)h()/600.0)*(double)default_map_off_y;
			map_off_x=(double)((double)w()/800.0)*(double)default_map_off_x;
			//draw tile map
			unsigned char max_map_w,max_map_h;//used to calulate the displayable tiles
			max_map_w=((placer_tile_size*8)+w()-map_off_x)/(placer_tile_size*8);//this will puroposly allow one tile to go partly off screen that is normal I added that on purpose
			max_map_h=((placer_tile_size*8)+h()-map_off_y)/(placer_tile_size*8);
			//see if shadow highlight is enabled
			if ((palTypeGen==0) || (currentProject->gameSystem != sega_genesis) || (showTrueColor==true)){
				//shadow highlight is disabled
				for (y=0;y<std::min((int)currentProject->tileMapC->mapSizeH-map_scroll_pos_y,(int)max_map_h);++y){
					for (x=0;x<std::min((int)currentProject->tileMapC->mapSizeW-map_scroll_pos_x,(int)max_map_w);++x){
						uint32_t tempx,tempy;
						tempx=x+map_scroll_pos_x;
						tempy=y+map_scroll_pos_y;
						if (rowSolo){
							int32_t tileTemp = currentProject->tileMapC->get_tileRow(tempx,tempy,tileMap_pal.theRow);
							if (tileTemp!=-1){
								if (showTrueColor)
									currentProject->tileC->draw_truecolor(tileTemp,map_off_x+((x*8)*placer_tile_size),map_off_y+((y*8)*placer_tile_size),currentProject->tileMapC->get_hflip(tempx,tempy),currentProject->tileMapC->get_vflip(tempx,tempy),placer_tile_size);
								else
									currentProject->tileC->draw_tile(map_off_x+((x*8)*placer_tile_size),map_off_y+((y*8)*placer_tile_size),tileTemp,placer_tile_size,currentProject->tileMapC->get_palette_map(x+map_scroll_pos_x,y+map_scroll_pos_y),currentProject->tileMapC->get_hflip(x+map_scroll_pos_x,y+map_scroll_pos_y),currentProject->tileMapC->get_vflip(x+map_scroll_pos_x,y+map_scroll_pos_y));
							}
						}else{
							if (showTrueColor)
								currentProject->tileC->draw_truecolor(currentProject->tileMapC->get_tile(tempx,tempy),map_off_x+((x*8)*placer_tile_size),map_off_y+((y*8)*placer_tile_size),currentProject->tileMapC->get_hflip(tempx,tempy),currentProject->tileMapC->get_vflip(tempx,tempy),placer_tile_size);
							else
								currentProject->tileC->draw_tile(map_off_x+((x*8)*placer_tile_size),map_off_y+((y*8)*placer_tile_size),currentProject->tileMapC->get_tile(x+map_scroll_pos_x,y+map_scroll_pos_y),placer_tile_size,currentProject->tileMapC->get_palette_map(x+map_scroll_pos_x,y+map_scroll_pos_y),currentProject->tileMapC->get_hflip(x+map_scroll_pos_x,y+map_scroll_pos_y),currentProject->tileMapC->get_vflip(x+map_scroll_pos_x,y+map_scroll_pos_y));
						}
						
					}
				}
			}else{
				uint8_t type_temp=palTypeGen;
				for (y=0;y<std::min((int)currentProject->tileMapC->mapSizeH-map_scroll_pos_y,(int)max_map_h);++y){
					for (x=0;x<std::min((int)currentProject->tileMapC->mapSizeW-map_scroll_pos_x,(int)max_map_w);++x){
						uint32_t tempx,tempy;
						tempx=x+map_scroll_pos_x;
						tempy=y+map_scroll_pos_y;
						uint8_t temp=(currentProject->tileMapC->get_prio(x+map_scroll_pos_x,y+map_scroll_pos_y)^1)*8;
						set_palette_type(temp);
						if (rowSolo){
							int32_t tileTemp = currentProject->tileMapC->get_tileRow(tempx,tempy,tileMap_pal.theRow);
							if (temp!=-1){
								if (showTrueColor)
									currentProject->tileC->draw_truecolor(tileTemp,tempx,tempy,currentProject->tileMapC->get_hflip(tempx,tempy),currentProject->tileMapC->get_vflip(tempx,tempy),placer_tile_size);
								else
									currentProject->tileC->draw_tile(map_off_x+((x*8)*placer_tile_size),map_off_y+((y*8)*placer_tile_size),tileTemp,placer_tile_size,currentProject->tileMapC->get_palette_map(tempx,tempy),currentProject->tileMapC->get_hflip(tempx,tempy),currentProject->tileMapC->get_vflip(tempx,tempy));
							}
						}else{
							if (showTrueColor)
								currentProject->tileC->draw_truecolor(currentProject->tileMapC->get_tile(tempx,tempy),tempx,tempy,currentProject->tileMapC->get_hflip(tempx,tempy),currentProject->tileMapC->get_vflip(tempx,tempy),placer_tile_size);
							else
								currentProject->tileC->draw_tile(map_off_x+((x*8)*placer_tile_size),map_off_y+((y*8)*placer_tile_size),currentProject->tileMapC->get_tile(tempx,tempy),placer_tile_size,currentProject->tileMapC->get_palette_map(tempx,tempy),currentProject->tileMapC->get_hflip(tempx,tempy),currentProject->tileMapC->get_vflip(tempx,tempy));
						}
					}
				}
				set_palette_type(type_temp);
			}
			if (show_grid_placer){
				//draw box over tiles
				for (y=0;y<std::min((int)currentProject->tileMapC->mapSizeH-map_scroll_pos_y,(int)max_map_h);++y){
					for (x=0;x<std::min((int)currentProject->tileMapC->mapSizeW-map_scroll_pos_x,(int)max_map_w);x++)
						fl_draw_box(FL_EMBOSSED_FRAME,map_off_x+((x*8)*placer_tile_size),map_off_y+((y*8)*placer_tile_size),placer_tile_size*8,placer_tile_size*8,NULL);
				}
				
			}
			if(tileEditModePlace_G){
				uint32_t xo,yo;
				xo=((selTileE_G[0]-map_scroll_pos_x)*8*placer_tile_size)+map_off_x;
				yo=((selTileE_G[1]-map_scroll_pos_y)*8*placer_tile_size)+map_off_y;
				if((xo>=map_off_x)&&(yo>=map_off_y))
					fl_rect(xo,yo,placer_tile_size*8+1,placer_tile_size*8+1,FL_BLUE);
			}
		break;
	}//end of switch statment
}

void editor::draw(){
	if (damage() == FL_DAMAGE_USER1){
		draw_non_gui();
		return;
	}
	//menu->redraw();
	//the_tabs->redraw();
	//draw_children();
	Fl_Group::draw();
	draw_non_gui();
}

// Create a window at the specified position
editor::editor(int X, int Y, int W, int H, const char *L)
    : Fl_Double_Window(X, Y, W, H, L){
	_editor();
}


// Create a block window
editor::editor(int W, int H, const char *L)
    : Fl_Double_Window(W, H, L) {
	_editor();
}
int editor::handle(int event){
	//printf("Event was %s (%d)\n", fl_eventnames[event], event);     // e.g. "Event was FL_PUSH (1)"
	if (Fl_Double_Window::handle(event)) return (1);
	//printf("Event was %s (%d)\n", fl_eventnames[event], event);     // e.g. "Event was FL_PUSH (1)"
	uint8_t tiles_size;
	switch(event){
		case FL_PUSH:
			switch (mode_editor){
				case pal_edit:
					palEdit.check_box(Fl::event_x(),Fl::event_y());
				break;
				case tile_edit:
					tileEdit_pal.check_box(Fl::event_x(),Fl::event_y());
				break;
				case tile_place:
					tileMap_pal.check_box(Fl::event_x(),Fl::event_y());
				break;
			}
			switch (mode_editor){
				case tile_edit:
					//first see if we are in a "valid" range
					tiles_size=tile_size->value();
					//start by handiling true color
					if ((Fl::event_x() > tile_edit_truecolor_off_x) && (Fl::event_y() > tile_edit_truecolor_off_y) && (Fl::event_x() < tile_edit_truecolor_off_x+(tiles_size*8))  && (Fl::event_y() < tile_edit_truecolor_off_y+(tiles_size*8))){
						//if all conditions have been met that means we are able to edit the truecolor tile
						uint8_t temp_two,temp_one;//geting the mouse posision is the same as with tile editing just different varibles that happens alot in c++ the same thing just slightly different
						temp_one=(Fl::event_x()-tile_edit_truecolor_off_x)/tiles_size;
						temp_two=(Fl::event_y()-tile_edit_truecolor_off_y)/tiles_size;
						//true color tiles are slightly easier to edit
						//I now have a proper function to calulate the offset so I am using that
						currentProject->tileC->truetileDat[cal_offset_truecolor(temp_one,temp_two,0,currentProject->tileC->current_tile)]=truecolor_temp[0];//red
						currentProject->tileC->truetileDat[cal_offset_truecolor(temp_one,temp_two,1,currentProject->tileC->current_tile)]=truecolor_temp[1];//green
						currentProject->tileC->truetileDat[cal_offset_truecolor(temp_one,temp_two,2,currentProject->tileC->current_tile)]=truecolor_temp[2];//blue
						currentProject->tileC->truetileDat[cal_offset_truecolor(temp_one,temp_two,3,currentProject->tileC->current_tile)]=truecolor_temp[3];//alpha
						currentProject->tileC->truecolor_to_tile(tileEdit_pal.theRow,currentProject->tileC->current_tile);
						damage(FL_DAMAGE_USER1);
					}
					if (Fl::event_x() > tile_edit_offset_x && Fl::event_y() > tile_edit_offset_y && Fl::event_x() < tile_edit_offset_x+(tiles_size*8) && Fl::event_y() < tile_edit_offset_y+(tiles_size*8)){
						uint8_t temp_two,temp_one;
						temp_one=(Fl::event_x()-tile_edit_offset_x)/tiles_size;
						temp_two=(Fl::event_y()-tile_edit_offset_y)/tiles_size;
						uint8_t get_pal=(tileEdit_pal.theRow*48)+(tileEdit_pal.box_sel*3);
						currentProject->tileC->truetileDat[cal_offset_truecolor(temp_one,temp_two,0,currentProject->tileC->current_tile)]=currentProject->rgbPal[get_pal];//red
						currentProject->tileC->truetileDat[cal_offset_truecolor(temp_one,temp_two,1,currentProject->tileC->current_tile)]=currentProject->rgbPal[get_pal+1];//green
						currentProject->tileC->truetileDat[cal_offset_truecolor(temp_one,temp_two,2,currentProject->tileC->current_tile)]=currentProject->rgbPal[get_pal+2];//blue
						currentProject->tileC->truecolor_to_tile(tileEdit_pal.theRow,currentProject->tileC->current_tile);
						damage(FL_DAMAGE_USER1);
					}
				break;
				case tile_place:
					tiles_size=place_tile_size->value();
					//see if the user placed a tile on the map
					if (Fl::event_x() > map_off_x && Fl::event_y() > map_off_y && Fl::event_x() < map_off_x+((tiles_size*8)*currentProject->tileMapC->mapSizeW) && Fl::event_y() < map_off_y+((tiles_size*8)*currentProject->tileMapC->mapSizeH)){
						uint32_t temp_two,temp_one;
						temp_one=((Fl::event_x()-map_off_x)/tiles_size)/8;
						temp_two=((Fl::event_y()-map_off_y)/tiles_size)/8;
						temp_one+=+map_scroll_pos_x;
						temp_two+=+map_scroll_pos_y;
						if (Fl::event_button()==FL_LEFT_MOUSE){
							if((selTileE_G[0]==temp_one)&&(selTileE_G[1]==temp_two)&&tileEditModePlace_G){
								tileEditModePlace_G=false;
								damage(FL_DAMAGE_USER1);
							}else{
								tileEditModePlace_G=false;
								currentProject->tileMapC->set_tile_full(currentProject->tileC->current_tile,temp_one,temp_two,tileMap_pal.theRow,G_hflip,G_vflip,G_highlow_p);
								damage(FL_DAMAGE_USER1);
							}
						}else{
							//fl_alert("Tile attributes id: %d h-flip: %d v-flip %d priority: %d pal row: %d\nAt location x: %d y: %d",currentProject->tileMapC->get_tile(temp_one,temp_two),currentProject->tileMapC->get_hflip(temp_one,temp_two),currentProject->tileMapC->get_vflip(temp_one,temp_two),currentProject->tileMapC->get_prio(temp_one,temp_two),currentProject->tileMapC->get_palette_map(temp_one,temp_two),temp_one,temp_two);
							if(tileEditModePlace_G){
								if((selTileE_G[0]==temp_one)&&(selTileE_G[1]==temp_two)){
									tileEditModePlace_G=false;
									damage(FL_DAMAGE_USER1);
									goto skipEditMode;
								}
							}
							tileEditModePlace_G=true;
							selTileE_G[0]=temp_one;
							selTileE_G[1]=temp_two;
							hflipCB->value(currentProject->tileMapC->get_hflip(temp_one,temp_two));
							vflipCB->value(currentProject->tileMapC->get_vflip(temp_one,temp_two));
							prioCB->value(currentProject->tileMapC->get_prio(temp_one,temp_two));
							{uint32_t cT=currentProject->tileMapC->get_tile(temp_one,temp_two);
							tile_select_2->value(cT);
							currentProject->tileC->current_tile=cT;
							uint8_t Rm=currentProject->tileMapC->get_palette_map(temp_one,temp_two);
							tileMap_pal.changeRow(Rm);
							for(int as=0;as<4;++as)
								palRTE[as]->value(as==Rm);}
							redraw();
				skipEditMode:;
						}
					}
					if (Fl::event_x() > tile_placer_tile_offset_x && Fl::event_y() > tile_placer_tile_offset_y && Fl::event_x() < tile_placer_tile_offset_x+(tiles_size*8) && Fl::event_y() < tile_placer_tile_offset_y+(tiles_size*8)){
						uint8_t temp_two,temp_one;
						temp_one=(Fl::event_x()-tile_placer_tile_offset_x)/tiles_size;
						temp_two=(Fl::event_y()-tile_placer_tile_offset_y)/tiles_size;
						if (G_hflip)
							temp_one=7-temp_one;
						if (G_vflip)
							temp_two=7-temp_two;
						//now we know which pixel we are editing
						//see if it is even or odd
						uint8_t temp_1,temp_2;
						if (temp_one & 1){//faster
							//odd
							//split pixels
							uint8_t temp=currentProject->tileC->tileDat[(currentProject->tileC->current_tile*32)+(temp_one/2)+(temp_two*4)];
							//first,second pixel
							temp_1=temp>>4;//first pixel
							temp_2=temp&15;//second pixel
							//put temp_1 back in proper place
							temp_1<<=4;
							temp_1|=tileMap_pal.box_sel;
							currentProject->tileC->tileDat[(currentProject->tileC->current_tile*32)+(temp_one/2)+(temp_two*4)]=temp_1;
						}else{
							//even
							//split pixels
							uint8_t temp=currentProject->tileC->tileDat[(currentProject->tileC->current_tile*32)+(temp_one/2)+(temp_two*4)];
							//first,second pixel
							temp_1=temp>>4;//first pixel
							temp_2=temp&15;//second pixel
							temp_2|=tileMap_pal.box_sel<<4;
							currentProject->tileC->tileDat[(currentProject->tileC->current_tile*32)+(temp_one/2)+(temp_two*4)]=temp_2;
						}
						damage(FL_DAMAGE_USER1);//no need to redraw the gui
					}
				break;
			}
		break;
	}
	return 0;
}
