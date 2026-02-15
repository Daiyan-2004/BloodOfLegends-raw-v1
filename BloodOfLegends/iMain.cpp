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

int clampInt(int v, int lo, int hi) { return (v < lo) ? lo : (v > hi) ? hi : v; }

// -------------------- Level 1 --------------------
int heroX = 200, heroY = 100;
int enemyX = 900, enemyY = 100;

int heroWidth = 100;
int heroHeight = 150;

int enemyWidth = 100;
int enemyHeight = 150;

int heroHealth = 100;
int enemyHealth = 100;

bool heroAttack = false;
bool enemyAttack = false;

bool level1Win = false;
bool level1Lose = false;

int attackCooldown = 0;     // frames
const int ATTACK_COOLDOWN = 12;  // smaller = faster attacks

// -------------------- Sprite Animation --------------------
enum AnimState { IDLE, MOVE, ATTACK };

AnimState heroState = IDLE;
AnimState enemyState = IDLE;

int heroDir = 1;   // 1 = right, -1 = left
int enemyDir = -1; // enemy initially faces left (toward hero)

int heroFrame = 0;
int enemyFrame = 0;

int heroAnimTick = 0;
int enemyAnimTick = 0;

// Adjust these to control animation speed (lower = faster)
const int HERO_ANIM_SPEED = 4;
const int ENEMY_ANIM_SPEED = 4;

// Attack control
int heroAttackCooldown = 0;
const int HERO_ATTACK_COOLDOWN_MAX = 18;
bool heroDidDamageThisAttack = false;

int enemyAttackCooldown = 0;
const int ENEMY_ATTACK_COOLDOWN_MAX = 28;
bool enemyDidDamageThisAttack = false;

// Draw sizes (tune for your PNG sizes)
int heroDrawW = 220, heroDrawH = 220;
int enemyDrawW = 260, enemyDrawH = 260;

// -------------------- Textures: Hero --------------------
unsigned int texHeroIdle = 0;             // hero_standing.png
unsigned int texHeroMoveR[6];             // hero_movement_01..06
unsigned int texHeroMoveL[7];             // hero_movement_flipped_01..07
unsigned int texHeroAtkR[14];             // 1.png..14.png (hero_attack/png/images)
unsigned int texHeroAtkL[20];             // left_01..left_20 (heavy attack left)
int HERO_MOVE_R_COUNT = 6;
int HERO_MOVE_L_COUNT = 7;
int HERO_ATK_R_COUNT = 14;
int HERO_ATK_L_COUNT = 20;

// -------------------- Textures: Enemy --------------------
unsigned int texEnemyIdle = 0;            // standing.png
unsigned int texEnemyMoveR[8];            // 1..8
unsigned int texEnemyMoveL[8];            // f1..f8
unsigned int texEnemyAtkR[16];            // R_01..R_16
unsigned int texEnemyAtkL[16];            // L_01..L_16
int ENEMY_MOVE_COUNT = 8;
int ENEMY_ATK_COUNT = 16;

// -------------------- Level 1 UI --------------------
int score = 0;
bool isPaused = false;

// UI positions (tuned for 1220x590)
int avatarSize = 90;

// Health bar config
int hpBarW = 320;
int hpBarH = 26;

// Left UI
int heroAvatarX = 35, heroAvatarY = SCR_H - 115;
int heroBarX = heroAvatarX + avatarSize + 20;
int heroBarY = SCR_H - 90;

// Right UI
int enemyAvatarX = SCR_W - 35 - avatarSize;
int enemyAvatarY = SCR_H - 115;
int enemyBarX = enemyAvatarX - 20 - hpBarW;
int enemyBarY = SCR_H - 90;

// Bottom buttons
struct UIButton { int x, y, w, h; const char* text; };
UIButton btnBack = { SCR_W / 2 - 420, 25, 200, 60, "Back" };
UIButton btnPause = { SCR_W / 2 - 120, 25, 200, 60, "Pause" };
UIButton btnRestart = { SCR_W / 2 + 180, 25, 200, 60, "Restart" };

// Images (optional)
unsigned int texHeroAvatar = 0;
unsigned int texEnemyAvatar = 0;

