#include "include/iGraphics.h"
#include <windows.h>
#include <mmsystem.h>
#include <math.h>
#include <stdio.h>

#pragma comment(lib, "winmm.lib")

// -------------------- Window --------------------
const int SCR_W = 1220;
const int SCR_H = 590;

// -------------------- Game State --------------------
enum Screen {
	SCREEN_LOADING,
	SCREEN_MENU,
	SCREEN_LEVEL,
	SCREEN_PLAY,
	SCREEN_HIGHSCORE,
	SCREEN_INSTRUCTIONS,
	SCREEN_ABOUT
};

Screen currentScreen = SCREEN_LOADING;

// -------------------- Loading Screen --------------------
float loadingProgress = 0.0f;     // 0.0 -> 1.0

void updateLoading()
{
	if (currentScreen != SCREEN_LOADING) return;

	loadingProgress += 0.008f;     // speed of loading bar
	if (loadingProgress >= 1.0f)
	{
		loadingProgress = 1.0f;
		currentScreen = SCREEN_MENU;
	}
}

// -------------------- Level Screen UI --------------------
struct LevelThumb {
	int x, y, w, h;
};

LevelThumb levelThumbs[6];
int selectedLevel = 1; // default selected level (1..6)

// Back & Play buttons (bottom left/right)
int levelBackX = 110, levelBackY = 50, levelBackW = 220, levelBackH = 60;
int levelPlayX = SCR_W - 110 - 220, levelPlayY = 50, levelPlayW = 220, levelPlayH = 60;

// OPTIONAL: Use one image as thumbnail for all 6 levels
// If you later add different images, you can load them separately.
unsigned int texLevelThumb = 0;

void buildLevelLayout()
{
	// Layout like your image: 3 columns x 2 rows
	int thumbW = 240;
	int thumbH = 120;

	int gapX = 50;
	int gapY = 50;

	int startX = SCR_W / 2 - (1.5 * thumbW + gapX);
	int startY = SCR_H - 280; // top row y

	// Row 1
	levelThumbs[0] = { startX + 0 * (thumbW + gapX), startY, thumbW, thumbH };
	levelThumbs[1] = { startX + 1 * (thumbW + gapX), startY, thumbW, thumbH };
	levelThumbs[2] = { startX + 2 * (thumbW + gapX), startY, thumbW, thumbH };

	// Row 2
	levelThumbs[3] = { startX + 0 * (thumbW + gapX), startY - (thumbH + gapY), thumbW, thumbH };
	levelThumbs[4] = { startX + 1 * (thumbW + gapX), startY - (thumbH + gapY), thumbW, thumbH };
	levelThumbs[5] = { startX + 2 * (thumbW + gapX), startY - (thumbH + gapY), thumbW, thumbH };
}

// -------------------- About Screen UI --------------------
struct AboutCard {
	int x, y, w, h;
	const char* name;
	const char* id;
	const char* batch;
};

int aboutBackX = 40, aboutBackY = SCR_H - 70, aboutBackW = 100, aboutBackH = 45;

AboutCard aboutCards[3] = {
	{ 120, 60, 250, 220, "Adnan Hossain", "00724205101054", "Inference 54" },
	{ 470, 60, 250, 220, "Araf Hossain", "00724205101058", "Inference 54" },
	{ 820, 60, 250, 220, "Mohammad Daiyan", "00724205101066", "Inference 54" },
};

// -------------------- Textures --------------------
unsigned int texLogo = 0;
unsigned int texMenuBg = 0;
unsigned int texGameBg = 0;

// Knight parts (Method 1)
/*unsigned int texKnightBody = 0;
unsigned int texKnightSword = 0;
unsigned int texKnightFlag = 0;*/

// -------------------- Buttons --------------------
struct Button {
	int x, y, w, h;
	const char* label;
};

Button menuBtns[5];
int hoveredBtn = -1;

bool pointInRect(int px, int py, int x, int y, int w, int h) {
	return (px >= x && px <= x + w && py >= y && py <= y + h);
}

void buildMenuButtons() {
	int bw = 320;
	int bh = 60;
	int gap = 18;

	int startX = 110;
	int topY = SCR_H - 220;

	menuBtns[0] = { startX, topY - 0 * (bh + gap), bw, bh, "START" };
	menuBtns[1] = { startX, topY - 1 * (bh + gap), bw, bh, "HIGH SCORE" };
	menuBtns[2] = { startX, topY - 2 * (bh + gap), bw, bh, "INSTRUCTIONS" };
	menuBtns[3] = { startX, topY - 3 * (bh + gap), bw, bh, "ABOUT" };
	menuBtns[4] = { startX, topY - 4 * (bh + gap), bw, bh, "EXIT" };
}

