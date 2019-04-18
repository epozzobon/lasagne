#pragma once

#define W 0xffff
#define _ 0
#define font_witdh 8
#define font_height 8
#define font_size (font_height * font_witdh)

static const uint16_t FONT_BLANK[font_size] = {
	_,_,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,
};

static const uint16_t FONT_A[font_size] = {
	_,_,_,_,_,_,_,_,
	_,_,_,W,W,_,_,_,
	_,_,_,W,W,_,_,_,
	_,_,W,_,_,W,_,_,
	_,_,W,W,W,W,_,_,
	_,W,_,_,_,_,W,_,
	_,W,_,_,_,_,W,_,
	_,_,_,_,_,_,_,_,
};

static const uint16_t FONT_B[font_size] = {
	_,_,_,_,_,_,_,_,
	_,W,W,W,W,W,_,_,
	_,W,_,_,_,_,W,_,
	_,W,W,W,W,W,_,_,
	_,W,_,_,_,_,W,_,
	_,W,_,_,_,_,W,_,
	_,W,W,W,W,W,_,_,
	_,_,_,_,_,_,_,_,
};

static const uint16_t FONT_C[font_size] = {
	_,_,_,_,_,_,_,_,
	_,_,_,W,W,W,_,_,
	_,_,W,_,_,_,W,_,
	_,W,_,_,_,_,_,_,
	_,W,_,_,_,_,_,_,
	_,_,W,_,_,_,W,_,
	_,_,_,W,W,W,_,_,
	_,_,_,_,_,_,_,_,
};
static const uint16_t FONT_D[font_size] = {
	_,_,_,_,_,_,_,_,
	_,W,W,W,W,_,_,_,
	_,W,_,_,_,W,_,_,
	_,W,_,_,_,W,_,_,
	_,W,_,_,_,W,_,_,
	_,W,_,_,_,W,_,_,
	_,W,W,W,W,_,_,_,
	_,_,_,_,_,_,_,_,
};
static const uint16_t FONT_E[font_size] = {
	_,_,_,_,_,_,_,_,
	_,W,W,W,W,W,_,_,
	_,W,_,_,_,_,_,_,
	_,W,W,W,W,_,_,_,
	_,W,_,_,_,_,_,_,
	_,W,_,_,_,_,_,_,
	_,W,W,W,W,W,_,_,
	_,_,_,_,_,_,_,_,
};
static const uint16_t FONT_F[font_size] = {
	_,_,_,_,_,_,_,_,
	_,W,W,W,W,W,_,_,
	_,W,_,_,_,_,_,_,
	_,W,W,W,W,_,_,_,
	_,W,_,_,_,_,_,_,
	_,W,_,_,_,_,_,_,
	_,W,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,
};
static const uint16_t FONT_G[font_size] = {
	_,_,_,_,_,_,_,_,
	_,_,_,W,W,W,_,_,
	_,_,W,_,_,_,W,_,
	_,W,_,_,_,_,_,_,
	_,W,_,_,W,W,W,_,
	_,_,W,_,_,_,W,_,
	_,_,_,W,W,W,_,_,
	_,_,_,_,_,_,_,_,
};
static const uint16_t FONT_0[font_size] = {
	_,_,_,_,_,_,_,_,
	_,_,_,W,W,_,_,_,
	_,_,W,_,_,W,_,_,
	_,W,_,W,_,_,W,_,
	_,W,_,_,W,_,W,_,
	_,_,W,_,_,W,_,_,
	_,_,_,W,W,_,_,_,
	_,_,_,_,_,_,_,_,
};
static const uint16_t FONT_1[font_size] = {
	_,_,_,_,_,_,_,_,
	_,_,_,_,W,_,_,_,
	_,_,_,W,W,_,_,_,
	_,_,_,_,W,_,_,_,
	_,_,_,_,W,_,_,_,
	_,_,_,_,W,_,_,_,
	_,_,_,W,W,W,_,_,
	_,_,_,_,_,_,_,_,
};
static const uint16_t FONT_2[font_size] = {
	_,_,_,_,_,_,_,_,
	_,_,W,W,W,W,_,_,
	_,W,_,_,_,_,W,_,
	_,_,_,_,_,W,_,_,
	_,_,_,W,W,_,_,_,
	_,_,W,_,_,_,_,_,
	_,W,W,W,W,W,W,_,
	_,_,_,_,_,_,_,_,
};
static const uint16_t FONT_3[font_size] = {
	_,_,_,_,_,_,_,_,
	_,_,W,W,W,W,_,_,
	_,W,_,_,_,_,W,_,
	_,_,_,_,_,W,_,_,
	_,_,_,W,W,W,_,_,
	_,W,_,_,_,_,W,_,
	_,_,W,W,W,W,_,_,
	_,_,_,_,_,_,_,_,
};
static const uint16_t FONT_4[font_size] = {
	_,_,_,_,_,_,_,_,
	_,_,_,_,W,_,_,_,
	_,_,_,W,W,_,_,_,
	_,_,W,_,W,_,_,_,
	_,W,_,_,W,_,_,_,
	_,W,W,W,W,W,_,_,
	_,_,_,_,W,_,_,_,
	_,_,_,_,_,_,_,_,
};
static const uint16_t FONT_5[font_size] = {
	_,_,_,_,_,_,_,_,
	_,W,W,W,W,W,W,_,
	_,W,_,_,_,_,_,_,
	_,W,W,W,W,W,_,_,
	_,_,_,_,_,_,W,_,
	_,W,_,_,_,_,W,_,
	_,_,W,W,W,W,_,_,
	_,_,_,_,_,_,_,_,
};
static const uint16_t FONT_6[font_size] = {
	_,_,_,_,_,_,_,_,
	_,_,W,W,W,W,_,_,
	_,W,_,_,_,_,_,_,
	_,W,W,W,W,W,_,_,
	_,W,_,_,_,_,W,_,
	_,W,_,_,_,_,W,_,
	_,_,W,W,W,W,_,_,
	_,_,_,_,_,_,_,_,
};
static const uint16_t FONT_7[font_size] = {
	_,_,_,_,_,_,_,_,
	_,W,W,W,W,W,W,_,
	_,_,_,_,_,W,_,_,
	_,_,_,_,W,_,_,_,
	_,_,_,W,_,_,_,_,
	_,_,W,_,_,_,_,_,
	_,W,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,
};
static const uint16_t FONT_8[font_size] = {
	_,_,_,_,_,_,_,_,
	_,_,W,W,W,W,_,_,
	_,W,_,_,_,_,W,_,
	_,_,W,W,W,W,_,_,
	_,W,_,_,_,_,W,_,
	_,W,_,_,_,_,W,_,
	_,_,W,W,W,W,_,_,
	_,_,_,_,_,_,_,_,
};
static const uint16_t FONT_9[font_size] = {
	_,_,_,_,_,_,_,_,
	_,_,W,W,W,W,_,_,
	_,W,_,_,_,_,W,_,
	_,W,_,_,_,_,W,_,
	_,_,W,W,W,W,W,_,
	_,_,_,_,_,_,W,_,
	_,_,W,W,W,W,_,_,
	_,_,_,_,_,_,_,_,
};
static const uint16_t FONT_COLON[font_size] = {
	_,_,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,
	_,_,_,_,W,_,_,_,
	_,_,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,
	_,_,_,_,W,_,_,_,
	_,_,_,_,_,_,_,_,
};
static const uint16_t FONT_SEMICOLON[font_size] = {
	_,_,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,
	_,_,_,_,W,_,_,_,
	_,_,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,
	_,_,_,_,W,_,_,_,
	_,_,_,W,_,_,_,_,
};
static const uint16_t FONT_LT[font_size] = {
	_,_,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,
	_,_,_,_,_,W,W,_,
	_,_,_,W,W,_,_,_,
	_,W,W,_,_,_,_,_,
	_,_,_,W,W,_,_,_,
	_,_,_,_,_,W,W,_,
	_,_,_,_,_,_,_,_,
};
static const uint16_t FONT_GT[font_size] = {
	_,_,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,
	_,W,W,_,_,_,_,_,
	_,_,_,W,W,_,_,_,
	_,_,_,_,_,W,W,_,
	_,_,_,W,W,_,_,_,
	_,W,W,_,_,_,_,_,
	_,_,_,_,_,_,_,_,
};
static const uint16_t FONT_EQUALS[font_size] = {
	_,_,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,
	_,W,W,W,W,W,W,_,
	_,_,_,_,_,_,_,_,
	_,W,W,W,W,W,W,_,
	_,_,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,
};
static const uint16_t FONT_MINUS[font_size] = {
	_,_,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,
	_,_,W,W,W,W,_,_,
	_,_,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,
};
static const uint16_t FONT_PLUS[font_size] = {
	_,_,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,
	_,_,_,_,W,_,_,_,
	_,_,_,_,W,_,_,_,
	_,_,W,W,W,W,W,_,
	_,_,_,_,W,_,_,_,
	_,_,_,_,W,_,_,_,
	_,_,_,_,_,_,_,_,
};
static const uint16_t FONT_STAR[font_size] = {
	_,_,_,_,_,_,_,_,
	_,_,W,_,W,_,W,_,
	_,_,_,W,W,W,_,_,
	_,_,W,W,W,W,W,_,
	_,_,_,W,W,W,_,_,
	_,_,W,_,W,_,W,_,
	_,_,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,
};