void resetLevel1()
{
	heroX = 200; heroY = 100;
	enemyX = 900; enemyY = 100;

	heroHealth = 100;
	enemyHealth = 100;

	level1Win = false;
	level1Lose = false;

	score = 0;
	isPaused = false;
}

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

void drawHealthBar(int x, int y, int w, int h, int hp, int maxHp)
{
	// background frame
	iSetColor(60, 0, 0);
	iFilledRectangle(x, y, w, h);

	// inner fill based on hp
	float ratio = (maxHp > 0) ? (float)hp / (float)maxHp : 0.0f;
	if (ratio < 0) ratio = 0;
	if (ratio > 1) ratio = 1;

	int fillW = (int)(w * ratio);

	iSetColor(170, 20, 20);          // red fill like screenshot
	iFilledRectangle(x, y, fillW, h);

	// outline
	iSetColor(20, 20, 20);
	iRectangle(x, y, w, h);
}

void drawUIButton(const UIButton& b)
{
	// button body
	iSetColor(120, 15, 15);
	iFilledRectangle(b.x, b.y, b.w, b.h);

	// border
	iSetColor(30, 30, 30);
	iRectangle(b.x, b.y, b.w, b.h);

	// text
	iSetColor(255, 255, 255);
	iText(b.x + b.w / 2 - 35, b.y + b.h / 2 - 10, (char*)b.text, GLUT_BITMAP_TIMES_ROMAN_24);
}

void drawLevel1HUD()
{
	// Avatars
	if (texHeroAvatar) iShowImage(heroAvatarX, heroAvatarY, avatarSize, avatarSize, texHeroAvatar);
	else { iSetColor(100, 0, 0); iFilledRectangle(heroAvatarX, heroAvatarY, avatarSize, avatarSize); }

	if (texEnemyAvatar) iShowImage(enemyAvatarX, enemyAvatarY, avatarSize, avatarSize, texEnemyAvatar);
	else { iSetColor(100, 0, 0); iFilledRectangle(enemyAvatarX, enemyAvatarY, avatarSize, avatarSize); }

	// Health bars
	drawHealthBar(heroBarX, heroBarY, hpBarW, hpBarH, heroHealth, 100);
	drawHealthBar(enemyBarX, enemyBarY, hpBarW, hpBarH, enemyHealth, 100);

	// Score center-top
	iSetColor(255, 255, 255);
	char sc[64];
	sprintf_s(sc, sizeof(sc), "Score: %d", score);
	iText(SCR_W / 2 - 80, SCR_H - 70, sc, GLUT_BITMAP_TIMES_ROMAN_24);

	// Bottom buttons
	// Pause button text changes
	btnPause.text = isPaused ? "Resume" : "Pause";
	drawUIButton(btnBack);
	drawUIButton(btnPause);
	drawUIButton(btnRestart);

	// Pause overlay (optional)
	if (isPaused)
	{
		//iSetColor(0, 0, 0);
		//iFilledRectangle(0, 0, SCR_W, SCR_H); // simple dark overlay
		iSetColor(255, 255, 255);
		iText(SCR_W / 2 - 60, SCR_H / 2 + 20, (char*)"PAUSED", GLUT_BITMAP_TIMES_ROMAN_24);

		// redraw buttons on top
		//drawUIButton(btnBack);
		//drawUIButton(btnPause);
		//drawUIButton(btnRestart);
	}
}

void drawHero()
{
	unsigned int tex = 0;

	if (heroState == ATTACK)
	{
		if (heroDir == 1)
		{
			int f = clampInt(heroFrame, 0, HERO_ATK_R_COUNT - 1);
			tex = texHeroAtkR[f];
		}
		else
		{
			int f = clampInt(heroFrame, 0, HERO_ATK_L_COUNT - 1);
			tex = texHeroAtkL[f];
		}
	}
	else if (heroState == MOVE)
	{
		if (heroDir == 1)
		{
			int f = clampInt(heroFrame, 0, HERO_MOVE_R_COUNT - 1);
			tex = texHeroMoveR[f];
		}
		else
		{
			int f = clampInt(heroFrame, 0, HERO_MOVE_L_COUNT - 1);
			tex = texHeroMoveL[f];
		}
	}
	else
	{
		tex = texHeroIdle;
	}

	if (tex) iShowImage(heroX, heroY, heroDrawW, heroDrawH, tex);
}