void drawButton(const Button& b, bool hover) {
	iSetColor(20, 20, 20);
	iFilledRectangle(b.x, b.y, b.w, b.h);

	if (hover) iSetColor(255, 60, 60);
	else       iSetColor(255, 255, 255);

	iRectangle(b.x, b.y, b.w, b.h);
	iRectangle(b.x - 2, b.y - 2, b.w + 4, b.h + 4);

	iSetColor(255, 255, 255);
	iText(b.x + 30, b.y + 22, (char*)b.label, GLUT_BITMAP_TIMES_ROMAN_24);
}

// -------------------- Center Window (WinAPI) --------------------
void centerGameWindow(const char* windowTitle)
{
	HWND hwnd = FindWindowA(NULL, windowTitle);
	if (!hwnd) return;

	RECT rc;
	GetWindowRect(hwnd, &rc);
	int winW = rc.right - rc.left;
	int winH = rc.bottom - rc.top;

	int screenW = GetSystemMetrics(SM_CXSCREEN);
	int screenH = GetSystemMetrics(SM_CYSCREEN);

	int x = (screenW - winW) / 2;
	int y = (screenH - winH) / 2 - 20;

	SetWindowPos(hwnd, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

// -------------------- Knight Animation (Method 1) --------------------
/*int knightW = 450;
int knightH = 520;
int knightX = 0;
int knightY = 0;

float tWave = 0.0f;
float swordAngle = 0.0f;
float flagAngle = 0.0f;

float swordPivotX = 165;
float swordPivotY = 185;

float flagPivotX = 260;
float flagPivotY = 420;

const float SRC_CANVAS_W = 1024.0f;
const float SRC_CANVAS_H = 1024.0f;

void animateKnight()
{
	if (currentScreen != SCREEN_MENU) return;

	tWave += 0.08f;
	swordAngle = sinf(tWave) * 10.0f;
	flagAngle = sinf(tWave * 0.9f) * 4.0f;
}

void drawKnightAnimated()
{
	if (!texKnightBody && !texKnightSword && !texKnightFlag) return;

	float sx = (float)knightW / SRC_CANVAS_W;
	float sy = (float)knightH / SRC_CANVAS_H;

	// FLAG (behind)
	if (texKnightFlag)
	{
		glPushMatrix();
		glTranslatef(knightX + flagPivotX * sx, knightY + flagPivotY * sy, 0);
		glRotatef(flagAngle, 0, 0, 1);
		glTranslatef(-(flagPivotX * sx), -(flagPivotY * sy), 0);

		iShowImage(knightX, knightY, knightW, knightH, texKnightFlag);
		glPopMatrix();
	}

	// BODY
	if (texKnightBody)
		iShowImage(knightX, knightY, knightW, knightH, texKnightBody);

	// SWORD (front)
	if (texKnightSword)
	{
		glPushMatrix();
		glTranslatef(knightX + swordPivotX * sx, knightY + swordPivotY * sy, 0);
		glRotatef(swordAngle, 0, 0, 1);
		glTranslatef(-(swordPivotX * sx), -(swordPivotY * sy), 0);

		iShowImage(knightX, knightY, knightW, knightH, texKnightSword);
		glPopMatrix();
	}
}
*/

// -------------------- Screens --------------------
void drawLoading()
{
	// Red background like your sample
	iSetColor(120, 10, 10);
	iFilledRectangle(0, 0, SCR_W, SCR_H);

	// Logo center-top
	if (texLogo)
	{
		int logoW = 520;
		int logoH = 260;
		int logoX = (SCR_W - logoW) / 2;
		int logoY = SCR_H - logoH - 70;
		iShowImage(logoX, logoY, logoW, logoH, texLogo);
	}


	// Progress bar (white outline + filled)
	int barW = 700;
	int barH = 28;
	int barX = (SCR_W - barW) / 2;
	int barY = 160;

	// "Loading..."
	iSetColor(255, 255, 255);
	iText(barX, 210, (char*)"Loading...", GLUT_BITMAP_TIMES_ROMAN_24);

	// outline
	iSetColor(255, 255, 255);
	iRectangle(barX, barY, barW, barH);

	// background of bar
	iSetColor(220, 220, 220);
	iFilledRectangle(barX + 2, barY + 2, barW - 4, barH - 4);

	// fill amount
	int fillW = (int)((barW - 4) * loadingProgress);

	iSetColor(180, 30, 30);
	iFilledRectangle(barX + 2, barY + 2, fillW, barH - 4);
}

void drawMenu()
{
	if (texMenuBg) iShowImage(0, 0, SCR_W, SCR_H, texMenuBg);

	// Logo (top center)
	if (texLogo)
	{
		int logoW = 520;
		int logoH = 260;
		int logoX = (SCR_W - logoW) / 2;
		int logoY = SCR_H - logoH - 10;
		iShowImage(logoX, logoY, logoW, logoH, texLogo);
	}

	// drawKnightAnimated();

	for (int i = 0; i < 5; i++)
		drawButton(menuBtns[i], (hoveredBtn == i));
}

void drawLevelScreen()
{
	// Background: dark gray like your sample
	iSetColor(55, 60, 65);
	iFilledRectangle(0, 0, SCR_W, SCR_H);

	// Title
	iSetColor(255, 0, 0);
	iText(SCR_W / 2 - 110, SCR_H - 80, (char*)"SELECT LEVEL", GLUT_BITMAP_TIMES_ROMAN_24);

	// Draw 6 thumbnails
	for (int i = 0; i < 6; i++)
	{
		LevelThumb t = levelThumbs[i];

		// Thumbnail image if available, else fallback rectangle
		if (texLevelThumb)
		{
			iShowImage(t.x, t.y, t.w, t.h, texLevelThumb);
		}
		else
		{
			iSetColor(20, 20, 20);
			iFilledRectangle(t.x, t.y, t.w, t.h);
		}

		// Border (highlight selected)
		if (selectedLevel == i + 1) iSetColor(255, 0, 0);
		else                       iSetColor(0, 0, 0);

		iRectangle(t.x, t.y, t.w, t.h);

		// Level text
		iSetColor(255, 0, 0);
		char txt[32];
		sprintf_s(txt, sizeof(txt), "Level %d", i + 1);
		iText(t.x + t.w / 2 - 40, t.y + 15, txt, GLUT_BITMAP_TIMES_ROMAN_24);
	}

	// Back button
	iSetColor(108, 29, 29); // using your button color style
	iFilledRectangle(levelBackX, levelBackY, levelBackW, levelBackH);
	iSetColor(255, 255, 255);
	iText(levelBackX + 70, levelBackY + 22, (char*)"Back", GLUT_BITMAP_TIMES_ROMAN_24);

	// Play button
	iSetColor(108, 29, 29);
	iFilledRectangle(levelPlayX, levelPlayY, levelPlayW, levelPlayH);
	iSetColor(255, 255, 255);
	iText(levelPlayX + 75, levelPlayY + 22, (char*)"Play", GLUT_BITMAP_TIMES_ROMAN_24);
}

void drawPlay() {
	if (texGameBg) iShowImage(0, 0, SCR_W, SCR_H, texGameBg);
	iSetColor(255, 255, 255);
	iText(30, SCR_H - 40, (char*)"PLAYING... (Press ESC to go back)", GLUT_BITMAP_HELVETICA_18);
}

void drawSimpleScreen(const char* title) {
	iSetColor(0, 0, 0);
	iFilledRectangle(0, 0, SCR_W, SCR_H);

	iSetColor(255, 255, 255);
	iText(60, SCR_H - 80, (char*)title, GLUT_BITMAP_TIMES_ROMAN_24);
	iText(60, SCR_H - 120, (char*)"Press ESC to return to MENU", GLUT_BITMAP_HELVETICA_18);
}

void drawBackButton() {
	// Button outline
	iSetColor(108, 29, 29);
	iFilledRectangle(aboutBackX, aboutBackY, aboutBackW, aboutBackH);

	// Button text
	iSetColor(255, 255, 255);
	iText(aboutBackX + 15, aboutBackY + 15, (char*)"↩️ Back", GLUT_BITMAP_HELVETICA_18);
}

void drawAboutCard(const AboutCard& c) {
	// Card outline
	iSetColor(0, 0, 0);
	iRectangle(c.x, c.y, c.w, c.h);

	// Avatar circle (placeholder)
	int cx = c.x + c.w / 2;
	int cy = c.y + c.h - 80;
	iCircle(cx, cy, 35);

	// Text
	iSetColor(255, 255, 255);
	char line1[256], line2[256], line3[256];
	sprintf_s(line1, sizeof(line1), "Name: %s", c.name);
	sprintf_s(line2, sizeof(line2), "ID: %s", c.id);
	sprintf_s(line3, sizeof(line3), "Batch: %s", c.batch);

	iText(c.x + 25, c.y + 55, line1, GLUT_BITMAP_HELVETICA_12);
	iText(c.x + 25, c.y + 35, line2, GLUT_BITMAP_HELVETICA_12);
	iText(c.x + 25, c.y + 15, line3, GLUT_BITMAP_HELVETICA_12);
}

void drawAbout()
{
	// Background like your sample (light gray)
	iSetColor(0, 0, 128);
	iFilledRectangle(0, 0, SCR_W, SCR_H);

	// Back button (top-left)
	drawBackButton();

	// Title "About"
	iSetColor(255, 255, 255);
	iText(160, SCR_H - 60, (char*)"About Us", GLUT_BITMAP_TIMES_ROMAN_24);

	// Paragraphs (split into lines manually)
	int tx = 40;
	int ty = SCR_H - 120;

	iText(tx, ty, (char*)"Lorem ipsum dolor sit amet consectetur adipisicing elit. Dicta rem aliquid aperiam! Magnam debitis in totam itaque est nihil eligendi autem perspiciatis voluptatibus consequatur! Quae quas perspiciatis deleniti", GLUT_BITMAP_HELVETICA_12);
	iText(tx, ty - 20, (char*)"aperiam! Magnam debitis in totam itaque est nihil eligendi autem Magnam debitis in totam itaque est nihil eligendi autem perspiciatis voluptatibus consequatur! Quae quas perspiciatis eligendi Quae", GLUT_BITMAP_HELVETICA_12);
	iText(tx, ty - 40, (char*)"perspiciatis voluptatibus consequatur! Quae quas perspiciatis deleniti  Magnam debitis in totam itaque est nihil eligendi autem perspiciatis voluptatibus consequatur! Quae quas voluptatibus consequatur", GLUT_BITMAP_HELVETICA_12);
	iText(tx, ty - 60, (char*)"libero suscipit.", GLUT_BITMAP_HELVETICA_12);

	iText(tx, ty - 100, (char*)"Lorem ipsum dolor sit amet consectetur adipisicing elit. Dicta rem aliquid aperiam! Magnam debitis in totam itaque est nihil eligendi autem perspiciatis voluptatibus consequatur! Quae quas perspiciatis deleniti", GLUT_BITMAP_HELVETICA_12);
	iText(tx, ty - 120, (char*)"aperiam! Magnam debitis in totam itaque est nihil eligendi autem Magnam debitis in totam itaque est nihil eligendi autem perspiciatis voluptatibus consequatur! Quae quas perspiciatis eligendi Quae", GLUT_BITMAP_HELVETICA_12);
	iText(tx, ty - 140, (char*)"perspiciatis voluptatibus consequatur! Quae quas perspiciatis deleniti  Magnam debitis in totam itaque est nihil eligendi autem perspiciatis voluptatibus consequatur! Quae quas voluptatibus consequatur", GLUT_BITMAP_HELVETICA_12);
	iText(tx, ty - 160, (char*)"libero suscipit.", GLUT_BITMAP_HELVETICA_12);

	// Cards row
	for (int i = 0; i < 3; i++) drawAboutCard(aboutCards[i]);
}

void iDraw() {
	iClear();

	if (currentScreen == SCREEN_LOADING) drawLoading();
	else if (currentScreen == SCREEN_MENU) drawMenu();
	else if (currentScreen == SCREEN_LEVEL) drawLevelScreen();
	else if (currentScreen == SCREEN_PLAY) drawPlay();
	else if (currentScreen == SCREEN_HIGHSCORE) drawSimpleScreen("HIGH SCORE");
	else if (currentScreen == SCREEN_INSTRUCTIONS) drawSimpleScreen("INSTRUCTIONS");
	else if (currentScreen == SCREEN_ABOUT) drawAbout();
}

// -------------------- Input --------------------
void iMouseMove(int mx, int my) {}

void iPassiveMouseMove(int mx, int my) {
	if (currentScreen != SCREEN_MENU) return;

	hoveredBtn = -1;
	for (int i = 0; i < 5; i++) {
		if (pointInRect(mx, my, menuBtns[i].x, menuBtns[i].y, menuBtns[i].w, menuBtns[i].h)) {
			hoveredBtn = i;
			break;
		}
	}
}

void iMouse(int button, int state, int mx, int my)
{
	if (currentScreen == SCREEN_LOADING) return; // block input during loading

	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		if (currentScreen == SCREEN_MENU)
		{
			for (int i = 0; i < 5; i++)
			{
				if (pointInRect(mx, my, menuBtns[i].x, menuBtns[i].y, menuBtns[i].w, menuBtns[i].h))
				{
					if (i == 0) currentScreen = SCREEN_LEVEL;   // Start opens Level Screen
					if (i == 1) currentScreen = SCREEN_HIGHSCORE;
					if (i == 2) currentScreen = SCREEN_INSTRUCTIONS;
					if (i == 3) currentScreen = SCREEN_ABOUT;
					if (i == 4) exit(0);
				}
			}
		}
		else if (currentScreen == SCREEN_LEVEL)
		{
			// Click on a level thumbnail -> select it
			for (int i = 0; i < 6; i++)
			{
				if (pointInRect(mx, my, levelThumbs[i].x, levelThumbs[i].y, levelThumbs[i].w, levelThumbs[i].h))
				{
					selectedLevel = i + 1;
					return;
				}
			}

			// Back
			if (pointInRect(mx, my, levelBackX, levelBackY, levelBackW, levelBackH))
			{
				currentScreen = SCREEN_MENU;
				return;
			}

			// Play
			if (pointInRect(mx, my, levelPlayX, levelPlayY, levelPlayW, levelPlayH))
			{
				// ✅ Here you can use selectedLevel later
				// For now: go to PLAY screen
				currentScreen = SCREEN_PLAY;
				return;
			}
		}
		else if (currentScreen == SCREEN_ABOUT)
		{
			if (pointInRect(mx, my, aboutBackX, aboutBackY, aboutBackW, aboutBackH))
			{
				currentScreen = SCREEN_MENU;
				return;
			}
		}
	}
}

void fixedUpdate() {
	if (currentScreen == SCREEN_LOADING) return;

	if (isKeyPressed(27)) { // ESC
		currentScreen = SCREEN_MENU;
	}
}

void setGameIcon(const char* windowTitle, const char* iconPath)
{
	HWND hwnd = FindWindowA(NULL, windowTitle);
	if (!hwnd) return;

	HICON hIcon = (HICON)LoadImageA(
		NULL,
		iconPath,
		IMAGE_ICON,
		0, 0,
		LR_LOADFROMFILE | LR_DEFAULTSIZE
		);

	if (hIcon)
	{
		SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);   // Taskbar
		SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon); // Title bar
	}
}