static const uint16_t FONT_SLASH[font_size] = {
	_,_,_,_,_,_,_,_,
	_,_,_,_,_,_,W,_,
	_,_,_,_,_,W,_,_,
	_,_,_,_,W,_,_,_,
	_,_,_,W,_,_,_,_,
	_,_,W,_,_,_,_,_,
	_,W,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,
};
static const uint16_t FONT_BACKSLASH[font_size] = {
	_,_,_,_,_,_,_,_,
	_,W,_,_,_,_,_,_,
	_,_,W,_,_,_,_,_,
	_,_,_,W,_,_,_,_,
	_,_,_,_,W,_,_,_,
	_,_,_,_,_,W,_,_,
	_,_,_,_,_,_,W,_,
	_,_,_,_,_,_,_,_,
};
static const uint16_t FONT_LPAREN[font_size] = {
	_,_,_,_,_,_,_,_,
	_,_,_,_,W,_,_,_,
	_,_,_,W,_,_,_,_,
	_,_,W,_,_,_,_,_,
	_,_,W,_,_,_,_,_,
	_,_,_,W,_,_,_,_,
	_,_,_,_,W,_,_,_,
	_,_,_,_,_,_,_,_,
};
static const uint16_t FONT_RPAREN[font_size] = {
	_,_,_,_,_,_,_,_,
	_,_,_,W,_,_,_,_,
	_,_,_,_,W,_,_,_,
	_,_,_,_,_,W,_,_,
	_,_,_,_,_,W,_,_,
	_,_,_,_,W,_,_,_,
	_,_,_,W,_,_,_,_,
	_,_,_,_,_,_,_,_,
};
static const uint16_t FONT_LBRACKET[font_size] = {
	_,_,_,_,_,_,_,_,
	_,_,W,W,W,_,_,_,
	_,_,W,_,_,_,_,_,
	_,_,W,_,_,_,_,_,
	_,_,W,_,_,_,_,_,
	_,_,W,_,_,_,_,_,
	_,_,W,W,W,_,_,_,
	_,_,_,_,_,_,_,_,
};
static const uint16_t FONT_RBRACKET[font_size] = {
	_,_,_,_,_,_,_,_,
	_,_,_,W,W,W,_,_,
	_,_,_,_,_,W,_,_,
	_,_,_,_,_,W,_,_,
	_,_,_,_,_,W,_,_,
	_,_,_,_,_,W,_,_,
	_,_,_,W,W,W,_,_,
	_,_,_,_,_,_,_,_,
};
static const uint16_t FONT_COMMA[font_size] = {
	_,_,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,
	_,_,_,_,W,_,_,_,
	_,_,_,W,_,_,_,_,
};
static const uint16_t FONT_PERIOD[font_size] = {
	_,_,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,
	_,_,_,_,W,_,_,_,
	_,_,_,_,_,_,_,_,
};
static const uint16_t FONT_UNDERSCORE[font_size] = {
	_,_,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,
	_,W,W,W,W,W,W,_,
	_,_,_,_,_,_,_,_,
};
static const uint16_t FONT_O[font_size] = {
	_,_,_,_,_,_,_,_,
	_,_,_,W,W,W,_,_,
	_,_,W,_,_,_,W,_,
	_,_,W,_,_,_,W,_,
	_,_,W,_,_,_,W,_,
	_,_,W,_,_,_,W,_,
	_,_,_,W,W,W,_,_,
	_,_,_,_,_,_,_,_,
};
static const uint16_t FONT_o[font_size] = {
	_,_,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,
	_,_,_,W,W,W,_,_,
	_,_,W,_,_,_,W,_,
	_,_,W,_,_,_,W,_,
	_,_,_,W,W,W,_,_,
	_,_,_,_,_,_,_,_,
};
static const uint16_t FONT_AT[font_size] = {
	_,_,_,_,_,_,_,_,
	_,_,W,W,W,W,_,_,
	_,W,_,_,_,W,_,_,
	W,_,_,W,W,W,_,_,
	W,_,_,W,_,W,_,_,
	_,W,_,W,W,W,_,_,
	_,_,W,_,_,_,_,_,
	_,_,_,W,W,W,_,_,
};
static const uint16_t FONT_CHECKERS[font_size] = {
	W,_,W,_,W,_,W,_,
	_,W,_,W,_,W,_,W,
	W,_,W,_,W,_,W,_,
	_,W,_,W,_,W,_,W,
	W,_,W,_,W,_,W,_,
	_,W,_,W,_,W,_,W,
	W,_,W,_,W,_,W,_,
	_,W,_,W,_,W,_,W,
};
static const uint16_t FONT_BLOCK[font_size] = {
	W,W,W,W,W,W,W,W,
	W,W,W,W,W,W,W,W,
	W,W,W,W,W,W,W,W,
	W,W,W,W,W,W,W,W,
	W,W,W,W,W,W,W,W,
	W,W,W,W,W,W,W,W,
	W,W,W,W,W,W,W,W,
	W,W,W,W,W,W,W,W,
};
static const uint16_t FONT_a[font_size] = {
	_,_,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,
	_,_,W,W,W,W,_,_,
	_,_,_,_,_,_,W,_,
	_,_,W,W,W,W,W,_,
	_,W,_,_,_,W,W,_,
	_,_,W,W,W,_,W,_,
	_,_,_,_,_,_,_,_,
};
static const uint16_t FONT_b[font_size] = {
	_,_,_,_,_,_,_,_,
	_,W,_,_,_,_,_,_,
	_,W,_,_,_,_,_,_,
	_,W,W,W,W,W,_,_,
	_,W,_,_,_,_,W,_,
	_,W,W,_,_,_,W,_,
	_,W,_,W,W,W,_,_,
	_,_,_,_,_,_,_,_,
};