void drawEnemy()
{
	unsigned int tex = 0;

	if (enemyState == ATTACK)
	{
		if (enemyDir == 1)
		{
			int f = clampInt(enemyFrame, 0, ENEMY_ATK_COUNT - 1);
			tex = texEnemyAtkR[f];
		}
		else
		{
			int f = clampInt(enemyFrame, 0, ENEMY_ATK_COUNT - 1);
			tex = texEnemyAtkL[f];
		}
	}
	else if (enemyState == MOVE)
	{
		if (enemyDir == 1)
		{
			int f = clampInt(enemyFrame, 0, ENEMY_MOVE_COUNT - 1);
			tex = texEnemyMoveR[f];
		}
		else
		{
			int f = clampInt(enemyFrame, 0, ENEMY_MOVE_COUNT - 1);
			tex = texEnemyMoveL[f];
		}
	}
	else
	{
		tex = texEnemyIdle;
	}

	if (tex) iShowImage(enemyX, enemyY, enemyDrawW, enemyDrawH, tex);
}

void drawPlay()
{
	if (texGameBg) iShowImage(0, 0, SCR_W, SCR_H, texGameBg);
	drawHero();
	drawEnemy();

	// draw characters (your rectangles for now)
	//iSetColor(0, 0, 255);
	//iFilledRectangle(heroX, heroY, heroWidth, heroHeight);

	//iSetColor(255, 0, 0);
	//iFilledRectangle(enemyX, enemyY, enemyWidth, enemyHeight);

	// win/lose text (your existing)
	if (level1Win) { iSetColor(255, 255, 0); iText(550, 300, "YOU WIN!", GLUT_BITMAP_TIMES_ROMAN_24); }
	if (level1Lose){ iSetColor(255, 0, 0);   iText(550, 300, "YOU LOSE!", GLUT_BITMAP_TIMES_ROMAN_24); }

	// ✅ HUD like screenshot
	drawLevel1HUD();
}

