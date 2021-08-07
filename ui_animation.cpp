#define GUILITE_ON  //Do not define this macro once more!!!
#include "GuiLite.h"//GUI库
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <Wire.h>
#include "ui_animation.h"
#include <Arduino.h>
#include "OLEDDisplayUi.h"
/***星空穿越特效相关参数*/
#define FACTOR_SIZE 2//星空粒子大小//方形时为方形的边长//圆形时为圆形半径
#define FACTOR_SHAPE 1//星空粒子形状//0 方形//1 圆形
/*****************/

const int UI_WIDTH = 128;
const int UI_HEIGHT = 64;
const int FRAME_COUNT = 16;//播放时钟动画的帧数

/************这两个不能动*****/
static c_surface* s_surface;
static c_display* s_display;
/*****************/

unsigned short int randomColor();//产生随机颜色函数声明
void screen_fill_circle(int32_t x0, int32_t y0, int32_t r,uint16_t color);//绘制填充圆形函数声明



/**星空穿越特效类*/
class c_star {
public:
  c_star(){
    initialize();
  }
  void initialize() {
    m_x = m_start_x = rand() % UI_WIDTH;
    m_y = m_start_y = rand() % UI_HEIGHT;
    m_size = FACTOR_SIZE;
    m_x_factor = UI_WIDTH;
    m_y_factor = UI_HEIGHT;
    m_size_factor = 1;
  }

  void move() {
    s_surface->fill_rect(m_x, m_y, m_x + m_size - 1, m_y + m_size - 1, 0, Z_ORDER_LEVEL_0);//clear star footprint

    m_x_factor -= 6;
    m_y_factor -= 6;
    m_size += m_size / 20;
    if (m_x_factor < 1 || m_y_factor < 1)
    {
      return initialize();
    }
    if (m_start_x > (UI_WIDTH / 2) && m_start_y > (UI_HEIGHT / 2))
    {
      m_x = (UI_WIDTH / 2) + (UI_WIDTH * (m_start_x - (UI_WIDTH / 2)) / m_x_factor);
      m_y = (UI_HEIGHT / 2) + (UI_HEIGHT * (m_start_y - (UI_HEIGHT / 2)) / m_y_factor);
    }
    else if (m_start_x <= (UI_WIDTH / 2) && m_start_y > (UI_HEIGHT / 2))
    {
      m_x = (UI_WIDTH / 2) - (UI_WIDTH * ((UI_WIDTH / 2) - m_start_x) / m_x_factor);
      m_y = (UI_HEIGHT / 2) + (UI_HEIGHT * (m_start_y - (UI_HEIGHT / 2)) / m_y_factor);
    }
    else if (m_start_x > (UI_WIDTH / 2) && m_start_y <= (UI_HEIGHT / 2))
    {
      m_x = (UI_WIDTH / 2) + (UI_WIDTH * (m_start_x - (UI_WIDTH / 2)) / m_x_factor);
      m_y = (UI_HEIGHT / 2) - (UI_HEIGHT * ((UI_HEIGHT / 2) - m_start_y) / m_y_factor);
    }
    else if (m_start_x <= (UI_WIDTH / 2) && m_start_y <= (UI_HEIGHT / 2))
    {
      m_x = (UI_WIDTH / 2) - (UI_WIDTH * ((UI_WIDTH / 2) - m_start_x) / m_x_factor);
      m_y = (UI_HEIGHT / 2) - (UI_HEIGHT * ((UI_HEIGHT / 2) - m_start_y) / m_y_factor);
    }

    if (m_x < 0 || (m_x + m_size - 1) >= UI_WIDTH ||
      m_y < 0 || (m_y + m_size - 1) >= UI_HEIGHT)
    {
      return initialize();
    }
    if(FACTOR_SHAPE == 0)
    {
      s_surface->fill_rect(m_x, m_y, m_x + m_size - 1, m_y + m_size - 1, randomColor(), Z_ORDER_LEVEL_0);//draw star
  
    }
    else if(FACTOR_SHAPE == 1)
    {
      screen_fill_circle(m_x, m_y, m_size, randomColor());

    }
  }
  int m_start_x, m_start_y;
  float m_x, m_y, m_x_factor, m_y_factor, m_size_factor, m_size;
};
/*****************/

c_star stars[36];//星星数量穿越特效
    
void create_ui(void* phy_fb, int screen_width, int screen_height, int color_bytes, struct EXTERNAL_GFX_OP* gfx_op)//ui的初始化函数
{
  /**********这部分使用时几乎不需要修改*******/
  if (phy_fb)
  {
    static c_surface surface(UI_WIDTH, UI_HEIGHT, color_bytes, Z_ORDER_LEVEL_0);
    static c_display display(phy_fb, screen_width, screen_height, &surface);
    s_surface = &surface;
    s_display = &display;
  }
  else
  {//for MCU without framebuffer
    static c_surface_no_fb surface_no_fb(UI_WIDTH, UI_HEIGHT, color_bytes, gfx_op, Z_ORDER_LEVEL_0);
    static c_display display(phy_fb, screen_width, screen_height, &surface_no_fb);
    s_surface = &surface_no_fb;
    s_display = &display;
  }

  //background
  s_surface->fill_rect(0, 0, UI_WIDTH, UI_HEIGHT, 0, Z_ORDER_LEVEL_0);
  /*****************/


}

void ui_update(void)
{
  
    /*星空背景特效使用范例*/   
    for (int i = 0; i < sizeof(stars)/sizeof(c_star); i++) 
    {
        stars[i].move();
    } 
    delay(50);//改变这个延时函数就能改变特效播放的快慢
}

unsigned short int randomColor()
{
    unsigned char red,green,blue;
    red = rand();
    //__ASM("NOP");
    green = rand();
    //__ASM("NOP");
    blue = rand();
    return (red << 11 | green << 5 | blue);
}

/*********填充圆用到的工具函数********/
void screen_draw_fastVLine(int32_t x, int32_t y, int32_t length,uint16_t color)  
{
  // Bounds check
    int32_t y0=y;
    do
    {
        s_surface->draw_pixel(x, y,color, Z_ORDER_LEVEL_0);   // 逐点显示，描出垂直线
        y++;
    }
    while(y0+length>=y);
}

void screen_fillCircle_helper(int32_t x0, int32_t y0, int32_t r, uint8_t corner, int32_t delta,uint16_t color)  {

  int32_t f     = 1 - r;
  int32_t ddF_x = 1;
  int32_t ddF_y = -2 * r;
  int32_t x     = 0;
  int32_t y     = r;

  while(x<y)  {
    if(f >= 0)  {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;

    if(corner & 0x1)  {
      screen_draw_fastVLine(x0+x, y0-y, 2*y+delta, color);
      screen_draw_fastVLine(x0+y, y0-x, 2*x+delta, color);
    }
    if(corner & 0x2)  {
      screen_draw_fastVLine(x0-x, y0-y, 2*y+delta, color);
      screen_draw_fastVLine(x0-y, y0-x, 2*x+delta, color);
    }
  }
}
/*****************/

void screen_fill_circle(int32_t x0, int32_t y0, int32_t r,uint16_t color) 
{
    screen_draw_fastVLine(x0, y0-r, 2*r, color);
    screen_fillCircle_helper(x0, y0, r, 3, 0, color);
}