static const uint16_t FONT_c[font_size] = {
	_,_,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,
	_,_,_,W,W,W,_,_,
	_,_,W,_,_,_,_,_,
	_,_,W,_,_,_,_,_,
	_,_,_,W,W,W,_,_,
	_,_,_,_,_,_,_,_,
};
static const uint16_t FONT_d[font_size] = {
	_,_,_,_,_,_,_,_,
	_,_,_,_,_,_,W,_,
	_,_,_,_,_,_,W,_,
	_,_,W,W,W,W,W,_,
	_,W,_,_,_,_,W,_,
	_,W,_,_,_,W,W,_,
	_,_,W,W,W,_,W,_,
	_,_,_,_,_,_,_,_,
};
static const uint16_t FONT_e[font_size] = {
	_,_,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,
	_,_,W,W,W,W,_,_,
	_,W,_,_,_,_,W,_,
	_,W,W,W,W,W,W,_,
	_,W,_,_,_,_,_,_,
	_,_,W,W,W,W,_,_,
	_,_,_,_,_,_,_,_,
};
static const uint16_t FONT_f[font_size] = {
	_,_,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,
	_,_,_,W,W,_,_,_,
	_,_,_,W,_,_,_,_,
	_,_,W,W,W,_,_,_,
	_,_,_,W,_,_,_,_,
	_,_,_,W,_,_,_,_,
	_,_,_,_,_,_,_,_,
};
static const uint16_t FONT_SQUARE[font_size] = {
	W,W,W,W,W,W,W,W,
	W,_,_,_,_,_,_,W,
	W,_,_,_,_,_,_,W,
	W,_,_,_,_,_,_,W,
	W,_,_,_,_,_,_,W,
	W,_,_,_,_,_,_,W,
	W,_,_,_,_,_,_,W,
	W,W,W,W,W,W,W,W,
};