void enemyAI()
{
	if (currentScreen != SCREEN_PLAY) return;
	if (isPaused) return;
	if (enemyHealth <= 0 || heroHealth <= 0) return;
	if (level1Win || level1Lose) return;

	// face hero
	enemyDir = (heroX > enemyX) ? 1 : -1;

	int dist = abs((enemyX + enemyDrawW / 2) - (heroX + heroDrawW / 2));

	// cooldown
	if (enemyAttackCooldown > 0) enemyAttackCooldown--;

	// If close -> attack
	if (dist < 160 && enemyAttackCooldown == 0)
	{
		enemyState = ATTACK;
		enemyFrame = 0;
		enemyAnimTick = 0;
		enemyAttackCooldown = ENEMY_ATTACK_COOLDOWN_MAX;
		enemyDidDamageThisAttack = false;
	}

	// If attacking -> animate attack
	if (enemyState == ATTACK)
	{
		enemyAnimTick++;
		if (enemyAnimTick >= ENEMY_ANIM_SPEED)
		{
			enemyAnimTick = 0;
			enemyFrame++;

			// damage once around mid frames
			if (!enemyDidDamageThisAttack && enemyFrame >= 6)
			{
				if (dist < 170)
				{
					heroHealth -= 8;
					if (heroHealth < 0) heroHealth = 0;
				}
				enemyDidDamageThisAttack = true;
			}

			if (enemyFrame >= ENEMY_ATK_COUNT)
			{
				enemyState = IDLE;
				enemyFrame = 0;
			}
		}
		return; // stop moving while attacking
	}

	// else move toward hero
	enemyState = MOVE;

	if (enemyX < heroX) enemyX += 3;
	else enemyX -= 3;

	enemyX = clampInt(enemyX, 0, SCR_W - enemyDrawW);

	// move animation
	enemyAnimTick++;
	if (enemyAnimTick >= ENEMY_ANIM_SPEED)
	{
		enemyAnimTick = 0;
		enemyFrame = (enemyFrame + 1) % ENEMY_MOVE_COUNT;
	}

	// win/lose
	if (enemyHealth <= 0) level1Win = true;
	if (heroHealth <= 0) level1Lose = true;
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
	iText(aboutBackX + 15, aboutBackY + 15, (char*)"<- Back", GLUT_BITMAP_HELVETICA_18);
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
				resetLevel1();
				currentScreen = SCREEN_PLAY;
				return;
			}
		}

		else if (currentScreen == SCREEN_PLAY)
		{
			if (pointInRect(mx, my, btnBack.x, btnBack.y, btnBack.w, btnBack.h))
			{
				isPaused = false;
				currentScreen = SCREEN_MENU;
				return;
			}

			if (pointInRect(mx, my, btnPause.x, btnPause.y, btnPause.w, btnPause.h))
			{
				isPaused = !isPaused;
				return;
			}

			if (pointInRect(mx, my, btnRestart.x, btnRestart.y, btnRestart.w, btnRestart.h))
			{
				resetLevel1();
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

void iKeyboard(unsigned char key)
{
	if (currentScreen == SCREEN_PLAY)
	{
		// Move Right
		if (key == 'd')
		{
			heroX += 20;
		}

		// Move Left
		if (key == 'a')
		{
			heroX -= 20;
		}

		// Attack
		if (key == ' ')
		{
			if (abs(heroX - enemyX) < 120)
			{
				enemyHealth -= 10;
			}
		}
	}

	// Exit game
	if (key == 'q')
	{
		exit(0);
	}
}

void iSpecialKeyboard(unsigned char key)
{
	if (currentScreen == SCREEN_PLAY)
	{
		if (key == GLUT_KEY_RIGHT)
		{
			heroX += 20;
		}

		if (key == GLUT_KEY_LEFT)
		{
			heroX -= 20;
		}
	}
}

void fixedUpdate()
{
	if (currentScreen == SCREEN_LOADING) return;

	if (isKeyPressed(27)) { currentScreen = SCREEN_MENU; return; }
	if (currentScreen != SCREEN_PLAY) return;
	if (level1Win || level1Lose) return;
	if (isPaused) return;

	// cooldown
	if (heroAttackCooldown > 0) heroAttackCooldown--;

	// movement input
	bool moving = false;

	if (isKeyPressed('a') || isSpecialKeyPressed(GLUT_KEY_LEFT))
	{
		heroX -= 10;
		heroDir = -1;
		moving = true;
	}
	if (isKeyPressed('d') || isSpecialKeyPressed(GLUT_KEY_RIGHT))
	{
		heroX += 10;
		heroDir = 1;
		moving = true;
	}

	heroX = clampInt(heroX, 0, SCR_W - heroDrawW);

	// attack input (use 'f' reliable; you can switch to space later)
	if (isKeyPressed('f') && heroAttackCooldown == 0)
	{
		heroState = ATTACK;
		heroFrame = 0;
		heroAnimTick = 0;
		heroAttackCooldown = HERO_ATTACK_COOLDOWN_MAX;
		heroDidDamageThisAttack = false;
	}

	// set state if not attacking
	if (heroState != ATTACK)
	{
		heroState = moving ? MOVE : IDLE;
	}

	// HERO animation update
	heroAnimTick++;
	if (heroAnimTick >= HERO_ANIM_SPEED)
	{
		heroAnimTick = 0;

		if (heroState == MOVE)
		{
			int maxF = (heroDir == 1) ? HERO_MOVE_R_COUNT : HERO_MOVE_L_COUNT;
			heroFrame = (heroFrame + 1) % maxF;
		}
		else if (heroState == ATTACK)
		{
			int maxF = (heroDir == 1) ? HERO_ATK_R_COUNT : HERO_ATK_L_COUNT;
			heroFrame++;

			// damage once around mid frames
			if (!heroDidDamageThisAttack && heroFrame >= 4)
			{
				if (abs((heroX + heroDrawW / 2) - (enemyX + enemyDrawW / 2)) < 140)
				{
					enemyHealth -= 10;
					if (enemyHealth < 0) enemyHealth = 0;
					score += 10;
				}
				heroDidDamageThisAttack = true;
			}

			if (heroFrame >= maxF)
			{
				heroState = IDLE;
				heroFrame = 0;
			}
		}
		else
		{
			heroFrame = 0;
		}
	}

	// win/lose
	if (enemyHealth <= 0) level1Win = true;
	if (heroHealth <= 0) level1Lose = true;
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

unsigned int loadPng(const char* path)
{
	return iLoadImage((char*)path);
}

void loadHeroTextures()
{
	texHeroIdle = loadPng("assets/images/hero/heroMovement/hero_standing.png");

	// Move Right: hero_movement_01..06
	for (int i = 0; i < HERO_MOVE_R_COUNT; i++)
	{
		char p[256];
		sprintf_s(p, "assets/images/hero/heroMovement/hero_movement_0%d.png", i + 1);
		texHeroMoveR[i] = loadPng(p);
	}

	// Move Left (flipped): hero_movement_flipped_01..07
	for (int i = 0; i < HERO_MOVE_L_COUNT; i++)
	{
		char p[256];
		sprintf_s(p, "assets/images/hero/heroMovement/hero_movement_flipped_0%d.png", i + 1);
		texHeroMoveL[i] = loadPng(p);
	}

	// Attack Right: hero_attack/png/images/1.png..14.png
	for (int i = 0; i < HERO_ATK_R_COUNT; i++)
	{
		char p[256];
		sprintf_s(p, "assets/images/hero/hero_attack/png/images/%d.png", i + 1);
		texHeroAtkR[i] = loadPng(p);
	}

	// Attack Left (heavy attack left): left_01..left_20
	for (int i = 0; i < HERO_ATK_L_COUNT; i++)
	{
		char p[256];
		sprintf_s(p, "assets/images/hero/hero_attack/heavy attack/left/images/left_%02d.png", i + 1);
		texHeroAtkL[i] = loadPng(p);
	}
}

void loadEnemyTextures()
{
	texEnemyIdle = loadPng("assets/images/enemy/movement/standing.png");

	// Move Right: 1..8
	for (int i = 0; i < ENEMY_MOVE_COUNT; i++)
	{
		char p[256];
		sprintf_s(p, "assets/images/enemy/movement/%d.png", i + 1);
		texEnemyMoveR[i] = loadPng(p);
	}

	// Move Left (flipped): f1..f8
	for (int i = 0; i < ENEMY_MOVE_COUNT; i++)
	{
		char p[256];
		sprintf_s(p, "assets/images/enemy/movement/f%d.png", i + 1);
		texEnemyMoveL[i] = loadPng(p);
	}

	// Enemy attack left: L_01..L_16
	for (int i = 0; i < ENEMY_ATK_COUNT; i++)
	{
		char p[256];
		sprintf_s(p, "assets/images/enemy/attack/images/L_%02d.png", i + 1);
		texEnemyAtkL[i] = loadPng(p);
	}

	// Enemy attack right: R_01..R_16
	for (int i = 0; i < ENEMY_ATK_COUNT; i++)
	{
		char p[256];
		sprintf_s(p, "assets/images/enemy/attack/images/R_%02d.png", i + 1);
		texEnemyAtkR[i] = loadPng(p);
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
	texHeroAvatar = iLoadImage((char*)"assets/images/hero_avatar.png");
	texEnemyAvatar = iLoadImage((char*)"assets/images/enemy_avatar.png");

	buildMenuButtons();

	loadHeroTextures();
	loadEnemyTextures();

	// Audio
	mciSendString("open \"assets/sounds/background.mp3\" alias bgsong", NULL, 0, NULL);
	mciSendString("open \"assets/sounds/gameover.mp3\" alias ggsong", NULL, 0, NULL);
	mciSendString("play bgsong repeat", NULL, 0, NULL);

	// Timers
	iSetTimer(16, updateLoading); // loading progress
	iSetTimer(16, fixedUpdate);   // keyboard + ESC handling
	iSetTimer(20, enemyAI);


	buildLevelLayout();                 // creates positions for 6 levels
	texLevelThumb = texGameBg;          // use your game_bg as level image

	iStart();
	return 0;
}