// -------------------- Main --------------------
int main()
{
	iInitialize(SCR_W, SCR_H, "Blood Of Legends");
	setGameIcon("Blood Of Legends", "assets/images/game_icon.ico");
	centerGameWindow("Blood Of Legends");

	// Load images
	texLogo = iLoadImage((char*)"assets/images/logo.png");
	texMenuBg = iLoadImage((char*)"assets/images/menu_bg.jpeg");
	texGameBg = iLoadImage((char*)"assets/images/game_bg.jpeg");

	/*texKnightBody = iLoadImage((char*)"assets/images/knight/body.png");
	texKnightSword = iLoadImage((char*)"assets/images/knight/sword.png");
	texKnightFlag = iLoadImage((char*)"assets/images/knight/flag.png");

	knightX = SCR_W - knightW - 70;
	knightY = 30;*/

	buildMenuButtons();

	// Audio
	mciSendString("open \"assets/sounds/background.mp3\" alias bgsong", NULL, 0, NULL);
	mciSendString("open \"assets/sounds/gameover.mp3\" alias ggsong", NULL, 0, NULL);
	mciSendString("play bgsong repeat", NULL, 0, NULL);

	// Timers
	iSetTimer(16, updateLoading); // loading progress
	// iSetTimer(16, animateKnight); // sword/flag animation
	iSetTimer(16, fixedUpdate);   // ESC handling

	buildLevelLayout();                 // creates positions for 6 levels
	texLevelThumb = texGameBg;          // use your game_bg as level image

	iStart();
	return 0;
}