static const uint16_t *FONT_ASCII[128] = {
	/*00*/ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/*08*/ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/*10*/ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/*18*/ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/*20*/ FONT_BLANK, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/*28*/ FONT_LPAREN, FONT_RPAREN, FONT_STAR, FONT_PLUS, FONT_COMMA, FONT_MINUS, FONT_PERIOD, FONT_SLASH,
	/*30*/ FONT_0, FONT_1, FONT_2, FONT_3, FONT_4, FONT_5, FONT_6, FONT_7,
	/*38*/ FONT_8, FONT_9, FONT_COLON, FONT_SEMICOLON, FONT_LT, FONT_EQUALS, FONT_GT, NULL,
	/*40*/ FONT_AT, FONT_A, FONT_B, FONT_C, FONT_D, FONT_E, FONT_F, FONT_G,
	/*48*/ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/*50*/ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/*58*/ NULL, NULL, NULL, FONT_LBRACKET, FONT_BACKSLASH, FONT_RBRACKET, NULL, FONT_UNDERSCORE,
	/*60*/ NULL, FONT_a, FONT_b, FONT_c, FONT_d, FONT_e, FONT_f, NULL,
	/*68*/ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/*70*/ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/*78*/ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
};
static const uint16_t *FONT_DIGITS[16] = {
	FONT_0, FONT_1, FONT_2, FONT_3,
	FONT_4, FONT_5, FONT_6, FONT_7,
	FONT_8, FONT_9, FONT_A, FONT_b,
	FONT_C, FONT_d, FONT_E, FONT_F,
};
