#include "graphics.h"

int i, j = 0, gd = DETECT, gm;

const int Width = 800;
const int Height = 600;
int g_offSet = 200;
typedef int Radius;

struct Point
{
    double x;
    double y;
    Point() {}
    Point(double a, double b)
    {
        x = a;
        y = b;
    }
};

template <typename A>
Point convertPixel(A &x, A &y)
{
    //    x+=Width/2;
    y = -y;
    y += Height - g_offSet;
    return {x, y};
}

void drawAxis()
{
    for (int i = 0; i < Height; i++)
    {
        putpixel(Width / 2, i, WHITE);
    }
    for (int i = 0; i < Width; i++)
    {
        putpixel(i, Height, WHITE);
    }
}

void drawAxis(int offSet)
{
    g_offSet = offSet;
    for (int i = 0; i < Width; i++)
    {
        putpixel(i, Height - g_offSet - 20, WHITE);
    }
}

void drawPixel(double x, double y, int color)
{
    convertPixel(x, y);
    putpixel(x, y, color);
}

void draw_line_DDA(Point a, Point b, int color)
{
    double x0 = a.x, y0 = a.y, x1 = b.x, y1 = b.y;
    double dx = x1 - x0;
    double dy = y1 - y0;

    double steps = std::max(abs(dx), abs(dy));

    double Xinc = dx / steps;
    double Yinc = dy / steps;
    for (int i = 0; i < steps; i++)
    {
        drawPixel(x0, y0, color);
        x0 += Xinc;
        y0 += Yinc;
    }
}

void draw_circle_Bressenham(Point c, Radius r, int col)
{
    double d = 3 - 2 * r;
    double y = r, x = 0;
    double h = c.x, k = c.y;

    while (y >= x)
    {
        drawPixel(h + x, k + y, col);
        drawPixel(h + y, k + x, col);
        drawPixel(h - x, k + y, col);
        drawPixel(h - y, k + x, col);
        drawPixel(h + x, k - y, col);
        drawPixel(h + y, k - x, col);
        drawPixel(h - x, k - y, col);
        drawPixel(h - y, k - x, col);
        x++;
        if (d < 0)
        {
            d = d + 4 * x + 6;
        }
        else
        {
            d = d + 4 * x - 4 * y + 6;
            y--;
        }
    }
}

void draw_circle(Point c, Radius r, int col)
{
    convertPixel(c.x, c.y);
    int old_col = getcolor();
    setcolor(col);
    circle(c.x, c.y, r);
    setcolor(old_col);
}

void floodFill(Point currPixel, int oldcolor, int fillColor)
{
    int tempx = currPixel.x, tempy = currPixel.y;
    Point pixel = convertPixel(tempx, tempy);
    int currColor = getpixel(pixel.x, pixel.y);
    if (currColor != oldcolor)
    {
        return;
    }
    drawPixel(currPixel.x, currPixel.y, fillColor);

    floodFill(Point(currPixel.x, currPixel.y + 1), oldcolor, fillColor);
    floodFill(Point(currPixel.x, currPixel.y - 1), oldcolor, fillColor);
    floodFill(Point(currPixel.x + 1, currPixel.y), oldcolor, fillColor);
    floodFill(Point(currPixel.x - 1, currPixel.y), oldcolor, fillColor);
}

class Car
{

public:
    int x, y;
    void DrawCar(double i)
    {
        draw_line_DDA({40 + i, 40}, {40 + i, 80}, YELLOW);
        draw_line_DDA({40 + i, 40}, {80 + i, 40}, YELLOW);
        draw_circle_Bressenham({80 + 20 + i, 40}, 20, YELLOW);
        draw_line_DDA({120 + i, 40}, {120 + 100 + i, 40}, YELLOW);
        draw_circle_Bressenham({220 + 20 + i, 40}, 20, YELLOW);
        draw_line_DDA({40 + i, 80}, {120 + i, 130}, YELLOW);
        draw_line_DDA({120 + i, 130}, {240 + i, 130}, YELLOW);
        draw_line_DDA({240 + i, 130}, {300 + i, 80}, YELLOW);
        draw_line_DDA({300 + i, 80}, {340 + i, 80}, YELLOW);
        draw_line_DDA({340 + i, 80}, {340 + i, 40}, YELLOW);
        draw_line_DDA({260 + i, 40}, {340 + i, 40}, YELLOW);
    }
};

int main()
{
    initwindow(Width, Height);
    drawAxis(400);
    drawAxis(250);

    Car car;
    int xpos, ypos;
    for (int i = 0; i < 500; i++)
    {
        if (i > 400)
        {
            i = -700;
        }
        car.DrawCar(i);
        if (ismouseclick(WM_LBUTTONDOWN))
        {
            getmouseclick(WM_LBUTTONDOWN, xpos, ypos);
            //            setfillstyle(SOLID_FILL,RED);
            //            floodfill(xpos,ypos,YELLOW);
            floodFill({xpos, ypos}, BLACK, YELLOW);
        };
        delay(1000);
        swapbuffers();
        cleardevice();
    }

    getchar();
    return 0;
}
